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
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "one-shot-sender.h"

#include "ns3/simulator.h"

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
                            .SetParent<LoraApplication>()
                            .AddConstructor<OneShotSender>()
                            .SetGroupName("lorawan");
    return tid;
}

OneShotSender::OneShotSender()
{
    NS_LOG_FUNCTION(this);
    SetInitialDelay(Seconds(0));
    SetPacketSize(10);
}

OneShotSender::OneShotSender(Time sendTime)
{
    NS_LOG_FUNCTION(this);
    SetInitialDelay(sendTime);
    SetPacketSize(10);
}

OneShotSender::~OneShotSender()
{
    NS_LOG_FUNCTION(this);
}

void
OneShotSender::SetSendTime(Time sendTime)
{
    NS_LOG_FUNCTION(this << sendTime);
    SetInitialDelay(sendTime);
}

void
OneShotSender::SendPacket()
{
    NS_LOG_FUNCTION(this);

    // Create and send a new packet
    Ptr<Packet> packet = Create<Packet>(m_basePktSize);
    m_mac->Send(packet);
}

void
OneShotSender::StartApplication()
{
    NS_LOG_FUNCTION(this);
    // Schedule the next SendPacket event
    Simulator::Cancel(m_sendEvent);
    m_sendEvent = Simulator::Schedule(m_initialDelay, &OneShotSender::SendPacket, this);
}

} // namespace lorawan
} // namespace ns3
