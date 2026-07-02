/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "lora-channel.h"

#include "lora-phy.h"

#include "ns3/node.h"
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
}

LoraChannel::~LoraChannel()
{
    m_phyList.clear();
}

LoraChannel::LoraChannel(Ptr<PropagationLossModel> loss, Ptr<PropagationDelayModel> delay)
    : m_loss(loss),
      m_delay(delay)
{
}

void
LoraChannel::Add(Ptr<LoraPhy> phy)
{
    NS_LOG_FUNCTION(this << phy);

    // Add the new phy to the vector
    m_phyList.push_back(phy);
}

void
LoraChannel::Remove(Ptr<LoraPhy> phy)
{
    NS_LOG_FUNCTION(this << phy);

    // Remove the phy from the vector
    m_phyList.erase(find(m_phyList.begin(), m_phyList.end(), phy));
}

std::size_t
LoraChannel::GetNDevices() const
{
    return m_phyList.size();
}

Ptr<NetDevice>
LoraChannel::GetDevice(std::size_t i) const
{
    return m_phyList[i]->GetDevice();
}

void
LoraChannel::Send(Ptr<LoraPhy> sender,
                  Ptr<Packet> packet,
                  uint32_t frequencyHz,
                  IQPolarity iqPolarity,
                  const LoraTxParameters& txParams,
                  double txPowerDbm,
                  Time duration) const
{
    NS_LOG_FUNCTION(this << sender << packet << frequencyHz << iqPolarity << txParams << txPowerDbm
                         << duration);

    // Get the mobility model of the sender
    auto senderMobility = sender->GetMobility();
    NS_ASSERT(senderMobility); // Make sure it's available
    NS_LOG_INFO("Sender mobility: " << senderMobility->GetPosition());

    // Cycle over all registered PHYs
    NS_LOG_INFO("Starting cycle over all " << m_phyList.size() << " PHYs");
    for (const auto& receiver : m_phyList)
    {
        // Do not deliver to the sender
        if (sender != receiver)
        {
            // Get the receiver's mobility model
            auto receiverMobility = receiver->GetMobility();
            NS_ASSERT(receiverMobility); // Make sure it's available
            NS_LOG_INFO("Receiver mobility: " << receiverMobility->GetPosition());

            // Compute delay using the delay model
            Time delay = m_delay->GetDelay(senderMobility, receiverMobility);

            // Compute received power using the loss model
            double rxPowerDbm = GetRxPower(txPowerDbm, senderMobility, receiverMobility);

            NS_LOG_DEBUG("Propagation: " << "txPower=" << txPowerDbm << "dbm, "
                                         << "rxPower=" << rxPowerDbm << "dbm, "
                                         << "distance="
                                         << senderMobility->GetDistanceFrom(receiverMobility)
                                         << "m, " << "delay=" << delay);

            // Get the id of the destination PHY to correctly format the context
            uint32_t dstNode = 0;
            if (auto dstNetDevice = receiver->GetDevice(); dstNetDevice)
            {
                NS_LOG_INFO("Getting node index from NetDevice, since it exists");
                dstNode = dstNetDevice->GetNode()->GetId();
                NS_LOG_DEBUG("dstNode=" << dstNode);
            }
            else
            {
                NS_LOG_INFO("No net device connected to the PHY, using context 0");
            }

            // Create the parameters object based on the calculations above
            LoraChannelParameters parameters;
            parameters.frequencyHz = frequencyHz;
            parameters.iqPolarity = iqPolarity;
            parameters.spreadingFactor = txParams.spreadingFactor;
            parameters.rxPowerDbm = rxPowerDbm;
            parameters.duration = duration;

            // Schedule the receive event
            NS_LOG_INFO("Scheduling reception of the packet");
            Simulator::ScheduleWithContext(dstNode,
                                           delay,
                                           &LoraChannel::Receive,
                                           this,
                                           receiver,
                                           packet,
                                           parameters);
            // Fire the trace source for sent packet
            m_packetSent(packet);
        }
    }
}

double
LoraChannel::GetRxPower(double txPowerDbm,
                        Ptr<MobilityModel> senderMobility,
                        Ptr<MobilityModel> receiverMobility) const
{
    return m_loss->CalcRxPower(txPowerDbm, senderMobility, receiverMobility);
}

void
LoraChannel::Receive(Ptr<LoraPhy> receiver,
                     Ptr<Packet> packet,
                     const LoraChannelParameters& parameters) const
{
    NS_LOG_FUNCTION(this << receiver << packet << parameters);
    // Call the appropriate PHY instance to let it begin reception
    receiver->StartReceive(packet,
                           parameters.frequencyHz,
                           parameters.iqPolarity,
                           parameters.spreadingFactor,
                           parameters.rxPowerDbm,
                           parameters.duration);
}

std::ostream&
operator<<(std::ostream& os, const LoraChannel::LoraChannelParameters& params)
{
    os << "LoraChannelParameters("
       << "frequencyHz=" << params.frequencyHz << " Hz, "
       << "IQPolarity=" << params.iqPolarity << ", "
       << "spreadingFactor=" << unsigned(params.spreadingFactor) << ", "
       << "rxPowerDbm=" << params.rxPowerDbm << " dBm, "
       << "duration=" << params.duration.As(Time::MS) << ")";
    return os;
}

} // namespace lorawan
} // namespace ns3
