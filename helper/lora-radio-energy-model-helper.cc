/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Romagnolo Stefano <romagnolostefano93@gmail.com>
 */

#include "lora-radio-energy-model-helper.h"

#include "ns3/lora-net-device.h"
#include "ns3/lora-radio-energy-model.h"
#include "ns3/lora-tx-current-model.h"

namespace ns3
{
namespace lorawan
{

LoraRadioEnergyModelHelper::LoraRadioEnergyModelHelper()
{
    m_radioEnergy.SetTypeId("ns3::LoraRadioEnergyModel");
}

LoraRadioEnergyModelHelper::~LoraRadioEnergyModelHelper()
{
}

void
LoraRadioEnergyModelHelper::Set(std::string name, const AttributeValue& v)
{
    m_radioEnergy.Set(name, v);
}

/*
 * Private function starts here.
 */

Ptr<energy::DeviceEnergyModel>
LoraRadioEnergyModelHelper::DoInstall(Ptr<NetDevice> device, Ptr<energy::EnergySource> source) const
{
    NS_ASSERT(device);
    NS_ASSERT(source);
    // check if device is LoraNetDevice
    std::string deviceName = device->GetInstanceTypeId().GetName();
    if (deviceName != "ns3::LoraNetDevice")
    {
        NS_FATAL_ERROR("NetDevice type is not LoraNetDevice!");
    }
    Ptr<Node> node = device->GetNode();
    Ptr<LoraRadioEnergyModel> model = m_radioEnergy.Create<LoraRadioEnergyModel>();
    NS_ASSERT(model);

    Ptr<LoraNetDevice> loraDevice = DynamicCast<LoraNetDevice>(device);
    Ptr<EndDeviceLoraPhy> loraPhy = DynamicCast<EndDeviceLoraPhy>(loraDevice->GetPhy());
    // add model to device model list in energy source
    source->AppendDeviceEnergyModel(model);
    // set energy source pointer
    model->SetEnergySource(source);
    // create and register energy model PHY listener
    loraPhy->RegisterListener(model->GetPhyListener());
    //
    if (m_txCurrentModel.GetTypeId().GetUid())
    {
        Ptr<LoraTxCurrentModel> txcurrent = m_txCurrentModel.Create<LoraTxCurrentModel>();
        model->SetTxCurrentModel(txcurrent);
    }
    return model;
}

} // namespace lorawan
} // namespace ns3
