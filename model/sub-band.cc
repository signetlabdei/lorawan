/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "sub-band.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("SubBand");

SubBand::SubBand(uint32_t firstFrequencyHz,
                 uint32_t lastFrequencyHz,
                 double dutyCycle,
                 double maxTxPowerDbm)
    : m_firstFrequencyHz(firstFrequencyHz),
      m_lastFrequencyHz(lastFrequencyHz),
      m_dutyCycle(dutyCycle),
      m_nextTransmissionTime(Time(0)),
      m_maxTxPowerDbm(maxTxPowerDbm)
{
    NS_LOG_FUNCTION(this << firstFrequencyHz << lastFrequencyHz << dutyCycle << maxTxPowerDbm);
}

uint32_t
SubBand::GetFirstFrequency() const
{
    return m_firstFrequencyHz;
}

uint32_t
SubBand::GetLastFrequency() const
{
    return m_lastFrequencyHz;
}

double
SubBand::GetDutyCycle() const
{
    return m_dutyCycle;
}

bool
SubBand::Contains(uint32_t frequencyHz) const
{
    return (frequencyHz > m_firstFrequencyHz) && (frequencyHz < m_lastFrequencyHz);
}

bool
SubBand::Contains(Ptr<const LogicalLoraChannel> logicalChannel) const
{
    return Contains(logicalChannel->GetFrequency());
}

void
SubBand::SetNextTransmissionTime(Time nextTime)
{
    m_nextTransmissionTime = nextTime;
}

Time
SubBand::GetNextTransmissionTime()
{
    return m_nextTransmissionTime;
}

void
SubBand::SetMaxTxPowerDbm(double maxTxPowerDbm)
{
    m_maxTxPowerDbm = maxTxPowerDbm;
}

double
SubBand::GetMaxTxPowerDbm() const
{
    return m_maxTxPowerDbm;
}
} // namespace lorawan
} // namespace ns3
