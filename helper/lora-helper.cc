/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 */

#include "ns3/lora-helper.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraHelper");

LoraHelper::LoraHelper ()
{
}

LoraHelper::~LoraHelper ()
{
}

NetDeviceContainer
LoraHelper::Install ( const LoraPhyHelper &phyHelper,
                      const LoraMacHelper &macHelper,
                      NodeContainer c) const
{
  NS_LOG_FUNCTION_NOARGS ();

  NetDeviceContainer devices;

  // Go over the various nodes in which to install the NetDevice
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;

      // Create the LoraNetDevice
      Ptr<LoraNetDevice> device = CreateObject<LoraNetDevice> ();

      // Create the PHY
      Ptr<LoraPhy> phy = phyHelper.Create (node, device);
      NS_ASSERT (phy != 0);
      device->SetPhy (phy);
      NS_LOG_DEBUG ("Done creating the PHY");

      // Create the MAC
      Ptr<LoraMac> mac = macHelper.Create (node, device);
      NS_ASSERT (mac != 0);
      mac->SetPhy (phy);
      NS_LOG_DEBUG ("Done creating the MAC");
      device->SetMac (mac);

      node->AddDevice (device);
      devices.Add (device);
      NS_LOG_DEBUG ("node=" << node << ", mob=" << node->GetObject<MobilityModel> ());
    }
  return devices;
}

NetDeviceContainer
LoraHelper::Install ( const LoraPhyHelper &phy,
                      const LoraMacHelper &mac,
                      Ptr<Node> node) const
{
  return Install (phy, mac, NodeContainer (node));
}
}
