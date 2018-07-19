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
 * Authors: Martina Capuzzo <capuzzom@dei.unipd.it>
 *          Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef NETWORK_STATUS_H
#define NETWORK_STATUS_H

#include "ns3/device-status.h"
#include "ns3/end-device-status.h"
#include "ns3/gateway-status.h"
#include "ns3/lora-device-address.h"
#include "ns3/network-server-controller.h"
#include "ns3/network-server-scheduler.h"

namespace ns3 {

  /**
   * This class represents the knowledge about the state of the network that is
   * available at the Network Server. It is essentially a collection of two maps:
   * one containing DeviceStatus objects, and the other containing GatewayStatus
   * objects.
   *
   * This class is meant to be queried by NetworkController components, which
   * can decide to take action based on the current status of the network.
   */
  class NetworkStatus : public Object
  {
  public:
    static TypeId GetTypeId (void);

    NetworkStatus ();
    virtual ~NetworkStatus ();

    /**
     * Add a device to the ones that are tracked by this NetworkStatus object.
     */
    void AddNode (Ptr<EndDeviceLoraMac> edMac);

    /**
     * Add this gateway to the list of gateways connected to the network.
     *
     * Each GW is identified by its Address in the NS-GW network.
     */
    void AddGateway (Address& address, GatewayStatus gwStatus);

    /**
     * Update network status on the received packet.
     *
     * \param packet the received packet.
     * \param address the gateway this packet was received from.
     */
    void OnReceivedPacket (Ptr<const Packet> packet, const Address& gwaddress);

  protected:
    // Ptr<NetworkServerScheduler> m_scheduler;
    Ptr<NetworkServerController> m_controller;
    std::map<LoraDeviceAddress, EndDeviceStatus> m_endDeviceStatuses;
    std::map<Address, GatewayStatus> m_gatewayStatuses;
  };

} /* namespace ns3 */

#endif /* NETWORK_STATUS_H */
