/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "network-controller.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("NetworkController");

NS_OBJECT_ENSURE_REGISTERED(NetworkController);

TypeId
NetworkController::GetTypeId()
{
    static TypeId tid = TypeId("ns3::NetworkController")
                            .SetParent<Object>()
                            .AddConstructor<NetworkController>()
                            .SetGroupName("lorawan");
    return tid;
}

NetworkController::NetworkController()
{
    NS_LOG_FUNCTION_NOARGS();
}

NetworkController::NetworkController(Ptr<NetworkStatus> networkStatus)
    : m_status(networkStatus)
{
    NS_LOG_FUNCTION_NOARGS();
}

NetworkController::~NetworkController()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
NetworkController::Install(Ptr<NetworkControllerComponent> component)
{
    NS_LOG_FUNCTION(this);
    m_components.push_back(component);
}

void
NetworkController::OnNewPacket(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    // NOTE As a future optimization, we can allow components to register their
    // callbacks and only be called in case a certain MAC command is contained.
    // For now, we call all components.

    // Inform each component about the new packet
    for (auto it = m_components.begin(); it != m_components.end(); ++it)
    {
        (*it)->OnReceivedPacket(packet, m_status->GetEndDeviceStatus(packet), m_status);
    }
}

void
NetworkController::BeforeSendingReply(Ptr<EndDeviceStatus> endDeviceStatus)
{
    NS_LOG_FUNCTION(this);

    // Inform each component about the imminent reply
    for (auto it = m_components.begin(); it != m_components.end(); ++it)
    {
        (*it)->BeforeSendingReply(endDeviceStatus, m_status);
    }
}

} // namespace lorawan
} // namespace ns3
