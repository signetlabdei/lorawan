/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "logical-lora-channel-helper.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LogicalLoraChannelHelper");

NS_OBJECT_ENSURE_REGISTERED(LogicalLoraChannelHelper);

TypeId
LogicalLoraChannelHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LogicalLoraChannelHelper").SetParent<Object>().SetGroupName("lorawan");
    return tid;
}

LogicalLoraChannelHelper::LogicalLoraChannelHelper()
    : m_nextAggregatedTransmissionTime(Seconds(0)),
      m_aggregatedDutyCycle(1)
{
    NS_LOG_FUNCTION(this);
}

LogicalLoraChannelHelper::~LogicalLoraChannelHelper()
{
    NS_LOG_FUNCTION(this);
}

std::vector<Ptr<LogicalLoraChannel>>
LogicalLoraChannelHelper::GetChannelList()
{
    NS_LOG_FUNCTION(this);

    // Make a copy of the channel vector
    std::vector<Ptr<LogicalLoraChannel>> vector;
    vector.reserve(m_channelList.size());
    std::copy(m_channelList.begin(), m_channelList.end(), std::back_inserter(vector));

    return vector;
}

std::vector<Ptr<LogicalLoraChannel>>
LogicalLoraChannelHelper::GetEnabledChannelList()
{
    NS_LOG_FUNCTION(this);

    // Make a copy of the channel vector
    std::vector<Ptr<LogicalLoraChannel>> vector;
    vector.reserve(m_channelList.size());
    std::copy(m_channelList.begin(),
              m_channelList.end(),
              std::back_inserter(vector)); // Working on a copy

    std::vector<Ptr<LogicalLoraChannel>> channels;
    std::vector<Ptr<LogicalLoraChannel>>::iterator it;
    for (it = vector.begin(); it != vector.end(); it++)
    {
        if ((*it)->IsEnabledForUplink())
        {
            channels.push_back(*it);
        }
    }

    return channels;
}

Ptr<SubBand>
LogicalLoraChannelHelper::GetSubBandFromChannel(Ptr<LogicalLoraChannel> channel)
{
    return GetSubBandFromFrequency(channel->GetFrequency());
}

Ptr<SubBand>
LogicalLoraChannelHelper::GetSubBandFromFrequency(uint32_t frequencyHz)
{
    // Get the SubBand this frequency belongs to
    std::list<Ptr<SubBand>>::iterator it;
    for (it = m_subBandList.begin(); it != m_subBandList.end(); it++)
    {
        if ((*it)->BelongsToSubBand(frequencyHz))
        {
            return *it;
        }
    }

    NS_LOG_ERROR("Requested frequency: " << frequencyHz);
    NS_ABORT_MSG("Warning: frequency is outside any known SubBand.");

    return nullptr; // If no SubBand is found, return 0
}

void
LogicalLoraChannelHelper::AddChannel(uint32_t frequencyHz)
{
    NS_LOG_FUNCTION(this << frequencyHz);

    // Create the new channel and increment the counter
    Ptr<LogicalLoraChannel> channel = Create<LogicalLoraChannel>(frequencyHz);

    // Add it to the list
    m_channelList.push_back(channel);

    NS_LOG_DEBUG("Added a channel. Current number of channels in list is " << m_channelList.size());
}

void
LogicalLoraChannelHelper::AddChannel(Ptr<LogicalLoraChannel> logicalChannel)
{
    NS_LOG_FUNCTION(this << logicalChannel);

    // Add it to the list
    m_channelList.push_back(logicalChannel);
}

void
LogicalLoraChannelHelper::SetChannel(uint8_t chIndex, Ptr<LogicalLoraChannel> logicalChannel)

{
    NS_LOG_FUNCTION(this << chIndex << logicalChannel);

    m_channelList.at(chIndex) = logicalChannel;
}

void
LogicalLoraChannelHelper::AddSubBand(uint32_t firstFrequencyHz,
                                     uint32_t lastFrequencyHz,
                                     double dutyCycle,
                                     double maxTxPowerDbm)
{
    NS_LOG_FUNCTION(this << firstFrequencyHz << lastFrequencyHz);

    Ptr<SubBand> subBand =
        Create<SubBand>(firstFrequencyHz, lastFrequencyHz, dutyCycle, maxTxPowerDbm);

    m_subBandList.push_back(subBand);
}

void
LogicalLoraChannelHelper::AddSubBand(Ptr<SubBand> subBand)
{
    NS_LOG_FUNCTION(this << subBand);

    m_subBandList.push_back(subBand);
}

void
LogicalLoraChannelHelper::RemoveChannel(Ptr<LogicalLoraChannel> logicalChannel)
{
    // Search and remove the channel from the list
    std::vector<Ptr<LogicalLoraChannel>>::iterator it;
    for (it = m_channelList.begin(); it != m_channelList.end(); it++)
    {
        Ptr<LogicalLoraChannel> currentChannel = *it;
        if (currentChannel == logicalChannel)
        {
            m_channelList.erase(it);
            return;
        }
    }
}

Time
LogicalLoraChannelHelper::GetAggregatedWaitingTime()
{
    // Aggregate waiting time
    Time aggregatedWaitingTime = m_nextAggregatedTransmissionTime - Simulator::Now();

    // Handle case in which waiting time is negative
    aggregatedWaitingTime = Seconds(std::max(aggregatedWaitingTime.GetSeconds(), double(0)));

    NS_LOG_DEBUG("Aggregated waiting time: " << aggregatedWaitingTime.GetSeconds());

    return aggregatedWaitingTime;
}

Time
LogicalLoraChannelHelper::GetWaitingTime(Ptr<LogicalLoraChannel> channel)
{
    NS_LOG_FUNCTION(this << channel);

    // SubBand waiting time
    Time subBandWaitingTime =
        GetSubBandFromChannel(channel)->GetNextTransmissionTime() - Simulator::Now();

    // Handle case in which waiting time is negative
    subBandWaitingTime = Seconds(std::max(subBandWaitingTime.GetSeconds(), double(0)));

    NS_LOG_DEBUG("Waiting time: " << subBandWaitingTime.GetSeconds());

    return subBandWaitingTime;
}

void
LogicalLoraChannelHelper::AddEvent(Time duration, Ptr<LogicalLoraChannel> channel)
{
    NS_LOG_FUNCTION(this << duration << channel);

    Ptr<SubBand> subBand = GetSubBandFromChannel(channel);

    double dutyCycle = subBand->GetDutyCycle();
    double timeOnAir = duration.GetSeconds();

    // Computation of necessary waiting time on this sub-band
    subBand->SetNextTransmissionTime(Simulator::Now() + Seconds(timeOnAir / dutyCycle - timeOnAir));

    // Computation of necessary aggregate waiting time
    m_nextAggregatedTransmissionTime =
        Simulator::Now() + Seconds(timeOnAir / m_aggregatedDutyCycle - timeOnAir);

    NS_LOG_DEBUG("Time on air: " << timeOnAir);
    NS_LOG_DEBUG("m_aggregatedDutyCycle: " << m_aggregatedDutyCycle);
    NS_LOG_DEBUG("Current time: " << Simulator::Now().GetSeconds());
    NS_LOG_DEBUG("Next transmission on this sub-band allowed at time: "
                 << (subBand->GetNextTransmissionTime()).GetSeconds());
    NS_LOG_DEBUG("Next aggregated transmission allowed at time "
                 << m_nextAggregatedTransmissionTime.GetSeconds());
}

double
LogicalLoraChannelHelper::GetTxPowerForChannel(Ptr<LogicalLoraChannel> logicalChannel)
{
    NS_LOG_FUNCTION_NOARGS();

    // Get the maxTxPowerDbm from the SubBand this channel is in
    std::list<Ptr<SubBand>>::iterator it;
    for (it = m_subBandList.begin(); it != m_subBandList.end(); it++)
    {
        // Check whether this channel is in this SubBand
        if ((*it)->BelongsToSubBand(logicalChannel->GetFrequency()))
        {
            return (*it)->GetMaxTxPowerDbm();
        }
    }
    NS_ABORT_MSG("Logical channel doesn't belong to a known SubBand");

    return 0;
}

void
LogicalLoraChannelHelper::DisableChannel(int index)
{
    NS_LOG_FUNCTION(this << index);

    m_channelList.at(index)->DisableForUplink();
}
} // namespace lorawan
} // namespace ns3
