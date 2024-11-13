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

SubBand::SubBand(double firstFrequencyMHz,
                 double lastFrequencyMHz,
                 double dutyCycle,
                 double maxTxPowerDbm)
    : m_firstFrequencyMHz(firstFrequencyMHz),
      m_lastFrequencyMHz(lastFrequencyMHz),
      m_dutyCycle(dutyCycle),
      m_nextTransmissionTime(Time(0)),
      m_maxTxPowerDbm(maxTxPowerDbm)
{
    NS_LOG_FUNCTION(this << firstFrequencyMHz << lastFrequencyMHz << dutyCycle << maxTxPowerDbm);
}

double
SubBand::GetFirstFrequency() const
{
    return m_firstFrequencyMHz;
}

double
SubBand::GetLastFrequency() const
{
    return m_lastFrequencyMHz;
}

double
SubBand::GetDutyCycle() const
{
    return m_dutyCycle;
}

bool
SubBand::Contains(double frequencyMHz) const
{
    return (frequencyMHz > m_firstFrequencyMHz) && (frequencyMHz < m_lastFrequencyMHz);
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
