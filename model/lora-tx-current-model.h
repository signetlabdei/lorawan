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
 * @brief Model the transmit current as a function of the transmit power and mode
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
 * @brief A linear model of the transmission current for a LoRa device
 *
 * This model assumes that the transmit current is a linear function of the nominal transmit power
 * used to send the frame. Adapted from LinearWifiTxCurrentModel, the default values are obtained by
 * line interpolation of the (7dBm, 18mA) and (13dBm, 28mA) values for TX power (converted to Watt)
 * and respective current found in the SX1272/73 Datasheet, Rev. 4, Jan. 2019.
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

    double CalcTxCurrent(double txPowerDbm) const override;

  private:
    double m_eta;         //!< ETA
    double m_voltage;     //!< Voltage
    double m_baseCurrent; //!< Base current
};

} // namespace lorawan
} // namespace ns3

#endif /* LORA_TX_CURRENT_MODEL_H */
