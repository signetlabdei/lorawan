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
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/mac64-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/uinteger.h"
#include "ns3/seq-ts-header.h"

#include "udp-forwarder.h"
#include "ns3/lora-tag.h"

#include <cstdlib>
#include <cstdio>
#include <arpa/inet.h>
#include "ns3/base64.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("UdpForwarder");

NS_OBJECT_ENSURE_REGISTERED (UdpForwarder);

TypeId
UdpForwarder::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::UdpForwarder")
          .SetParent<Application> ()
          .SetGroupName ("Applications")
          .AddConstructor<UdpForwarder> ()
          .AddAttribute ("RemoteAddress", "The destination Address of the outbound packets",
                         AddressValue (), MakeAddressAccessor (&UdpForwarder::m_peerAddress),
                         MakeAddressChecker ())
          .AddAttribute ("RemotePort", "The destination port of the outbound packets",
                         UintegerValue (100), MakeUintegerAccessor (&UdpForwarder::m_peerPort),
                         MakeUintegerChecker<uint16_t> ());
  return tid;
}

UdpForwarder::UdpForwarder ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_totalTx = 0;
  m_sockUp = 0;
  m_forwardEvent = EventId ();
  lgwm = 0;
  meas_up_network_byte = 0;
  meas_up_dgram_sent = 0;
  meas_up_ack_rcv = 0;
}

UdpForwarder::~UdpForwarder ()
{
  NS_LOG_FUNCTION (this);
}

void
UdpForwarder::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
UdpForwarder::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void
UdpForwarder::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

uint64_t
UdpForwarder::GetTotalTx () const
{
  return m_totalTx;
}

bool
UdpForwarder::ReceiveFromLora (Ptr<NetDevice> loraNetDevice, Ptr<const Packet> packet,
                               uint16_t protocol, const Address &sender)
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> pktcpy = packet->Copy ();
  LoraTag tag;
  pktcpy->RemovePacketTag (tag);

  lgw_pkt_rx_s p;
  p.freq_hz = tag.GetFrequency () * 1e6;
  p.if_chain = 0;
  p.status = STAT_CRC_OK;
  p.count_us = tag.GetReceptionTime ().GetMicroSeconds ();
  p.rf_chain = 0;
  p.modulation = MOD_LORA;
  p.bandwidth = BW_125KHZ;
  switch (tag.GetSpreadingFactor ())
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
  p.rssi = tag.GetReceivePower ();
  p.snr = tag.GetSnr ();
  p.snr_min = tag.GetSnr ();
  p.snr_max = tag.GetSnr ();
  p.crc = 0; // todo?
  p.size = pktcpy->GetSize ();
  pktcpy->CopyData (p.payload, 256);

  m_rxpktbuff.push (p);
  return true;
}

bool
UdpForwarder::ReceiveFromCsma (Ptr<NetDevice> csmaNetDevice, Ptr<const Packet> packet,
                               uint16_t protocol, const Address &sender)
{
  NS_LOG_FUNCTION (this << packet << protocol << sender);
  return false;
}

// This will act as the main of the protocol
void
UdpForwarder::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  uint8_t addr[8];
  Mac64Address::Allocate ().CopyTo (addr);
  memcpy (&lgwm, addr, sizeof lgwm);

  /* process some of the configuration variables */
  net_mac_h = htonl ((uint32_t) (0xFFFFFFFF & (lgwm >> 32)));
  net_mac_l = htonl ((uint32_t) (0xFFFFFFFF & lgwm));

  if (m_sockUp == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_sockUp = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType (m_peerAddress) == true)
        {
          if (m_sockUp->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_sockUp->Connect (
              InetSocketAddress (Ipv4Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType (m_peerAddress) == true)
        {
          if (m_sockUp->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_sockUp->Connect (
              Inet6SocketAddress (Ipv6Address::ConvertFrom (m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_sockUp->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_sockUp->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_sockUp->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_sockUp->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }
    }

#ifdef NS3_LOG_ENABLE
  std::stringstream peerAddressStringStream;
  if (Ipv4Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv4Address::ConvertFrom (m_peerAddress);
    }
  else if (Ipv6Address::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Ipv6Address::ConvertFrom (m_peerAddress);
    }
  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 ();
    }
  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
    {
      peerAddressStringStream << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 ();
    }
  m_peerAddressString = peerAddressStringStream.str ();
#endif // NS3_LOG_ENABLE

  m_sockUp->SetRecvCallback (MakeNullCallback<void, Ptr<Socket>> ());
  m_sockUp->SetAllowBroadcast (true);

  // Start threadUp loop
  m_forwardEvent = Simulator::Schedule (Seconds (0), &UdpForwarder::ThreadUp, this);
}

void
UdpForwarder::StopApplication (void)
{
  NS_LOG_FUNCTION (this);
  Simulator::Cancel (m_forwardEvent);
}

void
UdpForwarder::ThreadUp (void)
{
  int i, j; /* loop variables */
  unsigned pkt_in_dgram; /* nb on Lora packet in the current datagram */

  /* allocate memory for packet fetching and processing */
  lgw_pkt_rx_s rxpkt[NB_PKT_MAX]; /* array containing inbound packets + metadata */
  lgw_pkt_rx_s *p; /* pointer on a RX packet */
  int nb_pkt;

  /* data buffers */
  uint8_t buff_up[TX_BUFF_SIZE]; /* buffer to compose the upstream packet */
  int buff_index;
  //uint8_t buff_ack[32]; /* buffer to receive acknowledges */

  /* protocol variables */
  uint8_t token_h; /* random token for acknowledgement matching */
  uint8_t token_l; /* random token for acknowledgement matching */

  /* ping measurement variables */
  //timespec send_time;
  //timespec recv_time;

  /* report management variable */
  bool send_report = false;

  /* mote info variables */
  uint32_t mote_addr = 0;
  uint16_t mote_fcnt = 0;

  // TODO check if useful
  /* set upstream socket RX timeout */
  /* i = setsockopt(sock_up, SOL_SOCKET, SO_RCVTIMEO, (void *)&push_timeout_half, sizeof push_timeout_half);
  if (i != 0) {
      MSG("ERROR: [up] setsockopt returned %s\n", strerror(errno));
      exit(EXIT_FAILURE);
  } */

  /* pre-fill the data buffer with fixed fields */
  buff_up[0] = PROTOCOL_VERSION;
  buff_up[3] = PKT_PUSH_DATA;
  *(uint32_t *) (buff_up + 4) = net_mac_h;
  *(uint32_t *) (buff_up + 8) = net_mac_l;

  /* fetch packets */
  nb_pkt = lgw_receive (NB_PKT_MAX, rxpkt);
  /*   nb_pkt = 1;
  p = &rxpkt[0];
  p->count_us = 3512348611;
  p->freq_hz = 866349812;
  p->if_chain = 2;
  p->rf_chain = 0;
  p->status = STAT_CRC_OK;
  p->modulation = MOD_LORA;
  p->datarate = DR_LORA_SF7;
  p->bandwidth = BW_125KHZ;
  p->coderate = CR_LORA_4_6;
  p->snr = 5.1;
  p->rssi = -35;
  p->size = 23;
  char pay[] = "ADFGUkFEshgAdAoAAACyGADXQ5rzpZs=";
  b64_to_bin (pay, 32, p->payload, 256); */

  /* wait a short time if no packets, nor status report */
  if ((nb_pkt == 0) && (send_report == false))
    {
      m_forwardEvent = Simulator::Schedule (Seconds (10), &UdpForwarder::ThreadUp, this);
      return;
    }

  /* start composing datagram with the header */
  token_h = (uint8_t) rand (); /* random token */
  token_l = (uint8_t) rand (); /* random token */
  buff_up[1] = token_h;
  buff_up[2] = token_l;
  buff_index = 12; /* 12-byte header */

  /* start of JSON structure */
  memcpy ((void *) (buff_up + buff_index), (void *) "{\"rxpk\":[", 9);
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

      // TODO look into
      /* RAW timestamp, 8-17 useful chars */
      j = snprintf ((char *) (buff_up + buff_index), TX_BUFF_SIZE - buff_index, "\"tmst\":%u",
                    p->count_us);
      if (j > 0)
        {
          buff_index += j;
        }
      else
        {
          MSG ("ERROR: [up] snprintf failed line %u\n", (__LINE__ - 4));
          exit (EXIT_FAILURE);
        }

      /* Packet concentrator channel, RF chain & RX frequency, 34-36 useful chars */
      j = snprintf ((char *) (buff_up + buff_index), TX_BUFF_SIZE - buff_index,
                    ",\"chan\":%1u,\"rfch\":%1u,\"freq\":%.6lf", p->if_chain, p->rf_chain,
                    ((double) p->freq_hz / 1e6));
      if (j > 0)
        {
          buff_index += j;
        }
      else
        {
          MSG ("ERROR: [up] snprintf failed line %u\n", (__LINE__ - 4));
          exit (EXIT_FAILURE);
        }

      /* Packet status, 9-10 useful chars */
      switch (p->status)
        {
        case STAT_CRC_OK:
          memcpy ((void *) (buff_up + buff_index), (void *) ",\"stat\":1", 9);
          buff_index += 9;
          break;
        case STAT_CRC_BAD:
          memcpy ((void *) (buff_up + buff_index), (void *) ",\"stat\":-1", 10);
          buff_index += 10;
          break;
        case STAT_NO_CRC:
          memcpy ((void *) (buff_up + buff_index), (void *) ",\"stat\":0", 9);
          buff_index += 9;
          break;
        default:
          MSG ("ERROR: [up] received packet with unknown status\n");
          memcpy ((void *) (buff_up + buff_index), (void *) ",\"stat\":?", 9);
          buff_index += 9;
          exit (EXIT_FAILURE);
        }

      /* Packet modulation, 13-14 useful chars */
      if (p->modulation == MOD_LORA)
        {
          memcpy ((void *) (buff_up + buff_index), (void *) ",\"modu\":\"LORA\"", 14);
          buff_index += 14;

          /* Lora datarate & bandwidth, 16-19 useful chars */
          switch (p->datarate)
            {
            case DR_LORA_SF7:
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"datr\":\"SF7", 12);
              buff_index += 12;
              break;
            case DR_LORA_SF8:
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"datr\":\"SF8", 12);
              buff_index += 12;
              break;
            case DR_LORA_SF9:
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"datr\":\"SF9", 12);
              buff_index += 12;
              break;
            case DR_LORA_SF10:
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"datr\":\"SF10", 13);
              buff_index += 13;
              break;
            case DR_LORA_SF11:
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"datr\":\"SF11", 13);
              buff_index += 13;
              break;
            case DR_LORA_SF12:
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"datr\":\"SF12", 13);
              buff_index += 13;
              break;
            default:
              MSG ("ERROR: [up] lora packet with unknown datarate\n");
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"datr\":\"SF?", 12);
              buff_index += 12;
              exit (EXIT_FAILURE);
            }
          switch (p->bandwidth)
            {
            case BW_125KHZ:
              memcpy ((void *) (buff_up + buff_index), (void *) "BW125\"", 6);
              buff_index += 6;
              break;
            case BW_250KHZ:
              memcpy ((void *) (buff_up + buff_index), (void *) "BW250\"", 6);
              buff_index += 6;
              break;
            case BW_500KHZ:
              memcpy ((void *) (buff_up + buff_index), (void *) "BW500\"", 6);
              buff_index += 6;
              break;
            default:
              MSG ("ERROR: [up] lora packet with unknown bandwidth\n");
              memcpy ((void *) (buff_up + buff_index), (void *) "BW?\"", 4);
              buff_index += 4;
              exit (EXIT_FAILURE);
            }

          /* Packet ECC coding rate, 11-13 useful chars */
          switch (p->coderate)
            {
            case CR_LORA_4_5:
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"codr\":\"4/5\"", 13);
              buff_index += 13;
              break;
            case CR_LORA_4_6:
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"codr\":\"4/6\"", 13);
              buff_index += 13;
              break;
            case CR_LORA_4_7:
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"codr\":\"4/7\"", 13);
              buff_index += 13;
              break;
            case CR_LORA_4_8:
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"codr\":\"4/8\"", 13);
              buff_index += 13;
              break;
            case 0: /* treat the CR0 case (mostly false sync) */
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"codr\":\"OFF\"", 13);
              buff_index += 13;
              break;
            default:
              MSG ("ERROR: [up] lora packet with unknown coderate\n");
              memcpy ((void *) (buff_up + buff_index), (void *) ",\"codr\":\"?\"", 11);
              buff_index += 11;
              exit (EXIT_FAILURE);
            }

          /* Lora SNR, 11-13 useful chars */
          j = snprintf ((char *) (buff_up + buff_index), TX_BUFF_SIZE - buff_index,
                        ",\"lsnr\":%.1f", p->snr);
          if (j > 0)
            {
              buff_index += j;
            }
          else
            {
              MSG ("ERROR: [up] snprintf failed line %u\n", (__LINE__ - 4));
              exit (EXIT_FAILURE);
            }
        }
      else if (p->modulation == MOD_FSK)
        {
          memcpy ((void *) (buff_up + buff_index), (void *) ",\"modu\":\"FSK\"", 13);
          buff_index += 13;

          /* FSK datarate, 11-14 useful chars */
          j = snprintf ((char *) (buff_up + buff_index), TX_BUFF_SIZE - buff_index, ",\"datr\":%u",
                        p->datarate);
          if (j > 0)
            {
              buff_index += j;
            }
          else
            {
              MSG ("ERROR: [up] snprintf failed line %u\n", (__LINE__ - 4));
              exit (EXIT_FAILURE);
            }
        }
      else
        {
          MSG ("ERROR: [up] received packet with unknown modulation\n");
          exit (EXIT_FAILURE);
        }

      /* Packet RSSI, payload size, 18-23 useful chars */
      j = snprintf ((char *) (buff_up + buff_index), TX_BUFF_SIZE - buff_index,
                    ",\"rssi\":%.0f,\"size\":%u", p->rssi, p->size);
      if (j > 0)
        {
          buff_index += j;
        }
      else
        {
          MSG ("ERROR: [up] snprintf failed line %u\n", (__LINE__ - 4));
          exit (EXIT_FAILURE);
        }

      /* Packet base64-encoded payload, 14-350 useful chars */
      memcpy ((void *) (buff_up + buff_index), (void *) ",\"data\":\"", 9);
      buff_index += 9;
      j = bin_to_b64 (p->payload, p->size, (char *) (buff_up + buff_index),
                      341); /* 255 bytes = 340 chars in b64 + null char */
      if (j >= 0)
        {
          buff_index += j;
        }
      else
        {
          MSG ("ERROR: [up] bin_to_b64 failed line %u\n", (__LINE__ - 5));
          exit (EXIT_FAILURE);
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

  /* end of JSON datagram payload */
  buff_up[buff_index] = '}';
  ++buff_index;
  buff_up[buff_index] = 0; /* add string terminator, for safety */

  printf ("\nJSON up: %s\n", (char *) (buff_up + 12)); /* DEBUG: display JSON payload */

  /* send datagram to server */
  Ptr<Packet> upPkt = Create<Packet> ((uint8_t const *) buff_up, (uint32_t) buff_index);
  int size = upPkt->GetSize ();
  if (m_sockUp->Send (upPkt) >= 0) // send(sock_up, (void *)buff_up, buff_index, 0);
    {
      NS_LOG_DEBUG ("send #" << unsigned (m_sent));
      ++m_sent;
      m_totalTx += size;
#ifdef NS3_LOG_ENABLE
      NS_LOG_INFO ("TraceDelay TX " << size << " bytes to " << m_peerAddressString
                                    << " Uid: " << upPkt->GetUid ()
                                    << " Time: " << (Simulator::Now ()).As (Time::S));
#endif // NS3_LOG_ENABLE
    }
#ifdef NS3_LOG_ENABLE
  else
    {
      NS_LOG_INFO ("Error while sending " << size << " bytes to " << m_peerAddressString);
    }
#endif // NS3_LOG_ENABLE

  //clock_gettime(CLOCK_MONOTONIC, &send_time);
  meas_up_dgram_sent += 1;
  meas_up_network_byte += buff_index;

  /* wait for acknowledge (in 2 times, to catch extra packets) */

  m_forwardEvent = Simulator::Schedule (MilliSeconds (10), &UdpForwarder::ThreadUp, this);
  //MSG("\nINFO: End of upstream thread\n");
}

int
UdpForwarder::lgw_receive (int nb_pkt_max, lgw_pkt_rx_s rxpkt[])
{
  int nb = m_rxpktbuff.size ();
  int i = 0;
  for (; i < nb_pkt_max and i < nb; ++i)
    {
      rxpkt[i] = m_rxpktbuff.front ();
      m_rxpktbuff.pop ();
    }
  return i;
}

} // namespace lorawan
} // namespace ns3
