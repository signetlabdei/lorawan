/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2021 Alessandro Aimi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Alessandro Aimi <alessandro.aimi@cnam.fr>
 *                         <alessandro.aimi@orange.com>
 *
 */

#ifndef UDP_FORWARDER_H
#define UDP_FORWARDER_H

#include "ns3/lora-net-device.h"
#include "ns3/csma-net-device.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/socket.h"
#include "ns3/packet.h"

#include <queue>

/****************************** 
 * Semtech UDP Forwarder code *
 ******************************/

// From trace.h

#define MSG(args...) printf (args) /* message that is destined to the user */

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS - lora_pkt_fwd.c ----------------------------------- */

#define PUSH_TIMEOUT_MS 100
#define FETCH_SLEEP_MS 10 /* nb of ms waited when a fetch return no packets */

#define PROTOCOL_VERSION 2 /* v1.3 */

#define PKT_PUSH_DATA 0
#define PKT_PUSH_ACK 1

#define NB_PKT_MAX 8 /* max number of packets per fetch/send cycle */

#define MIN_LORA_PREAMB 6 /* minimum Lora preamble length for this application */
#define STD_LORA_PREAMB 8

#define STATUS_SIZE 200
#define TX_BUFF_SIZE ((540 * NB_PKT_MAX) + 30 + STATUS_SIZE)

/* -------------------------------------------------------------------------- */
/* --- PUBLIC CONSTANTS - loragw_hal.h -------------------------------------- */

/* values available for the 'modulation' parameters */
/* NOTE: arbitrary values */
#define MOD_UNDEFINED 0
#define MOD_LORA 0x10
#define MOD_FSK 0x20

/* values available for the 'bandwidth' parameters (LoRa & FSK) */
/* NOTE: directly encode FSK RX bandwidth, do not change */
#define BW_UNDEFINED 0
#define BW_500KHZ 0x01
#define BW_250KHZ 0x02
#define BW_125KHZ 0x03
#define BW_62K5HZ 0x04
#define BW_31K2HZ 0x05
#define BW_15K6HZ 0x06
#define BW_7K8HZ 0x07

/* values available for the 'datarate' parameters */
/* NOTE: LoRa values used directly to code SF bitmask in 'multi' modem, do not change */
#define DR_UNDEFINED 0
#define DR_LORA_SF7 0x02
#define DR_LORA_SF8 0x04
#define DR_LORA_SF9 0x08
#define DR_LORA_SF10 0x10
#define DR_LORA_SF11 0x20
#define DR_LORA_SF12 0x40
#define DR_LORA_MULTI 0x7E

/* values available for the 'coderate' parameters (LoRa only) */
/* NOTE: arbitrary values */
#define CR_UNDEFINED 0
#define CR_LORA_4_5 0x01
#define CR_LORA_4_6 0x02
#define CR_LORA_4_7 0x03
#define CR_LORA_4_8 0x04

/* values available for the 'status' parameter */
/* NOTE: values according to hardware specification */
#define STAT_UNDEFINED 0x00
#define STAT_NO_CRC 0x01
#define STAT_CRC_BAD 0x11
#define STAT_CRC_OK 0x10

namespace ns3 {
namespace lorawan {

/* -------------------------------------------------------------------------- */
/* --- PUBLIC TYPES - loragw_hal.h ------------------------------------------ */

/**
@struct lgw_conf_lbt_chan_s
@brief Configuration structure for LBT channels
*/
struct lgw_conf_lbt_chan_s
{
  uint32_t freq_hz;
  uint16_t scan_time_us;
};

/**
@struct lgw_pkt_rx_s
@brief Structure containing the metadata of a packet that was received and a pointer to the payload
*/
struct lgw_pkt_rx_s
{
  uint32_t freq_hz; /*!> central frequency of the IF chain */
  uint8_t if_chain; /*!> by which IF chain was packet received */
  uint8_t status; /*!> status of the received packet */
  uint32_t
      count_us; /*!> internal concentrator counter for timestamping, 1 microsecond resolution */
  uint8_t rf_chain; /*!> through which RF chain the packet was received */
  uint8_t modulation; /*!> modulation used by the packet */
  uint8_t bandwidth; /*!> modulation bandwidth (LoRa only) */
  uint32_t datarate; /*!> RX datarate of the packet (SF for LoRa) */
  uint8_t coderate; /*!> error-correcting code of the packet (LoRa only) */
  float rssi; /*!> average packet RSSI in dB */
  float snr; /*!> average packet SNR, in dB (LoRa only) */
  float snr_min; /*!> minimum packet SNR, in dB (LoRa only) */
  float snr_max; /*!> maximum packet SNR, in dB (LoRa only) */
  uint16_t crc; /*!> CRC that was received in the payload */
  uint16_t size; /*!> payload size in bytes */
  uint8_t payload[256]; /*!> buffer containing the payload */
};

/* -------------------------------------------------------------------------- */
/* --- PUBLIC TYPES - loragw_gps.h ------------------------------------------ */

/**
@struct coord_s
@brief Time solution required for timestamp to absolute time conversion
*/
struct tref
{
  time_t systime; /*!> system time when solution was calculated */
  uint32_t count_us; /*!> reference concentrator internal timestamp */
  struct timespec utc; /*!> reference UTC time (from GPS/NMEA) */
  struct timespec gps; /*!> reference GPS time (since 01.Jan.1980) */
  double xtal_err; /*!> raw clock error (eg. <1 'slow' XTAL) */
};

/**
 * \brief A Udp encapsulator and forwarder for lora packets.
 *
 */
class UdpForwarder : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  UdpForwarder ();

  virtual ~UdpForwarder ();

  /**
   * \brief set the remote address and port
   * \param ip remote IP address
   * \param port remote port
   */
  void SetRemote (Address ip, uint16_t port);
  /**
   * \brief set the remote address
   * \param addr remote address
   */
  void SetRemote (Address addr);

  /**
   * \return the total bytes sent by this app
   */
  uint64_t GetTotalTx () const;

  /**
   * Receive a packet from the LoraNetDevice.
   *
   * \param loraNetDevice The LoraNetDevice we received the packet from.
   * \param packet The packet we received.
   * \param protocol The protocol number associated to this packet.
   * \param sender The address of the sender.
   * \returns True if we can handle the packet, false otherwise.
   */
  bool ReceiveFromLora (Ptr<NetDevice> loraNetDevice, Ptr<const Packet> packet, uint16_t protocol,
                        const Address &sender);

  /**
   * Receive a packet from the CsmaNetDevice
   */ 
  // Use a listening thread
  bool ReceiveFromCsma (Ptr<NetDevice> csmaNetDevice, Ptr<const Packet> packet, uint16_t protocol,
                        const Address &sender);

protected:
  virtual void DoDispose (void);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ThreadUp (void);
  int lgw_receive (int nb_pkt_max, lgw_pkt_rx_s rxpkt[]);

  uint32_t m_sent; //!< Counter for sent packets
  uint64_t m_totalTx; //!< Total bytes sent
  Ptr<Socket> m_sockUp; //!< Socket
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port
  EventId m_forwardEvent; //!< Event to forward packets
#ifdef NS3_LOG_ENABLE
  std::string m_peerAddressString; //!< Remote peer address string
#endif // NS3_LOG_ENABLE

  /* -------------------------------------------------------------------------- */
  /* --- PRIVATE VARIABLES (GLOBAL) - lora_pkt_fwd.c -------------------------- */

  /* network configuration variables */
  uint64_t lgwm = 0; /* Lora gateway MAC address */

  /* gateway <-> MAC protocol variables */
  uint32_t net_mac_h; /* Most Significant Nibble, network order */
  uint32_t net_mac_l; /* Least Significant Nibble, network order */

  /* network sockets */
  int sock_up; /* socket for upstream traffic */
  int sock_down; /* socket for downstream traffic */

  /* network protocol variables */
  //timeval push_timeout_half = {0, (PUSH_TIMEOUT_MS * 500)}; /* cut in half, critical for throughput */

  /* measurements to establish statistics */
  uint32_t meas_up_network_byte; /* sum of UDP bytes sent for upstream traffic */
  uint32_t meas_up_dgram_sent; /* number of datagrams sent for upstream traffic */
  uint32_t meas_up_ack_rcv; /* number of datagrams acknowledged for upstream traffic */

  std::queue<lgw_pkt_rx_s> m_rxpktbuff;
};

} // namespace lorawan
} // namespace ns3

#endif /* UDP_FORWARDER_H */
