/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "one-shot-sender.h"

#include "class-a-end-device-lorawan-mac.h"
#include "lora-net-device.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("OneShotSender");

NS_OBJECT_ENSURE_REGISTERED(OneShotSender);

TypeId
OneShotSender::GetTypeId()
{
    static TypeId tid = TypeId("ns3::OneShotSender")
                            .SetParent<Application>()
                            .AddConstructor<OneShotSender>()
                            .SetGroupName("lorawan");
    return tid;
}

OneShotSender::OneShotSender()
{
    NS_LOG_FUNCTION_NOARGS();
}

OneShotSender::OneShotSender(Time sendTime)
    : m_sendTime(sendTime)
{
    NS_LOG_FUNCTION_NOARGS();
}

OneShotSender::~OneShotSender()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
OneShotSender::SetSendTime(Time sendTime)
{
    NS_LOG_FUNCTION(this << sendTime);

    m_sendTime = sendTime;
}

void
OneShotSender::SendPacket()
{
    NS_LOG_FUNCTION(this);

    // Create and send a new packet
    Ptr<Packet> packet = Create<Packet>(10);
    m_mac->Send(packet);
}

void
OneShotSender::StartApplication()
{
    NS_LOG_FUNCTION(this);

    // Make sure we have a MAC layer
    if (!m_mac)
    {
        // Assumes there's only one device
        Ptr<LoraNetDevice> loraNetDevice = DynamicCast<LoraNetDevice>(m_node->GetDevice(0));

        m_mac = loraNetDevice->GetMac();
        NS_ASSERT(m_mac);
    }

    // Schedule the next SendPacket event
    Simulator::Cancel(m_sendEvent);
    m_sendEvent = Simulator::Schedule(m_sendTime, &OneShotSender::SendPacket, this);
}

void
OneShotSender::StopApplication()
{
    NS_LOG_FUNCTION_NOARGS();
    Simulator::Cancel(m_sendEvent);
}
} // namespace lorawan
} // namespace ns3
