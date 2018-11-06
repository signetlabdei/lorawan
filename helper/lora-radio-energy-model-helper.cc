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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Romagnolo Stefano <romagnolostefano93@gmail.com>
 */

#include "lora-radio-energy-model-helper.h"
#include "ns3/lora-net-device.h"
#include "ns3/lora-tx-current-model.h"
#include "ns3/end-device-lora-phy.h"

namespace ns3 {
namespace lorawan {

LoraRadioEnergyModelHelper::LoraRadioEnergyModelHelper ()
{
  m_radioEnergy.SetTypeId ("ns3::LoraRadioEnergyModel");
}

LoraRadioEnergyModelHelper::~LoraRadioEnergyModelHelper ()
{
}

void
LoraRadioEnergyModelHelper::Set (std::string name, const AttributeValue &v)
{
  m_radioEnergy.Set (name, v);
}

void
LoraRadioEnergyModelHelper::SetTxCurrentModel (std::string name,
                                               std::string n0, const AttributeValue& v0,
                                               std::string n1, const AttributeValue& v1,
                                               std::string n2, const AttributeValue& v2,
                                               std::string n3, const AttributeValue& v3,
                                               std::string n4, const AttributeValue& v4,
                                               std::string n5, const AttributeValue& v5,
                                               std::string n6, const AttributeValue& v6,
                                               std::string n7, const AttributeValue& v7)
{
  ObjectFactory factory;
  factory.SetTypeId (name);
  factory.Set (n0, v0);
  factory.Set (n1, v1);
  factory.Set (n2, v2);
  factory.Set (n3, v3);
  factory.Set (n4, v4);
  factory.Set (n5, v5);
  factory.Set (n6, v6);
  factory.Set (n7, v7);
  m_txCurrentModel = factory;
}


/*
 * Private function starts here.
 */

Ptr<DeviceEnergyModel>
LoraRadioEnergyModelHelper::DoInstall (Ptr<NetDevice> device,
                                       Ptr<EnergySource> source) const
{
  NS_ASSERT (device != NULL);
  NS_ASSERT (source != NULL);
  // check if device is LoraNetDevice
  std::string deviceName = device->GetInstanceTypeId ().GetName ();
  if (deviceName.compare ("ns3::LoraNetDevice") != 0)
    {
      NS_FATAL_ERROR ("NetDevice type is not LoraNetDevice!");
    }
  Ptr<Node> node = device->GetNode ();
  Ptr<LoraRadioEnergyModel> model = m_radioEnergy.Create ()->GetObject<LoraRadioEnergyModel> ();
  NS_ASSERT (model != NULL);
  // set energy source pointer
  model->SetEnergySource (source);

  // set energy depletion callback
  // if none is specified, make a callback to EndDeviceLoraPhy::SetSleepMode
  Ptr<LoraNetDevice> loraDevice = device->GetObject<LoraNetDevice> ();
  Ptr<EndDeviceLoraPhy> loraPhy = loraDevice->GetPhy ()->GetObject<EndDeviceLoraPhy> ();
  // add model to device model list in energy source
  source->AppendDeviceEnergyModel (model);
  // create and register energy model phy listener
  loraPhy->RegisterListener (model->GetPhyListener ());

  if (m_txCurrentModel.GetTypeId ().GetUid ())
    {
      Ptr<LoraTxCurrentModel> txcurrent = m_txCurrentModel.Create<LoraTxCurrentModel> ();
      model->SetTxCurrentModel (txcurrent);
    }
  return model;
}

}
} // namespace ns3
