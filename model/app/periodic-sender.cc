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

#include "ns3/simulator.h"

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
                            .SetParent<LoraApplication>()
                            .AddConstructor<PeriodicSender>()
                            .SetGroupName("lorawan");
    return tid;
}

PeriodicSender::PeriodicSender()
{
    NS_LOG_FUNCTION(this);
}

PeriodicSender::~PeriodicSender()
{
    NS_LOG_FUNCTION(this);
}

void
PeriodicSender::StartApplication()
{
    NS_LOG_FUNCTION(this);
    // Schedule the next SendPacket event
    Simulator::Cancel(m_sendEvent);
    NS_LOG_DEBUG("Starting up application with a first event with a " << m_initialDelay.GetSeconds()
                                                                      << " seconds delay");
    m_sendEvent = Simulator::Schedule(m_initialDelay, &PeriodicSender::SendPacket, this);
    NS_LOG_DEBUG("Event Id: " << m_sendEvent.GetUid());
}

void
PeriodicSender::SendPacket()
{
    NS_LOG_FUNCTION(this);
    // Create and send a new packet
    Ptr<Packet> packet = Create<Packet>(m_basePktSize);
    m_mac->Send(packet);
    // Schedule the next SendPacket event
    m_sendEvent = Simulator::Schedule(m_avgInterval, &PeriodicSender::SendPacket, this);
    NS_LOG_DEBUG("Sent a packet of size " << packet->GetSize());
}

} // namespace lorawan
} // namespace ns3
