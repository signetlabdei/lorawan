/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Romagnolo Stefano <romagnolostefano93@gmail.com>
 *          Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LORA_TX_CURRENT_MODEL_H
#define LORA_TX_CURRENT_MODEL_H

#include "ns3/object.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Model the transmit current as a function of the transmit power and
 * mode.
 */
class LoraTxCurrentModel : public Object
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    LoraTxCurrentModel();           //!< Default constructor
    ~LoraTxCurrentModel() override; //!< Destructor

    /**
     * Get the current for transmission at this power.
     *
     * @param txPowerDbm The nominal tx power in dBm.
     * @return The transmit current (in Ampere).
     */
    virtual double CalcTxCurrent(double txPowerDbm) const = 0;
};

/**
 * @ingroup lorawan
 *
 * A linear model of the transmission current for a LoRa device, based on the
 * WiFi model.
 */
class LinearLoraTxCurrentModel : public LoraTxCurrentModel
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    LinearLoraTxCurrentModel();           //!< Default constructor
    ~LinearLoraTxCurrentModel() override; //!< Destructor

    /**
     * Set the power amplifier efficiency.
     *
     * @param eta The power amplifier efficiency.
     */
    void SetEta(double eta);

    /**
     * Set the supply voltage.
     *
     * @param voltage The supply voltage [Volts].
     */
    void SetVoltage(double voltage);

    /**
     * Set the current in the STANDBY state.
     *
     * @param idleCurrent The idle current value [Ampere].
     */
    void SetStandbyCurrent(double idleCurrent);

    /**
     * Get the power amplifier efficiency.
     *
     * @return The power amplifier efficiency.
     */
    double GetEta() const;

    /**
     * Get the supply voltage.
     *
     * @return The supply voltage [Volts].
     */
    double GetVoltage() const;

    /**
     * Get the current in the STANDBY state.
     *
     * @return The idle current value [Ampere].
     */
    double GetStandbyCurrent() const;

    double CalcTxCurrent(double txPowerDbm) const override;

  private:
    double m_eta;         //!< ETA
    double m_voltage;     //!< Voltage
    double m_idleCurrent; //!< Standby current
};

/**
 * @ingroup lorawan
 *
 * A constant model of the transmission current for a LoRa device, always yielding the same
 * current independently from the transmission power provided.
 */
class ConstantLoraTxCurrentModel : public LoraTxCurrentModel
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    ConstantLoraTxCurrentModel();           //!< Default constructor
    ~ConstantLoraTxCurrentModel() override; //!< Destructor

    /**
     * Set the current in the TX state.
     *
     * @param txCurrent The TX current value [Ampere].
     */
    void SetTxCurrent(double txCurrent);

    /**
     * Get the current of the TX state.
     *
     * @return The TX current value.
     */
    double GetTxCurrent() const;

    double CalcTxCurrent(double txPowerDbm) const override;

  private:
    double m_txCurrent; //!< The transmission current [Ampere]
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_TX_CURRENT_MODEL_H */
