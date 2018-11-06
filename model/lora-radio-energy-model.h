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
 * Authors: Romagnolo Stefano <romagnolostefano93@gmail.com>
 *          Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LORA_RADIO_ENERGY_MODEL_H
#define LORA_RADIO_ENERGY_MODEL_H

#include "ns3/device-energy-model.h"
#include "ns3/traced-value.h"
#include "end-device-lora-phy.h"
#include "lora-tx-current-model.h"

namespace ns3 {
namespace lorawan {

/**
 * \ingroup energy
 */
class LoraRadioEnergyModelPhyListener : public EndDeviceLoraPhyListener
{
public:
  /**
   * Callback type for updating the transmit current based on the nominal tx power.
   */
  typedef Callback<void, double> UpdateTxCurrentCallback;

  LoraRadioEnergyModelPhyListener ();
  virtual ~LoraRadioEnergyModelPhyListener ();

  /**
   * \brief Sets the change state callback. Used by helper class.
   *
   * \param callback Change state callback.
   */
  void SetChangeStateCallback (DeviceEnergyModel::ChangeStateCallback callback);

  /**
   * \brief Sets the update tx current callback.
   *
   * \param callback Update tx current callback.
   */
  void SetUpdateTxCurrentCallback (UpdateTxCurrentCallback callback);

  /**
   * \brief Switches the LoraRadioEnergyModel to RX state.
   *
   * \param duration the expected duration of the packet reception.
   *
   * Defined in ns3::LoraEndDevicePhyListener
   */
  void NotifyRxStart (void);

  /**
   * \brief Switches the LoraRadioEnergyModel to TX state and switches back to
   * STANDBY after TX duration.
   *
   * \param duration the expected transmission duration.
   * \param txPowerDbm the nominal tx power in dBm
   *
   * Defined in ns3::LoraEndDevicePhyListener
   */
  void NotifyTxStart (double txPowerDbm);

  /**
   * Defined in ns3::LoraEndDevicePhyListener
   */
  void NotifySleep (void);

  /**
   * Defined in ns3::LoraEndDevicePhyListener
   */
  void NotifyStandby (void);


private:
  /**
   * A helper function that makes scheduling m_changeStateCallback possible.
   */
  void SwitchToStandby (void);

  /**
   * Change state callback used to notify the LoraRadioEnergyModel of a state
   * change.
   */
  DeviceEnergyModel::ChangeStateCallback m_changeStateCallback;

  /**
   * Callback used to update the tx current stored in LoraRadioEnergyModel based on
   * the nominal tx power used to transmit the current frame.
   */
  UpdateTxCurrentCallback m_updateTxCurrentCallback;
};


/**
 * \ingroup energy
 * \brief A WiFi radio energy model.
 *
 * 4 states are defined for the radio: TX, RX, STANDBY, SLEEP. Default state is
 * STANDBY.
 * The different types of transactions that are defined are:
 *  1. Tx: State goes from STANDBY to TX, radio is in TX state for TX_duration,
 *     then state goes from TX to STANDBY.
 *  2. Rx: State goes from STANDBY to RX, radio is in RX state for RX_duration,
 *     then state goes from RX to STANDBY.
 *  3. Go_to_Sleep: State goes from STANDBY to SLEEP.
 *  4. End_of_Sleep: State goes from SLEEP to STANDBY.
 * The class keeps track of what state the radio is currently in.
 *
 * Energy calculation: For each transaction, this model notifies EnergySource
 * object. The EnergySource object will query this model for the total current.
 * Then the EnergySource object uses the total current to calculate energy.
 *
 */
class LoraRadioEnergyModel : public DeviceEnergyModel
{
public:
  /**
   * Callback type for energy depletion handling.
   */
  typedef Callback<void> LoraRadioEnergyDepletionCallback;

  /**
   * Callback type for energy recharged handling.
   */
  typedef Callback<void> LoraRadioEnergyRechargedCallback;

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  LoraRadioEnergyModel ();
  virtual ~LoraRadioEnergyModel ();

  /**
   * \brief Sets pointer to EnergySouce installed on node.
   *
   * \param source Pointer to EnergySource installed on node.
   *
   * Implements DeviceEnergyModel::SetEnergySource.
   */
  void SetEnergySource (Ptr<EnergySource> source);

  /**
   * \returns Total energy consumption of the wifi device.
   *
   * Implements DeviceEnergyModel::GetTotalEnergyConsumption.
   */
  double GetTotalEnergyConsumption (void) const;

  // Setter & getters for state power consumption.
  /**
   * \brief Gets idle current.
   *
   * \returns idle current of the lora device.
   */
  double GetStandbyCurrentA (void) const;
  /**
   * \brief Sets idle current.
   *
   * \param idleCurrentA the idle current
   */
  void SetStandbyCurrentA (double idleCurrentA);
  /**
   * \brief Gets transmit current.
   *
   * \returns transmit current of the lora device.
   */
  double GetTxCurrentA (void) const;
  /**
   * \brief Sets transmit current.
   *
   * \param txCurrentA the transmit current
   */
  void SetTxCurrentA (double txCurrentA);
  /**
   * \brief Gets receive current.
   *
   * \returns receive current of the lora device.
   */
  double GetRxCurrentA (void) const;
  /**
   * \brief Sets receive current.
   *
   * \param rxCurrentA the receive current
   */
  void SetRxCurrentA (double rxCurrentA);
  /**
   * \brief Gets sleep current.
   *
   * \returns sleep current of the lora device.
   */
  double GetSleepCurrentA (void) const;
  /**
   * \brief Sets sleep current.
   *
   * \param sleepCurrentA the sleep current
   */
  void SetSleepCurrentA (double sleepCurrentA);

  /**
   * \returns Current state.
   */
  EndDeviceLoraPhy::State GetCurrentState (void) const;

  /**
   * \param callback Callback function.
   *
   * Sets callback for energy depletion handling.
   */
  void SetEnergyDepletionCallback (LoraRadioEnergyDepletionCallback callback);

  /**
   * \param callback Callback function.
   *
   * Sets callback for energy recharged handling.
   */
  void SetEnergyRechargedCallback (LoraRadioEnergyRechargedCallback callback);

  /**
   * \param model the model used to compute the lora tx current.
   */
  // NOTICE VERY WELL: Current  Model linear or constant as possible choices
  void SetTxCurrentModel (Ptr<LoraTxCurrentModel> model);

  /**
   * \brief Calls the CalcTxCurrent method of the tx current model to
   *        compute the tx current based on such model
   *
   * \param txPowerDbm the nominal tx power in dBm
   */
  // NOTICE VERY WELL: Current  Model linear or constant as possible choices
  void SetTxCurrentFromModel (double txPowerDbm);

  /**
   * \brief Changes state of the LoraRadioEnergyMode.
   *
   * \param newState New state the lora radio is in.
   *
   * Implements DeviceEnergyModel::ChangeState.
   */
  void ChangeState (int newState);

  /**
   * \brief Handles energy depletion.
   *
   * Implements DeviceEnergyModel::HandleEnergyDepletion
   */
  void HandleEnergyDepletion (void);

  /**
   * \brief Handles energy recharged.
   *
   * Implements DeviceEnergyModel::HandleEnergyChanged
   */
  void HandleEnergyChanged (void);

  /**
   * \brief Handles energy recharged.
   *
   * Implements DeviceEnergyModel::HandleEnergyRecharged
   */
  void HandleEnergyRecharged (void);

  /**
   * \returns Pointer to the PHY listener.
   */
  LoraRadioEnergyModelPhyListener * GetPhyListener (void);


private:
  void DoDispose (void);

  /**
   * \returns Current draw of device, at current state.
   *
   * Implements DeviceEnergyModel::GetCurrentA.
   */
  double DoGetCurrentA (void) const;

  /**
   * \param state New state the radio device is currently in.
   *
   * Sets current state. This function is private so that only the energy model
   * can change its own state.
   */
  void SetLoraRadioState (const EndDeviceLoraPhy::State state);

  Ptr<EnergySource> m_source; ///< energy source

  // Member variables for current draw in different radio modes.
  double m_txCurrentA; ///< transmit current
  double m_rxCurrentA; ///< receive current
  double m_idleCurrentA; ///< idle current
  double m_sleepCurrentA; ///< sleep current
  // NOTICE VERY WELL: Current  Model linear or constant as possible choices
  Ptr<LoraTxCurrentModel> m_txCurrentModel; ///< current model

  /// This variable keeps track of the total energy consumed by this model.
  TracedValue<double> m_totalEnergyConsumption;

  // State variables.
  EndDeviceLoraPhy::State m_currentState;  ///< current state the radio is in
  Time m_lastUpdateTime;          ///< time stamp of previous energy update

  uint8_t m_nPendingChangeState; ///< pending state change
  bool m_isSupersededChangeState; ///< superseded change state

  /// Energy depletion callback
  LoraRadioEnergyDepletionCallback m_energyDepletionCallback;

  /// Energy recharged callback
  LoraRadioEnergyRechargedCallback m_energyRechargedCallback;

  /// EndDeviceLoraPhy listener
  LoraRadioEnergyModelPhyListener *m_listener;
};

} // namespace ns3

}
#endif /* LORA_RADIO_ENERGY_MODEL_H */
