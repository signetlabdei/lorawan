/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "periodic-sender.h"

#include "lora-net-device.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/string.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("PeriodicSender");

NS_OBJECT_ENSURE_REGISTERED(PeriodicSender);

TypeId
PeriodicSender::GetTypeId()
{
    static TypeId tid = TypeId("ns3::PeriodicSender")
                            .SetParent<Application>()
                            .AddConstructor<PeriodicSender>()
                            .SetGroupName("lorawan")
                            .AddAttribute("Interval",
                                          "The interval between packet sends of this app",
                                          TimeValue(Time(0)),
                                          MakeTimeAccessor(&PeriodicSender::GetInterval,
                                                           &PeriodicSender::SetInterval),
                                          MakeTimeChecker());
    // .AddAttribute ("PacketSizeRandomVariable", "The random variable that determines the shape of
    // the packet size, in bytes",
    //                StringValue ("ns3::UniformRandomVariable[Min=0,Max=10]"),
    //                MakePointerAccessor (&PeriodicSender::m_pktSizeRV),
    //                MakePointerChecker <RandomVariableStream>());
    return tid;
}

PeriodicSender::PeriodicSender()
    : m_interval(Seconds(10)),
      m_initialDelay(Seconds(1)),
      m_basePktSize(10),
      m_pktSizeRV(nullptr)

{
    NS_LOG_FUNCTION_NOARGS();
}

PeriodicSender::~PeriodicSender()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
PeriodicSender::SetInterval(Time interval)
{
    NS_LOG_FUNCTION(this << interval);
    m_interval = interval;
}

Time
PeriodicSender::GetInterval() const
{
    NS_LOG_FUNCTION(this);
    return m_interval;
}

void
PeriodicSender::SetInitialDelay(Time delay)
{
    NS_LOG_FUNCTION(this << delay);
    m_initialDelay = delay;
}

void
PeriodicSender::SetPacketSizeRandomVariable(Ptr<RandomVariableStream> rv)
{
    m_pktSizeRV = rv;
}

void
PeriodicSender::SetPacketSize(uint8_t size)
{
    m_basePktSize = size;
}

void
PeriodicSender::SendPacket()
{
    NS_LOG_FUNCTION(this);

    // Create and send a new packet
    Ptr<Packet> packet;
    if (m_pktSizeRV)
    {
        int randomsize = m_pktSizeRV->GetInteger();
        packet = Create<Packet>(m_basePktSize + randomsize);
    }
    else
    {
        packet = Create<Packet>(m_basePktSize);
    }
    m_mac->Send(packet);

    // Schedule the next SendPacket event
    m_sendEvent = Simulator::Schedule(m_interval, &PeriodicSender::SendPacket, this);

    NS_LOG_DEBUG("Sent a packet of size " << packet->GetSize());
}

void
PeriodicSender::StartApplication()
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
    NS_LOG_DEBUG("Starting up application with a first event with a " << m_initialDelay.As(Time::S)
                                                                      << " delay");
    m_sendEvent = Simulator::Schedule(m_initialDelay, &PeriodicSender::SendPacket, this);
    NS_LOG_DEBUG("Event Id: " << m_sendEvent.GetUid());
}

void
PeriodicSender::StopApplication()
{
    NS_LOG_FUNCTION_NOARGS();
    Simulator::Cancel(m_sendEvent);
}

} // namespace lorawan
} // namespace ns3
