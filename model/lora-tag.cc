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

#include "ns3/lora-tag.h"
#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3 {
namespace lorawan {

NS_OBJECT_ENSURE_REGISTERED (LoraTag);

TypeId
LoraTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraTag")
    .SetParent<Tag> ()
    .SetGroupName ("lorawan")
    .AddConstructor<LoraTag> ()
  ;
  return tid;
}

TypeId
LoraTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

LoraTag::LoraTag (uint8_t sf, uint8_t destroyedBy) :
  m_sf (sf),
  m_destroyedBy (destroyedBy),
  m_receivePower (0),
  m_dataRate (0),
  m_frequency (0)
{
}

LoraTag::~LoraTag ()
{
}

uint32_t
LoraTag::GetSerializedSize (void) const
{
  // Each datum about a SF is 1 byte + receivePower (the size of a double) +
  // frequency (the size of a double)
  return 3 + 2 * sizeof(double);
}

void
LoraTag::Serialize (TagBuffer i) const
{
  i.WriteU8 (m_sf);
  i.WriteU8 (m_destroyedBy);
  i.WriteDouble (m_receivePower);
  i.WriteU8 (m_dataRate);
  i.WriteDouble (m_frequency);
}

void
LoraTag::Deserialize (TagBuffer i)
{
  m_sf = i.ReadU8 ();
  m_destroyedBy = i.ReadU8 ();
  m_receivePower = i.ReadDouble ();
  m_dataRate = i.ReadU8 ();
  m_frequency = i.ReadDouble ();
}

void
LoraTag::Print (std::ostream &os) const
{
  os << m_sf << " " << m_destroyedBy << " " << m_receivePower << " " <<
    m_dataRate;
}

uint8_t
LoraTag::GetSpreadingFactor () const
{
  return m_sf;
}

uint8_t
LoraTag::GetDestroyedBy () const
{
  return m_destroyedBy;
}

double
LoraTag::GetReceivePower () const
{
  return m_receivePower;
}

void
LoraTag::SetDestroyedBy (uint8_t sf)
{
  m_destroyedBy = sf;
}

void
LoraTag::SetSpreadingFactor (uint8_t sf)
{
  m_sf = sf;
}

void
LoraTag::SetReceivePower (double receivePower)
{
  m_receivePower = receivePower;
}

void
LoraTag::SetFrequency (double frequency)
{
  m_frequency = frequency;
}

double
LoraTag::GetFrequency (void)
{
  return m_frequency;
}

uint8_t
LoraTag::GetDataRate (void)
{
  return m_dataRate;
}

void
LoraTag::SetDataRate (uint8_t dataRate)
{
  m_dataRate = dataRate;
}

}
} // namespace ns3
