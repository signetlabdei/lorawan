/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "ns3/logical-lora-channel-helper.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LogicalLoraChannelHelper");

NS_OBJECT_ENSURE_REGISTERED(LogicalLoraChannelHelper);

TypeId
LogicalLoraChannelHelper::GetTypeId(void)
{
    static TypeId tid =
        TypeId("ns3::LogicalLoraChannelHelper").SetParent<Object>().SetGroupName("lorawan");
    return tid;
}

LogicalLoraChannelHelper::LogicalLoraChannelHelper()
    : m_lastTxDuration(0),
      m_lastTxStart(0)
{
    NS_LOG_FUNCTION(this);
}

LogicalLoraChannelHelper::~LogicalLoraChannelHelper()
{
    NS_LOG_FUNCTION(this);
}

std::vector<Ptr<LogicalLoraChannel>>
LogicalLoraChannelHelper::GetChannelList(void)
{
    NS_LOG_FUNCTION(this);

    std::vector<Ptr<LogicalLoraChannel>> vector;
    for (auto& llc : m_channelList)
        vector.push_back(llc.second);

    return vector;
}

std::vector<Ptr<LogicalLoraChannel>>
LogicalLoraChannelHelper::GetEnabledChannelList(void)
{
    NS_LOG_FUNCTION(this);

    std::vector<Ptr<LogicalLoraChannel>> vector;
    for (auto& llc : m_channelList)
        if (llc.second->IsEnabledForUplink())
            vector.push_back(llc.second);

    return vector;
}

Ptr<SubBand>
LogicalLoraChannelHelper::GetSubBandFromChannel(const Ptr<LogicalLoraChannel> channel)
{
    return GetSubBandFromFrequency(channel->GetFrequency());
}

Ptr<SubBand>
LogicalLoraChannelHelper::GetSubBandFromFrequency(double frequency)
{
    // Get the SubBand this frequency belongs to
    for (auto& sub : m_subBandList)
        if (sub->BelongsToSubBand(frequency))
            return sub;

    NS_LOG_ERROR("Requested frequency: " << frequency);
    NS_ABORT_MSG("Warning: frequency is outside any known SubBand.");

    return 0; // If no SubBand is found, return 0
}

void
LogicalLoraChannelHelper::AddChannel(uint16_t chIndex, Ptr<LogicalLoraChannel> logicalChannel)
{
    NS_LOG_FUNCTION(this << (unsigned)chIndex << logicalChannel);
    m_channelList[chIndex] = logicalChannel;
}

void
LogicalLoraChannelHelper::AddSubBand(double firstFrequency,
                                     double lastFrequency,
                                     double dutyCycle,
                                     double maxTxPowerDbm)
{
    NS_LOG_FUNCTION(this << firstFrequency << lastFrequency);
    Ptr<SubBand> subBand = Create<SubBand>(firstFrequency, lastFrequency, dutyCycle, maxTxPowerDbm);
    AddSubBand(subBand);
}

void
LogicalLoraChannelHelper::AddSubBand(Ptr<SubBand> subBand)
{
    NS_LOG_FUNCTION(this << subBand);

    m_subBandList.push_back(subBand);
}

void
LogicalLoraChannelHelper::RemoveChannel(uint16_t chIndex)
{
    // Search and remove the channel from the list
    m_channelList.erase(chIndex);
}

Time
LogicalLoraChannelHelper::GetAggregatedWaitingTime(double aggregatedDutyCycle)
{
    NS_LOG_FUNCTION("Aggregated duty-cycle: " + std::to_string(aggregatedDutyCycle));

    // Aggregate waiting time
    Time nextTransmissionTime = (aggregatedDutyCycle)
                                    ? m_lastTxStart + m_lastTxDuration / aggregatedDutyCycle
                                    : Time::Max();
    Time aggregatedWaitingTime = nextTransmissionTime - Simulator::Now();

    // Handle case in which waiting time is negative
    aggregatedWaitingTime = Max(aggregatedWaitingTime, Seconds(0));

    NS_LOG_DEBUG("Aggregated waiting time: " << aggregatedWaitingTime.As(Time::S));

    return aggregatedWaitingTime;
}

Time
LogicalLoraChannelHelper::GetWaitingTime(const Ptr<LogicalLoraChannel> channel)
{
    NS_LOG_FUNCTION(this << channel);

    // SubBand waiting time
    Time subBandWaitingTime =
        GetSubBandFromChannel(channel)->GetNextTransmissionTime() - Simulator::Now();

    // Handle case in which waiting time is negative
    subBandWaitingTime = Max(subBandWaitingTime, Seconds(0));

    NS_LOG_DEBUG("Waiting time: " << subBandWaitingTime.GetSeconds());

    return subBandWaitingTime;
}

void
LogicalLoraChannelHelper::AddEvent(Time duration, Ptr<LogicalLoraChannel> channel)
{
    NS_LOG_FUNCTION(this << duration << channel);

    auto subBand = GetSubBandFromChannel(channel);

    double dutyCycle = subBand->GetDutyCycle();
    m_lastTxDuration = duration;
    m_lastTxStart = Simulator::Now();

    // Computation of necessary waiting time on this sub-band
    subBand->SetNextTransmissionTime(Simulator::Now() + m_lastTxDuration / dutyCycle);

    NS_LOG_DEBUG("Time on air: " << m_lastTxDuration.As(Time::MS));
    NS_LOG_DEBUG("Current time: " << Simulator::Now().As(Time::S));
    NS_LOG_DEBUG("Next transmission on this sub-band allowed at time: "
                 << (subBand->GetNextTransmissionTime()).As(Time::S));
}

double
LogicalLoraChannelHelper::GetTxPowerForChannel(Ptr<LogicalLoraChannel> logicalChannel)
{
    NS_LOG_FUNCTION_NOARGS();

    // Get the maxTxPowerDbm from the SubBand this channel is in
    for (const auto& sub : m_subBandList)
    {
        // Check whether this channel is in this SubBand
        if (sub->BelongsToSubBand(logicalChannel->GetFrequency()))
            return sub->GetMaxTxPowerDbm();
    }

    NS_ABORT_MSG("Logical channel doesn't belong to a known SubBand");
}

void
LogicalLoraChannelHelper::DisableChannel(uint16_t chIndex)
{
    NS_LOG_FUNCTION(this << (unsigned)chIndex);
    m_channelList.at(chIndex)->DisableForUplink();
}

} // namespace lorawan
} // namespace ns3
