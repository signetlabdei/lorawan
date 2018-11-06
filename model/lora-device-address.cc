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

#include "ns3/lora-device-address.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("LoraDeviceAddress");

// NwkID
////////

NwkID::NwkID (uint8_t nwkId) : m_nwkId (nwkId)
{
}

void
NwkID::Set (uint8_t nwkId)
{
  // Check whether the MSB is set
  if (nwkId >> 7)
    {
      NS_LOG_WARN ("Attempting to set too big a network ID. Will only consider the 7 least significant bits.");
    }
  m_nwkId = nwkId & 0x7F;     // 0x7f = ob01111111
}

uint8_t
NwkID::Get (void) const
{
  return m_nwkId;
}

// NwkAddr
//////////

NwkAddr::NwkAddr (uint32_t nwkAddr) : m_nwkAddr (nwkAddr)
{
}

void
NwkAddr::Set (uint32_t nwkAddr)
{
  // Check whether the most significant bits are set
  if (nwkAddr >> 25)
    {
      NS_LOG_WARN ("Attempting to set too big a network address. Will only consider the 25 least significant bits.");
    }
  m_nwkAddr = nwkAddr & 0x1FFFFFF;
}

uint32_t
NwkAddr::Get (void) const
{
  return m_nwkAddr;
}

// LoraDeviceAddress
////////////////////

LoraDeviceAddress::LoraDeviceAddress ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

LoraDeviceAddress::LoraDeviceAddress (uint32_t address)
{
  NS_LOG_FUNCTION (this << address);

  Set (address);
}

LoraDeviceAddress::LoraDeviceAddress (uint8_t nwkId, uint32_t nwkAddr)
{
  NS_LOG_FUNCTION (this << unsigned(nwkId) << nwkAddr);

  m_nwkId.Set (nwkId);
  m_nwkAddr.Set (nwkAddr);
}

LoraDeviceAddress::LoraDeviceAddress (NwkID nwkId, NwkAddr nwkAddr)
{
  NS_LOG_FUNCTION (this << unsigned(nwkId.Get ()) << nwkAddr.Get ());

  m_nwkId = nwkId;
  m_nwkAddr = nwkAddr;
}

void
LoraDeviceAddress::Serialize (uint8_t buf[4]) const
{
  NS_LOG_FUNCTION (this << &buf);

  uint32_t address = Get ();

  buf[0] = (address >> 24) & 0xff;
  buf[1] = (address >> 16) & 0xff;
  buf[2] = (address >> 8) & 0xff;
  buf[3] = (address >> 0) & 0xff;
}

LoraDeviceAddress
LoraDeviceAddress::Deserialize (const uint8_t buf[4])
{
  NS_LOG_FUNCTION (&buf);

  // Craft the address from the buffer
  uint32_t address = 0;
  address |= buf[0];
  address <<= 8;
  address |= buf[1];
  address <<= 8;
  address |= buf[2];
  address <<= 8;
  address |= buf[3];

  return LoraDeviceAddress (address);
}

Address
LoraDeviceAddress::ConvertTo (void) const
{
  NS_LOG_FUNCTION (this);

  uint8_t addressBuffer[4];
  Serialize (addressBuffer);
  return Address (GetType (), addressBuffer, 4);
}

LoraDeviceAddress
LoraDeviceAddress::ConvertFrom (const Address &address)
{
  // Create the new, empty address
  LoraDeviceAddress ad;
  uint8_t addressBuffer[4];

  // Check that the address we want to convert is compatible with a
  // LoraDeviceAddress
  NS_ASSERT (address.CheckCompatible (GetType (), 4));
  address.CopyTo (addressBuffer);
  ad = Deserialize (addressBuffer);
  return ad;
}

uint8_t
LoraDeviceAddress::GetType (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  static uint8_t type = Address::Register ();
  return type;
}

uint32_t
LoraDeviceAddress::Get (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  uint32_t address = 0;
  uint32_t nwkId = uint32_t (m_nwkId.Get () << 25);
  address |= (m_nwkAddr.Get () | nwkId);
  NS_LOG_DEBUG ("m_nwkId + m_nwkAddr = " << std::bitset<32> (address));

  return address;
}

void
LoraDeviceAddress::Set (uint32_t address)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_nwkId.Set (address >> 25);     // Only leave the 7 most significant bits
  m_nwkAddr.Set (address & 0x1FFFFFF);     // Only consider the 25 least significant bits
}

uint8_t
LoraDeviceAddress::GetNwkID (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_nwkId.Get ();
}

uint32_t
LoraDeviceAddress::GetNwkAddr (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_nwkAddr.Get ();
}

void
LoraDeviceAddress::SetNwkID (uint8_t nwkId)
{
  NS_LOG_FUNCTION (this << unsigned(nwkId));

  m_nwkId.Set (nwkId);
}

void
LoraDeviceAddress::SetNwkAddr (uint32_t nwkAddr)
{
  NS_LOG_FUNCTION (this << nwkAddr);

  m_nwkAddr.Set (nwkAddr);
}

std::string
LoraDeviceAddress::Print (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  std::string result;
  result += std::bitset<7> (m_nwkId.Get ()).to_string ();
  result += "|";
  result += std::bitset<25> (m_nwkAddr.Get ()).to_string ();
  return result;
}

bool LoraDeviceAddress::operator==
  (const LoraDeviceAddress &other) const
{
  return this->Get () == other.Get ();
}

bool LoraDeviceAddress::operator!=
  (const LoraDeviceAddress &other) const
{
  return this->Get () != other.Get ();
}

bool LoraDeviceAddress::operator<
  (const LoraDeviceAddress &other) const
{
  return this->Get () < other.Get ();
}

bool LoraDeviceAddress::operator>
  (const LoraDeviceAddress &other) const
{
  return !(this->Get () < other.Get ());
}

std::ostream& operator<< (std::ostream& os, const LoraDeviceAddress &address)
{
  os << address.Print ();
  return os;
}
}
}
