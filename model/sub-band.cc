/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "sub-band.h"

#include "ns3/log.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("SubBand");

NS_OBJECT_ENSURE_REGISTERED(SubBand);

TypeId
SubBand::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SubBand").SetParent<Object>().SetGroupName("lorawan");
    return tid;
}

SubBand::SubBand()
{
    NS_LOG_FUNCTION(this);
}

SubBand::SubBand(double firstFrequency,
                 double lastFrequency,
                 double dutyCycle,
                 double maxTxPowerDbm)
    : m_firstFrequency(firstFrequency),
      m_lastFrequency(lastFrequency),
      m_dutyCycle(dutyCycle),
      m_nextTransmissionTime(Seconds(0)),
      m_maxTxPowerDbm(maxTxPowerDbm)
{
    NS_LOG_FUNCTION(this << firstFrequency << lastFrequency << dutyCycle << maxTxPowerDbm);
}

SubBand::~SubBand()
{
    NS_LOG_FUNCTION(this);
}

double
SubBand::GetFirstFrequency() const
{
    return m_firstFrequency;
}

double
SubBand::GetDutyCycle() const
{
    return m_dutyCycle;
}

bool
SubBand::BelongsToSubBand(double frequency) const
{
    return (frequency > m_firstFrequency) && (frequency < m_lastFrequency);
}

bool
SubBand::BelongsToSubBand(Ptr<LogicalLoraChannel> logicalChannel) const
{
    double frequency = logicalChannel->GetFrequency();
    return BelongsToSubBand(frequency);
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
