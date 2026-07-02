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

#include "ns3/device-energy-model.h"
#include "ns3/traced-value.h"

namespace ns3
{
namespace lorawan
{

class LoraTxCurrentModel;

/**
 * @ingroup lorawan
 *
 * A EndDeviceLoraPhy listener class for notifying the LoraRadioEnergyModel of Lora radio
 * state change.
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
     * @brief Sets the change state callback. Used by helper class.
     *
     * @param callback Change state callback.
     */
    void SetChangeStateCallback(energy::DeviceEnergyModel::ChangeStateCallback callback);

    /**
     * @brief Sets the update tx current callback.
     *
     * @param callback Update tx current callback.
     */
    void SetUpdateTxCurrentCallback(UpdateTxCurrentCallback callback);

    void NotifySleep() override;
    void NotifyStandby() override;
    void NotifyTx(double txPowerDbm) override;
    void NotifyRxEnabled() override;
    void NotifyRxActive() override;

  private:
    /**
     * Change state callback used to notify the LoraRadioEnergyModel of a state
     * change.
     */
    energy::DeviceEnergyModel::ChangeStateCallback m_changeStateCallback;

    /**
     * Callback used to update the tx current stored in LoraRadioEnergyModel based on
     * the nominal tx power used to transmit the current frame.
     */
    UpdateTxCurrentCallback m_updateTxCurrentCallback;
};

/**
 * @ingroup lorawan
 *
 * @brief A LoRa radio energy model.
 *
 * 4 energy states are defined for the radio: TX, RX, STANDBY, SLEEP. Default state is STANDBY.
 * The different types of transactions that are defined are:
 *  1. Tx: State goes from SLEEP/STANDBY to TX, radio is in TX state,
 *     then state goes from TX to STANDBY.
 *  2. Rx: State goes from SLEEP/STANDBY to RX, radio is in RX state,
 *     then state goes from RX to STANDBY.
 *  3. Sleep: State goes from STANDBY to SLEEP.
 * The class keeps track of what state the radio is currently in.
 *
 * Energy calculation: For each transaction, this model notifies EnergySource
 * object. The EnergySource object will query this model for the total current.
 * Then the EnergySource object uses the total current to calculate energy.
 */
class LoraRadioEnergyModel : public energy::DeviceEnergyModel
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

    LoraRadioEnergyModel();           //!< Default constructor
    ~LoraRadioEnergyModel() override; //!< Destructor

    /**
     * @brief Sets pointer to EnergySource installed on node.
     *
     * @param source Pointer to EnergySource installed on node.
     *
     * Implements energy::DeviceEnergyModel::SetEnergySource.
     */
    void SetEnergySource(Ptr<energy::EnergySource> source) override;

    /**
     * @return Total energy consumption of the wifi device.
     *
     * Implements energy::DeviceEnergyModel::GetTotalEnergyConsumption.
     */
    double GetTotalEnergyConsumption() const override;

    // Setter & getters for state power consumption.
    /**
     * @brief Gets standby current.
     *
     * @return Standby current [A] of the lora device.
     */
    double GetStandbyCurrentA() const;
    /**
     * @brief Sets standby current.
     *
     * @param standbyCurrentA The standby current [A].
     */
    void SetStandbyCurrentA(double standbyCurrentA);
    /**
     * @brief Gets transmit current.
     *
     * @return Transmit current [A] of the lora device.
     */
    double GetTxCurrentA() const;
    /**
     * @brief Sets transmit current.
     *
     * @param txCurrentA The transmit current [A].
     */
    void SetTxCurrentA(double txCurrentA);
    /**
     * @brief Gets receive current.
     *
     * @return Receive current [A] of the lora device.
     */
    double GetRxCurrentA() const;
    /**
     * @brief Sets receive current.
     *
     * @param rxCurrentA The receive current [A].
     */
    void SetRxCurrentA(double rxCurrentA);
    /**
     * @brief Gets sleep current.
     *
     * @return Sleep current [A] of the lora device.
     */
    double GetSleepCurrentA() const;
    /**
     * @brief Sets sleep current.
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
     * @param model The model used to compute the lora TX current.
     */
    void SetTxCurrentModel(Ptr<LoraTxCurrentModel> model);

    /**
     * @brief Calls the CalcTxCurrent method of the tx current model to
     *        compute the tx current based on such model.
     *
     * @param txPowerDbm The nominal tx power in dBm.
     */
    void SetTxCurrentFromModel(double txPowerDbm);

    /**
     * Changes state of the LoraRadioEnergyModel.
     *
     * @param newState New state the lora radio is in.
     *
     * Implements energy::DeviceEnergyModel::ChangeState.
     */
    void ChangeState(int newState) override;

    /**
     * Handles energy depletion.
     *
     * Implements energy::DeviceEnergyModel::HandleEnergyDepletion.
     */
    void HandleEnergyDepletion() override;

    /**
     * Handles energy recharged.
     *
     * Implements energy::DeviceEnergyModel::HandleEnergyRecharged.
     */
    void HandleEnergyRecharged() override;

    /**
     * Handles energy changed.
     *
     * Implements energy::DeviceEnergyModel::HandleEnergyChanged.
     */
    void HandleEnergyChanged() override;

    /**
     * @return Pointer to the PHY listener.
     */
    std::shared_ptr<LoraRadioEnergyModelPhyListener> GetPhyListener();

  private:
    void DoDispose() override;

    /**
     * @param state the lora state
     * @returns draw of device at given state.
     */
    double GetStateA(EndDeviceLoraPhy::State state) const;

    /**
     * @return Current draw of device, at current state.
     *
     * Implements energy::DeviceEnergyModel::GetCurrentA.
     */
    double DoGetCurrentA() const override;

    /**
     * @param state New state the radio device is currently in.
     *
     * Sets current state. This function is private so that only the energy model
     * can change its own state.
     */
    void SetLoraRadioState(const EndDeviceLoraPhy::State state);

    Ptr<energy::EnergySource> m_source; //!< energy source

    // Member variables for current draw in different radio modes.
    double m_txCurrentA;                      //!< transmit current
    double m_rxCurrentA;                      //!< receive current
    double m_standbyCurrentA;                 //!< standby current
    double m_sleepCurrentA;                   //!< sleep current
    Ptr<LoraTxCurrentModel> m_txCurrentModel; ///< current model

    /// This variable keeps track of the total energy consumed by this model.
    TracedValue<double> m_totalEnergyConsumption;

    // State variables.
    EndDeviceLoraPhy::State m_currentState; ///< current state the radio is in
    Time m_lastUpdateTime;                  ///< time stamp of previous energy update

    uint8_t m_nPendingChangeState; ///< pending state change

    /// Energy depletion callback
    LoraRadioEnergyDepletionCallback m_energyDepletionCallback;

    /// Energy recharged callback
    LoraRadioEnergyRechargedCallback m_energyRechargedCallback;

    /// EndDeviceLoraPhy listener
    std::shared_ptr<LoraRadioEnergyModelPhyListener> m_listener;
};

} // namespace lorawan
} // namespace ns3

#endif /* LORA_RADIO_ENERGY_MODEL_H */
