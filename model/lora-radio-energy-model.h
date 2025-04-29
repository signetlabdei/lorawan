/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Romagnolo Stefano <romagnolostefano93@gmail.com>
 *          Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LORA_RADIO_ENERGY_MODEL_H
#define LORA_RADIO_ENERGY_MODEL_H

#include "end-device-lora-phy.h"
#include "lora-tx-current-model.h"

#include "ns3/device-energy-model.h"
#include "ns3/traced-value.h"

namespace ns3
{
namespace lorawan
{

using namespace energy;

/**
 * @ingroup lorawan
 *
 * Installable listener for LoRa physiscal layer state changes
 */
class LoraRadioEnergyModelPhyListener : public EndDeviceLoraPhyListener
{
  public:
    /**
     * Callback type for updating the transmit current based on the nominal tx power.
     */
    typedef Callback<void, double> UpdateTxCurrentCallback;

    LoraRadioEnergyModelPhyListener();           //!< Default constructor
    ~LoraRadioEnergyModelPhyListener() override; //!< Destructor

    /**
     * Sets the change state callback. Used by helper class.
     *
     * @param callback Change state callback.
     */
    void SetChangeStateCallback(DeviceEnergyModel::ChangeStateCallback callback);

    /**
     * Sets the update tx current callback.
     *
     * @param callback Update tx current callback.
     */
    void SetUpdateTxCurrentCallback(UpdateTxCurrentCallback callback);

    /**
     * Switches the LoraRadioEnergyModel to RX state.
     *
     * Defined in ns3::LoraEndDevicePhyListener.
     */
    void NotifyRxStart() override;

    /**
     * Switches the LoraRadioEnergyModel to TX state and switches back to
     * STANDBY after TX duration.
     *
     * @param txPowerDbm The nominal tx power in dBm.
     *
     * Defined in ns3::LoraEndDevicePhyListener.
     */
    void NotifyTxStart(double txPowerDbm) override;

    /**
     * Defined in ns3::LoraEndDevicePhyListener.
     */
    void NotifySleep() override;

    /**
     * Defined in ns3::LoraEndDevicePhyListener.
     */
    void NotifyStandby() override;

  private:
    /**
     * A helper function that makes scheduling m_changeStateCallback possible.
     */
    void SwitchToStandby();

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
 * @ingroup lorawan
 *
 * A LoRa radio energy model.
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
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    LoraRadioEnergyModel();
    ~LoraRadioEnergyModel() override; //!< Destructor

    /**
     * Sets pointer to EnergySouce installed on node.
     *
     * @param source Pointer to EnergySource installed on node.
     *
     * Implements DeviceEnergyModel::SetEnergySource.
     */
    void SetEnergySource(Ptr<EnergySource> source) override;

    /**
     * @return Total energy consumption of the wifi device.
     *
     * Implements DeviceEnergyModel::GetTotalEnergyConsumption.
     */
    double GetTotalEnergyConsumption() const override;

    // Setter & getters for state power consumption.
    /**
     * Gets idle current.
     *
     * @return Idle current [A] of the lora device.
     */
    double GetStandbyCurrentA() const;
    /**
     * Sets idle current.
     *
     * @param idleCurrentA The idle current [A].
     */
    void SetStandbyCurrentA(double idleCurrentA);
    /**
     * Gets transmit current.
     *
     * @return Transmit current [A] of the lora device.
     */
    double GetTxCurrentA() const;
    /**
     * Sets transmit current.
     *
     * @param txCurrentA The transmit current [A].
     */
    void SetTxCurrentA(double txCurrentA);
    /**
     * Gets receive current.
     *
     * @return Receive current [A] of the lora device.
     */
    double GetRxCurrentA() const;
    /**
     * Sets receive current.
     *
     * @param rxCurrentA The receive current [A].
     */
    void SetRxCurrentA(double rxCurrentA);
    /**
     * Gets sleep current.
     *
     * @return Sleep current [A] of the lora device.
     */
    double GetSleepCurrentA() const;
    /**
     * Sets sleep current.
     *
     * @param sleepCurrentA The sleep current [A].
     */
    void SetSleepCurrentA(double sleepCurrentA);

    /**
     * @return Current state.
     */
    EndDeviceLoraPhy::State GetCurrentState() const;

    /**
     * @param callback Callback function.
     *
     * Sets callback for energy depletion handling.
     */
    void SetEnergyDepletionCallback(LoraRadioEnergyDepletionCallback callback);

    /**
     * @param callback Callback function.
     *
     * Sets callback for energy recharged handling.
     */
    void SetEnergyRechargedCallback(LoraRadioEnergyRechargedCallback callback);

    /**
     * @param model The model used to compute the lora tx current.
     */
    // NOTICE VERY WELL: Current  Model linear or constant as possible choices
    void SetTxCurrentModel(Ptr<LoraTxCurrentModel> model);

    /**
     * Calls the CalcTxCurrent method of the tx current model to
     *        compute the tx current based on such model.
     *
     * @param txPowerDbm The nominal tx power in dBm.
     */
    // NOTICE VERY WELL: Current  Model linear or constant as possible choices
    void SetTxCurrentFromModel(double txPowerDbm);

    /**
     * Changes state of the LoraRadioEnergyMode.
     *
     * @param newState New state the lora radio is in.
     *
     * Implements DeviceEnergyModel::ChangeState.
     */
    void ChangeState(int newState) override;

    /**
     * Handles energy depletion.
     *
     * Implements DeviceEnergyModel::HandleEnergyDepletion.
     */
    void HandleEnergyDepletion() override;

    /**
     * Handles energy recharged.
     *
     * Implements DeviceEnergyModel::HandleEnergyChanged.
     */
    void HandleEnergyChanged() override;

    /**
     * Handles energy recharged.
     *
     * Implements DeviceEnergyModel::HandleEnergyRecharged.
     */
    void HandleEnergyRecharged() override;

    /**
     * @return Pointer to the PHY listener.
     */
    LoraRadioEnergyModelPhyListener* GetPhyListener();

  private:
    void DoDispose() override;

    /**
     * @return Current draw of device, at current state.
     *
     * Implements DeviceEnergyModel::GetCurrentA.
     */
    double DoGetCurrentA() const override;

    /**
     * @param state New state the radio device is currently in.
     *
     * Sets current state. This function is private so that only the energy model
     * can change its own state.
     */
    void SetLoraRadioState(const EndDeviceLoraPhy::State state);

    Ptr<EnergySource> m_source; ///< energy source

    // Member variables for current draw in different radio modes.
    double m_txCurrentA;    ///< transmit current
    double m_rxCurrentA;    ///< receive current
    double m_idleCurrentA;  ///< idle current
    double m_sleepCurrentA; ///< sleep current
    // NOTICE VERY WELL: Current  Model linear or constant as possible choices
    Ptr<LoraTxCurrentModel> m_txCurrentModel; ///< current model

    /// This variable keeps track of the total energy consumed by this model.
    TracedValue<double> m_totalEnergyConsumption;

    // State variables.
    EndDeviceLoraPhy::State m_currentState; ///< current state the radio is in
    Time m_lastUpdateTime;                  ///< time stamp of previous energy update

    uint8_t m_nPendingChangeState;  ///< pending state change
    bool m_isSupersededChangeState; ///< superseded change state

    /// Energy depletion callback
    LoraRadioEnergyDepletionCallback m_energyDepletionCallback;

    /// Energy recharged callback
    LoraRadioEnergyRechargedCallback m_energyRechargedCallback;

    /// EndDeviceLoraPhy listener
    LoraRadioEnergyModelPhyListener* m_listener;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_RADIO_ENERGY_MODEL_H */
