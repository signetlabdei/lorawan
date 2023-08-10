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

#include "lora-channel.h"

#include "ns3/end-device-lora-phy.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraChannel");

NS_OBJECT_ENSURE_REGISTERED(LoraChannel);

TypeId
LoraChannel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LoraChannel")
            .SetParent<Channel>()
            .SetGroupName("lorawan")
            .AddConstructor<LoraChannel>()
            .AddAttribute("PropagationLossModel",
                          "A pointer to the propagation loss model attached to this channel.",
                          PointerValue(),
                          MakePointerAccessor(&LoraChannel::m_loss),
                          MakePointerChecker<PropagationLossModel>())
            .AddAttribute("PropagationDelayModel",
                          "A pointer to the propagation delay model attached to this channel.",
                          PointerValue(),
                          MakePointerAccessor(&LoraChannel::m_delay),
                          MakePointerChecker<PropagationDelayModel>())
            .AddTraceSource("PacketSent",
                            "Trace source fired whenever a packet goes out on the channel",
                            MakeTraceSourceAccessor(&LoraChannel::m_packetSent),
                            "ns3::Packet::TracedCallback");
    return tid;
}

LoraChannel::LoraChannel()
{
    NS_LOG_FUNCTION(this);
}

LoraChannel::~LoraChannel()
{
    NS_LOG_FUNCTION(this);
    m_phyListUp.clear();
    m_phyListDown.clear();
    m_delay = nullptr;
    m_loss = nullptr;
}

LoraChannel::LoraChannel(Ptr<PropagationLossModel> loss, Ptr<PropagationDelayModel> delay)
    : m_loss(loss),
      m_delay(delay)
{
    NS_LOG_FUNCTION(this << loss << delay);
}

void
LoraChannel::Add(Ptr<LoraPhy> phy)
{
    NS_LOG_FUNCTION(this << phy);
    // Add the new phy to the right destination vector
    ((DynamicCast<EndDeviceLoraPhy>(phy)) ? m_phyListDown : m_phyListUp).push_back(phy);
}

void
LoraChannel::Remove(Ptr<LoraPhy> phy)
{
    NS_LOG_FUNCTION(this << phy);
    // Remove the phy from the right vector
    auto& phyList = (DynamicCast<EndDeviceLoraPhy>(phy)) ? m_phyListDown : m_phyListUp;
    auto i = find(phyList.begin(), phyList.end(), phy);
    if (i != phyList.end())
    {
        phyList.erase(i);
    }
}

std::size_t
LoraChannel::GetNDevices() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_phyListUp.size() + m_phyListDown.size();
}

Ptr<NetDevice>
LoraChannel::GetDevice(std::size_t i) const
{
    NS_LOG_FUNCTION(this << i);
    size_t numUp = m_phyListUp.size();
    auto phy = (i < numUp) ? m_phyListUp[i] : m_phyListDown[i - numUp];
    return phy->GetDevice();
}

void
LoraChannel::Send(Ptr<LoraPhy> sender,
                  Ptr<Packet> packet,
                  double txPowerDbm,
                  uint8_t sf,
                  Time duration,
                  double frequency) const
{
    NS_LOG_FUNCTION(this << sender << packet << txPowerDbm << (unsigned)sf << duration
                         << frequency);
    // Get the mobility model of the sender
    auto senderMobility = sender->GetMobility();
    NS_ASSERT(bool(senderMobility) != 0); // Make sure it's available
    NS_LOG_INFO("Sender mobility: " << senderMobility->GetPosition());
    // Determine direction (uplink or downlink)
    bool down = !DynamicCast<EndDeviceLoraPhy>(sender);
    auto& receivers = (down) ? m_phyListDown : m_phyListUp;
    NS_LOG_INFO("Starting cycle over " << receivers.size() << " PHYs"
                                       << ((down) ? " in downlink" : " in uplink"));
    // Cycle over all registered PHYs
    for (auto& phy : receivers)
    {
        // Get the receiver's mobility model
        auto receiverMobility = phy->GetMobility();
        NS_LOG_INFO("Receiver mobility: " << receiverMobility->GetPosition());
        // Compute delay using the delay model
        Time delay = m_delay->GetDelay(senderMobility, receiverMobility);
        // Compute received power using the loss model
        double rxPowerDbm = GetRxPower(txPowerDbm, senderMobility, receiverMobility);
        NS_LOG_DEBUG("Propagation: txPower="
                     << txPowerDbm << "dbm, rxPower=" << rxPowerDbm << "dbm, distance="
                     << senderMobility->GetDistanceFrom(receiverMobility) << "m, delay=" << delay);
        // Schedule the receive event
        NS_LOG_INFO("Scheduling reception of the packet");
        Simulator::Schedule(delay,
                            &LoraPhy::StartReceive,
                            phy,
                            packet,
                            rxPowerDbm,
                            sf,
                            duration,
                            frequency);
        // Fire the trace source for sent packet
        m_packetSent(packet);
    }
}

double
LoraChannel::GetRxPower(double txPowerDbm,
                        Ptr<MobilityModel> senderMobility,
                        Ptr<MobilityModel> receiverMobility) const
{
    NS_LOG_FUNCTION(this << txPowerDbm << senderMobility << receiverMobility);
    return m_loss->CalcRxPower(txPowerDbm, senderMobility, receiverMobility);
}

} // namespace lorawan
} // namespace ns3
