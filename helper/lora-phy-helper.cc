/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "ns3/lora-phy-helper.h"

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
    NS_LOG_FUNCTION(this << channel);
    m_channel = channel;
}

void
LoraPhyHelper::SetDeviceType(enum DeviceType dt)
{
    NS_LOG_FUNCTION(this << dt);
    switch (dt)
    {
    case GW:
        m_phy.SetTypeId("ns3::GatewayLoraPhy");
        break;
    case ED:
        m_phy.SetTypeId("ns3::SimpleEndDeviceLoraPhy");
        break;
    }
}

TypeId
LoraPhyHelper::GetDeviceType(void) const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_phy.GetTypeId();
}

void
LoraPhyHelper::Set(std::string name, const AttributeValue& v)
{
    NS_LOG_FUNCTION(this << name);
    m_phy.Set(name, v);
}

Ptr<LoraPhy>
LoraPhyHelper::Create(Ptr<Node> node, Ptr<NetDevice> device) const
{
    NS_LOG_FUNCTION(this << node->GetId() << device);
    // Create the PHY and set its channel
    auto phy = m_phy.Create<LoraPhy>();
    phy->SetChannel(m_channel);
    // Configuration is different based on the kind of device we have to create
    auto typeId = m_phy.GetTypeId().GetName();
    if (typeId == "ns3::GatewayLoraPhy")
    {
        // Inform the channel of the presence of this PHY
        m_channel->Add(phy);
        // Create the reception paths of the gateway
        phy->GetObject<GatewayLoraPhy>()->CreateReceptionPaths(m_maxReceptionPaths);
    }
    else if (typeId == "ns3::SimpleEndDeviceLoraPhy")
    {
        // The line below can be commented to speed up uplink-only simulations.
        // This implies that the LoraChannel instance will only know about
        // Gateways, and it will not lose time delivering packets and interference
        // information to devices which will never listen.
        m_channel->Add(phy, true);
    }
    // Link the PHY to its net device
    phy->SetDevice(device);
    phy->SetMobility(node->GetObject<MobilityModel>());
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
    NS_LOG_FUNCTION(this << txPriority);
    m_txPriority = txPriority;
}
} // namespace lorawan
} // namespace ns3
