/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "end-device-lora-phy.h"

#include "lora-tag.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

#include <algorithm>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("EndDeviceLoraPhy");

NS_OBJECT_ENSURE_REGISTERED(EndDeviceLoraPhy);

/**************************
 *  Listener destructor  *
 *************************/

EndDeviceLoraPhyListener::~EndDeviceLoraPhyListener()
{
}

TypeId
EndDeviceLoraPhy::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::EndDeviceLoraPhy")
            .SetParent<LoraPhy>()
            .SetGroupName("lorawan")
            .AddTraceSource("LostPacketBecauseWrongFrequency",
                            "Trace source indicating a packet "
                            "could not be correctly decoded because"
                            "the end device was listening on a different frequency",
                            MakeTraceSourceAccessor(&EndDeviceLoraPhy::m_wrongFrequency),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("LostPacketBecauseWrongSpreadingFactor",
                            "Trace source indicating a packet "
                            "could not be correctly decoded because"
                            "the end device was listening for a different Spreading Factor",
                            MakeTraceSourceAccessor(&EndDeviceLoraPhy::m_wrongSf),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("EndDeviceState",
                            "The current state of the device",
                            MakeTraceSourceAccessor(&EndDeviceLoraPhy::m_state),
                            "ns3::TracedValueCallback::EndDeviceLoraPhy::State");
    return tid;
}

// Initialize the device with some common settings.
// These will then be changed by helpers.
EndDeviceLoraPhy::EndDeviceLoraPhy()
    : m_state(SLEEP),
      m_frequencyHz(868100000),
      m_sf(7)
{
}

EndDeviceLoraPhy::~EndDeviceLoraPhy()
{
}

// Downlink sensitivity (from SX1272 datasheet)
// {SF7, SF8, SF9, SF10, SF11, SF12}
// These sensitivities are for a bandwidth of 125000 Hz
const double EndDeviceLoraPhy::sensitivity[6] = {-124, -127, -130, -133, -135, -137};

void
EndDeviceLoraPhy::SetSpreadingFactor(uint8_t sf)
{
    m_sf = sf;
}

uint8_t
EndDeviceLoraPhy::GetSpreadingFactor() const
{
    return m_sf;
}

bool
EndDeviceLoraPhy::IsTransmitting()
{
    return m_state == TX;
}

bool
EndDeviceLoraPhy::IsOnFrequency(uint32_t frequencyHz)
{
    return m_frequencyHz == frequencyHz;
}

void
EndDeviceLoraPhy::SetFrequency(uint32_t frequencyHz)
{
    m_frequencyHz = frequencyHz;
}

void
EndDeviceLoraPhy::TxFinished(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    // Switch back to STANDBY mode.
    // For reference see SX1272 datasheet, section 4.1.6
    SwitchToStandby();
    // Forward packet to the upper layer (if the callback was set).
    if (!m_txFinishedCallback.IsNull())
    {
        m_txFinishedCallback(packet);
    }
}

void
EndDeviceLoraPhy::SwitchToStandby()
{
    NS_LOG_FUNCTION_NOARGS();

    m_state = STANDBY;

    // Notify listeners of the state change
    for (auto i = m_listeners.begin(); i != m_listeners.end(); i++)
    {
        (*i)->NotifyStandby();
    }
}

void
EndDeviceLoraPhy::SwitchToRx()
{
    NS_LOG_FUNCTION_NOARGS();

    NS_ASSERT(m_state == STANDBY);

    m_state = RX;

    // Notify listeners of the state change
    for (auto i = m_listeners.begin(); i != m_listeners.end(); i++)
    {
        (*i)->NotifyRxStart();
    }
}

void
EndDeviceLoraPhy::SwitchToTx(double txPowerDbm)
{
    NS_LOG_FUNCTION_NOARGS();

    NS_ASSERT(m_state != RX);

    m_state = TX;

    // Notify listeners of the state change
    for (auto i = m_listeners.begin(); i != m_listeners.end(); i++)
    {
        (*i)->NotifyTxStart(txPowerDbm);
    }
}

void
EndDeviceLoraPhy::SwitchToSleep()
{
    NS_LOG_FUNCTION_NOARGS();

    NS_ASSERT(m_state == STANDBY);

    m_state = SLEEP;

    // Notify listeners of the state change
    for (auto i = m_listeners.begin(); i != m_listeners.end(); i++)
    {
        (*i)->NotifySleep();
    }
}

EndDeviceLoraPhy::State
EndDeviceLoraPhy::GetState()
{
    NS_LOG_FUNCTION_NOARGS();

    return m_state;
}

void
EndDeviceLoraPhy::RegisterListener(EndDeviceLoraPhyListener* listener)
{
    m_listeners.push_back(listener);
}

void
EndDeviceLoraPhy::UnregisterListener(EndDeviceLoraPhyListener* listener)
{
    auto i = find(m_listeners.begin(), m_listeners.end(), listener);
    if (i != m_listeners.end())
    {
        m_listeners.erase(i);
    }
}

} // namespace lorawan
} // namespace ns3
