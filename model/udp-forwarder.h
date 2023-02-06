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
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 *
 */

#ifndef UDP_FORWARDER_H
#define UDP_FORWARDER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ipv4-address.h"
#include "ns3/jitqueue.h"
#include "ns3/lora-net-device.h"
#include "ns3/loragw_hal.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/socket.h"

#include <queue>

/******************************
 * Semtech UDP Forwarder code *
 ******************************/

/* -------------------------------------------------------------------------- */
/* --- PRIVATE MACROS ------------------------------------------------------- */

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define STRINGIFY(x) #x
#define STR(x) STRINGIFY(x)
#define CHECK_NULL(a)                                                                              \
    if (a == NULL)                                                                                 \
    {                                                                                              \
        return LGW_HAL_ERROR;                                                                      \
    }

/* -------------------------------------------------------------------------- */
/* --- PRIVATE CONSTANTS ---------------------------------------------------- */

#ifndef VERSION_STRING
#define VERSION_STRING "undefined"
#endif

#define DEFAULT_SERVER 127.0.0.1 /* hostname also supported */
#define DEFAULT_PORT_UP 1700
#define DEFAULT_PORT_DW 1700
#define DEFAULT_KEEPALIVE 5 /* default time interval for downstream keep-alive packet */
#define DEFAULT_STAT 30     /* default time interval for statistics */
#define PUSH_TIMEOUT_MS 100
#define PULL_TIMEOUT_MS 200
#define GPS_REF_MAX_AGE                                                                            \
    30 /* maximum admitted delay in seconds of GPS loss before considering latest GPS sync         \
          unusable */
#define FETCH_SLEEP_MS 10 /* nb of ms waited when a fetch return no packets */

#define PROTOCOL_VERSION 2 /* v1.3 */

#define PKT_PUSH_DATA 0
#define PKT_PUSH_ACK 1
#define PKT_PULL_DATA 2
#define PKT_PULL_RESP 3
#define PKT_PULL_ACK 4
#define PKT_TX_ACK 5

#define NB_PKT_MAX 8 /* max number of packets per fetch/send cycle */

#define MIN_LORA_PREAMB 6 /* minimum Lora preamble length for this application */
#define STD_LORA_PREAMB 8
#define MIN_FSK_PREAMB 3 /* minimum FSK preamble length for this application */
#define STD_FSK_PREAMB 5

#define STATUS_SIZE 200
#define TX_BUFF_SIZE ((540 * NB_PKT_MAX) + 30 + STATUS_SIZE)

#define UNIX_GPS_EPOCH_OFFSET                                                                      \
    315964800 /* Number of seconds elapsed between 01.Jan.1970 00:00:00 and 06.Jan.1980 00:00:00   \
               */

namespace ns3
{
namespace lorawan
{

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
    static TypeId GetTypeId(void);

    UdpForwarder();

    virtual ~UdpForwarder();

    /**
     * \brief set the remote address and port
     * \param ip remote IP address
     * \param port remote port
     */
    void SetRemote(Address ip, uint16_t port);
    /**
     * \brief set the remote address
     * \param addr remote address
     */
    void SetRemote(Address addr);

    /**
     * Sets the device to use to communicate with the EDs.
     *
     * \param loraNetDevice The LoraNetDevice on this node.
     */
    void SetLoraNetDevice(Ptr<LoraNetDevice> loraNetDevice);

    /**
     * Receive a packet from the LoraNetDevice.
     *
     * \param loraNetDevice The LoraNetDevice we received the packet from.
     * \param packet The packet we received.
     * \param protocol The protocol number associated to this packet.
     * \param sender The address of the sender.
     * \returns True if we can handle the packet, false otherwise.
     */
    bool ReceiveFromLora(Ptr<NetDevice> loraNetDevice,
                         Ptr<const Packet> packet,
                         uint16_t protocol,
                         const Address& sender);

  protected:
    virtual void DoDispose(void);

  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    Address m_peerAddress; //!< Remote peer address
    uint16_t m_peerPort;   //!< Remote peer port
#ifdef NS3_LOG_ENABLE
    std::string m_peerAddressString; //!< Remote peer address string
#endif                               // NS3_LOG_ENABLE

    Ptr<LoraNetDevice> m_loraNetDevice; //!< Pointer to the node's LoraNetDevice

    /* -------------------------------------------------------------------------- */
    /* ---------------- Ns-3 INTEGRATION of lora_pkt_fwd.c ---------------------- */

    /* Emulate lora_pkt_fwd.c parse_SX1301_configuration() & parse_gateway_configuration() */
    void Configure(void);
    static const struct coord_s m_center;

    void ThreadUp(void); //!< Emulate lora_pkt_fwd.c uplink forwarding loop
    /* THREAD UP auxiliary variables */
    Ptr<Socket> m_sockUp; //!< Socket Up
    EventId m_upEvent;    //!< Event to forward packets uplink
    /* protocol variables */
    uint8_t m_upTokenH; /* random token for acknowledgement matching */
    uint8_t m_upTokenL; /* random token for acknowledgement matching */
    /* ping measurement variables */
    timespec m_upSendTime;
    timespec m_upRecvTime;
    uint8_t m_remainingRecvAckAttempts;
    void ReceiveAck(Ptr<Socket> sockUp);

    void ThreadDown(void); //!< Emulate lora_pkt_fwd.c downlink reception loop
    /* THREAD DOWN auxiliary variables */
    Ptr<Socket> m_sockDown; //!< Socket Down
    EventId m_downEvent;
    /* protocol variables */
    uint8_t m_downTokenH; /* random token for acknowledgement matching */
    uint8_t m_downTokenL; /* random token for acknowledgement matching */
    bool m_reqAck;        /* keep track of whether PULL_DATA was acknowledged or not */
    /* local timekeeping variables */
    timespec m_downSendTime;
    timespec m_downRecvTime;
    /* auto-quit variable */
    uint32_t m_autoquitCnt; /* count the number of PULL_DATA sent since the latest PULL_ACK */
    void CheckPullCondition(void);
    void SockDownTimeout(void);
    void ReceiveDatagram(Ptr<Socket> sockDown);

    void ThreadJit(void); //!< Emulate lora_pkt_fwd.c loop to send downlink packets in jit queue
    EventId m_jitEvent;

    void CollectStatistics(void); //!< Emulate lora_pkt_fwd.c stats collection loop
    EventId m_statsEvent;

    /* -------------------------------------------------------------------------- */
    /* ---------- PUBLIC FUNCTIONS re-implemented from loragw_hal.h ------------- */

    int LgwReceive(int nb_pkt_max, lgw_pkt_rx_s rxpkt[]); //!< Implements concentrator lgw_receive
    std::queue<lgw_pkt_rx_s> m_rxPktBuff; //!< Emulate the concentrator reception packet buffer

    int LgwStatus(uint8_t select, uint8_t* code);
    int LgwSend(struct lgw_pkt_tx_s pkt_data);

    /* -------------------------------------------------------------------------- */
    /* ---------- GLOBAL VARIABLES & FUNCTIONS from lora_pkt_fwd.c -------------- */

    /* packets filtering configuration variables */
    bool fwd_valid_pkt = true;  /* packets with PAYLOAD CRC OK are forwarded */
    bool fwd_error_pkt = false; /* packets with PAYLOAD CRC ERROR are NOT forwarded */
    bool fwd_nocrc_pkt = false; /* packets with NO PAYLOAD CRC are NOT forwarded */

    /* network configuration variables */
    uint64_t lgwm = 0; /* Lora gateway MAC address */
    int keepalive_time =
        DEFAULT_KEEPALIVE; /* send a PULL_DATA request every X seconds, negative = disabled */

    /* statistics collection configuration variables */
    unsigned stat_interval =
        DEFAULT_STAT; /* time interval (in sec) at which statistics are collected and displayed */

    /* gateway <-> MAC protocol variables */
    uint32_t net_mac_h; /* Most Significant Nibble, network order */
    uint32_t net_mac_l; /* Least Significant Nibble, network order */

    /* network protocol variables */
    struct timeval push_timeout_half = {
        0,
        (PUSH_TIMEOUT_MS * 500)}; /* cut in half, critical for throughput */
    struct timeval pull_timeout = {0, (PULL_TIMEOUT_MS * 1000)}; /* non critical for throughput */

    /* Reference coordinates */
    struct coord_s reference_coord;

    /* Enable faking the GPS coordinates of the gateway */
    bool gps_fake_enable; /* enable the feature */

    /* measurements to establish statistics */
    uint32_t meas_nb_rx_rcv = 0;       /* count packets received */
    uint32_t meas_nb_rx_ok = 0;        /* count packets received with PAYLOAD CRC OK */
    uint32_t meas_nb_rx_bad = 0;       /* count packets received with PAYLOAD CRC ERROR */
    uint32_t meas_nb_rx_nocrc = 0;     /* count packets received with NO PAYLOAD CRC */
    uint32_t meas_up_pkt_fwd = 0;      /* number of radio packet forwarded to the server */
    uint32_t meas_up_network_byte = 0; /* sum of UDP bytes sent for upstream traffic */
    uint32_t meas_up_payload_byte = 0; /* sum of radio payload bytes sent for upstream traffic */
    uint32_t meas_up_dgram_sent = 0;   /* number of datagrams sent for upstream traffic */
    uint32_t meas_up_ack_rcv = 0;      /* number of datagrams acknowledged for upstream traffic */

    uint32_t meas_dw_pull_sent = 0; /* number of PULL requests sent for downstream traffic */
    uint32_t meas_dw_ack_rcv = 0; /* number of PULL requests acknowledged for downstream traffic */
    uint32_t meas_dw_dgram_rcv =
        0; /* count PULL response packets received for downstream traffic */
    uint32_t meas_dw_network_byte = 0; /* sum of UDP bytes sent for upstream traffic */
    uint32_t meas_dw_payload_byte = 0; /* sum of radio payload bytes sent for upstream traffic */
    uint32_t meas_nb_tx_ok = 0;        /* count packets emitted successfully */
    uint32_t meas_nb_tx_fail = 0;      /* count packets were TX failed for other reasons */
    uint32_t meas_nb_tx_requested = 0; /* count TX request from server (downlinks) */
    uint32_t meas_nb_tx_rejected_collision_packet =
        0; /* count packets were TX request were rejected due to collision with another packet
              already programmed */
    uint32_t meas_nb_tx_rejected_collision_beacon =
        0; /* count packets were TX request were rejected due to collision with a beacon already
              programmed */
    uint32_t meas_nb_tx_rejected_too_late =
        0; /* count packets were TX request were rejected because it is too late to program it */
    uint32_t meas_nb_tx_rejected_too_early = 0; /* count packets were TX request were rejected
                                                   because timestamp is too much in advance */
    uint32_t meas_nb_beacon_queued = 0;         /* count beacon inserted in jit queue */
    uint32_t meas_nb_beacon_sent = 0;           /* count beacon actually sent to concentrator */
    uint32_t meas_nb_beacon_rejected = 0;       /* count beacon rejected for queuing */

    bool report_ready = false;       /* true when there is a new report to send to the server */
    char status_report[STATUS_SIZE]; /* status report as a JSON object */

    /* auto-quit function */
    uint32_t autoquit_threshold =
        0; /* enable auto-quit after a number of non-acknowledged PULL_DATA (0 = disabled)*/

    /* Just In Time TX scheduling */
    struct jit_queue_s jit_queue;

    /* Gateway specificities */
    int8_t antenna_gain = 0;

    /* TX capabilities */
    struct lgw_tx_gain_lut_s txlut;        /* TX gain table */
    uint32_t tx_freq_min[LGW_RF_CHAIN_NB]; /* lowest frequency supported by TX chain */
    uint32_t tx_freq_max[LGW_RF_CHAIN_NB]; /* highest frequency supported by TX chain */

    static double difftimespec(struct timespec end, struct timespec beginning);

    int send_tx_ack(uint8_t token_h, uint8_t token_l, enum jit_error_e error);

    static void print_tx_status(uint8_t tx_status);
};

} // namespace lorawan
} // namespace ns3

#endif /* UDP_FORWARDER_H */
