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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "ns3/lora-mac-header.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraMacHeader");

LoraMacHeader::LoraMacHeader () : m_major (0)
{
}

LoraMacHeader::~LoraMacHeader ()
{
}

TypeId
LoraMacHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("LoraMacHeader")
    .SetParent<Header> ()
    .AddConstructor<LoraMacHeader> ()
  ;
  return tid;
}

TypeId
LoraMacHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LoraMacHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  return 1;   // This header only consists in 8 bits
}

void
LoraMacHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // The header we need to fill
  uint8_t header = 0;

  // The MType
  header |= m_mtype << 5;

  // Do nothing for the bits that are RFU

  // The major version bits
  header |= m_major;

  // Write the byte
  start.WriteU8 (header);

  NS_LOG_DEBUG ("Serialization of MAC header: " << std::bitset<8>(header));
}

uint32_t
LoraMacHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Save the byte on a temporary variable
  uint8_t byte;
  byte = start.ReadU8 ();

  // Get the 2 least significant bits to have the Major
  m_major = byte & 0b11;

  // Move the three most significant bits to the least significant positions
  // to get the MType
  m_mtype = byte >> 5;

  return 1;   // the number of bytes consumed.
}

void
LoraMacHeader::Print (std::ostream &os) const
{
  os << "MessageType=" << unsigned(m_mtype) << std::endl;
  os << "Major=" << unsigned(m_major) << std::endl;
}

void
LoraMacHeader::SetMType (enum MType mtype)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_mtype = mtype;
}

uint8_t
LoraMacHeader::GetMType (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_mtype;
}

void
LoraMacHeader::SetMajor (uint8_t major)
{
  NS_LOG_FUNCTION_NOARGS ();

  NS_ASSERT (0 <= major && major < 4);

  m_major = major;
}

uint8_t
LoraMacHeader::GetMajor (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_major;
}

bool
LoraMacHeader::IsUplink (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  return (m_mtype == JOIN_REQUEST) ||
         (m_mtype == UNCONFIRMED_DATA_UP) ||
         (m_mtype == CONFIRMED_DATA_UP);
}
}
