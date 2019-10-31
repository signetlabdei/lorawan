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

#include "ns3/gateway-status.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("GatewayStatus");

TypeId
GatewayStatus::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GatewayStatus")
    .AddConstructor<GatewayStatus> ()
    .SetGroupName ("lorawan");
  return tid;
}


GatewayStatus::GatewayStatus ()
{
  NS_LOG_FUNCTION (this);
}

GatewayStatus::~GatewayStatus ()
{
  NS_LOG_FUNCTION (this);
}

GatewayStatus::GatewayStatus (Address address, Ptr<NetDevice> netDevice,
                              Ptr<GatewayLorawanMac> gwMac) :
  m_address (address),
  m_netDevice (netDevice),
  m_gatewayMac (gwMac),
  m_nextTransmissionTime (Seconds (0))
{
  NS_LOG_FUNCTION (this);
}

Address
GatewayStatus::GetAddress ()
{
  NS_LOG_FUNCTION (this);

  return m_address;
}

void
GatewayStatus::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this);

  m_address = address;
}

Ptr<NetDevice>
GatewayStatus::GetNetDevice ()
{
  return m_netDevice;
}

void
GatewayStatus::SetNetDevice (Ptr<NetDevice> netDevice)
{
  m_netDevice = netDevice;
}

Ptr<GatewayLorawanMac>
GatewayStatus::GetGatewayMac (void)
{
  return m_gatewayMac;
}

bool
GatewayStatus::IsAvailableForTransmission (double frequency)
{
  // We can't send multiple packets at once, see SX1301 V2.01 page 29

  // Check that the gateway was not already "booked"
  if (m_nextTransmissionTime > Simulator::Now () - MilliSeconds (1))
    {
      NS_LOG_INFO ("This gateway is already booked for a transmission");
      return false;
    }

  // Check that the gateway is not already in TX mode
  if (m_gatewayMac->IsTransmitting ())
    {
      NS_LOG_INFO ("This gateway is currently transmitting");
      return false;
    }

  // Check that the gateway is not constrained by the duty cycle
  Time waitingTime = m_gatewayMac->GetWaitingTime (frequency);
  if (waitingTime > Seconds (0))
    {
      NS_LOG_INFO ("Gateway cannot be used because of duty cycle");
      NS_LOG_INFO ("Waiting time at current GW: " << waitingTime.GetSeconds ()
                                                  << " seconds");

      return false;
    }

  return true;
}

void
GatewayStatus::SetNextTransmissionTime (Time nextTransmissionTime)
{
  m_nextTransmissionTime = nextTransmissionTime;
}
}
}
