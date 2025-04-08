/*
 * Copyright (c) 2018 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "ns3/forwarder-helper.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/network-server-helper.h"
#include "ns3/position-allocator.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Stores the main elements of a simulated LoRaWAN network
 */
struct NetworkComponents
{
    Ptr<LoraChannel> channel; //!< A pointer to the LoraChannel object.
    NodeContainer endDevices; //!< Container of the end device nodes.
    NodeContainer gateways;   //!< Container of the gateway nodes.
    Ptr<Node> nsNode;         //!< A pointer to the network server Node.
};

Ptr<LoraChannel> CreateChannel();

NodeContainer CreateEndDevices(int nDevices, MobilityHelper mobility, Ptr<LoraChannel> channel);

NodeContainer CreateGateways(int nGateways, MobilityHelper mobility, Ptr<LoraChannel> channel);

Ptr<Node> CreateNetworkServer(NodeContainer endDevices, NodeContainer gateways);

template <typename T>
Ptr<T>
GetMacLayerFromNode(Ptr<Node> n)
{
    return DynamicCast<T>(DynamicCast<LoraNetDevice>(n->GetDevice(0))->GetMac());
}

NetworkComponents InitializeNetwork(int nDevices, int nGateways);
} // namespace lorawan

} // namespace ns3
#endif
