/*
 * Copyright (c) 2022 Orange SA
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
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#include "poisson-sender.h"

#include "ns3/double.h"
#include "ns3/lora-net-device.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("PoissonSender");

NS_OBJECT_ENSURE_REGISTERED(PoissonSender);

TypeId
PoissonSender::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::PoissonSender")
                            .SetParent<LoraApplication>()
                            .AddConstructor<PoissonSender>()
                            .SetGroupName("lorawan");
    return tid;
}

PoissonSender::PoissonSender()
{
    m_interval = CreateObject<ExponentialRandomVariable>();
}

PoissonSender::~PoissonSender()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
PoissonSender::StartApplication(void)
{
    NS_LOG_FUNCTION(this);

    m_interval->SetAttribute("Mean", DoubleValue(m_avgInterval.ToDouble(Time::S)));

    // Make sure we have a MAC layer
    if (bool(m_mac) != 0)
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
    m_sendEvent = Simulator::Schedule(m_initialDelay, &PoissonSender::SendPacket, this);
    NS_LOG_DEBUG("Event Id: " << m_sendEvent.GetUid());
}

void
PoissonSender::StopApplication(void)
{
    NS_LOG_FUNCTION_NOARGS();
    Simulator::Cancel(m_sendEvent);
}

void
PoissonSender::SendPacket(void)
{
    NS_LOG_FUNCTION(this);

    // Create and send a new packet
    Ptr<Packet> packet;
    packet = Create<Packet>(m_basePktSize);
    m_mac->Send(packet);

    Time interval = Min(Seconds(m_interval->GetValue()), Days(1));

    // Schedule the next SendPacket event
    m_sendEvent = Simulator::Schedule(interval, &PoissonSender::SendPacket, this);

    NS_LOG_DEBUG("Sent a packet of size " << packet->GetSize());
}

} // namespace lorawan
} // namespace ns3
