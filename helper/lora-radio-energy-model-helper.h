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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
 * \ingroup energy
 * \brief Assign LoraRadioEnergyModel to wifi devices.
 *
 * This installer installs LoraRadioEnergyModel for only LoraNetDevice objects.
 *
 */
class LoraRadioEnergyModelHelper : public DeviceEnergyModelHelper
{
  public:
    /**
     * Construct a helper which is used to add a radio energy model to a node
     */
    LoraRadioEnergyModelHelper();

    /**
     * Destroy a RadioEnergy Helper
     */
    ~LoraRadioEnergyModelHelper() override;

    /**
     * \param name the name of the attribute to set
     * \param v the value of the attribute
     *
     * Sets an attribute of the underlying PHY object.
     */
    void Set(std::string name, const AttributeValue& v) override;

    /**
     * \param name the name of the model to set
     * \param n0 the name of the attribute to set
     * \param v0 the value of the attribute to set
     * \param n1 the name of the attribute to set
     * \param v1 the value of the attribute to set
     * \param n2 the name of the attribute to set
     * \param v2 the value of the attribute to set
     * \param n3 the name of the attribute to set
     * \param v3 the value of the attribute to set
     * \param n4 the name of the attribute to set
     * \param v4 the value of the attribute to set
     * \param n5 the name of the attribute to set
     * \param v5 the value of the attribute to set
     * \param n6 the name of the attribute to set
     * \param v6 the value of the attribute to set
     * \param n7 the name of the attribute to set
     * \param v7 the value of the attribute to set
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
     * \param device Pointer to the NetDevice to install DeviceEnergyModel.
     * \param source Pointer to EnergySource to install.
     * \returns Ptr<DeviceEnergyModel>
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
