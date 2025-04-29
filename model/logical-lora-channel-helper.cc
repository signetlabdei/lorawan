/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "logical-lora-channel-helper.h"

#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LogicalLoraChannelHelper");

LogicalLoraChannelHelper::LogicalLoraChannelHelper(uint8_t size)
    : m_channelVec(size)
{
    NS_LOG_FUNCTION(this);
}

LogicalLoraChannelHelper::~LogicalLoraChannelHelper()
{
    NS_LOG_FUNCTION(this);
    m_subBandList.clear();
    m_channelVec.clear();
}

std::vector<Ptr<LogicalLoraChannel>>
LogicalLoraChannelHelper::GetRawChannelArray() const
{
    NS_LOG_FUNCTION(this);
    return m_channelVec;
}

Ptr<SubBand>
LogicalLoraChannelHelper::GetSubBandFromFrequency(double frequencyMHz) const
{
    NS_LOG_FUNCTION(this << frequencyMHz);
    for (const auto& sb : m_subBandList)
    {
        if (sb->Contains(frequencyMHz))
        {
            return sb;
        }
    }
    NS_LOG_ERROR("[ERROR] Requested frequency " << frequencyMHz << "MHz outside known sub-bands.");
    return nullptr; // If no SubBand is found, return nullptr
}

void
LogicalLoraChannelHelper::SetChannel(uint8_t chIndex, Ptr<LogicalLoraChannel> channel)

{
    NS_LOG_FUNCTION(this << unsigned(chIndex) << channel);
    NS_ASSERT_MSG(m_channelVec.size() > chIndex, "ChIndex > channel storage bounds");
    m_channelVec.at(chIndex) = channel;
}

void
LogicalLoraChannelHelper::AddSubBand(Ptr<SubBand> subBand)
{
    NS_LOG_FUNCTION(this << subBand);
    m_subBandList.emplace_back(subBand);
}

Time
LogicalLoraChannelHelper::GetWaitingTime(Ptr<LogicalLoraChannel> channel) const
{
    NS_LOG_FUNCTION(this << channel);
    return GetWaitingTime(channel->GetFrequency());
}

Time
LogicalLoraChannelHelper::GetWaitingTime(double frequencyMHz) const
{
    NS_LOG_FUNCTION(this << frequencyMHz);
    auto subBand = GetSubBandFromFrequency(frequencyMHz);
    NS_ASSERT_MSG(subBand, "Input frequency is out-of-band");
    Time waitingTime = subBand->GetNextTransmissionTime() - Now();
    waitingTime = Max(waitingTime, Time(0)); // Handle negative values
    NS_LOG_DEBUG("waitingTime=" << waitingTime.As(Time::S));
    return waitingTime;
}

void
LogicalLoraChannelHelper::AddEvent(Time duration, Ptr<LogicalLoraChannel> channel)
{
    NS_LOG_FUNCTION(this << channel);
    AddEvent(duration, channel->GetFrequency());
}

void
LogicalLoraChannelHelper::AddEvent(Time duration, double frequencyMHz)
{
    NS_LOG_FUNCTION(this << duration << frequencyMHz);
    NS_LOG_DEBUG("frequency=" << frequencyMHz << "MHz, timeOnAir=" << duration.As(Time::S));
    auto subBand = GetSubBandFromFrequency(frequencyMHz);
    NS_ASSERT_MSG(subBand, "Input frequency is out-of-band");
    Time nextTxTime = Now() + duration / subBand->GetDutyCycle();
    subBand->SetNextTransmissionTime(nextTxTime);
    NS_LOG_DEBUG("now=" << Now().As(Time::S) << ", nextTxTime=" << nextTxTime.As(Time::S));
}

double
LogicalLoraChannelHelper::GetTxPowerForChannel(Ptr<LogicalLoraChannel> channel) const
{
    NS_LOG_FUNCTION(this << channel);
    return GetTxPowerForChannel(channel->GetFrequency());
}

double
LogicalLoraChannelHelper::GetTxPowerForChannel(double frequencyMHz) const
{
    NS_LOG_FUNCTION(this << frequencyMHz);
    auto subBand = GetSubBandFromFrequency(frequencyMHz);
    NS_ASSERT_MSG(subBand, "Input frequency is out-of-band");
    return subBand->GetMaxTxPowerDbm();
}

bool
LogicalLoraChannelHelper::IsFrequencyValid(double frequencyMHz) const
{
    return (bool)(GetSubBandFromFrequency(frequencyMHz));
}

} // namespace lorawan
} // namespace ns3
