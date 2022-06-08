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

#include "ns3/loratap-header.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("LoratapHeader");

// Initialization list
LoratapHeader::LoratapHeader () :
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

LoratapHeader::~LoratapHeader ()
{
}

TypeId
LoratapHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("LoratapHeader")
    .SetParent<Header> ()
    .AddConstructor<LoratapHeader> ()
  ;
  return tid;
}

TypeId
LoratapHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LoratapHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Size in bytes, always the same
  uint32_t size = 15;

  NS_LOG_INFO ("LoratapHeader serialized size: " << size);

  return size;
}

void
LoratapHeader::Serialize (Buffer::Iterator start) const
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
LoratapHeader::Deserialize (Buffer::Iterator start)
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
LoratapHeader::Print (std::ostream &os) const
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
LoratapHeader::Fill (LoraTag &tag)
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_frequency = unsigned(int(tag.GetFrequency () * 1000000));
  m_bandwidth = 1; // 1 * 125kHz
  m_sf = tag.GetSpreadingFactor ();
  int snr = tag.GetSnr () + 0.5 - (tag.GetSnr () < 0); //.5 to approximate to nearest int
  int rssi = tag.GetReceivePower () + 0.5 - (tag.GetReceivePower () < 0);
  m_packet_rssi = rssi + 139;
  m_max_rssi = m_packet_rssi; // Arbitrary
  m_current_rssi = 0; // -139.0 dBm, arbitrary
  m_snr = snr * 4;
}


}
}
