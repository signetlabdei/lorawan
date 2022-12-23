/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2013 Semtech-Cycleo

Description:
    LoRa concentrator : Timer synchronization
        Provides synchronization between unix, concentrator and gps clocks

License: Revised BSD License, see LICENSE file in this directory
Maintainer: Michael Coracin
*/

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdio.h> /* printf, fprintf, snprintf, fopen, fputs */
#include <stdint.h> /* C99 types */
#include <pthread.h>

#include "trace.h"
#include "timersync.h"
#include "ns3/loragw_hal.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS & TYPES -------------------------------------------- */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define timersub(a, b, result)                         \
  do                                                   \
    {                                                  \
      (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;    \
      (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
      if ((result)->tv_usec < 0)                       \
        {                                              \
          --(result)->tv_sec;                          \
          (result)->tv_usec += 1000000;                \
        }                                              \
  } while (0)

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES (GLOBAL) ------------------------------------------- */

struct timeval offset_unix_concent = {0, 30000}; /* timer offset between unix host and concentrator */

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

int
get_concentrator_time (struct timeval *concent_time, struct timeval unix_time)
{
  struct timeval local_timeval;

  if (concent_time == NULL)
    {
      MSG ("ERROR: %s invalid parameter\n", __FUNCTION__);
      return -1;
    }

  timersub (&unix_time, &offset_unix_concent, &local_timeval);

  /* TODO: handle sx1301 coutner wrap-up !! */
  concent_time->tv_sec = local_timeval.tv_sec;
  concent_time->tv_usec = local_timeval.tv_usec;

  MSG_DEBUG (DEBUG_TIMERSYNC, " --> TIME: unix current time is   %ld,%ld\n", unix_time.tv_sec,
             unix_time.tv_usec);
  MSG_DEBUG (DEBUG_TIMERSYNC, "           offset is              %ld,%ld\n",
             offset_unix_concent.tv_sec, offset_unix_concent.tv_usec);
  MSG_DEBUG (DEBUG_TIMERSYNC, "           sx1301 current time is %ld,%ld\n", local_timeval.tv_sec,
             local_timeval.tv_usec);

  return 0;
}
