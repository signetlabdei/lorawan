/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "gateway-status.h"

#include "ns3/log.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("GatewayStatus");

TypeId
GatewayStatus::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::GatewayStatus").AddConstructor<GatewayStatus>().SetGroupName("lorawan");
    return tid;
}

GatewayStatus::GatewayStatus()
{
    NS_LOG_FUNCTION(this);
}

GatewayStatus::~GatewayStatus()
{
    NS_LOG_FUNCTION(this);
}

GatewayStatus::GatewayStatus(Address address,
                             Ptr<NetDevice> netDevice,
                             Ptr<GatewayLorawanMac> gwMac)
    : m_address(address),
      m_netDevice(netDevice),
      m_gatewayMac(gwMac),
      m_nextTransmissionTime(Time(0))
{
    NS_LOG_FUNCTION(this);
}

Address
GatewayStatus::GetAddress()
{
    NS_LOG_FUNCTION(this);

    return m_address;
}

void
GatewayStatus::SetAddress(Address address)
{
    NS_LOG_FUNCTION(this);

    m_address = address;
}

Ptr<NetDevice>
GatewayStatus::GetNetDevice()
{
    return m_netDevice;
}

void
GatewayStatus::SetNetDevice(Ptr<NetDevice> netDevice)
{
    m_netDevice = netDevice;
}

Ptr<GatewayLorawanMac>
GatewayStatus::GetGatewayMac()
{
    return m_gatewayMac;
}

bool
GatewayStatus::IsAvailableForTransmission(uint32_t frequencyHz)
{
    // We can't send multiple packets at once, see SX1301 V2.01 page 29

    // Check that the gateway was not already "booked"
    if (m_nextTransmissionTime > Now() - MilliSeconds(1))
    {
        NS_LOG_INFO("This gateway is already booked for a transmission");
        return false;
    }

    // Check that the gateway is not already in TX mode
    if (m_gatewayMac->IsTransmitting())
    {
        NS_LOG_INFO("This gateway is currently transmitting");
        return false;
    }

    // Check that the gateway is not constrained by the duty cycle
    Time waitTime = m_gatewayMac->GetWaitTime(frequencyHz);
    if (waitTime.IsStrictlyPositive())
    {
        NS_LOG_INFO("Gateway cannot be used because of duty cycle");
        NS_LOG_INFO("Wait time at current gateway: " << waitTime.As(Time::S));

        return false;
    }

    return true;
}

void
GatewayStatus::SetNextTransmissionTime(Time nextTransmissionTime)
{
    m_nextTransmissionTime = nextTransmissionTime;
}
} // namespace lorawan
} // namespace ns3
