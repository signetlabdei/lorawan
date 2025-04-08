/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Romagnolo Stefano <romagnolostefano93@gmail.com>
 */

#ifndef LORA_RADIO_ENERGY_MODEL_HELPER_H
#define LORA_RADIO_ENERGY_MODEL_HELPER_H

#include "ns3/energy-model-helper.h"
#include "ns3/lora-radio-energy-model.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Installs LoraRadioEnergyModel on devices.
 *
 * This installer installs LoraRadioEnergyModel for only LoraNetDevice objects.
 */
class LoraRadioEnergyModelHelper : public DeviceEnergyModelHelper
{
  public:
    LoraRadioEnergyModelHelper();           //!< Default constructor
    ~LoraRadioEnergyModelHelper() override; //!< Destructor

    /**
     * @param name The name of the attribute to set.
     * @param v The value of the attribute.
     *
     * Sets an attribute of the underlying PHY object.
     */
    void Set(std::string name, const AttributeValue& v) override;

    /**
     * @param name The name of the model to set.
     * @param n0 The name of the attribute to set.
     * @param v0 The value of the attribute to set.
     * @param n1 The name of the attribute to set.
     * @param v1 The value of the attribute to set.
     * @param n2 The name of the attribute to set.
     * @param v2 The value of the attribute to set.
     * @param n3 The name of the attribute to set.
     * @param v3 The value of the attribute to set.
     * @param n4 The name of the attribute to set.
     * @param v4 The value of the attribute to set.
     * @param n5 The name of the attribute to set.
     * @param v5 The value of the attribute to set.
     * @param n6 The name of the attribute to set.
     * @param v6 The value of the attribute to set.
     * @param n7 The name of the attribute to set.
     * @param v7 The value of the attribute to set.
     *
     * Configure a Transmission Current model for this EnergySource.
     */
    void SetTxCurrentModel(std::string name,
                           std::string n0 = "",
                           const AttributeValue& v0 = EmptyAttributeValue(),
                           std::string n1 = "",
                           const AttributeValue& v1 = EmptyAttributeValue(),
                           std::string n2 = "",
                           const AttributeValue& v2 = EmptyAttributeValue(),
                           std::string n3 = "",
                           const AttributeValue& v3 = EmptyAttributeValue(),
                           std::string n4 = "",
                           const AttributeValue& v4 = EmptyAttributeValue(),
                           std::string n5 = "",
                           const AttributeValue& v5 = EmptyAttributeValue(),
                           std::string n6 = "",
                           const AttributeValue& v6 = EmptyAttributeValue(),
                           std::string n7 = "",
                           const AttributeValue& v7 = EmptyAttributeValue());

  private:
    /**
     * @param device Pointer to the NetDevice to install DeviceEnergyModel.
     * @param source Pointer to EnergySource to install.
     * @return Ptr<DeviceEnergyModel>.
     *
     * Implements DeviceEnergyModel::Install.
     */
    Ptr<DeviceEnergyModel> DoInstall(Ptr<NetDevice> device,
                                     Ptr<EnergySource> source) const override;

  private:
    ObjectFactory m_radioEnergy;    ///< radio energy
    ObjectFactory m_txCurrentModel; ///< transmit current model
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_RADIO_ENERGY_MODEL_HELPER_H */
