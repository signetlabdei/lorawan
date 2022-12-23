/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
  (C)2013 Semtech-Cycleo

Description:
    LoRa concentrator Hardware Abstraction Layer

License: Revised BSD License, see LICENSE file in this directory
Maintainer: Sylvain Miermont
*/

/* -------------------------------------------------------------------------- */
/* --- DEPENDANCIES --------------------------------------------------------- */

#include <stdint.h> /* C99 types */
#include <stdbool.h> /* bool type */
#include <stdio.h> /* printf fprintf */
#include <string.h> /* memcpy */
#include <math.h> /* pow, cell */

#include "loragw_hal.h"

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#if DEBUG_HAL == 1
#define DEBUG_MSG(str) fprintf (stderr, str)
#define DEBUG_PRINTF(fmt, args...) fprintf (stderr, "%s:%d: " fmt, __FUNCTION__, __LINE__, args)
#define DEBUG_ARRAY(a, b, c)       \
  for (a = 0; a < b; ++a)          \
    fprintf (stderr, "%x.", c[a]); \
  fprintf (stderr, "end\n")
#define CHECK_NULL(a)                                                                       \
  if (a == NULL)                                                                            \
    {                                                                                       \
      fprintf (stderr, "%s:%d: ERROR: NULL POINTER AS ARGUMENT\n", __FUNCTION__, __LINE__); \
      return LGW_HAL_ERROR;                                                                 \
    }
#else
#define DEBUG_MSG(str)
#define DEBUG_PRINTF(fmt, args...)
#define DEBUG_ARRAY(a, b, c) \
  for (a = 0; a != 0;)       \
    {                        \
    }
#define CHECK_NULL(a)       \
  if (a == NULL)            \
    {                       \
      return LGW_HAL_ERROR; \
    }
#endif

#define IF_HZ_TO_REG(f) (f << 5) / 15625
#define SET_PPM_ON(bw, dr)                                                  \
  (((bw == BW_125KHZ) && ((dr == DR_LORA_SF11) || (dr == DR_LORA_SF12))) || \
   ((bw == BW_250KHZ) && (dr == DR_LORA_SF12)))
#define TRACE() fprintf (stderr, "@ %s %d\n", __FUNCTION__, __LINE__);

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS & TYPES -------------------------------------------- */

#define MCU_ARB 0
#define MCU_AGC 1
#define MCU_ARB_FW_BYTE 8192 /* size of the firmware IN BYTES (= twice the number of 14b words) */
#define MCU_AGC_FW_BYTE 8192 /* size of the firmware IN BYTES (= twice the number of 14b words) */
#define FW_VERSION_ADDR 0x20 /* Address of firmware version in data memory */
#define FW_VERSION_CAL 2 /* Expected version of calibration firmware */
#define FW_VERSION_AGC 4 /* Expected version of AGC firmware */
#define FW_VERSION_ARB 1 /* Expected version of arbiter firmware */

#define TX_METADATA_NB 16
#define RX_METADATA_NB 16

#define AGC_CMD_WAIT 16
#define AGC_CMD_ABORT 17

#define MIN_LORA_PREAMBLE 6
#define STD_LORA_PREAMBLE 8
#define MIN_FSK_PREAMBLE 3
#define STD_FSK_PREAMBLE 5

#define RSSI_MULTI_BIAS \
  -35 /* difference between "multi" modem RSSI offset and "stand-alone" modem RSSI offset */
#define RSSI_FSK_POLY_0 60 /* polynomiam coefficients to linearize FSK RSSI */
#define RSSI_FSK_POLY_1 1.5351
#define RSSI_FSK_POLY_2 0.003

/* Useful bandwidth of SX125x radios to consider depending on channel bandwidth */
/* Note: the below values come from lab measurements. For any question, please contact Semtech support */
#define LGW_RF_RX_BANDWIDTH_125KHZ 925000 /* for 125KHz channels */
#define LGW_RF_RX_BANDWIDTH_250KHZ 1000000 /* for 250KHz channels */
#define LGW_RF_RX_BANDWIDTH_500KHZ 1100000 /* for 500KHz channels */

#define TX_START_DELAY_DEFAULT 1497 /* Calibrated value for 500KHz BW and notch filter disabled */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */

static uint8_t fsk_sync_word_size = 3; /* default number of bytes for FSK sync word */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DECLARATION ---------------------------------------- */

int32_t lgw_sf_getval (int x);
int32_t lgw_bw_getval (int x);

/* -------------------------------------------------------------------------- */
/* --- PRIVATE FUNCTIONS DEFINITION ----------------------------------------- */

int32_t
lgw_bw_getval (int x)
{
  switch (x)
    {
    case BW_500KHZ:
      return 500000;
    case BW_250KHZ:
      return 250000;
    case BW_125KHZ:
      return 125000;
    case BW_62K5HZ:
      return 62500;
    case BW_31K2HZ:
      return 31200;
    case BW_15K6HZ:
      return 15600;
    case BW_7K8HZ:
      return 7800;
    default:
      return -1;
    }
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int32_t
lgw_sf_getval (int x)
{
  switch (x)
    {
    case DR_LORA_SF7:
      return 7;
    case DR_LORA_SF8:
      return 8;
    case DR_LORA_SF9:
      return 9;
    case DR_LORA_SF10:
      return 10;
    case DR_LORA_SF11:
      return 11;
    case DR_LORA_SF12:
      return 12;
    default:
      return -1;
    }
}

/* -------------------------------------------------------------------------- */
/* --- PUBLIC FUNCTIONS DEFINITION ------------------------------------------ */

uint32_t
lgw_time_on_air (struct lgw_pkt_tx_s *packet)
{
  int32_t val;
  uint8_t SF, H, DE;
  uint16_t BW;
  uint32_t payloadSymbNb, Tpacket;
  double Tsym, Tpreamble, Tpayload, Tfsk;

  if (packet == NULL)
    {
      DEBUG_MSG ("ERROR: Failed to compute time on air, wrong parameter\n");
      return 0;
    }

  if (packet->modulation == MOD_LORA)
    {
      /* Get bandwidth */
      val = lgw_bw_getval (packet->bandwidth);
      if (val != -1)
        {
          BW = (uint16_t) (val / 1E3);
        }
      else
        {
          DEBUG_PRINTF (
              "ERROR: Cannot compute time on air for this packet, unsupported bandwidth (0x%02X)\n",
              packet->bandwidth);
          return 0;
        }

      /* Get datarate */
      val = lgw_sf_getval (packet->datarate);
      if (val != -1)
        {
          SF = (uint8_t) val;
        }
      else
        {
          DEBUG_PRINTF (
              "ERROR: Cannot compute time on air for this packet, unsupported datarate (0x%02X)\n",
              packet->datarate);
          return 0;
        }

      /* Duration of 1 symbol */
      Tsym = pow (2, SF) / BW;

      /* Duration of preamble */
      Tpreamble = ((double) (packet->preamble) + 4.25) * Tsym;

      /* Duration of payload */
      H = (packet->no_header == false) ? 0 : 1; /* header is always enabled, except for beacons */
      DE = (SF >= 11) ? 1 : 0; /* Low datarate optimization enabled for SF11 and SF12 */

      payloadSymbNb = 8 + (ceil ((double) (8 * packet->size - 4 * SF + 28 + 16 - 20 * H) /
                                 (double) (4 * (SF - 2 * DE))) *
                           (packet->coderate +
                            4)); /* Explicitely cast to double to keep precision of the division */

      Tpayload = payloadSymbNb * Tsym;

      /* Duration of packet */
      Tpacket = Tpreamble + Tpayload;
    }
  else if (packet->modulation == MOD_FSK)
    {
      /* PREAMBLE + SYNC_WORD + PKT_LEN + PKT_PAYLOAD + CRC
                PREAMBLE: default 5 bytes
                SYNC_WORD: default 3 bytes
                PKT_LEN: 1 byte (variable length mode)
                PKT_PAYLOAD: x bytes
                CRC: 0 or 2 bytes
        */
      Tfsk = (8 *
              (double) (packet->preamble + fsk_sync_word_size + 1 + packet->size +
                        ((packet->no_crc == true) ? 0 : 2)) /
              (double) packet->datarate) *
             1E3;

      /* Duration of packet */
      Tpacket = (uint32_t) Tfsk + 1; /* add margin for rounding */
    }
  else
    {
      Tpacket = 0;
      DEBUG_PRINTF (
          "ERROR: Cannot compute time on air for this packet, unsupported modulation (0x%02X)\n",
          packet->modulation);
    }

  return Tpacket;
}

/* --- EOF ------------------------------------------------------------------ */
