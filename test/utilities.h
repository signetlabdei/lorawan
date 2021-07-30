/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Padova
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
 * Authors: Davide Magrin <magrinda@dei.unipd.it>
 */
#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/forwarder-helper.h"
#include "ns3/network-server-helper.h"

namespace ns3 {
namespace lorawan {

struct NetworkComponents
{
  Ptr<LoraChannel> channel;
  NodeContainer endDevices;
  NodeContainer gateways;
  Ptr<Node> nsNode;
};

Ptr<LoraChannel> CreateChannel (void);

NodeContainer CreateEndDevices (int nDevices, MobilityHelper mobility,
                                Ptr<LoraChannel> channel);

NodeContainer CreateGateways (int nGateways, MobilityHelper mobility,
                              Ptr<LoraChannel> channel);

Ptr<Node> CreateNetworkServer (NodeContainer endDevices,
                               NodeContainer gateways);

template <typename T>
Ptr<T>
GetMacLayerFromNode (Ptr<Node> n)
{
  return n->GetDevice (0)->GetObject<LoraNetDevice> ()->GetMac ()->GetObject<T> ();
}

NetworkComponents InitializeNetwork (int nDevices, int nGateways);
}

}
#endif
