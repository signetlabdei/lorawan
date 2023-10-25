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

#include "class-a-end-device-lorawan-mac.h"
#include "end-device-status.h"
#include "gateway-status.h"
#include "lora-device-address.h"
#include "network-scheduler.h"

#include <iterator>

namespace ns3
{
namespace lorawan
{

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
    static TypeId GetTypeId();

    NetworkStatus();
    ~NetworkStatus() override;

    /**
     * Add a device to the ones that are tracked by this NetworkStatus object.
     */
    void AddNode(Ptr<ClassAEndDeviceLorawanMac> edMac);

    /**
     * Add this gateway to the list of gateways connected to the network.
     *
     * Each GW is identified by its Address in the NS-GW network.
     */
    void AddGateway(Address& address, Ptr<GatewayStatus> gwStatus);

    /**
     * Update network status on the received packet.
     *
     * \param packet the received packet.
     * \param address the gateway this packet was received from.
     */
    void OnReceivedPacket(Ptr<const Packet> packet, const Address& gwaddress);

    /**
     * Return whether the specified device needs a reply.
     *
     * \param deviceAddress the address of the device we are interested in.
     */
    bool NeedsReply(LoraDeviceAddress deviceAddress);

    /**
     * Return whether we have a gateway that is available to send a reply to the
     * specified device.
     *
     * \param deviceAddress the address of the device we are interested in.
     */
    Address GetBestGatewayForDevice(LoraDeviceAddress deviceAddress, int window);

    /**
     * Send a packet through a Gateway.
     *
     * This function assumes that the packet is already tagged with a LoraTag
     * that will inform the gateway of the parameters to use for the
     * transmission.
     */
    void SendThroughGateway(Ptr<Packet> packet, Address gwAddress);

    /**
     * Get the reply for the specified device address.
     */
    Ptr<Packet> GetReplyForDevice(LoraDeviceAddress edAddress, int windowNumber);

    /**
     * Get the EndDeviceStatus for the device that sent a packet.
     */
    Ptr<EndDeviceStatus> GetEndDeviceStatus(Ptr<const Packet> packet);

    /**
     * Get the EndDeviceStatus corresponding to a LoraDeviceAddress.
     */
    Ptr<EndDeviceStatus> GetEndDeviceStatus(LoraDeviceAddress address);

    /**
     * Return the number of end devices currently managed by the server.
     */
    int CountEndDevices();

  public:
    std::map<LoraDeviceAddress, Ptr<EndDeviceStatus>> m_endDeviceStatuses;
    std::map<Address, Ptr<GatewayStatus>> m_gatewayStatuses;
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_STATUS_H */
