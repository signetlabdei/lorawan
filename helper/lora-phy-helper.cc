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
{
}

void
LoraPhyHelper::SetChannel(Ptr<LoraChannel> channel)
{
    m_channel = channel;
}

void
LoraPhyHelper::SetDeviceType(enum DeviceType dt)
{
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

void
LoraPhyHelper::Set(std::string name, const AttributeValue& v)
{
    m_phy.Set(name, v);
}

Ptr<LoraPhy>
LoraPhyHelper::Create(Ptr<Node> node, Ptr<LoraNetDevice> device) const
{
    auto phy = m_phy.Create<LoraPhy>();
    phy->SetChannel(m_channel);
    phy->SetDevice(device);
    phy->SetMobility(node->GetObject<MobilityModel>());
    device->SetPhy(phy);
    return phy;
}

} // namespace lorawan
} // namespace ns3
