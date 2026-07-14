/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Romagnolo Stefano <romagnolostefano93@gmail.com>
 */

#include "lora-radio-energy-model.h"

#include "lora-tx-current-model.h"

#include "ns3/energy-source.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraRadioEnergyModel");

NS_OBJECT_ENSURE_REGISTERED(LoraRadioEnergyModel);

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
    energy::DeviceEnergyModel::ChangeStateCallback callback)
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
LoraRadioEnergyModelPhyListener::NotifySleep()
{
    NS_LOG_FUNCTION(this);
    if (m_changeStateCallback.IsNull())
    {
        NS_FATAL_ERROR("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
    m_changeStateCallback(static_cast<int>(EndDeviceLoraPhy::State::SLEEP));
}

void
LoraRadioEnergyModelPhyListener::NotifyStandby()
{
    NS_LOG_FUNCTION(this);
    if (m_changeStateCallback.IsNull())
    {
        NS_FATAL_ERROR("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
    m_changeStateCallback(static_cast<int>(EndDeviceLoraPhy::State::STANDBY));
}

void
LoraRadioEnergyModelPhyListener::NotifyTx(double txPowerDbm)
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
    m_changeStateCallback(static_cast<int>(EndDeviceLoraPhy::State::TX));
}

void
LoraRadioEnergyModelPhyListener::NotifyRxEnabled()
{
    NS_LOG_FUNCTION(this);
    if (m_changeStateCallback.IsNull())
    {
        NS_FATAL_ERROR("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
    m_changeStateCallback(static_cast<int>(EndDeviceLoraPhy::State::RX_ENABLED));
}

void
LoraRadioEnergyModelPhyListener::NotifyRxActive()
{
    NS_LOG_FUNCTION(this);
    if (m_changeStateCallback.IsNull())
    {
        NS_FATAL_ERROR("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
    m_changeStateCallback(static_cast<int>(EndDeviceLoraPhy::State::RX_ACTIVE));
}

TypeId
LoraRadioEnergyModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LoraRadioEnergyModel")
            .SetParent<DeviceEnergyModel>()
            .SetGroupName("Energy")
            .AddConstructor<LoraRadioEnergyModel>()
            .AddAttribute("SleepCurrentA",
                          "The radio Sleep current in Ampere.",
                          DoubleValue(0.1e-6), // sleep mode = 0.1uA
                          MakeDoubleAccessor(&LoraRadioEnergyModel::SetSleepCurrentA,
                                             &LoraRadioEnergyModel::GetSleepCurrentA),
                          MakeDoubleChecker<double>())
            .AddAttribute("StandbyCurrentA",
                          "The default radio Standby current in Ampere.",
                          DoubleValue(1.4e-3), // standby mode = 1.4mA
                          MakeDoubleAccessor(&LoraRadioEnergyModel::SetStandbyCurrentA,
                                             &LoraRadioEnergyModel::GetStandbyCurrentA),
                          MakeDoubleChecker<double>())
            .AddAttribute("TxCurrentA",
                          "The radio Tx current in Ampere.",
                          DoubleValue(28e-3), // transmit at 13dBm = 28mA
                          MakeDoubleAccessor(&LoraRadioEnergyModel::SetTxCurrentA,
                                             &LoraRadioEnergyModel::GetTxCurrentA),
                          MakeDoubleChecker<double>())
            .AddAttribute("RxCurrentA",
                          "The radio Rx current in Ampere.",
                          DoubleValue(10.8e-3), // receive mode = 10.8mA
                          MakeDoubleAccessor(&LoraRadioEnergyModel::SetRxCurrentA,
                                             &LoraRadioEnergyModel::GetRxCurrentA),
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
    : m_source(nullptr),
      m_currentState(EndDeviceLoraPhy::State::STANDBY),
      m_lastUpdateTime(),
      m_nPendingChangeState(0)
{
    NS_LOG_FUNCTION(this);
    m_energyDepletionCallback.Nullify();
    // set callback for EndDeviceLoraPhy listener
    m_listener = std::make_shared<LoraRadioEnergyModelPhyListener>();
    m_listener->SetChangeStateCallback(MakeCallback(&DeviceEnergyModel::ChangeState, this));
    // set callback for updating the tx current
    m_listener->SetUpdateTxCurrentCallback(
        MakeCallback(&LoraRadioEnergyModel::SetTxCurrentFromModel, this));
}

LoraRadioEnergyModel::~LoraRadioEnergyModel()
{
    NS_LOG_FUNCTION(this);
    m_txCurrentModel = nullptr;
    m_listener.reset();
}

void
LoraRadioEnergyModel::SetEnergySource(Ptr<energy::EnergySource> source)
{
    NS_LOG_FUNCTION(this << source);
    NS_ASSERT(source);
    m_source = source;
}

double
LoraRadioEnergyModel::GetTotalEnergyConsumption() const
{
    NS_LOG_FUNCTION(this);

    const auto duration = Simulator::Now() - m_lastUpdateTime;
    NS_ASSERT(duration.IsPositive()); // check if duration is valid

    // energy to decrease = current * voltage * time
    const auto supplyVoltage = m_source->GetSupplyVoltage();
    const auto energyToDecrease = duration.GetSeconds() * GetStateA(m_currentState) * supplyVoltage;

    // notify energy source
    m_source->UpdateEnergySource();

    return m_totalEnergyConsumption + energyToDecrease;
}

double
LoraRadioEnergyModel::GetStandbyCurrentA() const
{
    NS_LOG_FUNCTION(this);
    return m_standbyCurrentA;
}

void
LoraRadioEnergyModel::SetStandbyCurrentA(double standbyCurrentA)
{
    NS_LOG_FUNCTION(this << standbyCurrentA);
    m_standbyCurrentA = standbyCurrentA;
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

void
LoraRadioEnergyModel::ChangeState(int newState)
{
    EndDeviceLoraPhy::State newPhyState{newState};
    NS_LOG_FUNCTION(this << newPhyState);

    m_nPendingChangeState++;

    const auto duration = Simulator::Now() - m_lastUpdateTime;
    NS_ASSERT(duration.IsPositive()); // check if duration is valid

    // energy to decrease = current * voltage * time
    const auto supplyVoltage = m_source->GetSupplyVoltage();
    const auto energyToDecrease = duration.GetSeconds() * GetStateA(m_currentState) * supplyVoltage;

    // update total energy consumption
    m_totalEnergyConsumption += energyToDecrease;
    NS_ASSERT(m_totalEnergyConsumption <= m_source->GetInitialEnergy());

    // update last update time stamp
    m_lastUpdateTime = Simulator::Now();

    // notify energy source
    m_source->UpdateEnergySource();

    // in case the energy source is found to be depleted during the last update, a callback might be
    // invoked that might cause a change in the Lora PHY state (e.g., the PHY is put into SLEEP
    // mode). This in turn causes a new call to this member function, with the consequence that the
    // previous instance is resumed after the termination of the new instance. In particular, the
    // state set by the previous instance is erroneously the final state stored in m_currentState.
    // The check below ensures that previous instances do not change m_currentState.

    if (m_nPendingChangeState <= 1)
    {
        // update current state & last update time stamp
        SetLoraRadioState(newPhyState);

        // some debug message
        NS_LOG_DEBUG("LoraRadioEnergyModel:Total energy consumption is " << m_totalEnergyConsumption
                                                                         << "J");
    }

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

void
LoraRadioEnergyModel::HandleEnergyChanged()
{
    NS_LOG_FUNCTION(this);
    NS_LOG_DEBUG("LoraRadioEnergyModel:Energy changed!");
}

std::shared_ptr<LoraRadioEnergyModelPhyListener>
LoraRadioEnergyModel::GetPhyListener()
{
    NS_LOG_FUNCTION(this);
    return m_listener;
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
    switch (state)
    {
    case EndDeviceLoraPhy::State::SLEEP:
        return m_sleepCurrentA;
    case EndDeviceLoraPhy::State::STANDBY:
        return m_standbyCurrentA;
    case EndDeviceLoraPhy::State::TX:
        return m_txCurrentA;
    case EndDeviceLoraPhy::State::RX_ENABLED:
    case EndDeviceLoraPhy::State::RX_ACTIVE:
        return m_rxCurrentA;
    }
    NS_FATAL_ERROR("LoraRadioEnergyModel: undefined radio state " << m_currentState);
}

double
LoraRadioEnergyModel::DoGetCurrentA() const
{
    return GetStateA(m_currentState);
}

void
LoraRadioEnergyModel::SetLoraRadioState(const EndDeviceLoraPhy::State state)
{
    NS_LOG_FUNCTION(this << state);
    m_currentState = state;
    NS_LOG_DEBUG("LoraRadioEnergyModel: Switching to state: " << state
                                                              << " at time = " << Simulator::Now());
}

} // namespace lorawan
} // namespace ns3
