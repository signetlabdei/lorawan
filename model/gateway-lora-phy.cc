/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "gateway-lora-phy.h"

#include "lora-tag.h"

#include "ns3/log-macros-enabled.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("GatewayLoraPhy");

NS_OBJECT_ENSURE_REGISTERED(GatewayLoraPhy);

/**************************************
 *    ReceptionPath implementation    *
 **************************************/
GatewayLoraPhy::ReceptionPath::ReceptionPath()
    : m_available(true),
      m_event(nullptr),
      m_endReceiveEventId(EventId())
{
    NS_LOG_FUNCTION_NOARGS();
}

GatewayLoraPhy::ReceptionPath::~ReceptionPath()
{
    NS_LOG_FUNCTION_NOARGS();
}

bool
GatewayLoraPhy::ReceptionPath::IsAvailable() const
{
    return m_available;
}

void
GatewayLoraPhy::ReceptionPath::Free()
{
    m_available = true;
    m_event = nullptr;
    m_endReceiveEventId = EventId();
}

void
GatewayLoraPhy::ReceptionPath::LockOnEvent(Ptr<LoraInterferenceHelper::Event> event)
{
    m_available = false;
    m_event = event;
}

void
GatewayLoraPhy::ReceptionPath::SetEvent(Ptr<LoraInterferenceHelper::Event> event)
{
    m_event = event;
}

Ptr<LoraInterferenceHelper::Event>
GatewayLoraPhy::ReceptionPath::GetEvent()
{
    return m_event;
}

EventId
GatewayLoraPhy::ReceptionPath::GetEndReceive()
{
    return m_endReceiveEventId;
}

void
GatewayLoraPhy::ReceptionPath::SetEndReceive(EventId endReceiveEventId)
{
    m_endReceiveEventId = endReceiveEventId;
}

/***********************************************************************
 *                 Implementation of gateway methods                   *
 ***********************************************************************/

TypeId
GatewayLoraPhy::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::GatewayLoraPhy")
            .SetParent<LoraPhy>()
            .SetGroupName("lorawan")
            .AddTraceSource(
                "NoReceptionBecauseTransmitting",
                "Trace source indicating a packet "
                "could not be correctly received because"
                "the gateway is in transmission mode",
                MakeTraceSourceAccessor(&GatewayLoraPhy::m_noReceptionBecauseTransmitting),
                "ns3::Packet::TracedCallback")
            .AddTraceSource("LostPacketBecauseNoMoreReceivers",
                            "Trace source indicating a packet "
                            "could not be correctly received because"
                            "there are no more demodulators available",
                            MakeTraceSourceAccessor(&GatewayLoraPhy::m_noMoreDemodulators),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("OccupiedReceptionPaths",
                            "Number of currently occupied reception paths",
                            MakeTraceSourceAccessor(&GatewayLoraPhy::m_occupiedReceptionPaths),
                            "ns3::TracedValueCallback::Int");
    return tid;
}

GatewayLoraPhy::GatewayLoraPhy()
    : m_isTransmitting(false)
{
    NS_LOG_FUNCTION_NOARGS();
}

GatewayLoraPhy::~GatewayLoraPhy()
{
    NS_LOG_FUNCTION_NOARGS();
}

// Uplink sensitivity (Source: SX1301 datasheet)
// {SF7, SF8, SF9, SF10, SF11, SF12}
const double GatewayLoraPhy::sensitivity[6] = {-130.0, -132.5, -135.0, -137.5, -140.0, -142.5};

void
GatewayLoraPhy::AddReceptionPath()
{
    NS_LOG_FUNCTION_NOARGS();

    m_receptionPaths.push_back(Create<GatewayLoraPhy::ReceptionPath>());
}

void
GatewayLoraPhy::ResetReceptionPaths()
{
    NS_LOG_FUNCTION(this);

    m_receptionPaths.clear();
}

void
GatewayLoraPhy::TxFinished(Ptr<const Packet> packet)
{
    m_isTransmitting = false;
}

bool
GatewayLoraPhy::IsTransmitting()
{
    return m_isTransmitting;
}

void
GatewayLoraPhy::AddFrequency(uint32_t frequencyHz)
{
    NS_LOG_FUNCTION(this << frequencyHz);

    m_frequenciesHz.push_back(frequencyHz);

    NS_ASSERT(m_frequenciesHz.size() <= 8);
}

bool
GatewayLoraPhy::IsOnFrequency(uint32_t frequencyHz)
{
    NS_LOG_FUNCTION(this << frequencyHz);

    // Look into our list of frequencies
    for (auto& f : m_frequenciesHz)
    {
        if (f == frequencyHz)
        {
            return true;
        }
    }
    return false;
}
} // namespace lorawan
} // namespace ns3
