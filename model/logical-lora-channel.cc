/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "logical-lora-channel.h"

#include "ns3/log.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LogicalLoraChannel");

NS_OBJECT_ENSURE_REGISTERED(LogicalLoraChannel);

TypeId
LogicalLoraChannel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LogicalLoraChannel").SetParent<Object>().SetGroupName("lorawan");
    return tid;
}

LogicalLoraChannel::LogicalLoraChannel()
    : m_frequency(0),
      m_minDataRate(0),
      m_maxDataRate(5),
      m_enabledForUplink(true)
{
    NS_LOG_FUNCTION(this);
}

LogicalLoraChannel::~LogicalLoraChannel()
{
    NS_LOG_FUNCTION(this);
}

LogicalLoraChannel::LogicalLoraChannel(double frequency)
    : m_frequency(frequency),
      m_enabledForUplink(true)
{
    NS_LOG_FUNCTION(this);
}

LogicalLoraChannel::LogicalLoraChannel(double frequency, uint8_t minDataRate, uint8_t maxDataRate)
    : m_frequency(frequency),
      m_minDataRate(minDataRate),
      m_maxDataRate(maxDataRate),
      m_enabledForUplink(true)
{
    NS_LOG_FUNCTION(this);
}

double
LogicalLoraChannel::GetFrequency() const
{
    return m_frequency;
}

void
LogicalLoraChannel::SetMinimumDataRate(uint8_t minDataRate)
{
    m_minDataRate = minDataRate;
}

void
LogicalLoraChannel::SetMaximumDataRate(uint8_t maxDataRate)
{
    m_maxDataRate = maxDataRate;
}

uint8_t
LogicalLoraChannel::GetMinimumDataRate() const
{
    return m_minDataRate;
}

uint8_t
LogicalLoraChannel::GetMaximumDataRate() const
{
    return m_maxDataRate;
}

void
LogicalLoraChannel::SetEnabledForUplink()
{
    m_enabledForUplink = true;
}

void
LogicalLoraChannel::DisableForUplink()
{
    m_enabledForUplink = false;
}

bool
LogicalLoraChannel::IsEnabledForUplink() const
{
    return m_enabledForUplink;
}

bool
operator==(const Ptr<LogicalLoraChannel>& first, const Ptr<LogicalLoraChannel>& second)
{
    double thisFreq = first->GetFrequency();
    double otherFreq = second->GetFrequency();

    NS_LOG_DEBUG("Checking equality between logical lora channels: " << thisFreq << " "
                                                                     << otherFreq);

    NS_LOG_DEBUG("Result:" << (thisFreq == otherFreq));
    return (thisFreq == otherFreq);
}

bool
operator!=(const Ptr<LogicalLoraChannel>& first, const Ptr<LogicalLoraChannel>& second)
{
    return !(first == second);
}
} // namespace lorawan
} // namespace ns3
