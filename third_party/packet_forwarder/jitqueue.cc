/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2013 Semtech-Cycleo

Description:
    LoRa concentrator : Just In Time TX scheduling queue

License: Revised BSD License, see LICENSE file in this directory
Maintainer: Michael Coracin
*/

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdlib.h> /* qsort_r */
#include <stdio.h> /* printf, fprintf, snprintf, fopen, fputs */
#include <string.h> /* memset, memcpy */
#include <pthread.h>
#include <assert.h>
#include <math.h>
#include <sstream>

#include "trace.h"
#include "jitqueue.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS & TYPES -------------------------------------------- */
#define TX_START_DELAY 1500 /* microseconds */
/* TODO: get this value from HAL? */
#define TX_MARGIN_DELAY 1000 /* Packet overlap margin in microseconds */
/* TODO: How much margin should we take? */
#define TX_JIT_DELAY 30000 /* Pre-delay to program packet for TX in microseconds */
#define TX_MAX_ADVANCE_DELAY             \
  ((JIT_NUM_BEACON_IN_QUEUE + 1) * 128 * \
   1E6) /* Maximum advance delay accepted for a TX packet, compared to current time */


/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES (GLOBAL) ------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ----------------------------------------- */

bool
jit_queue_is_full (struct jit_queue_s *queue)
{
  return (queue->num_pkt == JIT_QUEUE_MAX) ? true : false;
}

bool
jit_queue_is_empty (struct jit_queue_s *queue)
{
  return (queue->num_pkt == 0) ? true : false;
}

void
jit_queue_init (struct jit_queue_s *queue)
{
  int i;

  memset (queue, 0, sizeof (*queue));
  for (i = 0; i < JIT_QUEUE_MAX; i++)
    {
      queue->nodes[i].pre_delay = 0;
      queue->nodes[i].post_delay = 0;
    }
}

int
compare (const void *a, const void *b, void *arg)
{
  struct jit_node_s *p = (struct jit_node_s *) a;
  struct jit_node_s *q = (struct jit_node_s *) b;
  int *counter = (int *) arg;
  int p_count, q_count;

  p_count = p->pkt.count_us;
  q_count = q->pkt.count_us;

  if (p_count > q_count)
    *counter = *counter + 1;

  return p_count - q_count;
}

void
jit_sort_queue (struct jit_queue_s *queue)
{
  int counter = 0;

  if (queue->num_pkt == 0)
    {
      return;
    }

  MSG_DEBUG (DEBUG_JIT, "sorting queue in ascending order packet timestamp - queue size:%u\n",
             queue->num_pkt);
  qsort_r (queue->nodes, queue->num_pkt, sizeof (queue->nodes[0]), compare, &counter);
  MSG_DEBUG (DEBUG_JIT, "sorting queue done - swapped:%d\n", counter);
}

bool
jit_collision_test (uint32_t p1_count_us, uint32_t p1_pre_delay, uint32_t p1_post_delay,
                    uint32_t p2_count_us, uint32_t p2_pre_delay, uint32_t p2_post_delay)
{
  if (((p1_count_us - p2_count_us) <= (p1_pre_delay + p2_post_delay + TX_MARGIN_DELAY)) ||
      ((p2_count_us - p1_count_us) <= (p2_pre_delay + p1_post_delay + TX_MARGIN_DELAY)))
    {
      return true;
    }
  else
    {
      return false;
    }
}

enum jit_error_e
jit_enqueue (struct jit_queue_s *queue, struct timeval *time, struct lgw_pkt_tx_s *packet,
             enum jit_pkt_type_e pkt_type)
{
  int i = 0;
  uint32_t time_us = time->tv_sec * 1000000UL + time->tv_usec; /* convert time in µs */
  uint32_t packet_post_delay = 0;
  uint32_t packet_pre_delay = 0;
  uint32_t target_pre_delay = 0;

  MSG_DEBUG (DEBUG_JIT, "Current concentrator time is %u, pkt_type=%d\n", time_us, pkt_type);

  if (packet == NULL)
    {
      MSG_DEBUG (DEBUG_JIT_ERROR, "ERROR: invalid parameter\n");
      return JIT_ERROR_INVALID;
    }

  if (jit_queue_is_full (queue))
    {
      MSG_DEBUG (DEBUG_JIT_ERROR, "ERROR: cannot enqueue packet, JIT queue is full\n");
      return JIT_ERROR_FULL;
    }

  /* Compute packet pre/post delays depending on packet's type */
  packet_pre_delay = TX_START_DELAY + TX_JIT_DELAY;
  packet_post_delay = lgw_time_on_air (packet) * 1000UL; /* in us */

  /* Check criteria_1: is it already too late to send this packet ?
     *  The packet should arrive at least at (tmst - TX_START_DELAY) to be programmed into concentrator
     *  Note: - Also add some margin, to be checked how much is needed, if needed
     *
     *  Warning: unsigned arithmetic (handle roll-over)
     *      t_packet < t_current + TX_START_DELAY + MARGIN
     */
  if ((packet->count_us - time_us) <= (TX_START_DELAY + TX_MARGIN_DELAY + TX_JIT_DELAY))
    {
      MSG_DEBUG (
          DEBUG_JIT_ERROR,
          "ERROR: Packet REJECTED, already too late to send it (current=%u, packet=%u, type=%d)\n",
          time_us, packet->count_us, pkt_type);
      return JIT_ERROR_TOO_LATE;
    }

  /* Check criteria_2: Does packet timestamp seem plausible compared to current time
     *  We do not expect the server to program a downlink too early compared to current time
     *  Class A: downlink has to be sent in a 1s or 2s time window after RX
     *  Class B: downlink has to occur in a 128s time window
     *  Class C: no check needed, departure time has been calculated previously
     *  So let's define a safe delay above which we can say that the packet is out of bound: TX_MAX_ADVANCE_DELAY
     *
     *  Warning: unsigned arithmetic (handle roll-over)
                t_packet > t_current + TX_MAX_ADVANCE_DELAY
     */
  if ((packet->count_us - time_us) > TX_MAX_ADVANCE_DELAY)
    {
      MSG_DEBUG (DEBUG_JIT_ERROR,
                 "ERROR: Packet REJECTED, timestamp seems wrong, too much in advance "
                 "(current=%u, packet=%u, type=%d)\n",
                 time_us, packet->count_us, pkt_type);
      return JIT_ERROR_TOO_EARLY;
    }

  /* Check criteria_3: does this new packet overlap with a packet already enqueued ?
     *  Note: - need to take into account packet's pre_delay and post_delay of each packet
     */
  for (i = 0; i < queue->num_pkt; i++)
    {
      target_pre_delay = queue->nodes[i].pre_delay;

      /* Check if there is a collision
         *  Warning: unsigned arithmetic (handle roll-over)
         *      t_packet_new - pre_delay_packet_new < t_packet_prev + post_delay_packet_prev (OVERLAP on post delay)
         *      t_packet_new + post_delay_packet_new > t_packet_prev - pre_delay_packet_prev (OVERLAP on pre delay)
         */
      if (jit_collision_test (packet->count_us, packet_pre_delay, packet_post_delay,
                              queue->nodes[i].pkt.count_us, target_pre_delay,
                              queue->nodes[i].post_delay) == true)
        {
          MSG_DEBUG (DEBUG_JIT_ERROR,
                     "ERROR: Packet (type=%d) REJECTED, collision with packet already "
                     "programmed at %u (%u)\n",
                     pkt_type, queue->nodes[i].pkt.count_us, packet->count_us);
          return JIT_ERROR_COLLISION_PACKET;
        }
    }

  /* Finally enqueue it */
  /* Insert packet at the end of the queue */
  memcpy (&(queue->nodes[queue->num_pkt].pkt), packet, sizeof (struct lgw_pkt_tx_s));
  queue->nodes[queue->num_pkt].pre_delay = packet_pre_delay;
  queue->nodes[queue->num_pkt].post_delay = packet_post_delay;
  queue->nodes[queue->num_pkt].pkt_type = pkt_type;
  queue->num_pkt++;
  /* Sort the queue in ascending order of packet timestamp */
  jit_sort_queue (queue);

  /* Done */

  jit_print_queue (queue, false, DEBUG_JIT);

  MSG_DEBUG (DEBUG_JIT, "enqueued packet with count_us=%u (size=%u bytes, toa=%u us, type=%u)\n",
             packet->count_us, packet->size, packet_post_delay, pkt_type);

  return JIT_ERROR_OK;
}

enum jit_error_e
jit_dequeue (struct jit_queue_s *queue, int index, struct lgw_pkt_tx_s *packet,
             enum jit_pkt_type_e *pkt_type)
{
  if (packet == NULL)
    {
      MSG ("ERROR: invalid parameter\n");
      return JIT_ERROR_INVALID;
    }

  if ((index < 0) || (index >= JIT_QUEUE_MAX))
    {
      MSG ("ERROR: invalid parameter\n");
      return JIT_ERROR_INVALID;
    }

  if (jit_queue_is_empty (queue))
    {
      MSG ("ERROR: cannot dequeue packet, JIT queue is empty\n");
      return JIT_ERROR_EMPTY;
    }

  /* Dequeue requested packet */
  memcpy (packet, &(queue->nodes[index].pkt), sizeof (struct lgw_pkt_tx_s));
  queue->num_pkt--;
  *pkt_type = queue->nodes[index].pkt_type;

  /* Replace dequeued packet with last packet of the queue */
  memcpy (&(queue->nodes[index]), &(queue->nodes[queue->num_pkt]), sizeof (struct jit_node_s));
  memset (&(queue->nodes[queue->num_pkt]), 0, sizeof (struct jit_node_s));

  /* Sort queue in ascending order of packet timestamp */
  jit_sort_queue (queue);

  /* Done */

  jit_print_queue (queue, false, DEBUG_JIT);

  MSG_DEBUG (DEBUG_JIT, "dequeued packet with count_us=%u from index %d\n", packet->count_us,
             index);

  return JIT_ERROR_OK;
}

enum jit_error_e
jit_peek (struct jit_queue_s *queue, struct timeval *time, int *pkt_idx)
{
  /* Return index of node containing a packet inline with given time */
  int i = 0;
  int idx_highest_priority = -1;
  uint32_t time_us;

  if ((time == NULL) || (pkt_idx == NULL))
    {
      MSG ("ERROR: invalid parameter\n");
      return JIT_ERROR_INVALID;
    }

  if (jit_queue_is_empty (queue))
    {
      return JIT_ERROR_EMPTY;
    }

  time_us = time->tv_sec * 1000000UL + time->tv_usec;

  /* Search for highest priority packet to be sent */
  for (i = 0; i < queue->num_pkt; i++)
    {
      /* First check if that packet is outdated:
         *  If a packet seems too much in advance, and was not rejected at enqueue time,
         *  it means that we missed it for peeking, we need to drop it
         *
         *  Warning: unsigned arithmetic
         *      t_packet > t_current + TX_MAX_ADVANCE_DELAY
         */
      if ((queue->nodes[i].pkt.count_us - time_us) >= TX_MAX_ADVANCE_DELAY)
        {
          /* We drop the packet to avoid lock-up */
          queue->num_pkt--;
          MSG_DEBUG (DEBUG_JIT_WARN, "Packet dropped (current_time=%u, packet_time=%u) ---\n",
                     time_us, queue->nodes[i].pkt.count_us);

          /* Replace dropped packet with last packet of the queue */
          memcpy (&(queue->nodes[i]), &(queue->nodes[queue->num_pkt]), sizeof (struct jit_node_s));
          memset (&(queue->nodes[queue->num_pkt]), 0, sizeof (struct jit_node_s));

          /* Sort queue in ascending order of packet timestamp */
          jit_sort_queue (queue);

          /* restart loop  after purge to find packet to be sent */
          i = 0;
          continue;
        }

      /* Then look for highest priority packet to be sent:
         *  Warning: unsigned arithmetic (handle roll-over)
         *      t_packet < t_highest
         */
      if ((idx_highest_priority == -1) ||
          (((queue->nodes[i].pkt.count_us - time_us) <
            (queue->nodes[idx_highest_priority].pkt.count_us - time_us))))
        {
          idx_highest_priority = i;
        }
    }

  /* Peek criteria 1: look for a packet to be sent in next TX_JIT_DELAY ms timeframe
     *  Warning: unsigned arithmetic (handle roll-over)
     *      t_packet < t_current + TX_JIT_DELAY
     */
  if ((queue->nodes[idx_highest_priority].pkt.count_us - time_us) < TX_JIT_DELAY)
    {
      *pkt_idx = idx_highest_priority;
      MSG_DEBUG (DEBUG_JIT, "peek packet with count_us=%u at index %d\n",
                 queue->nodes[idx_highest_priority].pkt.count_us, idx_highest_priority);
    }
  else
    {
      *pkt_idx = -1;
    }

  return JIT_ERROR_OK;
}

void
jit_print_queue (struct jit_queue_s *queue, bool show_all, int debug_level)
{
  int i = 0;
  int loop_end;

  if (jit_queue_is_empty (queue))
    {
      MSG_DEBUG (debug_level, "INFO: [jit] queue is empty\n");
    }
  else
    {
      MSG_DEBUG (debug_level, "INFO: [jit] queue contains %d packets:\n", queue->num_pkt);
      loop_end = (show_all == true) ? JIT_QUEUE_MAX : queue->num_pkt;
      for (i = 0; i < loop_end; i++)
        {
          MSG_DEBUG (debug_level, " - node[%d]: count_us=%u - type=%d\n", i,
                     queue->nodes[i].pkt.count_us, queue->nodes[i].pkt_type);
        }
    }
}

std::string
jit_get_print_queue (struct jit_queue_s *queue, bool show_all, int debug_level)
{
  std::stringstream ss;

  int i = 0;
  int loop_end;

  if (!debug_level)
    return ss.str ();

  if (jit_queue_is_empty (queue))
    {
      ss << "[jit] queue is empty\n";
    }
  else
    {
      ss << "[jit] queue contains " << (unsigned) queue->num_pkt << " packets:\n";
      loop_end = (show_all == true) ? JIT_QUEUE_MAX : queue->num_pkt;
      for (i = 0; i < loop_end; i++)
        {
          ss << " - node[" << i << "]: count_us=" << (unsigned) queue->nodes[i].pkt.count_us
             << " - type=" << queue->nodes[i].pkt_type << "\n";
        }
    }
  return ss.str ();
}
