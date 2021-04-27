/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Alessandro Aimi <alleaimi95@gmail.com>
 */

#include "ns3/loratap-pcap-header.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("LoratapPcapHeader");

// Initialization list
LoratapPcapHeader::LoratapPcapHeader () :
  m_lt_version (0),
  m_lt_padding (0),
  m_lt_length (0),
  m_frequency (0),
  m_bandwidth (0),
  m_sf (0),
  m_packet_rssi (0),
  m_max_rssi (0),
  m_current_rssi (0),
  m_snr (0),
  m_sync_word (52)
{
}

LoratapPcapHeader::~LoratapPcapHeader ()
{
}

TypeId
LoratapPcapHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("LoratapPcapHeader")
    .SetParent<Header> ()
    .AddConstructor<LoratapPcapHeader> ()
  ;
  return tid;
}

TypeId
LoratapPcapHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LoratapPcapHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Size in bytes, always the same
  uint32_t size = 15;

  NS_LOG_INFO ("LoratapPcapHeader serialized size: " << size);

  return size;
}

void
LoratapPcapHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  start.WriteU8 (m_lt_version);
  start.WriteU8 (m_lt_padding);
  start.WriteHtonU16 (m_lt_length);
  start.WriteHtonU32 (m_frequency);
  start.WriteU8 (m_bandwidth);
  start.WriteU8 (m_sf);
  start.WriteU8 (m_packet_rssi);
  start.WriteU8 (m_max_rssi);
  start.WriteU8 (m_current_rssi);
  start.WriteU8 (m_snr);
  start.WriteU8 (m_sync_word);
}

uint32_t
LoratapPcapHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Read from buffer and save into local variables
  m_lt_version = start.ReadU8 ();
  m_lt_padding = start.ReadU8 ();
  m_lt_length = start.ReadNtohU16 ();
  m_frequency = start.ReadNtohU32 ();
  m_bandwidth = start.ReadU8 ();
  m_sf = start.ReadU8 ();
  m_packet_rssi = start.ReadU8 ();
  m_max_rssi = start.ReadU8 ();
  m_current_rssi = start.ReadU8 ();
  m_snr = start.ReadU8 ();
  m_sync_word = start.ReadU8 ();

  NS_LOG_DEBUG ("Deserialized data: ");
  NS_LOG_DEBUG ("lt_version: " << unsigned(m_lt_version));
  NS_LOG_DEBUG ("lt_padding: " << unsigned(m_lt_padding));
  NS_LOG_DEBUG ("lt_length: " << unsigned(m_lt_length));
  NS_LOG_DEBUG ("frequency: " << unsigned(m_frequency));
  NS_LOG_DEBUG ("bandwidth: " << unsigned(m_bandwidth));
  NS_LOG_DEBUG ("sf: " << unsigned(m_sf));
  NS_LOG_DEBUG ("packet_rssi: " << unsigned(m_packet_rssi));
  NS_LOG_DEBUG ("max_rssi: " << unsigned(m_max_rssi));
  NS_LOG_DEBUG ("current_rssi: " << unsigned(m_current_rssi));
  NS_LOG_DEBUG ("snr: " << unsigned(m_snr));
  NS_LOG_DEBUG ("sync_word: " << unsigned(m_sync_word));

  return 15; // the number of bytes consumed.
}

void
LoratapPcapHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "(lt_version " << unsigned(m_lt_version)
     << " lt_padding " << unsigned(m_lt_padding)
     << " lt_length " << unsigned(m_lt_length)
     << ") frequency " << unsigned(m_frequency)
     << " bandwidth " << unsigned(m_bandwidth)
     << " sf " << unsigned(m_sf)
     << " packet_rssi " << unsigned(m_packet_rssi)
     << " max_rssi " << unsigned(m_max_rssi)
     << " current_rssi " << unsigned(m_current_rssi)
     << " snr " << unsigned(m_snr)
     << " sync_word " << unsigned(m_sync_word);
}

void
LoratapPcapHeader::FillHeader (LoraTag &tag)
{
  //NS_LOG_FUNCTION (this << tag); This doesn't work and I don't know why
  
  m_frequency = unsigned(int(tag.GetFrequency () * 1000000));
  m_bandwidth = 1; // 1 * 125kHz
  m_sf = tag.GetSpreadingFactor ();
  std::cout << tag.GetReceivePower () << "\n";
  m_packet_rssi = unsigned(int(139.5 + tag.GetReceivePower ())); //139.5 insted of 139 to approximate to nearest int
  //m_max_rssi = ?
  //m_current_rssi = ?
  //m_snr = ?
}


}
}