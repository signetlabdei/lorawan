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

SubBand::SubBand(uint32_t firstFrequencyHz,
                 uint32_t lastFrequencyHz,
                 double dutyCycle,
                 double maxTxPowerDbm)
    : m_firstFrequencyHz(firstFrequencyHz),
      m_lastFrequencyHz(lastFrequencyHz),
      m_dutyCycle(dutyCycle),
      m_nextTransmissionTime(Seconds(0)),
      m_maxTxPowerDbm(maxTxPowerDbm)
{
    NS_LOG_FUNCTION(this << firstFrequencyHz << lastFrequencyHz << dutyCycle << maxTxPowerDbm);
}

SubBand::~SubBand()
{
    NS_LOG_FUNCTION(this);
}

uint32_t
SubBand::GetFirstFrequency() const
{
    return m_firstFrequencyHz;
}

double
SubBand::GetDutyCycle() const
{
    return m_dutyCycle;
}

bool
SubBand::BelongsToSubBand(uint32_t frequencyHz) const
{
    return (frequencyHz > m_firstFrequencyHz) && (frequencyHz < m_lastFrequencyHz);
}

bool
SubBand::BelongsToSubBand(Ptr<LogicalLoraChannel> logicalChannel) const
{
    uint32_t frequency = logicalChannel->GetFrequency();
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
