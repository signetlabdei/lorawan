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
 *          Martina Capuzzo <capuzzom@dei.unipd.it>
 */

#ifndef NETWORK_STATUS_H
#define NETWORK_STATUS_H

#include "ns3/device-status.h"
#include "ns3/gateway-status.h"

namespace ns3 {

/**
 * This class represents the knowledge about the state of the network that is
 * available at the Network Server. It is essentially a collection of two maps:
 * one containing DeviceStatus objects, and the other containing GatewayStatus
 * objects.
 */
class NetworkStatus
{
public:
  static TypeId GetTypeId (void);

  NetworkStatus ();
  virtual ~NetworkStatus ();

  /**
   * Inform the NetworkStatus that this node is connected to the network.
   * This method will create a DeviceStatus object for the new node (if it
   * doesn't already exist).
   */
  void AddNode (Ptr<Node> node);

  /**
   * Add this gateway to the list of gateways connected to the network.
   * Each GW is identified by its Address in the NS-GWs network.
   */
  void AddGateway (Ptr<Node> gateway, Address& address);

  /**
   * Receive a packet from a gateway.
   * \param packet the received packet
   */
  bool Receive (Ptr<NetDevice> device, Ptr<const Packet> packet,
                uint16_t protocol, const Address& address);

protected:
  Ptr<NetworkStatusScheduler> m_scheduler;
  Ptr<NetworkStatusController> m_controller;
  Ptr<NetworkStatus> m_status;
};

} /* namespace ns3 */

#endif /* NETWORK_SERVER_H */

std::map<LoraDeviceAddress,DeviceStatus> m_deviceStatuses;

std::map<Address,GatewayStatus> m_gatewayStatuses;
