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
     * @tparam Ts \deduced Argument types
     * @param name the name of the model to set
     * @param [in] args Name and AttributeValue pairs to set.
     *
     * Configure a Transmission Current model for this EnergySource.
     */
    template <typename... Ts>
    void SetTxCurrentModel(std::string name, Ts&&... args);

  private:
    /**
     * @param device Pointer to the NetDevice to install DeviceEnergyModel.
     * @param source Pointer to EnergySource to install.
     * @return Ptr<DeviceEnergyModel>.
     *
     * Implements DeviceEnergyModel::Install.
     */
    Ptr<energy::DeviceEnergyModel> DoInstall(Ptr<NetDevice> device,
                                             Ptr<energy::EnergySource> source) const override;

  private:
    ObjectFactory m_radioEnergy;    ///< radio energy
    ObjectFactory m_txCurrentModel; ///< transmit current model
};

template <typename... Ts>
void
LoraRadioEnergyModelHelper::SetTxCurrentModel(std::string name, Ts&&... args)
{
    m_txCurrentModel = ObjectFactory(name, std::forward<Ts>(args)...);
}

} // namespace lorawan
} // namespace ns3

#endif /* LORA_RADIO_ENERGY_MODEL_HELPER_H */
