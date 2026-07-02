/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "end-device-lora-phy.h"

#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("EndDeviceLoraPhy");

NS_OBJECT_ENSURE_REGISTERED(EndDeviceLoraPhy);

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
            .AddTraceSource("LostPacketBecauseWrongPolarity",
                            "Trace source indicating a packet "
                            "could not be correctly decoded because"
                            "the end device was expecting a different I/Q polarity",
                            MakeTraceSourceAccessor(&EndDeviceLoraPhy::m_wrongPolarity),
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

// Defaults from SX1272/73 datasheet (Rev. 4, Jan. 2019)
EndDeviceLoraPhy::EndDeviceLoraPhy()
    : m_regs{
          .spreadingFactor = 7,
          .bandwidthHz = 125000,
          .codingRate = CodingRate::CR_4_5,
          .lowDataRateOptimize = false,
          .preambleLenSymb = 8,
          .payloadLenBytes = 1,
          .implicitHeader = false,
          .crcEnabled = false,
          .iqPolarity = IQPolarity::UP,
          .frequencyHz = 915'000'000,
          .txPowerDbm = 14,
          .syncWord = 0xF5,
          .symbNumTimeout = 100,
      },
      m_state(State::STANDBY)
{
    NS_LOG_FUNCTION(this);
}

EndDeviceLoraPhy::~EndDeviceLoraPhy()
{
    NS_LOG_FUNCTION(this);
}

// Sensitivity (from SX1272 datasheet)
// {SF7, SF8, SF9, SF10, SF11, SF12}
// These sensitivities are for a bandwidth of 125000 Hz
const double EndDeviceLoraPhy::SENSITIVITY[6] = {-124, -127, -130, -133, -135, -137};

bool
EndDeviceLoraPhy::IsTransmitting() const
{
    NS_LOG_FUNCTION(this);
    return m_state == State::TX;
}

bool
EndDeviceLoraPhy::IsOnFrequency(uint32_t frequencyHz) const
{
    NS_LOG_FUNCTION(this);
    return m_regs.frequencyHz == frequencyHz;
}

EndDeviceLoraPhy::State
EndDeviceLoraPhy::GetState()
{
    NS_LOG_FUNCTION(this);
    return m_state;
}

void
EndDeviceLoraPhy::Sleep()
{
    NS_LOG_FUNCTION(this);
    // Enter SLEEP mode
    RequestSleepMode();
}

void
EndDeviceLoraPhy::ReceiveSingle(uint32_t frequencyHz,
                                IQPolarity iqPolarity,
                                uint8_t spreadingFactor,
                                uint32_t bandwidthHz,
                                uint8_t symbNumTimeout,
                                RxTimeoutCallback rxTimeoutCallback)
{
    NS_LOG_FUNCTION(this);
    // Write hardware registers of the LoRa chip
    m_regs.frequencyHz = frequencyHz;
    m_regs.bandwidthHz = bandwidthHz;
    m_regs.iqPolarity = iqPolarity;
    m_regs.spreadingFactor = spreadingFactor;
    m_regs.symbNumTimeout = symbNumTimeout;
    // Set timeout callback
    m_rxTimeoutCallback = rxTimeoutCallback;
    // Enter RXSINGLE mode
    RequestRxSingleMode();
}

void
EndDeviceLoraPhy::RegisterListener(const std::shared_ptr<EndDeviceLoraPhyListener>& listener)
{
    m_listeners.emplace_back(listener);
}

void
EndDeviceLoraPhy::UnregisterListener(const std::shared_ptr<EndDeviceLoraPhyListener>& listener)
{
    m_listeners.remove_if([&listener](auto&& weakPtr) { return weakPtr.lock() == listener; });
}

// protected

void
EndDeviceLoraPhy::TxFinished(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    NS_ASSERT_MSG(m_state == State::TX, "Improper switch to STANDBY from m_state=" << m_state);
    // Automatically switch to STANDBY mode.
    SwitchToStandBy();
    // Forward packet to the upper layer (if the callback was set).
    if (!m_txFinishedCallback.IsNull())
    {
        m_txFinishedCallback(packet);
    }
}

void
EndDeviceLoraPhy::RequestSleepMode()
{
    NS_LOG_FUNCTION(this);
    // Ignore if already in right state
    if (m_state == State::SLEEP)
    {
        return;
    }

    if (m_state != State::STANDBY)
    {
        NS_LOG_ERROR("Cannot switch to SLEEP from m_state=" << m_state);
        return;
    }

    SwitchToSleep();
}

void
EndDeviceLoraPhy::RequestTxMode()
{
    NS_LOG_FUNCTION(this);
    SwitchToTx();
}

void
EndDeviceLoraPhy::RequestRxSingleMode()
{
    NS_LOG_FUNCTION(this);
    // rx single mode: switch to RX_ENABLED + start HW timer
    SwitchToRxEnabled();
    auto tSym = GetTSym(m_regs.spreadingFactor, m_regs.bandwidthHz);
    m_rxTimeoutEvent =
        Simulator::Schedule(m_regs.symbNumTimeout * tSym, &EndDeviceLoraPhy::RxTimeout, this);
}

void
EndDeviceLoraPhy::DoStartReceive()
{
    NS_LOG_FUNCTION(this);
    // cancel schedule RX timeout, if any
    m_rxTimeoutEvent.Cancel();
    SwitchToRxActive();
}

void
EndDeviceLoraPhy::DoEndReceive()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(m_state == State::RX_ACTIVE,
                  "Improper switch to STANDBY from m_state=" << m_state);
    SwitchToStandBy();
}

// private

void
EndDeviceLoraPhy::RxTimeout()
{
    SwitchToStandBy();
    // Notify the upper layer (in reality this is an hardware interrupt)
    if (!m_rxTimeoutCallback.IsNull())
    {
        m_rxTimeoutCallback();
    }
}

void
EndDeviceLoraPhy::SwitchToSleep()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(m_state == State::STANDBY, "Cannot switch to SLEEP from m_state=" << m_state);
    m_state = State::SLEEP;
    // Notify listeners of the state change
    NotifyListeners(&EndDeviceLoraPhyListener::NotifySleep);
}

void
EndDeviceLoraPhy::SwitchToStandBy()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(m_state != State::SLEEP && m_state != State::STANDBY,
                  "Improper switch to STANDBY from m_state=" << m_state);
    m_state = State::STANDBY;
    NotifyListeners(&EndDeviceLoraPhyListener::NotifyStandby);
}

void
EndDeviceLoraPhy::SwitchToTx()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(m_state == State::SLEEP || m_state == State::STANDBY,
                  "Cannot switch to TX from m_state=" << m_state);
    m_state = State::TX;
    NotifyListeners(&EndDeviceLoraPhyListener::NotifyTx, m_regs.txPowerDbm);
}

void
EndDeviceLoraPhy::SwitchToRxEnabled()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(m_state == State::SLEEP || m_state == State::STANDBY,
                  "Cannot switch to RX_ENABLED from m_state=" << m_state);
    m_state = State::RX_ENABLED;
    NotifyListeners(&EndDeviceLoraPhyListener::NotifyRxEnabled);
}

void
EndDeviceLoraPhy::SwitchToRxActive()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(m_state == State::RX_ENABLED,
                  "Cannot switch to RX_ACTIVE from m_state=" << m_state);
    m_state = State::RX_ACTIVE;
    NotifyListeners(&EndDeviceLoraPhyListener::NotifyRxActive);
}

// external

std::ostream&
operator<<(std::ostream& os, const EndDeviceLoraPhy::State& state)
{
    switch (state)
    {
    case EndDeviceLoraPhy::State::SLEEP:
        return (os << "SLEEP");
    case EndDeviceLoraPhy::State::STANDBY:
        return (os << "STANDBY");
    case EndDeviceLoraPhy::State::TX:
        return (os << "TX");
    case EndDeviceLoraPhy::State::RX_ENABLED:
        return (os << "RX_ENABLED");
    case EndDeviceLoraPhy::State::RX_ACTIVE:
        return (os << "RX_ACTIVE");
    default:
        NS_FATAL_ERROR("Invalid LoRa device PHY state");
    }
}

} // namespace lorawan
} // namespace ns3
