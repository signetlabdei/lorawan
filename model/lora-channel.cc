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

#include "ns3/lora-channel.h"
#include "ns3/pointer.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraChannel");

NS_OBJECT_ENSURE_REGISTERED(LoraChannel);

TypeId
LoraChannel::GetTypeId(void)
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
    NS_LOG_FUNCTION_NOARGS();
}

LoraChannel::~LoraChannel()
{
    NS_LOG_FUNCTION_NOARGS();
    m_phyList.clear();
    m_phyListDown.clear();
}

LoraChannel::LoraChannel(Ptr<PropagationLossModel> loss, Ptr<PropagationDelayModel> delay)
    : m_loss(loss),
      m_delay(delay)
{
    NS_LOG_FUNCTION(this << loss << delay);
}

void
LoraChannel::Add(Ptr<LoraPhy> phy, bool down)
{
    NS_LOG_FUNCTION(this << phy);
    // Add the new phy to the right vector
    auto& list = (down) ? m_phyListDown : m_phyList;
    list.push_back(phy);
}

void
LoraChannel::Remove(Ptr<LoraPhy> phy, bool down)
{
    NS_LOG_FUNCTION(this << phy << down);
    // Remove the phy from the right vector
    auto& list = (down) ? m_phyListDown : m_phyList;
    list.erase(find(m_phyList.begin(), m_phyList.end(), phy));
}

std::size_t
LoraChannel::GetNDevices(void) const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_phyList.size();
}

Ptr<NetDevice>
LoraChannel::GetDevice(std::size_t i) const
{
    NS_LOG_FUNCTION(this << i);
    return m_phyList[i]->GetDevice()->GetObject<NetDevice>();
}

void
LoraChannel::Send(Ptr<LoraPhy> sender,
                  Ptr<Packet> packet,
                  double txPowerDbm,
                  LoraTxParameters txParams,
                  Time duration,
                  double frequency,
                  bool down) const
{
    NS_LOG_FUNCTION(this << sender << packet << txPowerDbm << txParams << duration << frequency
                         << down);
    // Determine direction (uplink or downlink)
    auto& receivers = (down) ? m_phyListDown : m_phyList;
    // Get the mobility model of the sender
    auto senderMobility = sender->GetMobility();
    NS_ASSERT(bool(senderMobility) != 0); // Make sure it's available
    NS_LOG_INFO("Starting cycle over " << receivers.size() << " PHYs"
                                       << ((down) ? " in downlink" : " in uplink"));
    NS_LOG_INFO("Sender mobility: " << senderMobility->GetPosition());
    // Cycle over all registered PHYs
    uint32_t j = 0;
    for (auto& phy : receivers)
    {
        if (sender != phy) // Do not deliver to the sender
        {
            // Get the receiver's mobility model
            auto receiverMobility = phy->GetMobility();
            NS_LOG_INFO("Receiver mobility: " << receiverMobility->GetPosition());
            // Compute delay using the delay model
            Time delay = m_delay->GetDelay(senderMobility, receiverMobility);
            // Compute received power using the loss model
            double rxPowerDbm = GetRxPower(txPowerDbm, senderMobility, receiverMobility);
            NS_LOG_DEBUG("Propagation: txPower="
                         << txPowerDbm << "dbm, rxPower=" << rxPowerDbm
                         << "dbm, distance=" << senderMobility->GetDistanceFrom(receiverMobility)
                         << "m, delay=" << delay);
            // Create the parameters object based on the calculations above
            LoraChannelParameters parameters = {rxPowerDbm, txParams.sf, duration, frequency};
            // Schedule the receive event
            NS_LOG_INFO("Scheduling reception of the packet");
            Simulator::Schedule(delay, &LoraChannel::Receive, this, j, packet, parameters, down);
            // Fire the trace source for sent packet
            m_packetSent(packet);
        }
        ++j;
    }
}

void
LoraChannel::Receive(uint32_t i,
                     Ptr<Packet> packet,
                     LoraChannelParameters parameters,
                     bool down) const
{
    NS_LOG_FUNCTION(this << i << packet << parameters << down);
    // Determine direction (uplink or downlink)
    auto& receivers = (down) ? m_phyListDown : m_phyList;
    // Call the appropriate PHY instance to let it begin reception
    receivers[i]->StartReceive(packet,
                               parameters.rxPowerDbm,
                               parameters.sf,
                               parameters.duration,
                               parameters.frequency);
}

double
LoraChannel::GetRxPower(double txPowerDbm,
                        Ptr<MobilityModel> senderMobility,
                        Ptr<MobilityModel> receiverMobility) const
{
    NS_LOG_FUNCTION(this << txPowerDbm << senderMobility << receiverMobility);
    return m_loss->CalcRxPower(txPowerDbm, senderMobility, receiverMobility);
}

std::ostream&
operator<<(std::ostream& os, const LoraChannelParameters& params)
{
    os << "(rxPowerDbm: " << params.rxPowerDbm << ", SF: " << unsigned(params.sf)
       << ", durationSec: " << params.duration.GetSeconds() << ", frequency: " << params.frequency
       << ")";
    return os;
}
} // namespace lorawan
} // namespace ns3
