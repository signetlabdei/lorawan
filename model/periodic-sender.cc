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
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "periodic-sender.h"

#include "ns3/lora-net-device.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("PeriodicSender");

NS_OBJECT_ENSURE_REGISTERED(PeriodicSender);

TypeId
PeriodicSender::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::PeriodicSender")
                            .SetParent<LoraApplication>()
                            .AddConstructor<PeriodicSender>()
                            .SetGroupName("lorawan");
    // .AddAttribute ("PacketSizeRandomVariable", "The random variable that determines the shape of
    // the packet size, in bytes",
    //                StringValue ("ns3::UniformRandomVariable[Min=0,Max=10]"),
    //                MakePointerAccessor (&PeriodicSender::m_pktSizeRV),
    //                MakePointerChecker <RandomVariableStream>());
    return tid;
}

PeriodicSender::PeriodicSender()
{
    NS_LOG_FUNCTION_NOARGS();
}

PeriodicSender::~PeriodicSender()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
PeriodicSender::SetPacketSizeRandomVariable(Ptr<RandomVariableStream> rv)
{
    m_pktSizeRV = rv;
}

void
PeriodicSender::StartApplication(void)
{
    NS_LOG_FUNCTION(this);

    // Make sure we have a MAC layer
    if (bool(m_mac) == 0)
    {
        // Assumes there's only one device
        Ptr<LoraNetDevice> loraNetDevice = m_node->GetDevice(0)->GetObject<LoraNetDevice>();

        m_mac = loraNetDevice->GetMac();
        NS_ASSERT(bool(m_mac) != 0);
    }

    // Schedule the next SendPacket event
    Simulator::Cancel(m_sendEvent);
    NS_LOG_DEBUG("Starting up application with a first event with a " << m_initialDelay.GetSeconds()
                                                                      << " seconds delay");
    m_sendEvent = Simulator::Schedule(m_initialDelay, &PeriodicSender::SendPacket, this);
    NS_LOG_DEBUG("Event Id: " << m_sendEvent.GetUid());
}

void
PeriodicSender::StopApplication(void)
{
    NS_LOG_FUNCTION_NOARGS();
    Simulator::Cancel(m_sendEvent);
}

void
PeriodicSender::SendPacket(void)
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
    m_sendEvent = Simulator::Schedule(m_avgInterval, &PeriodicSender::SendPacket, this);

    NS_LOG_DEBUG("Sent a packet of size " << packet->GetSize());
}

} // namespace lorawan
} // namespace ns3
