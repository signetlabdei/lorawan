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

SubBand::SubBand(double firstFrequency,
                 double lastFrequency,
                 double dutyCycle,
                 double maxTxPowerDbm)
    : m_firstFrequency(firstFrequency),
      m_lastFrequency(lastFrequency),
      m_dutyCycle(dutyCycle),
      m_nextTransmissionTime(Time(0)),
      m_maxTxPowerDbm(maxTxPowerDbm)
{
    NS_LOG_FUNCTION(this << firstFrequency << lastFrequency << dutyCycle << maxTxPowerDbm);
}

double
SubBand::GetFirstFrequency() const
{
    return m_firstFrequency;
}

double
SubBand::GetLastFrequency() const
{
    return m_lastFrequency;
}

double
SubBand::GetDutyCycle() const
{
    return m_dutyCycle;
}

bool
SubBand::Contains(double frequency) const
{
    return (frequency > m_firstFrequency) && (frequency < m_lastFrequency);
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
