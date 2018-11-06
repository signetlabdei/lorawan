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

#include "ns3/lora-device-address-generator.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("LoraDeviceAddressGenerator");

TypeId
LoraDeviceAddressGenerator::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraDeviceAddressGenerator")
    .SetParent<Object> ()
    .SetGroupName ("lorawan")
    .AddConstructor<LoraDeviceAddressGenerator> ();
  return tid;
}

LoraDeviceAddressGenerator::LoraDeviceAddressGenerator (const uint8_t nwkId,
                                                        const uint32_t nwkAddr)
{
  NS_LOG_FUNCTION (this << unsigned(nwkId) << nwkAddr);

  m_currentNwkId.Set (nwkId);
  m_currentNwkAddr.Set (nwkAddr);
}

LoraDeviceAddress
LoraDeviceAddressGenerator::NextNetwork (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_currentNwkId.Set (m_currentNwkId.Get () + 1);
  m_currentNwkAddr.Set (0);

  return LoraDeviceAddress (m_currentNwkId, m_currentNwkAddr);
}

LoraDeviceAddress
LoraDeviceAddressGenerator::NextAddress (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  NwkAddr oldNwkAddr = m_currentNwkAddr;
  m_currentNwkAddr.Set (m_currentNwkAddr.Get () + 1);

  return LoraDeviceAddress (m_currentNwkId, oldNwkAddr);
}

LoraDeviceAddress
LoraDeviceAddressGenerator::GetNextAddress (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return LoraDeviceAddress (m_currentNwkId.Get (), m_currentNwkAddr.Get () + 1);
}
}
}
