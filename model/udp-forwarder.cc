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

#include "udp-forwarder.h"

#include "ns3/base64.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/inet-socket-address.h"
#include "ns3/log.h"
#include "ns3/lora-tag.h"
#include "ns3/mac64-address.h"
#include "ns3/nstime.h"
#include "ns3/parson.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/timersync.h"
#include "ns3/trace.h"
#include "ns3/uinteger.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("UdpForwarder");

NS_OBJECT_ENSURE_REGISTERED(UdpForwarder);

TypeId
UdpForwarder::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::UdpForwarder")
                            .SetParent<Application>()
                            .SetGroupName("Applications")
                            .AddConstructor<UdpForwarder>()
                            .AddAttribute("RemoteAddress",
                                          "The destination Address of the outbound packets",
                                          AddressValue(),
                                          MakeAddressAccessor(&UdpForwarder::m_peerAddress),
                                          MakeAddressChecker())
                            .AddAttribute("RemotePort",
                                          "The destination port of the outbound packets",
                                          UintegerValue(1700),
                                          MakeUintegerAccessor(&UdpForwarder::m_peerPort),
                                          MakeUintegerChecker<uint16_t>());
    return tid;
}

UdpForwarder::UdpForwarder()
{
    NS_LOG_FUNCTION(this);
}

UdpForwarder::~UdpForwarder()
{
    NS_LOG_FUNCTION(this);
}

void
UdpForwarder::DoDispose(void)
{
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
UdpForwarder::SetRemote(Address ip, uint16_t port)
{
    NS_LOG_FUNCTION(this << ip << port);
    m_peerAddress = ip;
    m_peerPort = port;
}

void
UdpForwarder::SetRemote(Address addr)
{
    NS_LOG_FUNCTION(this << addr);
    m_peerAddress = addr;
}

void
UdpForwarder::SetLoraNetDevice(Ptr<LoraNetDevice> loraNetDevice)
{
    NS_LOG_FUNCTION(this << loraNetDevice);

    m_loraNetDevice = loraNetDevice;
}

bool
UdpForwarder::ReceiveFromLora(Ptr<NetDevice> loraNetDevice,
                              Ptr<const Packet> packet,
                              uint16_t protocol,
                              const Address& sender)
{
    NS_LOG_FUNCTION(this);
    Ptr<Packet> pktcpy = packet->Copy();

    LoraTag tag;
    pktcpy->RemovePacketTag(tag);

    timeval raw_time;
    gettimeofday(&raw_time, NULL);

    lgw_pkt_rx_s p;
    p.freq_hz = (uint32_t)((double)(1.0e6) * tag.GetFrequency());
    p.if_chain = 0;
    p.status = STAT_CRC_OK;
    p.count_us = raw_time.tv_sec * 1000000UL + raw_time.tv_usec; /* convert time in µs */
    p.rf_chain = 0;
    p.modulation = MOD_LORA;
    p.bandwidth = BW_125KHZ;
    switch (tag.GetSpreadingFactor())
    {
    case 7:
        p.datarate = DR_LORA_SF7;
        break;
    case 8:
        p.datarate = DR_LORA_SF8;
        break;
    case 9:
        p.datarate = DR_LORA_SF9;
        break;
    case 10:
        p.datarate = DR_LORA_SF10;
        break;
    case 11:
        p.datarate = DR_LORA_SF11;
        break;
    case 12:
        p.datarate = DR_LORA_SF12;
        break;
    default:
        p.datarate = DR_LORA_SF12;
        break;
    }
    p.coderate = CR_LORA_4_5;
    p.rssi = tag.GetReceivePower();
    p.snr = tag.GetSnr();
    p.snr_min = tag.GetSnr();
    p.snr_max = tag.GetSnr();
    p.crc = 0; //!> TODO: ?
    p.size = pktcpy->GetSize();
    pktcpy->CopyData(p.payload, 256);

    m_rxPktBuff.push(p);
    return true;
}

// This will act as the main of the protocol
void
UdpForwarder::StartApplication(void)
{
    NS_LOG_FUNCTION(this);

    /* load configuration files */
    Configure();

    /* process some of the configuration variables */
    net_mac_h = htonl((uint32_t)(0xFFFFFFFF & (lgwm >> 32)));
    net_mac_l = htonl((uint32_t)(0xFFFFFFFF & lgwm));

    /* Socket up */
    if (m_sockUp == 0)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_sockUp = Socket::CreateSocket(GetNode(), tid);
        if (Ipv4Address::IsMatchingType(m_peerAddress))
        {
            if (m_sockUp->Bind() == -1)
                NS_FATAL_ERROR("Failed to bind socket");
            m_sockUp->Connect(
                InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
        else
            NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);
    }
    /* set upstream socket RX callback */
    m_sockUp->SetRecvCallback(MakeCallback(&UdpForwarder::ReceiveAck, this));

    /* Socket down */
    if (m_sockDown == 0)
    {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_sockDown = Socket::CreateSocket(GetNode(), tid);
        if (Ipv4Address::IsMatchingType(m_peerAddress))
        {
            if (m_sockDown->Bind() == -1)
                NS_FATAL_ERROR("Failed to bind socket");
            m_sockDown->Connect(
                InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
        else
            NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);
    }
    /* set downstream socket RX callback */
    m_sockDown->SetRecvCallback(MakeCallback(&UdpForwarder::ReceiveDatagram, this));

#ifdef NS3_LOG_ENABLE
    std::stringstream peerAddressStringStream;
    if (Ipv4Address::IsMatchingType(m_peerAddress))
        peerAddressStringStream << Ipv4Address::ConvertFrom(m_peerAddress);
    m_peerAddressString = peerAddressStringStream.str();
#endif // NS3_LOG_ENABLE

    // Start uplink thread loop
    m_upEvent = Simulator::ScheduleNow(&UdpForwarder::ThreadUp, this);

    // Start downlink thread loop
    m_autoquitCnt = 0;
    /* JIT queue initialization */
    jit_queue_init(&jit_queue);
    m_downEvent = Simulator::ScheduleNow(&UdpForwarder::ThreadDown, this);

    /* start jit thread */
    m_jitEvent = Simulator::Schedule(MilliSeconds(10), &UdpForwarder::ThreadJit, this);

    /* main loop task : statistics collection */
    m_statsEvent = Simulator::Schedule(MilliSeconds(1000 * stat_interval),
                                       &UdpForwarder::CollectStatistics,
                                       this);
}

void
UdpForwarder::StopApplication(void)
{
    NS_LOG_FUNCTION(this);

    Simulator::Cancel(m_statsEvent);

    Simulator::Cancel(m_upEvent);
    NS_LOG_INFO("\nEnd of upstream thread");

    Simulator::Cancel(m_downEvent);
    NS_LOG_INFO("\nEnd of downstream thread");

    Simulator::Cancel(m_jitEvent);
    NS_LOG_INFO("\nEnd of jit queue thread");
}

void
UdpForwarder::Configure(void)
{
    int i;

    /* from global_conf.json */

    /* CONFIGURATIONS FROM parse_SX1301_configuration () */
    antenna_gain = 0;
    NS_LOG_INFO("antenna_gain " << (unsigned)antenna_gain << " dBi");

    /* set configuration for tx gains */
    memset(&txlut, 0, sizeof txlut); /* initialize configuration structure */
    txlut.size = TX_GAIN_LUT_SIZE_MAX;
    std::vector<int8_t> rf_power = {-6, -3, 0, 3, 6, 10, 11, 12, 13, 14, 16, 20, 23, 25, 26, 27};
    for (i = 0; i < txlut.size; ++i)
        txlut.lut[i].rf_power = rf_power[i];
    /* all parameters parsed, submitting configuration to the HAL */
    if (txlut.size > 0)
    {
        NS_LOG_INFO("Configuring TX LUT with " << (unsigned)txlut.size << " indexes");
    }
    else
    {
        NS_LOG_WARN("No TX gain LUT defined");
    }

    /* set configuration for RF chains */
    /* tx is enabled on this rf chain, we need its frequency range */
    tx_freq_min[0] = 863000000;
    tx_freq_max[0] = 870000000;
    NS_LOG_INFO("radio 0 enabled (type SX1257)");

    /* CONFIGURATIONS FROM parse_gateway_configuration () */

    /* gateway unique identifier */
    char eui[17];
    lgwm = GetNode()->GetId();
    snprintf(eui, 17, "%016lx", lgwm);
    NS_LOG_INFO("gateway ID is configured to " << eui);

    /* server addr and port are set as Ns3 attributes */

    /* get keep-alive interval (in seconds) for downstream (optional) */
    keepalive_time = 10;
    NS_LOG_INFO("downstream keep-alive interval is configured to " << keepalive_time << " seconds");

    /* get interval (in seconds) for statistics display (optional) */
    stat_interval = 30;
    NS_LOG_INFO("statistics display interval is configured to " << stat_interval << " seconds");

    /* get time-out value (in ms) for upstream datagrams (optional) */
    push_timeout_half.tv_usec = 500 * 100;
    NS_LOG_INFO("upstream PUSH_DATA time-out is configured to "
                << (unsigned)(push_timeout_half.tv_usec / 500) << " ms");

    /* packet filtering parameters */
    fwd_valid_pkt = true;
    NS_LOG_INFO("packets received with a valid CRC will" << (fwd_valid_pkt ? "" : " NOT")
                                                         << " be forwarded");
    fwd_error_pkt = false;
    NS_LOG_INFO("packets received with a CRC error will" << (fwd_error_pkt ? "" : " NOT")
                                                         << " be forwarded");
    fwd_nocrc_pkt = false;
    NS_LOG_INFO("packets received with no CRC will" << (fwd_nocrc_pkt ? "" : " NOT")
                                                    << " be forwarded");

    /* get reference coordinates */
    double r_earth = 6371000.0;
    Vector position = GetNode()->GetObject<MobilityModel>()->GetPosition();
    reference_coord.lat = m_center.lat + (position.y / r_earth) * (180.0 / M_PI);
    NS_LOG_INFO("Reference latitude is configured to " << reference_coord.lat << " deg");
    reference_coord.lon =
        m_center.lon + (position.x / r_earth) * (180.0 / M_PI) / cos(m_center.lat * M_PI / 180.0);
    NS_LOG_INFO("Reference longitude is configured to " << reference_coord.lon << " deg");
    reference_coord.alt = m_center.alt + (short)position.z;
    NS_LOG_INFO("Reference altitude is configured to " << reference_coord.alt << " meters");

    /* Gateway GPS coordinates hardcoding (aka. faking) option */
    gps_fake_enable = true;
    if (gps_fake_enable == true)
    {
        NS_LOG_INFO("fake GPS is enabled");
    }
    else
    {
        NS_LOG_INFO("fake GPS is disabled");
    }
}

const struct coord_s UdpForwarder::m_center = {48.866831, 2.356719, 42};

void
UdpForwarder::ThreadUp(void)
{
    int i, j;              /* loop variables */
    unsigned pkt_in_dgram; /* nb on Lora packet in the current datagram */

    /* allocate memory for packet fetching and processing */
    lgw_pkt_rx_s rxpkt[NB_PKT_MAX]; /* array containing inbound packets + metadata */
    lgw_pkt_rx_s* p;                /* pointer on a RX packet */
    int nb_pkt;

    /* data buffers */
    uint8_t buff_up[TX_BUFF_SIZE]; /* buffer to compose the upstream packet */
    int buff_index;

    /* report management variable */
    bool send_report = false;

    /* mote info variables */
    uint32_t mote_addr = 0;
    uint16_t mote_fcnt = 0;

    /* pre-fill the data buffer with fixed fields */
    buff_up[0] = PROTOCOL_VERSION;
    buff_up[3] = PKT_PUSH_DATA;
    *(uint32_t*)(buff_up + 4) = net_mac_h;
    *(uint32_t*)(buff_up + 8) = net_mac_l;

    /* ORIGINAL LOOP START */

    /* fetch packets */
    nb_pkt = LgwReceive(NB_PKT_MAX, rxpkt);

    /* check if there are status report to send */
    send_report = report_ready; /* copy the variable so it doesn't change mid-function */

    /* wait a short time if no packets, nor status report */
    if ((nb_pkt == 0) && (send_report == false))
    {
        m_upEvent =
            Simulator::Schedule(MilliSeconds(FETCH_SLEEP_MS), &UdpForwarder::ThreadUp, this);
        /* do not listen for acks in the meantime */
        m_remainingRecvAckAttempts = 0;
        return;
    }

    /* start composing datagram with the header */
    m_upTokenH = (uint8_t)rand(); /* random token */
    m_upTokenL = (uint8_t)rand(); /* random token */
    buff_up[1] = m_upTokenH;
    buff_up[2] = m_upTokenL;
    buff_index = 12; /* 12-byte header */

    /* start of JSON structure */
    memcpy((void*)(buff_up + buff_index), (void*)"{\"rxpk\":[", 9);
    buff_index += 9;

    /* serialize Lora packets metadata and payload */
    pkt_in_dgram = 0;
    for (i = 0; i < nb_pkt; ++i)
    {
        p = &rxpkt[i];

        /* Get mote information from current packet (addr, fcnt) */
        /* FHDR - DevAddr */
        mote_addr = p->payload[1];
        mote_addr |= p->payload[2] << 8;
        mote_addr |= p->payload[3] << 16;
        mote_addr |= p->payload[4] << 24;
        /* FHDR - FCnt */
        mote_fcnt = p->payload[6];
        mote_fcnt |= p->payload[7] << 8;

        /* basic packet filtering */
        meas_nb_rx_rcv += 1;
        switch (p->status)
        {
        case STAT_CRC_OK:
            meas_nb_rx_ok += 1;
#ifdef NS3_LOG_ENABLE
            {
                uint8_t buf_len = 100;
                char buf[buf_len];
                snprintf(buf,
                         buf_len,
                         "Received pkt from mote: %08X (fcnt=%u)",
                         mote_addr,
                         mote_fcnt);
                NS_LOG_INFO(buf);
            }
#endif // NS3_LOG_ENABLE
            if (!fwd_valid_pkt)
            {
                continue; /* skip that packet */
            }
            break;
        case STAT_CRC_BAD:
            meas_nb_rx_bad += 1;
            if (!fwd_error_pkt)
            {
                continue; /* skip that packet */
            }
            break;
        case STAT_NO_CRC:
            meas_nb_rx_nocrc += 1;
            if (!fwd_nocrc_pkt)
            {
                continue; /* skip that packet */
            }
            break;
        default:
            NS_LOG_WARN("[up] received packet with unknown status "
                        << (unsigned)p->status << " (size " << (unsigned)p->size << ", modulation "
                        << (unsigned)p->modulation << ", BW " << (unsigned)p->bandwidth << ", DR "
                        << (unsigned)p->datarate << ", RSSI " << p->rssi << ")");
            continue; /* skip that packet */
                      // exit(EXIT_FAILURE);
        }
        meas_up_pkt_fwd += 1;
        meas_up_payload_byte += p->size;

        /* Start of packet, add inter-packet separator if necessary */
        if (pkt_in_dgram == 0)
        {
            buff_up[buff_index] = '{';
            ++buff_index;
        }
        else
        {
            buff_up[buff_index] = ',';
            buff_up[buff_index + 1] = '{';
            buff_index += 2;
        }

        /* RAW timestamp, 8-17 useful chars */
        j = snprintf((char*)(buff_up + buff_index),
                     TX_BUFF_SIZE - buff_index,
                     "\"tmst\":%u",
                     p->count_us);
        if (j > 0)
        {
            buff_index += j;
        }
        else
        {
            NS_FATAL_ERROR("[up] snprintf failed line " << (unsigned)(__LINE__ - 4));
        }

        /* Packet concentrator channel, RF chain & RX frequency, 34-36 useful chars */
        j = snprintf((char*)(buff_up + buff_index),
                     TX_BUFF_SIZE - buff_index,
                     ",\"chan\":%1u,\"rfch\":%1u,\"freq\":%.6lf",
                     p->if_chain,
                     p->rf_chain,
                     ((double)p->freq_hz / 1e6));
        if (j > 0)
        {
            buff_index += j;
        }
        else
        {
            NS_FATAL_ERROR("[up] snprintf failed line " << (unsigned)(__LINE__ - 4));
        }

        /* Packet status, 9-10 useful chars */
        switch (p->status)
        {
        case STAT_CRC_OK:
            memcpy((void*)(buff_up + buff_index), (void*)",\"stat\":1", 9);
            buff_index += 9;
            break;
        case STAT_CRC_BAD:
            memcpy((void*)(buff_up + buff_index), (void*)",\"stat\":-1", 10);
            buff_index += 10;
            break;
        case STAT_NO_CRC:
            memcpy((void*)(buff_up + buff_index), (void*)",\"stat\":0", 9);
            buff_index += 9;
            break;
        default:
            memcpy((void*)(buff_up + buff_index), (void*)",\"stat\":?", 9);
            buff_index += 9;
            NS_FATAL_ERROR("[up] received packet with unknown status");
        }

        /* Packet modulation, 13-14 useful chars */
        if (p->modulation == MOD_LORA)
        {
            memcpy((void*)(buff_up + buff_index), (void*)",\"modu\":\"LORA\"", 14);
            buff_index += 14;

            /* Lora datarate & bandwidth, 16-19 useful chars */
            switch (p->datarate)
            {
            case DR_LORA_SF7:
                memcpy((void*)(buff_up + buff_index), (void*)",\"datr\":\"SF7", 12);
                buff_index += 12;
                break;
            case DR_LORA_SF8:
                memcpy((void*)(buff_up + buff_index), (void*)",\"datr\":\"SF8", 12);
                buff_index += 12;
                break;
            case DR_LORA_SF9:
                memcpy((void*)(buff_up + buff_index), (void*)",\"datr\":\"SF9", 12);
                buff_index += 12;
                break;
            case DR_LORA_SF10:
                memcpy((void*)(buff_up + buff_index), (void*)",\"datr\":\"SF10", 13);
                buff_index += 13;
                break;
            case DR_LORA_SF11:
                memcpy((void*)(buff_up + buff_index), (void*)",\"datr\":\"SF11", 13);
                buff_index += 13;
                break;
            case DR_LORA_SF12:
                memcpy((void*)(buff_up + buff_index), (void*)",\"datr\":\"SF12", 13);
                buff_index += 13;
                break;
            default:
                memcpy((void*)(buff_up + buff_index), (void*)",\"datr\":\"SF?", 12);
                buff_index += 12;
                NS_FATAL_ERROR("[up] lora packet with unknown datarate");
            }
            switch (p->bandwidth)
            {
            case BW_125KHZ:
                memcpy((void*)(buff_up + buff_index), (void*)"BW125\"", 6);
                buff_index += 6;
                break;
            case BW_250KHZ:
                memcpy((void*)(buff_up + buff_index), (void*)"BW250\"", 6);
                buff_index += 6;
                break;
            case BW_500KHZ:
                memcpy((void*)(buff_up + buff_index), (void*)"BW500\"", 6);
                buff_index += 6;
                break;
            default:
                memcpy((void*)(buff_up + buff_index), (void*)"BW?\"", 4);
                buff_index += 4;
                NS_FATAL_ERROR("[up] lora packet with unknown bandwidth");
            }

            /* Packet ECC coding rate, 11-13 useful chars */
            switch (p->coderate)
            {
            case CR_LORA_4_5:
                memcpy((void*)(buff_up + buff_index), (void*)",\"codr\":\"4/5\"", 13);
                buff_index += 13;
                break;
            case CR_LORA_4_6:
                memcpy((void*)(buff_up + buff_index), (void*)",\"codr\":\"4/6\"", 13);
                buff_index += 13;
                break;
            case CR_LORA_4_7:
                memcpy((void*)(buff_up + buff_index), (void*)",\"codr\":\"4/7\"", 13);
                buff_index += 13;
                break;
            case CR_LORA_4_8:
                memcpy((void*)(buff_up + buff_index), (void*)",\"codr\":\"4/8\"", 13);
                buff_index += 13;
                break;
            case 0: /* treat the CR0 case (mostly false sync) */
                memcpy((void*)(buff_up + buff_index), (void*)",\"codr\":\"OFF\"", 13);
                buff_index += 13;
                break;
            default:
                memcpy((void*)(buff_up + buff_index), (void*)",\"codr\":\"?\"", 11);
                buff_index += 11;
                NS_FATAL_ERROR("[up] lora packet with unknown coderate");
            }

            /* Lora SNR, 11-13 useful chars */
            j = snprintf((char*)(buff_up + buff_index),
                         TX_BUFF_SIZE - buff_index,
                         ",\"lsnr\":%.1f",
                         p->snr);
            if (j > 0)
            {
                buff_index += j;
            }
            else
            {
                NS_FATAL_ERROR("[up] snprintf failed line " << (unsigned)(__LINE__ - 4));
            }
        }
        else if (p->modulation == MOD_FSK)
        {
            memcpy((void*)(buff_up + buff_index), (void*)",\"modu\":\"FSK\"", 13);
            buff_index += 13;

            /* FSK datarate, 11-14 useful chars */
            j = snprintf((char*)(buff_up + buff_index),
                         TX_BUFF_SIZE - buff_index,
                         ",\"datr\":%u",
                         p->datarate);
            if (j > 0)
            {
                buff_index += j;
            }
            else
            {
                NS_FATAL_ERROR("[up] snprintf failed line " << (unsigned)(__LINE__ - 4));
            }
        }
        else
        {
            NS_FATAL_ERROR("[up] received packet with unknown modulation");
        }

        /* Packet RSSI, payload size, 18-23 useful chars */
        j = snprintf((char*)(buff_up + buff_index),
                     TX_BUFF_SIZE - buff_index,
                     ",\"rssi\":%.0f,\"size\":%u",
                     p->rssi,
                     p->size);
        if (j > 0)
        {
            buff_index += j;
        }
        else
        {
            NS_FATAL_ERROR("[up] snprintf failed line " << (unsigned)(__LINE__ - 4));
        }

        /* Packet base64-encoded payload, 14-350 useful chars */
        memcpy((void*)(buff_up + buff_index), (void*)",\"data\":\"", 9);
        buff_index += 9;
        j = bin_to_b64(p->payload,
                       p->size,
                       (char*)(buff_up + buff_index),
                       341); /* 255 bytes = 340 chars in b64 + null char */
        if (j >= 0)
        {
            buff_index += j;
        }
        else
        {
            NS_FATAL_ERROR("[up] bin_to_b64 failed line " << (unsigned)(__LINE__ - 5));
        }
        buff_up[buff_index] = '"';
        ++buff_index;

        /* End of packet serialization */
        buff_up[buff_index] = '}';
        ++buff_index;
        ++pkt_in_dgram;
    }

    /* restart fetch sequence without sending empty JSON if all packets have been filtered out */
    if (pkt_in_dgram == 0)
    {
        if (send_report == true)
        {
            /* need to clean up the beginning of the payload */
            buff_index -= 8; /* removes "rxpk":[ */
        }
        else
        {
            /* all packet have been filtered out and no report, restart loop */
            m_upEvent = Simulator::ScheduleNow(&UdpForwarder::ThreadUp, this);
            /* do not listen for acks in the meantime */
            m_remainingRecvAckAttempts = 0;
            return;
        }
    }
    else
    {
        /* end of packet array */
        buff_up[buff_index] = ']';
        ++buff_index;
        /* add separator if needed */
        if (send_report == true)
        {
            buff_up[buff_index] = ',';
            ++buff_index;
        }
    }

    /* add status report if a new one is available */
    if (send_report == true)
    {
        report_ready = false;
        j = snprintf((char*)(buff_up + buff_index), TX_BUFF_SIZE - buff_index, "%s", status_report);
        if (j > 0)
        {
            buff_index += j;
        }
        else
        {
            NS_FATAL_ERROR("[up] snprintf failed line " << (unsigned)(__LINE__ - 5));
        }
    }

    /* end of JSON datagram payload */
    buff_up[buff_index] = '}';
    ++buff_index;
    buff_up[buff_index] = 0; /* add string terminator, for safety */

    NS_LOG_DEBUG("JSON up: " << (char*)(buff_up + 12)); /* DEBUG: display JSON payload */

    /* send datagram to server */
    if (m_sockUp->Send(buff_up, buff_index, 0) >= 0)
    {
#ifdef NS3_LOG_ENABLE
        NS_LOG_INFO("UPLINK TX " << buff_index << " bytes to " << m_peerAddressString
                                 << " Time: " << (Simulator::Now()).As(Time::S));
#endif // NS3_LOG_ENABLE
    }
#ifdef NS3_LOG_ENABLE
    else
    {
        NS_LOG_INFO("Error while sending " << buff_index << " bytes to " << m_peerAddressString);
    }
#endif // NS3_LOG_ENABLE
    clock_gettime(CLOCK_MONOTONIC, &m_upSendTime);
    meas_up_dgram_sent += 1;
    meas_up_network_byte += buff_index;

    /* wait for acknowledge (in 2 times, to catch extra packets) */
    m_remainingRecvAckAttempts = 2;
    /* by default, act as if both ack recv timed out; re-scheduled sooner by ack recv */
    m_upEvent = Simulator::Schedule(MicroSeconds(push_timeout_half.tv_usec * 2),
                                    &UdpForwarder::ThreadUp,
                                    this);
}

void
UdpForwarder::ReceiveAck(Ptr<Socket> sockUp)
{
    if (!m_remainingRecvAckAttempts or !m_upEvent.IsRunning())
        return;

    int j;
    uint8_t buff_ack[32]; /* buffer to receive acknowledges */

    j = sockUp->Recv(buff_ack, sizeof buff_ack, 0);
    clock_gettime(CLOCK_MONOTONIC, &m_upRecvTime);
    if ((j < 4) || (buff_ack[0] != PROTOCOL_VERSION) || (buff_ack[3] != PKT_PUSH_ACK))
    {
        // MSG("WARNING: [up] ignored invalid non-ACL packet");
        m_remainingRecvAckAttempts--; /* continue; */
    }
    else if ((buff_ack[1] != m_upTokenH) || (buff_ack[2] != m_upTokenL))
    {
        // MSG("WARNING: [up] ignored out-of sync ACK packet");
        m_remainingRecvAckAttempts--; /* continue; */
    }
    else
    {
        NS_LOG_INFO("[up] PUSH_ACK received in "
                    << (int)(1000 * difftimespec(m_upRecvTime, m_upSendTime)) << " ms");
        meas_up_ack_rcv += 1;
        m_remainingRecvAckAttempts = 0; /* break; */
    }

    if (!m_remainingRecvAckAttempts)
    {
        Simulator::Cancel(m_upEvent);
        m_upEvent = Simulator::ScheduleNow(&UdpForwarder::ThreadUp, this);
    }
}

void
UdpForwarder::ThreadDown(void)
{
    /* data buffers */
    uint8_t buff_req[12]; /* buffer to compose pull requests */

    /* pre-fill the pull request buffer with fixed fields */
    buff_req[0] = PROTOCOL_VERSION;
    buff_req[3] = PKT_PULL_DATA;
    *(uint32_t*)(buff_req + 4) = net_mac_h;
    *(uint32_t*)(buff_req + 8) = net_mac_l;

    /* ORIGINAL LOOP START */

    /* auto-quit if the threshold is crossed */
    if ((autoquit_threshold > 0) && (m_autoquitCnt >= autoquit_threshold))
    {
        NS_FATAL_ERROR("[down] the last " << (unsigned)autoquit_threshold
                                          << " PULL_DATA were not ACKed, exiting application");
    }

    /* generate random token for request */
    m_downTokenH = (uint8_t)rand(); /* random token */
    m_downTokenL = (uint8_t)rand(); /* random token */
    buff_req[1] = m_downTokenH;
    buff_req[2] = m_downTokenL;

    /* send PULL request and record time */
    if (m_sockDown->Send(buff_req, sizeof buff_req, 0) >= 0)
    {
#ifdef NS3_LOG_ENABLE
        NS_LOG_INFO("PULL_REQ " << sizeof buff_req << " bytes to " << m_peerAddressString
                                << " Time: " << (Simulator::Now()).As(Time::S));
#endif // NS3_LOG_ENABLE
    }
#ifdef NS3_LOG_ENABLE
    else
    {
        NS_LOG_INFO("Error while sending " << sizeof buff_req << " bytes to "
                                           << m_peerAddressString);
    }
#endif // NS3_LOG_ENABLE
    clock_gettime(CLOCK_MONOTONIC, &m_downSendTime);
    meas_dw_pull_sent += 1;
    m_reqAck = false;
    m_autoquitCnt++;

    m_downRecvTime = m_downSendTime;

    /* listen to packets and process them until a new PULL request must be sent */
    CheckPullCondition();

    /* ThreadDown is called again by CheckPullCondition once condition for PULL req. is true */
}

void
UdpForwarder::CheckPullCondition(void)
{
    if ((int)difftimespec(m_downRecvTime, m_downSendTime) < keepalive_time)
    {
        /* emulate socket blocked by recv */
        Simulator::Cancel(m_downEvent);
        m_downEvent = Simulator::Schedule(MicroSeconds(pull_timeout.tv_usec),
                                          &UdpForwarder::SockDownTimeout,
                                          this);
    }
    else
    {
        /* too much time passed between last datagram recv and last PULL request: new pull request
         */
        ThreadDown();
    }
}

void
UdpForwarder::SockDownTimeout(void)
{
    clock_gettime(CLOCK_MONOTONIC, &m_downRecvTime);
    CheckPullCondition();
}

void
UdpForwarder::ReceiveDatagram(Ptr<Socket> sockDown)
{
    int i; /* loop variables */

    /* configuration and metadata for an outbound packet */
    struct lgw_pkt_tx_s txpkt;
    bool sent_immediate = false; /* option to sent the packet immediately */

    /* data buffers */
    uint8_t buff_down[1000]; /* buffer to receive downstream packets */
    int msg_len;

    /* JSON parsing variables */
    JSON_Value* root_val = NULL;
    JSON_Object* txpk_obj = NULL;
    JSON_Value* val = NULL; /* needed to detect the absence of some fields */
    const char* str;        /* pointer to sub-strings in the JSON data */
    short x0, x1;

    /* Just In Time downlink */
    struct timeval current_unix_time;
    struct timeval current_concentrator_time;
    enum jit_error_e jit_result = JIT_ERROR_OK;
    enum jit_pkt_type_e downlink_type;

    /* try to receive a datagram */
    msg_len = sockDown->Recv(buff_down, (sizeof buff_down) - 1, 0);
    clock_gettime(CLOCK_MONOTONIC, &m_downRecvTime);

    /* if no network message was received, got back to listening sock_down socket */
    if (msg_len == -1)
    {
        // MSG("WARNING: [down] recv returned %s", strerror(errno)); /* too verbose */
        return CheckPullCondition();
    }

    /* if the datagram does not respect protocol, just ignore it */
    if ((msg_len < 4) || (buff_down[0] != PROTOCOL_VERSION) ||
        ((buff_down[3] != PKT_PULL_RESP) && (buff_down[3] != PKT_PULL_ACK)))
    {
        NS_LOG_WARN("[down] ignoring invalid packet len=" << msg_len << ", protocol_version="
                                                          << (unsigned)buff_down[0]
                                                          << ", id=" << (unsigned)buff_down[3]);
        return CheckPullCondition();
    }

    /* if the datagram is an ACK, check token */
    if (buff_down[3] == PKT_PULL_ACK)
    {
        if ((buff_down[1] == m_downTokenH) && (buff_down[2] == m_downTokenL))
        {
            if (m_reqAck)
            {
                NS_LOG_INFO("[down] duplicate ACK received :)");
            }
            else
            { /* if that packet was not already acknowledged */
                m_reqAck = true;
                m_autoquitCnt = 0;
                meas_dw_ack_rcv += 1;
                NS_LOG_INFO("[down] PULL_ACK received in "
                            << (int)(1000 * difftimespec(m_downRecvTime, m_downSendTime)) << " ms");
            }
        }
        else
        { /* out-of-sync token */
            NS_LOG_INFO("[down] received out-of-sync ACK");
        }
        return CheckPullCondition();
    }

    /* the datagram is a PULL_RESP */
    buff_down[msg_len] = 0; /* add string terminator, just to be safe */
    NS_LOG_INFO("[down] PULL_RESP received  - token[" << (unsigned)buff_down[1] << ":"
                                                      << (unsigned)buff_down[2]
                                                      << "] :)"); /* very verbose */
    NS_LOG_DEBUG("JSON down: " << (char*)(buff_down + 4));        /* DEBUG: display JSON payload */

    /* initialize TX struct and try to parse JSON */
    memset(&txpkt, 0, sizeof txpkt);
    root_val = json_parse_string_with_comments((const char*)(buff_down + 4)); /* JSON offset */
    if (root_val == NULL)
    {
        NS_LOG_WARN("[down] invalid JSON, TX aborted");
        return CheckPullCondition();
    }

    /* look for JSON sub-object 'txpk' */
    txpk_obj = json_object_get_object(json_value_get_object(root_val), "txpk");
    if (txpk_obj == NULL)
    {
        NS_LOG_WARN("[down] no \"txpk\" object in JSON, TX aborted");
        json_value_free(root_val);
        return CheckPullCondition();
    }

    /* Parse "immediate" tag, or target timestamp, or UTC time to be converted by GPS (mandatory) */
    i = json_object_get_boolean(
        txpk_obj,
        "imme"); /* can be 1 if true, 0 if false, or -1 if not a JSON boolean */
    if (i == 1)
    {
        /* TX procedure: send immediately */
        sent_immediate = true;
        downlink_type = JIT_PKT_TYPE_DOWNLINK_CLASS_C;
        NS_LOG_INFO("[down] a packet will be sent in \"immediate\" mode");
    }
    else
    {
        sent_immediate = false;
        val = json_object_get_value(txpk_obj, "tmst");
        if (val != NULL)
        {
            /* TX procedure: send on timestamp value */
            txpkt.count_us = (uint32_t)json_value_get_number(val);

            /* Concentrator timestamp is given, we consider it is a Class A downlink */
            downlink_type = JIT_PKT_TYPE_DOWNLINK_CLASS_A;
        }
        else
        {
            /* TX procedure: send on GPS time (converted to timestamp value) */
            val = json_object_get_value(txpk_obj, "tmms");
            if (val == NULL)
            {
                NS_LOG_WARN("[down] no mandatory \"txpk.tmst\" or \"txpk.tmms\" objects in "
                            "JSON, TX aborted");
                json_value_free(root_val);
                return CheckPullCondition();
            }
            else
            {
                NS_LOG_WARN("[down] GPS disabled, impossible to send packet on specific GPS "
                            "time, TX aborted");
                json_value_free(root_val);

                /* send acknoledge datagram to server */
                send_tx_ack(buff_down[1], buff_down[2], JIT_ERROR_GPS_UNLOCKED);
                return CheckPullCondition();
            }
        }
    }

    /* Parse "No CRC" flag (optional field) */
    val = json_object_get_value(txpk_obj, "ncrc");
    if (val != NULL)
    {
        txpkt.no_crc = (bool)json_value_get_boolean(val);
    }

    /* parse target frequency (mandatory) */
    val = json_object_get_value(txpk_obj, "freq");
    if (val == NULL)
    {
        NS_LOG_WARN("[down] no mandatory \"txpk.freq\" object in JSON, TX aborted");
        json_value_free(root_val);
        return CheckPullCondition();
    }
    txpkt.freq_hz = (uint32_t)((double)(1.0e6) * json_value_get_number(val));

    /* parse RF chain used for TX (mandatory) */
    val = json_object_get_value(txpk_obj, "rfch");
    if (val == NULL)
    {
        NS_LOG_WARN("[down] no mandatory \"txpk.rfch\" object in JSON, TX aborted");
        json_value_free(root_val);
        return CheckPullCondition();
    }
    txpkt.rf_chain = (uint8_t)json_value_get_number(val);

    /* parse TX power (optional field) */
    val = json_object_get_value(txpk_obj, "powe");
    if (val != NULL)
    {
        txpkt.rf_power = (int8_t)json_value_get_number(val) - antenna_gain;
    }

    /* Parse modulation (mandatory) */
    str = json_object_get_string(txpk_obj, "modu");
    if (str == NULL)
    {
        NS_LOG_WARN("[down] no mandatory \"txpk.modu\" object in JSON, TX aborted");
        json_value_free(root_val);
        return CheckPullCondition();
    }
    if (strcmp(str, "LORA") == 0)
    {
        /* Lora modulation */
        txpkt.modulation = MOD_LORA;

        /* Parse Lora spreading-factor and modulation bandwidth (mandatory) */
        str = json_object_get_string(txpk_obj, "datr");
        if (str == NULL)
        {
            NS_LOG_WARN("[down] no mandatory \"txpk.datr\" object in JSON, TX aborted");
            json_value_free(root_val);
            return CheckPullCondition();
        }
        i = sscanf(str, "SF%2hdBW%3hd", &x0, &x1);
        if (i != 2)
        {
            NS_LOG_WARN("[down] format error in \"txpk.datr\", TX aborted");
            json_value_free(root_val);
            return CheckPullCondition();
        }
        switch (x0)
        {
        case 7:
            txpkt.datarate = DR_LORA_SF7;
            break;
        case 8:
            txpkt.datarate = DR_LORA_SF8;
            break;
        case 9:
            txpkt.datarate = DR_LORA_SF9;
            break;
        case 10:
            txpkt.datarate = DR_LORA_SF10;
            break;
        case 11:
            txpkt.datarate = DR_LORA_SF11;
            break;
        case 12:
            txpkt.datarate = DR_LORA_SF12;
            break;
        default:
            NS_LOG_WARN("[down] format error in \"txpk.datr\", invalid SF, TX aborted");
            json_value_free(root_val);
            return CheckPullCondition();
        }
        switch (x1)
        {
        case 125:
            txpkt.bandwidth = BW_125KHZ;
            break;
        case 250:
            txpkt.bandwidth = BW_250KHZ;
            break;
        case 500:
            txpkt.bandwidth = BW_500KHZ;
            break;
        default:
            NS_LOG_WARN("[down] format error in \"txpk.datr\", invalid BW, TX aborted");
            json_value_free(root_val);
            return CheckPullCondition();
        }

        /* Parse ECC coding rate (optional field) */
        str = json_object_get_string(txpk_obj, "codr");
        if (str == NULL)
        {
            NS_LOG_WARN("[down] no mandatory \"txpk.codr\" object in json, TX aborted");
            json_value_free(root_val);
            return CheckPullCondition();
        }
        if (strcmp(str, "4/5") == 0)
            txpkt.coderate = CR_LORA_4_5;
        else if (strcmp(str, "4/6") == 0)
            txpkt.coderate = CR_LORA_4_6;
        else if (strcmp(str, "2/3") == 0)
            txpkt.coderate = CR_LORA_4_6;
        else if (strcmp(str, "4/7") == 0)
            txpkt.coderate = CR_LORA_4_7;
        else if (strcmp(str, "4/8") == 0)
            txpkt.coderate = CR_LORA_4_8;
        else if (strcmp(str, "1/2") == 0)
            txpkt.coderate = CR_LORA_4_8;
        else
        {
            NS_LOG_WARN("[down] format error in \"txpk.codr\", TX aborted");
            json_value_free(root_val);
            return CheckPullCondition();
        }

        /* Parse signal polarity switch (optional field) */
        val = json_object_get_value(txpk_obj, "ipol");
        if (val != NULL)
        {
            txpkt.invert_pol = (bool)json_value_get_boolean(val);
        }

        /* parse Lora preamble length (optional field, optimum min value enforced) */
        val = json_object_get_value(txpk_obj, "prea");
        if (val != NULL)
        {
            i = (int)json_value_get_number(val);
            if (i >= MIN_LORA_PREAMB)
            {
                txpkt.preamble = (uint16_t)i;
            }
            else
            {
                txpkt.preamble = (uint16_t)MIN_LORA_PREAMB;
            }
        }
        else
        {
            txpkt.preamble = (uint16_t)STD_LORA_PREAMB;
        }
    }
    else if (strcmp(str, "FSK") == 0)
    {
        /* FSK modulation */
        txpkt.modulation = MOD_FSK;

        /* parse FSK bitrate (mandatory) */
        val = json_object_get_value(txpk_obj, "datr");
        if (val == NULL)
        {
            NS_LOG_WARN("[down] no mandatory \"txpk.datr\" object in JSON, TX aborted");
            json_value_free(root_val);
            return CheckPullCondition();
        }
        txpkt.datarate = (uint32_t)(json_value_get_number(val));

        /* parse frequency deviation (mandatory) */
        val = json_object_get_value(txpk_obj, "fdev");
        if (val == NULL)
        {
            NS_LOG_WARN("[down] no mandatory \"txpk.fdev\" object in JSON, TX aborted");
            json_value_free(root_val);
            return CheckPullCondition();
        }
        txpkt.f_dev = (uint8_t)(json_value_get_number(val) /
                                1000.0); /* JSON value in Hz, txpkt.f_dev in kHz */

        /* parse FSK preamble length (optional field, optimum min value enforced) */
        val = json_object_get_value(txpk_obj, "prea");
        if (val != NULL)
        {
            i = (int)json_value_get_number(val);
            if (i >= MIN_FSK_PREAMB)
            {
                txpkt.preamble = (uint16_t)i;
            }
            else
            {
                txpkt.preamble = (uint16_t)MIN_FSK_PREAMB;
            }
        }
        else
        {
            txpkt.preamble = (uint16_t)STD_FSK_PREAMB;
        }
    }
    else
    {
        NS_LOG_WARN("[down] invalid modulation in \"txpk.modu\", TX aborted");
        json_value_free(root_val);
        return CheckPullCondition();
    }

    /* Parse payload length (mandatory) */
    val = json_object_get_value(txpk_obj, "size");
    if (val == NULL)
    {
        NS_LOG_WARN("[down] no mandatory \"txpk.size\" object in JSON, TX aborted");
        json_value_free(root_val);
        return CheckPullCondition();
    }
    txpkt.size = (uint16_t)json_value_get_number(val);

    /* Parse payload data (mandatory) */
    str = json_object_get_string(txpk_obj, "data");
    if (str == NULL)
    {
        NS_LOG_WARN("[down] no mandatory \"txpk.data\" object in JSON, TX aborted");
        json_value_free(root_val);
        return CheckPullCondition();
    }
    i = b64_to_bin(str, strlen(str), txpkt.payload, sizeof txpkt.payload);
    if (i != txpkt.size)
    {
        NS_LOG_WARN("[down] mismatch between .size and .data size once converter to binary");
    }

    /* free the JSON parse tree from memory */
    json_value_free(root_val);

    /* select TX mode */
    if (sent_immediate)
    {
        txpkt.tx_mode = IMMEDIATE;
    }
    else
    {
        txpkt.tx_mode = TIMESTAMPED;
    }

    /* record measurement data */
    meas_dw_dgram_rcv += 1;          /* count only datagrams with no JSON errors */
    meas_dw_network_byte += msg_len; /* meas_dw_network_byte */
    meas_dw_payload_byte += txpkt.size;

    /* check TX parameter before trying to queue packet */
    jit_result = JIT_ERROR_OK;
    if ((txpkt.freq_hz < tx_freq_min[txpkt.rf_chain]) ||
        (txpkt.freq_hz > tx_freq_max[txpkt.rf_chain]))
    {
        jit_result = JIT_ERROR_TX_FREQ;
        NS_LOG_ERROR("Packet REJECTED, unsupported frequency - "
                     << (unsigned)txpkt.freq_hz << " (min:" << (unsigned)tx_freq_min[txpkt.rf_chain]
                     << ",max:" << (unsigned)tx_freq_max[txpkt.rf_chain] << ")");
    }
    if (jit_result == JIT_ERROR_OK)
    {
        for (i = 0; i < txlut.size; i++)
        {
            if (txlut.lut[i].rf_power == txpkt.rf_power)
            {
                /* this RF power is supported, we can continue */
                break;
            }
        }
        if (i == txlut.size)
        {
            /* this RF power is not supported */
            jit_result = JIT_ERROR_TX_POWER;
            NS_LOG_ERROR("Packet REJECTED, unsupported RF power for TX - "
                         << (unsigned)txpkt.rf_power);
        }
    }

    /* insert packet to be sent into JIT queue */
    if (jit_result == JIT_ERROR_OK)
    {
        gettimeofday(&current_unix_time, NULL);
        get_concentrator_time(&current_concentrator_time, current_unix_time);
        jit_result = jit_enqueue(&jit_queue, &current_concentrator_time, &txpkt, downlink_type);
        if (jit_result != JIT_ERROR_OK)
        {
            NS_LOG_ERROR("Packet REJECTED (jit error=" << jit_result << ")");
        }
        meas_nb_tx_requested += 1;
    }

    /* Send acknoledge datagram to server */
    send_tx_ack(buff_down[1], buff_down[2], jit_result);

    CheckPullCondition();
}

void
UdpForwarder::ThreadJit(void)
{
    int result = LGW_HAL_SUCCESS;
    struct lgw_pkt_tx_s pkt;
    int pkt_index = -1;
    struct timeval current_unix_time;
    struct timeval current_concentrator_time;
    enum jit_error_e jit_result;
    enum jit_pkt_type_e pkt_type;
    uint8_t tx_status;

    /* transfer data and metadata to the concentrator, and schedule TX */
    gettimeofday(&current_unix_time, NULL);
    get_concentrator_time(&current_concentrator_time, current_unix_time);
    jit_result = jit_peek(&jit_queue, &current_concentrator_time, &pkt_index);
    if (jit_result == JIT_ERROR_OK)
    {
        if (pkt_index > -1)
        {
            jit_result = jit_dequeue(&jit_queue, pkt_index, &pkt, &pkt_type);
            if (jit_result == JIT_ERROR_OK)
            {
                /* check if concentrator is free for sending new packet */
                result = LgwStatus(TX_STATUS, &tx_status);
                if (result == LGW_HAL_ERROR)
                {
                    NS_LOG_WARN("[jit] lgw_status failed");
                }
                else
                {
                    if (tx_status == TX_EMITTING)
                    {
                        NS_LOG_ERROR("concentrator is currently emitting");
                        print_tx_status(tx_status);
                        m_jitEvent =
                            Simulator::Schedule(MilliSeconds(10), &UdpForwarder::ThreadJit, this);
                        return;
                    }
                    else if (tx_status == TX_SCHEDULED)
                    {
                        NS_LOG_WARN("a downlink was already scheduled, overwriting it...");
                        print_tx_status(tx_status);
                    }
                    else
                    {
                        /* Nothing to do */
                    }
                }

                /* send packet to concentrator */
                result = LgwSend(pkt);
                if (result == LGW_HAL_ERROR)
                {
                    meas_nb_tx_fail += 1;
                    NS_LOG_WARN("[jit] lgw_send failed");
                    m_jitEvent =
                        Simulator::Schedule(MilliSeconds(10), &UdpForwarder::ThreadJit, this);
                    return;
                }
                else
                {
                    meas_nb_tx_ok += 1;
                    NS_LOG_DEBUG("lgw_send done: count_us=" << (unsigned)pkt.count_us);
                }
            }
            else
            {
                NS_LOG_ERROR("jit_dequeue failed with " << jit_result);
            }
        }
    }
    else if (jit_result == JIT_ERROR_EMPTY)
    {
        /* Do nothing, it can happen */
    }
    else
    {
        NS_LOG_ERROR("jit_peek failed with " << jit_result);
    }

    m_jitEvent = Simulator::Schedule(MilliSeconds(10), &UdpForwarder::ThreadJit, this);
}

void
UdpForwarder::CollectStatistics(void)
{
    /* statistics variable */
    time_t t;
    char stat_timestamp[24];
    float rx_ok_ratio;
    float rx_bad_ratio;
    float rx_nocrc_ratio;
    float up_ack_ratio;
    float dw_ack_ratio;

    /* get timestamp for statistics */
    t = time(NULL);
    strftime(stat_timestamp, sizeof stat_timestamp, "%F %T %Z", gmtime(&t));

    /* aggregate upstream statistics */
    if (meas_nb_rx_rcv > 0)
    {
        rx_ok_ratio = (float)meas_nb_rx_ok / (float)meas_nb_rx_rcv;
        rx_bad_ratio = (float)meas_nb_rx_bad / (float)meas_nb_rx_rcv;
        rx_nocrc_ratio = (float)meas_nb_rx_nocrc / (float)meas_nb_rx_rcv;
    }
    else
    {
        rx_ok_ratio = 0.0;
        rx_bad_ratio = 0.0;
        rx_nocrc_ratio = 0.0;
    }
    if (meas_up_dgram_sent > 0)
    {
        up_ack_ratio = (float)meas_up_ack_rcv / (float)meas_up_dgram_sent;
    }
    else
    {
        up_ack_ratio = 0.0;
    }

    /* aggregate downstream statistics */
    if (meas_dw_pull_sent > 0)
    {
        dw_ack_ratio = (float)meas_dw_ack_rcv / (float)meas_dw_pull_sent;
    }
    else
    {
        dw_ack_ratio = 0.0;
    }

    /* display a report */
#ifdef NS3_LOG_ENABLE
    std::stringstream ss;
    char buf[120];
    snprintf(buf, 120, "\n\n##### %s #####\n", stat_timestamp);
    ss << buf;
    snprintf(buf, 120, "### [UPSTREAM] ###\n");
    ss << buf;
    snprintf(buf, 120, "# RF packets received by concentrator: %u\n", meas_nb_rx_rcv);
    snprintf(buf,
             120,
             "# CRC_OK: %.2f%%, CRC_FAIL: %.2f%%, NO_CRC: %.2f%%\n",
             100.0 * rx_ok_ratio,
             100.0 * rx_bad_ratio,
             100.0 * rx_nocrc_ratio);
    ss << buf;
    snprintf(buf,
             120,
             "# RF packets forwarded: %u (%u bytes)\n",
             meas_up_pkt_fwd,
             meas_up_payload_byte);
    ss << buf;
    snprintf(buf,
             120,
             "# PUSH_DATA datagrams sent: %u (%u bytes)\n",
             meas_up_dgram_sent,
             meas_up_network_byte);
    ss << buf;
    snprintf(buf, 120, "# PUSH_DATA acknowledged: %.2f%%\n", 100.0 * up_ack_ratio);
    ss << buf;
    snprintf(buf, 120, "### [DOWNSTREAM] ###\n");
    ss << buf;
    snprintf(buf,
             120,
             "# PULL_DATA sent: %u (%.2f%% acknowledged)\n",
             meas_dw_pull_sent,
             100.0 * dw_ack_ratio);
    ss << buf;
    snprintf(buf,
             120,
             "# PULL_RESP(onse) datagrams received: %u (%u bytes)\n",
             meas_dw_dgram_rcv,
             meas_dw_network_byte);
    ss << buf;
    snprintf(buf,
             120,
             "# RF packets sent to concentrator: %u (%u bytes)\n",
             (meas_nb_tx_ok + meas_nb_tx_fail),
             meas_dw_payload_byte);
    ss << buf;
    snprintf(buf, 120, "# TX errors: %u\n", meas_nb_tx_fail);
    ss << buf;
    if (meas_nb_tx_requested != 0)
    {
        snprintf(buf,
                 120,
                 "# TX rejected (collision packet): %.2f%% (req:%u, rej:%u)\n",
                 100.0 * meas_nb_tx_rejected_collision_packet / meas_nb_tx_requested,
                 meas_nb_tx_requested,
                 meas_nb_tx_rejected_collision_packet);
        ss << buf;
        snprintf(buf,
                 120,
                 "# TX rejected (collision beacon): %.2f%% (req:%u, rej:%u)\n",
                 100.0 * meas_nb_tx_rejected_collision_beacon / meas_nb_tx_requested,
                 meas_nb_tx_requested,
                 meas_nb_tx_rejected_collision_beacon);
        ss << buf;
        snprintf(buf,
                 120,
                 "# TX rejected (too late): %.2f%% (req:%u, rej:%u)\n",
                 100.0 * meas_nb_tx_rejected_too_late / meas_nb_tx_requested,
                 meas_nb_tx_requested,
                 meas_nb_tx_rejected_too_late);
        ss << buf;
        snprintf(buf,
                 120,
                 "# TX rejected (too early): %.2f%% (req:%u, rej:%u)\n",
                 100.0 * meas_nb_tx_rejected_too_early / meas_nb_tx_requested,
                 meas_nb_tx_requested,
                 meas_nb_tx_rejected_too_early);
        ss << buf;
    }
    snprintf(buf, 120, "# BEACON queued: %u\n", meas_nb_beacon_queued);
    ss << buf;
    snprintf(buf, 120, "# BEACON sent so far: %u\n", meas_nb_beacon_sent);
    ss << buf;
    snprintf(buf, 120, "# BEACON rejected: %u\n", meas_nb_beacon_rejected);
    ss << buf;
    snprintf(buf, 120, "### [JIT] ###\n");
    ss << buf;
    /* get timestamp captured on PPM pulse  */
    snprintf(buf, 120, "# SX1301 time (PPS): unknown\n");
    ss << buf;
    ss << jit_get_print_queue(&jit_queue, false, DEBUG_LOG);
    snprintf(buf, 120, "### [GPS] ###\n");
    ss << buf;
    if (gps_fake_enable == true)
    {
        snprintf(buf,
                 120,
                 "# GPS *FAKE* coordinates: latitude %.5f, longitude %.5f, altitude %i m\n",
                 reference_coord.lat,
                 reference_coord.lon,
                 reference_coord.alt);
        ss << buf;
    }
    else
    {
        snprintf(buf, 120, "# GPS sync is disabled\n");
        ss << buf;
    }
    snprintf(buf, 120, "##### END #####\n");
    ss << buf;
    NS_LOG_INFO(ss.str());
#endif // NS3_LOG_ENABLE

    /* generate a JSON report (will be sent to server by upstream thread) */
    if (gps_fake_enable == true)
    {
        snprintf(status_report,
                 STATUS_SIZE,
                 "\"stat\":{\"time\":\"%s\",\"lati\":%.5f,\"long\":%.5f,\"alti\":%i,\"rxnb\":%u,"
                 "\"rxok\":%u,\"rxfw\":%u,\"ackr\":%.1f,\"dwnb\":%u,\"txnb\":%u}",
                 stat_timestamp,
                 reference_coord.lat,
                 reference_coord.lon,
                 reference_coord.alt,
                 meas_nb_rx_rcv,
                 meas_nb_rx_ok,
                 meas_up_pkt_fwd,
                 100.0 * up_ack_ratio,
                 meas_dw_dgram_rcv,
                 meas_nb_tx_ok);
    }
    else
    {
        snprintf(status_report,
                 STATUS_SIZE,
                 "\"stat\":{\"time\":\"%s\",\"rxnb\":%u,\"rxok\":%u,\"rxfw\":%u,\"ackr\":%.1f,"
                 "\"dwnb\":%u,\"txnb\":%u}",
                 stat_timestamp,
                 meas_nb_rx_rcv,
                 meas_nb_rx_ok,
                 meas_up_pkt_fwd,
                 100.0 * up_ack_ratio,
                 meas_dw_dgram_rcv,
                 meas_nb_tx_ok);
    }
    report_ready = true;

    /* reset upstream statistics variables */
    meas_nb_rx_rcv = 0;
    meas_nb_rx_ok = 0;
    meas_nb_rx_bad = 0;
    meas_nb_rx_nocrc = 0;
    meas_up_pkt_fwd = 0;
    meas_up_network_byte = 0;
    meas_up_payload_byte = 0;
    meas_up_dgram_sent = 0;
    meas_up_ack_rcv = 0;

    /* reset downstream statistics variables */
    meas_dw_pull_sent = 0;
    meas_dw_ack_rcv = 0;
    meas_dw_dgram_rcv = 0;
    meas_dw_network_byte = 0;
    meas_dw_payload_byte = 0;
    meas_nb_tx_ok = 0;
    meas_nb_tx_fail = 0;

    /* wait for next reporting interval */
    m_statsEvent = Simulator::Schedule(MilliSeconds(1000 * stat_interval),
                                       &UdpForwarder::CollectStatistics,
                                       this);
}

int
UdpForwarder::LgwReceive(int nb_pkt_max, lgw_pkt_rx_s rxpkt[])
{
    int nb = m_rxPktBuff.size();
    int i = 0;
    for (; i < nb_pkt_max and i < nb; ++i)
    {
        rxpkt[i] = m_rxPktBuff.front();
        m_rxPktBuff.pop();
    }
    return i;
}

int
UdpForwarder::LgwStatus(uint8_t select, uint8_t* code)
{
    bool lgw_is_started = true;
    Ptr<GatewayLorawanMac> mac = m_loraNetDevice->GetMac()->GetObject<GatewayLorawanMac>();
    int32_t read_value;

    /* check input variables */
    CHECK_NULL(code);

    if (select == TX_STATUS)
    {
        read_value = (mac->IsTransmitting()) ? 0x70 : 0x0;
        if (lgw_is_started == false)
        {
            *code = TX_OFF;
        }
        else if ((read_value & 0x10) == 0)
        { /* bit 4 @1: TX programmed */
            *code = TX_FREE;
        }
        else if ((read_value & 0x60) != 0)
        { /* bit 5 or 6 @1: TX sequence */
            *code = TX_EMITTING;
        }
        else
        {
            *code = TX_SCHEDULED;
        }
        return LGW_HAL_SUCCESS;
    }
    else if (select == RX_STATUS)
    {
        *code = RX_STATUS_UNKNOWN; /* todo */
        return LGW_HAL_SUCCESS;
    }
    else
    {
        NS_LOG_ERROR("SELECTION INVALID, NO STATUS TO RETURN");
        return LGW_HAL_ERROR;
    }
}

int
UdpForwarder::LgwSend(struct lgw_pkt_tx_s pkt_data)
{
    bool lgw_is_started = true;
    LoraTag tag;
    Ptr<Packet> pkt;
    Ptr<GatewayLorawanMac> mac = m_loraNetDevice->GetMac()->GetObject<GatewayLorawanMac>();
    bool tx_allowed = false;

    /* check if the concentrator is running */
    if (lgw_is_started == false)
    {
        NS_LOG_ERROR("CONCENTRATOR IS NOT RUNNING, START IT BEFORE SENDING");
        return LGW_HAL_ERROR;
    }

    /* check input range (segfault prevention) */
    if (pkt_data.rf_chain >= LGW_RF_CHAIN_NB)
    {
        NS_LOG_ERROR("INVALID RF_CHAIN TO SEND PACKETS");
        return LGW_HAL_ERROR;
    }

    /* check input variables */
    if (false) // rf_tx_enable[pkt_data.rf_chain] == false)
    {
        NS_LOG_ERROR("SELECTED RF_CHAIN IS DISABLED FOR TX ON SELECTED BOARD");
        return LGW_HAL_ERROR;
    }
    if (false) // rf_enable[pkt_data.rf_chain] == false)
    {
        NS_LOG_ERROR("SELECTED RF_CHAIN IS DISABLED");
        return LGW_HAL_ERROR;
    }
    if (!IS_TX_MODE(pkt_data.tx_mode))
    {
        NS_LOG_ERROR("TX_MODE NOT SUPPORTED");
        return LGW_HAL_ERROR;
    }
    if (pkt_data.modulation == MOD_LORA)
    {
        if (!IS_LORA_BW(pkt_data.bandwidth))
        {
            NS_LOG_ERROR("BANDWIDTH NOT SUPPORTED BY LORA TX");
            return LGW_HAL_ERROR;
        }
        if (!IS_LORA_STD_DR(pkt_data.datarate))
        {
            NS_LOG_ERROR("DATARATE NOT SUPPORTED BY LORA TX");
            return LGW_HAL_ERROR;
        }
        if (!IS_LORA_CR(pkt_data.coderate))
        {
            NS_LOG_ERROR("CODERATE NOT SUPPORTED BY LORA TX");
            return LGW_HAL_ERROR;
        }
        if (pkt_data.size > 255)
        {
            NS_LOG_ERROR("PAYLOAD LENGTH TOO BIG FOR LORA TX");
            return LGW_HAL_ERROR;
        }
    }
    else if (pkt_data.modulation == MOD_FSK)
    {
        if ((pkt_data.f_dev < 1) || (pkt_data.f_dev > 200))
        {
            NS_LOG_ERROR("TX FREQUENCY DEVIATION OUT OF ACCEPTABLE RANGE");
            return LGW_HAL_ERROR;
        }
        if (!IS_FSK_DR(pkt_data.datarate))
        {
            NS_LOG_ERROR("DATARATE NOT SUPPORTED BY FSK IF CHAIN");
            return LGW_HAL_ERROR;
        }
        if (pkt_data.size > 255)
        {
            NS_LOG_ERROR("PAYLOAD LENGTH TOO BIG FOR FSK TX");
            return LGW_HAL_ERROR;
        }
    }
    else
    {
        NS_LOG_ERROR("INVALID TX MODULATION");
        return LGW_HAL_ERROR;
    }

    switch (pkt_data.datarate)
    {
    case DR_LORA_SF7:
        tag.SetSpreadingFactor(7);
        break;
    case DR_LORA_SF8:
        tag.SetSpreadingFactor(8);
        break;
    case DR_LORA_SF9:
        tag.SetSpreadingFactor(9);
        break;
    case DR_LORA_SF10:
        tag.SetSpreadingFactor(10);
        break;
    case DR_LORA_SF11:
        tag.SetSpreadingFactor(11);
        break;
    case DR_LORA_SF12:
        tag.SetSpreadingFactor(12);
        break;
    default:
        tag.SetSpreadingFactor(12);
        break;
    }
    tag.SetFrequency((double)pkt_data.freq_hz / 1e6);

    pkt = Create<Packet>(pkt_data.payload, pkt_data.size);
    pkt->AddPacketTag(tag);

    /* We assume LBT is disabled: same outcome, more interference downlink */
    tx_allowed = true;
    if (tx_allowed == true)
    {
        mac->Send(pkt);
    }
    else
    {
        NS_LOG_ERROR("Cannot send packet, channel is busy (LBT)");
        return LGW_LBT_ISSUE;
    }

    return LGW_HAL_SUCCESS;
}

double
UdpForwarder::difftimespec(struct timespec end, struct timespec beginning)
{
    double x;

    x = 1E-9 * (double)(end.tv_nsec - beginning.tv_nsec);
    x += (double)(end.tv_sec - beginning.tv_sec);

    return x;
}

int
UdpForwarder::send_tx_ack(uint8_t token_h, uint8_t token_l, enum jit_error_e error)
{
    uint8_t buff_ack[64]; /* buffer to give feedback to server */
    int buff_index;

    /* reset buffer */
    memset(&buff_ack, 0, sizeof buff_ack);

    /* Prepare downlink feedback to be sent to server */
    buff_ack[0] = PROTOCOL_VERSION;
    buff_ack[1] = token_h;
    buff_ack[2] = token_l;
    buff_ack[3] = PKT_TX_ACK;
    *(uint32_t*)(buff_ack + 4) = net_mac_h;
    *(uint32_t*)(buff_ack + 8) = net_mac_l;
    buff_index = 12; /* 12-byte header */

    /* Put no JSON string if there is nothing to report */
    if (error != JIT_ERROR_OK)
    {
        /* start of JSON structure */
        memcpy((void*)(buff_ack + buff_index), (void*)"{\"txpk_ack\":{", 13);
        buff_index += 13;
        /* set downlink error status in JSON structure */
        memcpy((void*)(buff_ack + buff_index), (void*)"\"error\":", 8);
        buff_index += 8;
        switch (error)
        {
        case JIT_ERROR_FULL:
        case JIT_ERROR_COLLISION_PACKET:
            memcpy((void*)(buff_ack + buff_index), (void*)"\"COLLISION_PACKET\"", 18);
            buff_index += 18;
            /* update stats */
            meas_nb_tx_rejected_collision_packet += 1;
            break;
        case JIT_ERROR_TOO_LATE:
            memcpy((void*)(buff_ack + buff_index), (void*)"\"TOO_LATE\"", 10);
            buff_index += 10;
            /* update stats */
            meas_nb_tx_rejected_too_late += 1;
            break;
        case JIT_ERROR_TOO_EARLY:
            memcpy((void*)(buff_ack + buff_index), (void*)"\"TOO_EARLY\"", 11);
            buff_index += 11;
            /* update stats */
            meas_nb_tx_rejected_too_early += 1;
            break;
        case JIT_ERROR_COLLISION_BEACON:
            memcpy((void*)(buff_ack + buff_index), (void*)"\"COLLISION_BEACON\"", 18);
            buff_index += 18;
            /* update stats */
            meas_nb_tx_rejected_collision_beacon += 1;
            break;
        case JIT_ERROR_TX_FREQ:
            memcpy((void*)(buff_ack + buff_index), (void*)"\"TX_FREQ\"", 9);
            buff_index += 9;
            break;
        case JIT_ERROR_TX_POWER:
            memcpy((void*)(buff_ack + buff_index), (void*)"\"TX_POWER\"", 10);
            buff_index += 10;
            break;
        case JIT_ERROR_GPS_UNLOCKED:
            memcpy((void*)(buff_ack + buff_index), (void*)"\"GPS_UNLOCKED\"", 14);
            buff_index += 14;
            break;
        default:
            memcpy((void*)(buff_ack + buff_index), (void*)"\"UNKNOWN\"", 9);
            buff_index += 9;
            break;
        }
        /* end of JSON structure */
        memcpy((void*)(buff_ack + buff_index), (void*)"}}", 2);
        buff_index += 2;
    }

    buff_ack[buff_index] = 0; /* add string terminator, for safety */

    /* send datagram to server */
    int size = m_sockDown->Send(buff_ack, buff_index, 0);
    if (size >= 0)
    {
#ifdef NS3_LOG_ENABLE
        NS_LOG_INFO("Ack UP " << buff_index << " bytes to " << m_peerAddressString
                              << " Time: " << (Simulator::Now()).As(Time::S));
#endif // NS3_LOG_ENABLE
    }
#ifdef NS3_LOG_ENABLE
    else
    {
        NS_LOG_INFO("Error while sending " << buff_index << " bytes to " << m_peerAddressString);
    }
#endif // NS3_LOG_ENABLE
    return size;
}

void
UdpForwarder::print_tx_status(uint8_t tx_status)
{
    switch (tx_status)
    {
    case TX_OFF:
        NS_LOG_INFO("[jit] lgw_status returned TX_OFF");
        break;
    case TX_FREE:
        NS_LOG_INFO("[jit] lgw_status returned TX_FREE");
        break;
    case TX_EMITTING:
        NS_LOG_INFO("[jit] lgw_status returned TX_EMITTING");
        break;
    case TX_SCHEDULED:
        NS_LOG_INFO("[jit] lgw_status returned TX_SCHEDULED");
        break;
    default:
        NS_LOG_INFO("[jit] lgw_status returned UNKNOWN (" << tx_status << ")");
        break;
    }
}

} // namespace lorawan
} // namespace ns3
