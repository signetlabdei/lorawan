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

#ifndef SIMPLE_NETWORK_SERVER_H
#define SIMPLE_NETWORK_SERVER_H

#include "ns3/application.h"
#include "ns3/net-device.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/packet.h"
#include "ns3/lora-device-address.h"
#include "ns3/device-status.h"
#include "ns3/gateway-status.h"
#include "ns3/node-container.h"

namespace ns3 {

/**
 * A SimpleNetworkServer is an application standing on top of a node equipped with
 * links that connect it with the gateways.
 *
 * This version of the NetworkServer isn't smart enough to handle MAC commands,
 * but can reply correctly with ACKs to confirmed uplink messages.
 */
class SimpleNetworkServer : public Application
{
public:
  static TypeId GetTypeId (void);

  SimpleNetworkServer ();
  virtual ~SimpleNetworkServer ();

  /**
   * Parse and take action on the commands contained on the frameHeader
   */
  // void ParseCommands (LoraFrameHeader frameHeader);

  /**
   * Start the NS application
   */
  void StartApplication (void);

  /**
   * Stop the NS application
   */
  void StopApplication (void);

  /**
   * Inform the SimpleNetworkServer that these nodes are connected to the network
   * This method will create a DeviceStatus object for each new node, and add it to the list
   */
  void AddNodes (NodeContainer nodes);

  /**
   * Inform the SimpleNetworkServer that this node is connected to the network
   * This method will create a DeviceStatus object for the new node (if it doesn't already exist)
   */
  void AddNode (Ptr<Node> node);

  /**
   * Add this gateway to the list of gateways connected to this NS
   */
  void AddGateway (Ptr<Node> gateway, Ptr<NetDevice> netDevice);

  /**
   * Receive a packet from a gateway
   * \param packet the received packet
   */
  bool Receive (Ptr<NetDevice> device, Ptr<const Packet> packet,
                uint16_t protocol, const Address& address);

  /**
   * Send a packet through a gateway to an ED, using the first receive window
   */
  void SendOnFirstWindow (LoraDeviceAddress address);

  /**
   * Send a packet through a gateway to an ED, using the second receive window
   */
  void SendOnSecondWindow (LoraDeviceAddress address);

  /**
   * Check whether a reply to the device with a certain address already exists
   */
  // bool HasReply (LoraDeviceAddress address);

  /**
   * Get the data rate that should be used when replying in the first receive window
   */
  uint8_t GetDataRateForReply (uint8_t receivedDataRate);

  /**
   * Get the best gateway that is available to reply to this device.
   *
   * This method assumes the gateway needs to be available at the time that
   * it is called.
   */
  Address GetGatewayForReply (LoraDeviceAddress deviceAddress, double frequency);

  /**
   * Initialize the hasReply value of the device to the given value, so that the
   * Reply can be overwritten.
   *
   * \param addr The address oh the device
   * \param hasRep The new value of the hasReply field,
   */
  void InitializeReply (LoraDeviceAddress addr, bool hasRep);


protected:
  std::map<LoraDeviceAddress,DeviceStatus> m_deviceStatuses;

  std::map<Address,GatewayStatus> m_gatewayStatuses;
};

} /* namespace ns3 */

#endif /* SIMPLE_NETWORK_SERVER_H */
