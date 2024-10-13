/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "forwarder.h"

#include "ns3/log.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("Forwarder");

NS_OBJECT_ENSURE_REGISTERED(Forwarder);

TypeId
Forwarder::GetTypeId()
{
    static TypeId tid = TypeId("ns3::Forwarder")
                            .SetParent<Application>()
                            .AddConstructor<Forwarder>()
                            .SetGroupName("lorawan");
    return tid;
}

Forwarder::Forwarder()
{
    NS_LOG_FUNCTION_NOARGS();
}

Forwarder::~Forwarder()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
Forwarder::SetPointToPointNetDevice(Ptr<PointToPointNetDevice> pointToPointNetDevice)
{
    NS_LOG_FUNCTION(this << pointToPointNetDevice);

    m_pointToPointNetDevice = pointToPointNetDevice;
}

void
Forwarder::SetLoraNetDevice(Ptr<LoraNetDevice> loraNetDevice)
{
    NS_LOG_FUNCTION(this << loraNetDevice);

    m_loraNetDevice = loraNetDevice;
}

bool
Forwarder::ReceiveFromLora(Ptr<NetDevice> loraNetDevice,
                           Ptr<const Packet> packet,
                           uint16_t protocol,
                           const Address& sender)
{
    NS_LOG_FUNCTION(this << packet << protocol << sender);

    Ptr<Packet> packetCopy = packet->Copy();

    m_pointToPointNetDevice->Send(packetCopy, m_pointToPointNetDevice->GetBroadcast(), 0x800);

    return true;
}

bool
Forwarder::ReceiveFromPointToPoint(Ptr<NetDevice> pointToPointNetDevice,
                                   Ptr<const Packet> packet,
                                   uint16_t protocol,
                                   const Address& sender)
{
    NS_LOG_FUNCTION(this << packet << protocol << sender);

    Ptr<Packet> packetCopy = packet->Copy();

    m_loraNetDevice->Send(packetCopy);

    return true;
}

void
Forwarder::StartApplication()
{
    NS_LOG_FUNCTION(this);

    // TODO Make sure we are connected to both needed devices
}

void
Forwarder::StopApplication()
{
    NS_LOG_FUNCTION_NOARGS();

    // TODO Get rid of callbacks
}

} // namespace lorawan
} // namespace ns3
