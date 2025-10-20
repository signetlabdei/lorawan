/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Romagnolo Stefano <romagnolostefano93@gmail.com>
 */

#include "lora-radio-energy-model.h"

#include "ns3/energy-source.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraRadioEnergyModel");

NS_OBJECT_ENSURE_REGISTERED(LoraRadioEnergyModel);

TypeId
LoraRadioEnergyModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LoraRadioEnergyModel")
            .SetParent<DeviceEnergyModel>()
            .SetGroupName("Energy")
            .AddConstructor<LoraRadioEnergyModel>()
            .AddAttribute("StandbyCurrentA",
                          "The default radio Standby current in Ampere.",
                          DoubleValue(0.0014), // idle mode = 1.4mA
                          MakeDoubleAccessor(&LoraRadioEnergyModel::SetStandbyCurrentA,
                                             &LoraRadioEnergyModel::GetStandbyCurrentA),
                          MakeDoubleChecker<double>())
            .AddAttribute("TxCurrentA",
                          "The radio Tx current in Ampere.",
                          DoubleValue(0.028), // transmit at 0dBm = 28mA
                          MakeDoubleAccessor(&LoraRadioEnergyModel::SetTxCurrentA,
                                             &LoraRadioEnergyModel::GetTxCurrentA),
                          MakeDoubleChecker<double>())
            .AddAttribute("RxCurrentA",
                          "The radio Rx current in Ampere.",
                          DoubleValue(0.0112), // receive mode = 11.2mA
                          MakeDoubleAccessor(&LoraRadioEnergyModel::SetRxCurrentA,
                                             &LoraRadioEnergyModel::GetRxCurrentA),
                          MakeDoubleChecker<double>())
            .AddAttribute("SleepCurrentA",
                          "The radio Sleep current in Ampere.",
                          DoubleValue(0.0000015), // sleep mode = 1.5microA
                          MakeDoubleAccessor(&LoraRadioEnergyModel::SetSleepCurrentA,
                                             &LoraRadioEnergyModel::GetSleepCurrentA),
                          MakeDoubleChecker<double>())
            .AddAttribute("TxCurrentModel",
                          "A pointer to the attached tx current model.",
                          PointerValue(),
                          MakePointerAccessor(&LoraRadioEnergyModel::m_txCurrentModel),
                          MakePointerChecker<LoraTxCurrentModel>())
            .AddTraceSource(
                "TotalEnergyConsumption",
                "Total energy consumption of the radio device.",
                MakeTraceSourceAccessor(&LoraRadioEnergyModel::m_totalEnergyConsumption),
                "ns3::TracedValueCallback::Double");
    return tid;
}

LoraRadioEnergyModel::LoraRadioEnergyModel()
{
    NS_LOG_FUNCTION(this);
    m_currentState = EndDeviceLoraPhy::State::SLEEP;
    m_lastUpdateTime = Seconds(0.0);
    m_nPendingChangeState = 0;
    m_isSupersededChangeState = false;
    m_energyDepletionCallback.Nullify();
    m_source = nullptr;
    // set callback for EndDeviceLoraPhy listener
    m_listener = new LoraRadioEnergyModelPhyListener;
    m_listener->SetChangeStateCallback(MakeCallback(&DeviceEnergyModel::ChangeState, this));
    // set callback for updating the tx current
    m_listener->SetUpdateTxCurrentCallback(
        MakeCallback(&LoraRadioEnergyModel::SetTxCurrentFromModel, this));
}

LoraRadioEnergyModel::~LoraRadioEnergyModel()
{
    NS_LOG_FUNCTION(this);
    delete m_listener;
}

void
LoraRadioEnergyModel::SetEnergySource(Ptr<EnergySource> source)
{
    NS_LOG_FUNCTION(this << source);
    NS_ASSERT(source);
    m_source = source;

    ScheduleSwitchToOff(m_currentState);
}

double
LoraRadioEnergyModel::GetTotalEnergyConsumption() const
{
    NS_LOG_FUNCTION(this);
    return m_totalEnergyConsumption;
}

double
LoraRadioEnergyModel::GetStandbyCurrentA() const
{
    NS_LOG_FUNCTION(this);
    return m_idleCurrentA;
}

void
LoraRadioEnergyModel::SetStandbyCurrentA(double idleCurrentA)
{
    NS_LOG_FUNCTION(this << idleCurrentA);
    m_idleCurrentA = idleCurrentA;
}

double
LoraRadioEnergyModel::GetTxCurrentA() const
{
    NS_LOG_FUNCTION(this);
    return m_txCurrentA;
}

void
LoraRadioEnergyModel::SetTxCurrentA(double txCurrentA)
{
    NS_LOG_FUNCTION(this << txCurrentA);
    m_txCurrentA = txCurrentA;
}

double
LoraRadioEnergyModel::GetRxCurrentA() const
{
    NS_LOG_FUNCTION(this);
    return m_rxCurrentA;
}

void
LoraRadioEnergyModel::SetRxCurrentA(double rxCurrentA)
{
    NS_LOG_FUNCTION(this << rxCurrentA);
    m_rxCurrentA = rxCurrentA;
}

double
LoraRadioEnergyModel::GetSleepCurrentA() const
{
    NS_LOG_FUNCTION(this);
    return m_sleepCurrentA;
}

void
LoraRadioEnergyModel::SetSleepCurrentA(double sleepCurrentA)
{
    NS_LOG_FUNCTION(this << sleepCurrentA);
    m_sleepCurrentA = sleepCurrentA;
}

EndDeviceLoraPhy::State
LoraRadioEnergyModel::GetCurrentState() const
{
    NS_LOG_FUNCTION(this);
    return m_currentState;
}

void
LoraRadioEnergyModel::SetEnergyDepletionCallback(LoraRadioEnergyDepletionCallback callback)
{
    NS_LOG_FUNCTION(this);
    if (callback.IsNull())
    {
        NS_LOG_DEBUG("LoraRadioEnergyModel:Setting NULL energy depletion callback!");
    }
    m_energyDepletionCallback = callback;
}

void
LoraRadioEnergyModel::SetEnergyRechargedCallback(LoraRadioEnergyRechargedCallback callback)
{
    NS_LOG_FUNCTION(this);
    if (callback.IsNull())
    {
        NS_LOG_DEBUG("LoraRadioEnergyModel:Setting NULL energy recharged callback!");
    }
    m_energyRechargedCallback = callback;
}

void
LoraRadioEnergyModel::SetTxCurrentModel(Ptr<LoraTxCurrentModel> model)
{
    m_txCurrentModel = model;
}

void
LoraRadioEnergyModel::SetTxCurrentFromModel(double txPowerDbm)
{
    if (m_txCurrentModel)
    {
        m_txCurrentA = m_txCurrentModel->CalcTxCurrent(txPowerDbm);
    }
}

Time
LoraRadioEnergyModel::GetMaximumTimeInState(EndDeviceLoraPhy::State state) const
{
    if (state == EndDeviceLoraPhy::State::OFF)
    {
        NS_FATAL_ERROR("Requested maximum remaining time for OFF state");
    }
    const auto remainingEnergy = m_source->GetRemainingEnergy();
    const auto supplyVoltage = m_source->GetSupplyVoltage();
    const auto current = GetStateA(state);
    return Seconds(remainingEnergy / (current * supplyVoltage));
}

void
LoraRadioEnergyModel::ChangeState(int newState)
{
    const auto newPhyState = EndDeviceLoraPhy::State(newState);
    NS_LOG_FUNCTION(this << newPhyState);

    // renew schedule switch to OFF when we change state
    ScheduleSwitchToOff(newPhyState);

    Time duration = Now() - m_lastUpdateTime;
    NS_ASSERT(duration.IsPositive()); // check if duration is valid

    // energy to decrease = current * voltage * time
    double supplyVoltage = m_source->GetSupplyVoltage();
    double energyToDecrease = duration.GetSeconds() * GetStateA(m_currentState) * supplyVoltage;

    // update total energy consumption
    m_totalEnergyConsumption += energyToDecrease;

    // update last update time stamp
    m_lastUpdateTime = Now();

    m_nPendingChangeState++;

    // notify energy source
    m_source->UpdateEnergySource();

    // in case the energy source is found to be depleted during the last update, a callback might be
    // invoked that might cause a change in the Lora PHY state (e.g., the PHY is put into SLEEP
    // mode). This in turn causes a new call to this member function, with the consequence that the
    // previous instance is resumed after the termination of the new instance. In particular, the
    // state set by the previous instance is erroneously the final state stored in m_currentState.
    // The check below ensures that previous instances do not change m_currentState.

    if (!m_isSupersededChangeState)
    {
        // update current state & last update time stamp
        SetLoraRadioState(newPhyState);

        // some debug message
        NS_LOG_DEBUG("LoraRadioEnergyModel:Total energy consumption is " << m_totalEnergyConsumption
                                                                         << "J");
    }

    m_isSupersededChangeState = (m_nPendingChangeState > 1);

    m_nPendingChangeState--;
}

void
LoraRadioEnergyModel::HandleEnergyDepletion()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("LoraRadioEnergyModel:Energy is depleted!");
    // invoke energy depletion callback, if set.
    if (!m_energyDepletionCallback.IsNull())
    {
        m_energyDepletionCallback();
    }
}

void
LoraRadioEnergyModel::HandleEnergyChanged()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("LoraRadioEnergyModel:Energy changed!");

    ScheduleSwitchToOff(m_currentState);
}

void
LoraRadioEnergyModel::HandleEnergyRecharged()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("LoraRadioEnergyModel:Energy is recharged!");
    // invoke energy recharged callback, if set.
    if (!m_energyRechargedCallback.IsNull())
    {
        m_energyRechargedCallback();
    }
}

LoraRadioEnergyModelPhyListener*
LoraRadioEnergyModel::GetPhyListener()
{
    NS_LOG_FUNCTION(this);
    return m_listener;
}

LoraRadioEnergyModel::LoraRadioEnergyDepletionCallback
LoraRadioEnergyModel::GetEnergyDepletionCallback() const
{
    NS_LOG_FUNCTION(this);
    return m_energyDepletionCallback;
}

/*
 * Private functions start here.
 */

void
LoraRadioEnergyModel::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_source = nullptr;
    m_energyDepletionCallback.Nullify();
}

double
LoraRadioEnergyModel::GetStateA(EndDeviceLoraPhy::State state) const
{
    NS_LOG_FUNCTION(this);
    switch (state)
    {
    case EndDeviceLoraPhy::State::STANDBY:
        return m_idleCurrentA;
    case EndDeviceLoraPhy::State::TX:
        return m_txCurrentA;
    case EndDeviceLoraPhy::State::RX:
        return m_rxCurrentA;
    case EndDeviceLoraPhy::State::SLEEP:
        return m_sleepCurrentA;
    case EndDeviceLoraPhy::State::OFF:
        return 0.0;
    default:
        NS_FATAL_ERROR("LoraRadioEnergyModel:Undefined radio state:" << state);
    }
}

double
LoraRadioEnergyModel::DoGetCurrentA() const
{
    NS_LOG_FUNCTION(this);
    return GetStateA(m_currentState);
}

void
LoraRadioEnergyModel::SetLoraRadioState(const EndDeviceLoraPhy::State state)
{
    NS_LOG_FUNCTION(this << state);
    m_currentState = state;
    NS_LOG_DEBUG("Switching to state: " << state << " at time = " << Now().As(Time::S));
}

void
LoraRadioEnergyModel::ScheduleSwitchToOff(EndDeviceLoraPhy::State state)
{
    NS_LOG_FUNCTION(this << state);

    if (state == EndDeviceLoraPhy::State::OFF)
    {
        return;
    }
    m_switchToOffEvent.Cancel();
    const auto durationToOff = GetMaximumTimeInState(state);
    m_switchToOffEvent = Simulator::Schedule(durationToOff,
                                             &LoraRadioEnergyModel::ChangeState,
                                             this,
                                             static_cast<int>(EndDeviceLoraPhy::State::OFF));
}

// -------------------------------------------------------------------------- //

LoraRadioEnergyModelPhyListener::LoraRadioEnergyModelPhyListener()
{
    NS_LOG_FUNCTION(this);
    m_changeStateCallback.Nullify();
    m_updateTxCurrentCallback.Nullify();
}

LoraRadioEnergyModelPhyListener::~LoraRadioEnergyModelPhyListener()
{
    NS_LOG_FUNCTION(this);
}

void
LoraRadioEnergyModelPhyListener::SetChangeStateCallback(
    DeviceEnergyModel::ChangeStateCallback callback)
{
    NS_LOG_FUNCTION(this << &callback);
    NS_ASSERT(!callback.IsNull());
    m_changeStateCallback = callback;
}

void
LoraRadioEnergyModelPhyListener::SetUpdateTxCurrentCallback(UpdateTxCurrentCallback callback)
{
    NS_LOG_FUNCTION(this << &callback);
    NS_ASSERT(!callback.IsNull());
    m_updateTxCurrentCallback = callback;
}

void
LoraRadioEnergyModelPhyListener::NotifyRxStart()
{
    NS_LOG_FUNCTION(this);
    if (m_changeStateCallback.IsNull())
    {
        NS_FATAL_ERROR("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
    m_changeStateCallback(int(EndDeviceLoraPhy::State::RX));
}

void
LoraRadioEnergyModelPhyListener::NotifyTxStart(double txPowerDbm)
{
    NS_LOG_FUNCTION(this << txPowerDbm);
    if (m_updateTxCurrentCallback.IsNull())
    {
        NS_FATAL_ERROR("LoraRadioEnergyModelPhyListener:Update tx current callback not set!");
    }
    m_updateTxCurrentCallback(txPowerDbm);
    if (m_changeStateCallback.IsNull())
    {
        NS_FATAL_ERROR("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
    m_changeStateCallback(int(EndDeviceLoraPhy::State::TX));
}

void
LoraRadioEnergyModelPhyListener::NotifySleep()
{
    NS_LOG_FUNCTION(this);
    if (m_changeStateCallback.IsNull())
    {
        NS_FATAL_ERROR("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
    m_changeStateCallback(int(EndDeviceLoraPhy::State::SLEEP));
}

void
LoraRadioEnergyModelPhyListener::NotifyStandby()
{
    NS_LOG_FUNCTION(this);
    if (m_changeStateCallback.IsNull())
    {
        NS_FATAL_ERROR("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
    m_changeStateCallback(int(EndDeviceLoraPhy::State::STANDBY));
}

void
LoraRadioEnergyModelPhyListener::NotifyOff()
{
    NS_LOG_FUNCTION(this);
    if (m_changeStateCallback.IsNull())
    {
        NS_FATAL_ERROR("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
    m_changeStateCallback(EndDeviceLoraPhy::State::OFF);
}

/*
 * Private function state here.
 */

void
LoraRadioEnergyModelPhyListener::SwitchToStandby()
{
    NS_LOG_FUNCTION(this);
    if (m_changeStateCallback.IsNull())
    {
        NS_FATAL_ERROR("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
    m_changeStateCallback(int(EndDeviceLoraPhy::State::STANDBY));
}

} // namespace lorawan
} // namespace ns3
