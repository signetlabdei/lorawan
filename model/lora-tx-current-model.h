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

#ifndef LORA_TX_CURRENT_MODEL_H
#define LORA_TX_CURRENT_MODEL_H

#include "ns3/object.h"

namespace ns3
{
namespace lorawan
{

/**
 * \ingroup energy
 *
 * \brief Model the transmit current as a function of the transmit power and
 * mode.
 */
class LoraTxCurrentModel : public Object
{
  public:
    static TypeId GetTypeId();

    LoraTxCurrentModel();
    ~LoraTxCurrentModel() override;

    /**
     * Get the current for transmission at this power.
     *
     * \param txPowerDbm The nominal tx power in dBm
     * \returns The transmit current (in Ampere)
     */
    virtual double CalcTxCurrent(double txPowerDbm) const = 0;
};

/**
 * A linear model of the transmission current for a LoRa device, based on the
 * WiFi model.
 */
class LinearLoraTxCurrentModel : public LoraTxCurrentModel
{
  public:
    static TypeId GetTypeId();

    LinearLoraTxCurrentModel();
    ~LinearLoraTxCurrentModel() override;

    /**
     * \param eta (dimension-less)
     *
     * Set the power amplifier efficiency.
     */
    void SetEta(double eta);

    /**
     * \param voltage (Volts)
     *
     * Set the supply voltage.
     */
    void SetVoltage(double voltage);

    /**
     * \param idleCurrent (Ampere)
     *
     * Set the current in the STANDBY state.
     */
    void SetStandbyCurrent(double idleCurrent);

    /**
     * \return the power amplifier efficiency.
     */
    double GetEta() const;

    /**
     * \return the supply voltage.
     */
    double GetVoltage() const;

    /**
     * \return the current in the STANDBY state.
     */
    double GetStandbyCurrent() const;

    double CalcTxCurrent(double txPowerDbm) const override;

  private:
    double m_eta;         //!< ETA
    double m_voltage;     //!< Voltage
    double m_idleCurrent; //!< Standby current
};

class ConstantLoraTxCurrentModel : public LoraTxCurrentModel
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    ConstantLoraTxCurrentModel();
    ~ConstantLoraTxCurrentModel() override;

    /**
     * \param txCurrent (Ampere)
     *
     * Set the current in the TX state.
     */
    void SetTxCurrent(double txCurrent);

    /**
     * \return the current in the TX state.
     */
    double GetTxCurrent() const;

    double CalcTxCurrent(double txPowerDbm) const override;

  private:
    double m_txCurrent;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_TX_CURRENT_MODEL_H */
