/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "lora-phy-helper.h"

#include "ns3/log.h"
#include "ns3/sub-band.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraPhyHelper");

LoraPhyHelper::LoraPhyHelper()
    : m_maxReceptionPaths(8),
      m_txPriority(true)
{
    NS_LOG_FUNCTION(this);
}

void
LoraPhyHelper::SetChannel(Ptr<LoraChannel> channel)
{
    m_channel = channel;
}

void
LoraPhyHelper::SetDeviceType(enum DeviceType dt)
{
    NS_LOG_FUNCTION(this << dt);
    switch (dt)
    {
    case GW:
        m_phy.SetTypeId("ns3::SimpleGatewayLoraPhy");
        break;
    case ED:
        m_phy.SetTypeId("ns3::SimpleEndDeviceLoraPhy");
        break;
    }
}

TypeId
LoraPhyHelper::GetDeviceType() const
{
    NS_LOG_FUNCTION(this);
    return m_phy.GetTypeId();
}

void
LoraPhyHelper::Set(std::string name, const AttributeValue& v)
{
    m_phy.Set(name, v);
}

Ptr<LoraPhy>
LoraPhyHelper::Install(Ptr<Node> node, Ptr<NetDevice> device) const
{
    NS_LOG_FUNCTION(this << node->GetId() << device);

    // Create the PHY and set its channel
    Ptr<LoraPhy> phy = m_phy.Create<LoraPhy>();
    phy->SetChannel(m_channel);

    // Configuration is different based on the kind of device we have to create
    std::string typeId = m_phy.GetTypeId().GetName();
    if (typeId == "ns3::SimpleGatewayLoraPhy")
    {
        // Inform the channel of the presence of this PHY
        m_channel->Add(phy);

        // For now, assume that the PHY will listen to the default EU channels
        // with this ReceivePath configuration:
        // 3 ReceivePaths on 868.1 MHz
        // 3 ReceivePaths on 868.3 MHz
        // 2 ReceivePaths on 868.5 MHz

        // We expect that MacHelper instances will overwrite this setting if the
        // device will operate in a different region
        std::vector<uint32_t> frequenciesHz;
        frequenciesHz.push_back(868100000);
        frequenciesHz.push_back(868300000);
        frequenciesHz.push_back(868500000);

        for (auto& f : frequenciesHz)
        {
            DynamicCast<SimpleGatewayLoraPhy>(phy)->AddFrequency(f);
        }

        int receptionPaths = 0;
        // Set maxReceptionPaths as a parameter
        // int maxReceptionPaths = 8;
        while (receptionPaths < m_maxReceptionPaths)
        {
            DynamicCast<SimpleGatewayLoraPhy>(phy)->AddReceptionPath();
            receptionPaths++;
        }
    }
    else if (typeId == "ns3::SimpleEndDeviceLoraPhy")
    {
        // The line below can be commented to speed up uplink-only simulations.
        // This implies that the LoraChannel instance will only know about
        // Gateways, and it will not lose time delivering packets and interference
        // information to devices which will never listen.

        m_channel->Add(phy);
    }

    // Link the PHY to its net device
    phy->SetDevice(device);

    return phy;
}

void
LoraPhyHelper::SetMaxReceptionPaths(int maxReceptionPaths)
{
    NS_LOG_FUNCTION(this << maxReceptionPaths);
    m_maxReceptionPaths = maxReceptionPaths;
}

void
LoraPhyHelper::SetGatewayTransmissionPriority(bool txPriority)
{
    m_txPriority = txPriority;
}
} // namespace lorawan
} // namespace ns3
