/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "logical-lora-channel.h"

#include "sub-band.h"

#include "ns3/log.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LogicalLoraChannel");

LogicalLoraChannel::LogicalLoraChannel(uint32_t frequencyHz,
                                       uint8_t minDataRate,
                                       uint8_t maxDataRate)
    : m_frequencyHz(frequencyHz),
      m_minDataRate(minDataRate),
      m_maxDataRate(maxDataRate),
      m_enabledForUplink(true)
{
    NS_LOG_FUNCTION(this);
}

uint32_t
LogicalLoraChannel::GetFrequency() const
{
    return m_frequencyHz;
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
LogicalLoraChannel::EnableForUplink()
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
    uint32_t thisFreq = first->GetFrequency();
    uint32_t otherFreq = second->GetFrequency();

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
