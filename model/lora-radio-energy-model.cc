/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 *
 * Author: Romagnolo Stefano <romagnolostefano93@gmail.com>
 */

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/pointer.h"
#include "ns3/energy-source.h"
#include "lora-radio-energy-model.h"


namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("LoraRadioEnergyModel");

NS_OBJECT_ENSURE_REGISTERED (LoraRadioEnergyModel);

TypeId
LoraRadioEnergyModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraRadioEnergyModel")
    .SetParent<DeviceEnergyModel> ()
    .SetGroupName ("Energy")
    .AddConstructor<LoraRadioEnergyModel> ()
    .AddAttribute ("StandbyCurrentA",
                   "The default radio Standby current in Ampere.",
                   DoubleValue (0.0014),      // idle mode = 1.4mA
                   MakeDoubleAccessor (&LoraRadioEnergyModel::SetStandbyCurrentA,
                                       &LoraRadioEnergyModel::GetStandbyCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxCurrentA",
                   "The radio Tx current in Ampere.",
                   DoubleValue (0.028),        // transmit at 0dBm = 28mA
                   MakeDoubleAccessor (&LoraRadioEnergyModel::SetTxCurrentA,
                                       &LoraRadioEnergyModel::GetTxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("RxCurrentA",
                   "The radio Rx current in Ampere.",
                   DoubleValue (0.0112),        // receive mode = 11.2mA
                   MakeDoubleAccessor (&LoraRadioEnergyModel::SetRxCurrentA,
                                       &LoraRadioEnergyModel::GetRxCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SleepCurrentA",
                   "The radio Sleep current in Ampere.",
                   DoubleValue (0.0000015),      // sleep mode = 1.5microA
                   MakeDoubleAccessor (&LoraRadioEnergyModel::SetSleepCurrentA,
                                       &LoraRadioEnergyModel::GetSleepCurrentA),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("TxCurrentModel", "A pointer to the attached tx current model.",
                   PointerValue (),
                   MakePointerAccessor (&LoraRadioEnergyModel::m_txCurrentModel),
                   MakePointerChecker<LoraTxCurrentModel> ())
    .AddTraceSource ("TotalEnergyConsumption",
                     "Total energy consumption of the radio device.",
                     MakeTraceSourceAccessor (&LoraRadioEnergyModel::m_totalEnergyConsumption),
                     "ns3::TracedValueCallback::Double")
  ;
  return tid;
}

LoraRadioEnergyModel::LoraRadioEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  m_currentState = EndDeviceLoraPhy::SLEEP;      // initially STANDBY
  m_lastUpdateTime = Seconds (0.0);
  m_nPendingChangeState = 0;
  m_isSupersededChangeState = false;
  m_energyDepletionCallback.Nullify ();
  m_source = NULL;
  // set callback for EndDeviceLoraPhy listener
  m_listener = new LoraRadioEnergyModelPhyListener;
  m_listener->SetChangeStateCallback (MakeCallback (&DeviceEnergyModel::ChangeState, this));
  // set callback for updating the tx current
  m_listener->SetUpdateTxCurrentCallback (MakeCallback (&LoraRadioEnergyModel::SetTxCurrentFromModel, this));
}

LoraRadioEnergyModel::~LoraRadioEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  delete m_listener;
}

void
LoraRadioEnergyModel::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
}

double
LoraRadioEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

double
LoraRadioEnergyModel::GetStandbyCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_idleCurrentA;
}

void
LoraRadioEnergyModel::SetStandbyCurrentA (double idleCurrentA)
{
  NS_LOG_FUNCTION (this << idleCurrentA);
  m_idleCurrentA = idleCurrentA;
}

double
LoraRadioEnergyModel::GetTxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_txCurrentA;
}

void
LoraRadioEnergyModel::SetTxCurrentA (double txCurrentA)
{
  NS_LOG_FUNCTION (this << txCurrentA);
  m_txCurrentA = txCurrentA;
}

double
LoraRadioEnergyModel::GetRxCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_rxCurrentA;
}

void
LoraRadioEnergyModel::SetRxCurrentA (double rxCurrentA)
{
  NS_LOG_FUNCTION (this << rxCurrentA);
  m_rxCurrentA = rxCurrentA;
}

double
LoraRadioEnergyModel::GetSleepCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_sleepCurrentA;
}

void
LoraRadioEnergyModel::SetSleepCurrentA (double sleepCurrentA)
{
  NS_LOG_FUNCTION (this << sleepCurrentA);
  m_sleepCurrentA = sleepCurrentA;
}

EndDeviceLoraPhy::State
LoraRadioEnergyModel::GetCurrentState (void) const
{
  NS_LOG_FUNCTION (this);
  return m_currentState;
}

void
LoraRadioEnergyModel::SetEnergyDepletionCallback (
  LoraRadioEnergyDepletionCallback callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("LoraRadioEnergyModel:Setting NULL energy depletion callback!");
    }
  m_energyDepletionCallback = callback;
}

void
LoraRadioEnergyModel::SetEnergyRechargedCallback (
  LoraRadioEnergyRechargedCallback callback)
{
  NS_LOG_FUNCTION (this);
  if (callback.IsNull ())
    {
      NS_LOG_DEBUG ("LoraRadioEnergyModel:Setting NULL energy recharged callback!");
    }
  m_energyRechargedCallback = callback;
}

void
LoraRadioEnergyModel::SetTxCurrentModel (Ptr<LoraTxCurrentModel> model)
{
  m_txCurrentModel = model;
}

void
LoraRadioEnergyModel::SetTxCurrentFromModel (double txPowerDbm)
{
  if (m_txCurrentModel)
    {
      m_txCurrentA = m_txCurrentModel->CalcTxCurrent (txPowerDbm);
    }
}

void
LoraRadioEnergyModel::ChangeState (int newState)
{
  NS_LOG_FUNCTION (this << newState);

  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.GetNanoSeconds () >= 0);     // check if duration is valid

  // energy to decrease = current * voltage * time
  double energyToDecrease = 0.0;
  double supplyVoltage = m_source->GetSupplyVoltage ();
  switch (m_currentState)
    {
    case EndDeviceLoraPhy::STANDBY:
      energyToDecrease = duration.GetSeconds () * m_idleCurrentA * supplyVoltage;
      break;
    case EndDeviceLoraPhy::TX:
      energyToDecrease = duration.GetSeconds () * m_txCurrentA * supplyVoltage;
      break;
    case EndDeviceLoraPhy::RX:
      energyToDecrease = duration.GetSeconds () * m_rxCurrentA * supplyVoltage;
      break;
    case EndDeviceLoraPhy::SLEEP:
      energyToDecrease = duration.GetSeconds () * m_sleepCurrentA * supplyVoltage;
      break;
    default:
      NS_FATAL_ERROR ("LoraRadioEnergyModel:Undefined radio state: " << m_currentState);
    }

  // update total energy consumption
  m_totalEnergyConsumption += energyToDecrease;

  // update last update time stamp
  m_lastUpdateTime = Simulator::Now ();

  m_nPendingChangeState++;

  // notify energy source
  m_source->UpdateEnergySource ();

  // in case the energy source is found to be depleted during the last update, a callback might be
  // invoked that might cause a change in the Lora PHY state (e.g., the PHY is put into SLEEP mode).
  // This in turn causes a new call to this member function, with the consequence that the previous
  // instance is resumed after the termination of the new instance. In particular, the state set
  // by the previous instance is erroneously the final state stored in m_currentState. The check below
  // ensures that previous instances do not change m_currentState.

  if (!m_isSupersededChangeState)
    {
      // update current state & last update time stamp
      SetLoraRadioState ((EndDeviceLoraPhy::State) newState);

      // some debug message
      NS_LOG_DEBUG ("LoraRadioEnergyModel:Total energy consumption is " <<
                    m_totalEnergyConsumption << "J");
    }

  m_isSupersededChangeState = (m_nPendingChangeState > 1);

  m_nPendingChangeState--;
}

void
LoraRadioEnergyModel::HandleEnergyDepletion (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("LoraRadioEnergyModel:Energy is depleted!");
  // invoke energy depletion callback, if set.
  if (!m_energyDepletionCallback.IsNull ())
    {
      m_energyDepletionCallback ();
    }
}

void
LoraRadioEnergyModel::HandleEnergyChanged (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("LoraRadioEnergyModel:Energy changed!");
}

void
LoraRadioEnergyModel::HandleEnergyRecharged (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("LoraRadioEnergyModel:Energy is recharged!");
  // invoke energy recharged callback, if set.
  if (!m_energyRechargedCallback.IsNull ())
    {
      m_energyRechargedCallback ();
    }
}

LoraRadioEnergyModelPhyListener *
LoraRadioEnergyModel::GetPhyListener (void)
{
  NS_LOG_FUNCTION (this);
  return m_listener;
}

/*
 * Private functions start here.
 */

void
LoraRadioEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_source = NULL;
  m_energyDepletionCallback.Nullify ();
}

double
LoraRadioEnergyModel::DoGetCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  switch (m_currentState)
    {
    case EndDeviceLoraPhy::STANDBY:
      return m_idleCurrentA;
    case EndDeviceLoraPhy::TX:
      return m_txCurrentA;
    case EndDeviceLoraPhy::RX:
      return m_rxCurrentA;
    case EndDeviceLoraPhy::SLEEP:
      return m_sleepCurrentA;
    default:
      NS_FATAL_ERROR ("LoraRadioEnergyModel:Undefined radio state:" << m_currentState);
    }
}

void
LoraRadioEnergyModel::SetLoraRadioState (const EndDeviceLoraPhy::State state)
{
  NS_LOG_FUNCTION (this << state);
  m_currentState = state;
  std::string stateName;
  switch (state)
    {
    case EndDeviceLoraPhy::STANDBY:
      stateName = "STANDBY";
      break;
    case EndDeviceLoraPhy::TX:
      stateName = "TX";
      break;
    case EndDeviceLoraPhy::RX:
      stateName = "RX";
      break;
    case EndDeviceLoraPhy::SLEEP:
      stateName = "SLEEP";
      break;
    }
  NS_LOG_DEBUG ("LoraRadioEnergyModel:Switching to state: " << stateName <<
                " at time = " << Simulator::Now ().GetSeconds () << " s");
}

// -------------------------------------------------------------------------- //

LoraRadioEnergyModelPhyListener::LoraRadioEnergyModelPhyListener ()
{
  NS_LOG_FUNCTION (this);
  m_changeStateCallback.Nullify ();
  m_updateTxCurrentCallback.Nullify ();
}

LoraRadioEnergyModelPhyListener::~LoraRadioEnergyModelPhyListener ()
{
  NS_LOG_FUNCTION (this);
}

void
LoraRadioEnergyModelPhyListener::SetChangeStateCallback (DeviceEnergyModel::ChangeStateCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);
  NS_ASSERT (!callback.IsNull ());
  m_changeStateCallback = callback;
}

void
LoraRadioEnergyModelPhyListener::SetUpdateTxCurrentCallback (UpdateTxCurrentCallback callback)
{
  NS_LOG_FUNCTION (this << &callback);
  NS_ASSERT (!callback.IsNull ());
  m_updateTxCurrentCallback = callback;
}

void
LoraRadioEnergyModelPhyListener::NotifyRxStart ()
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (EndDeviceLoraPhy::RX);
}

void
LoraRadioEnergyModelPhyListener::NotifyTxStart (double txPowerDbm)
{
  NS_LOG_FUNCTION (this << txPowerDbm);
  if (m_updateTxCurrentCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraRadioEnergyModelPhyListener:Update tx current callback not set!");
    }
  m_updateTxCurrentCallback (txPowerDbm);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (EndDeviceLoraPhy::TX);
}

void
LoraRadioEnergyModelPhyListener::NotifySleep (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (EndDeviceLoraPhy::SLEEP);
}

void
LoraRadioEnergyModelPhyListener::NotifyStandby (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (EndDeviceLoraPhy::STANDBY);
}

/*
 * Private function state here.
 */

void
LoraRadioEnergyModelPhyListener::SwitchToStandby (void)
{
  NS_LOG_FUNCTION (this);
  if (m_changeStateCallback.IsNull ())
    {
      NS_FATAL_ERROR ("LoraRadioEnergyModelPhyListener:Change state callback not set!");
    }
  m_changeStateCallback (EndDeviceLoraPhy::STANDBY);
}

}
} // namespace ns3
