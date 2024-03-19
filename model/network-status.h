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
 * \ingroup lorawan
 *
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
    /**
     *  Register this type.
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    NetworkStatus();
    ~NetworkStatus() override;

    /**
     * \brief Add a device to the ones that are tracked by this NetworkStatus object.
     *
     * \param edMac pointer to the MAC layer object of the device to be tracked.
     */
    void AddNode(Ptr<ClassAEndDeviceLorawanMac> edMac);

    /**
     * \brief Add a new gateway to the list of gateways connected to the network.
     *
     * Each GW is identified by its NetDevice Address in the NS-GW connection.
     *
     * \param address the gateway's NetDevice Address
     * \param gwStatus a pointer to a GatewayStatus object for the gateway
     */
    void AddGateway(Address& address, Ptr<GatewayStatus> gwStatus);

    /**
     * \brief Update network status on a received packet.
     *
     * \param packet the received packet.
     * \param gwaddress address of the gateway this packet was received from.
     */
    void OnReceivedPacket(Ptr<const Packet> packet, const Address& gwaddress);

    /**
     * \brief Return whether the specified device needs a reply.
     *
     * \param deviceAddress the address of the device we are interested in.
     * \return true if we need to reply to the last packet from the device, false otherwise.
     */
    bool NeedsReply(LoraDeviceAddress deviceAddress);

    /**
     * \brief Return whether we have a gateway that is available to send a reply to the specified
     * device.
     *
     * \param deviceAddress the address of the device we are interested in
     * \param window the device reception window we are currently targeting (1 or 2)
     * \return the Address of the gateway which measured the best RSSI of the last packet from the
     * device, selected among the gateways being currently available for downlink transmission
     */
    Address GetBestGatewayForDevice(LoraDeviceAddress deviceAddress, int window);

    /**
     * \brief Send a packet through a Gateway.
     *
     * This function assumes that the packet is already tagged with a LoraTag
     * that will inform the gateway of the parameters to use for the
     * transmission.
     *
     * \param packet the packet
     * \param gwAddress the address of the gateway
     */
    void SendThroughGateway(Ptr<Packet> packet, Address gwAddress);

    /**
     * \brief Get the reply packet prepared for a reception window of a device.
     *
     * \param edAddress the address of the device
     * \param windowNumber the reception window number (1 or 2)
     * \return the reply packet
     */
    Ptr<Packet> GetReplyForDevice(LoraDeviceAddress edAddress, int windowNumber);

    /**
     * \brief Get the EndDeviceStatus of the device that sent a packet.
     *
     * \param packet the packet sent by the end device
     * \return a pointer to the end device status
     */
    Ptr<EndDeviceStatus> GetEndDeviceStatus(Ptr<const Packet> packet);

    /**
     * \brief Get the EndDeviceStatus corresponding to a LoraDeviceAddress.
     *
     * \param address the LoraDeviceAddress of the end device
     * \return a pointer to the end device status
     */
    Ptr<EndDeviceStatus> GetEndDeviceStatus(LoraDeviceAddress address);

    /**
     * \brief Return the number of end devices currently managed by the server.
     *
     * \return the number of end devices as an int
     */
    int CountEndDevices();

  public:
    std::map<LoraDeviceAddress, Ptr<EndDeviceStatus>>
        m_endDeviceStatuses; //!< map tracking the state of devices connected to this network server
    std::map<Address, Ptr<GatewayStatus>>
        m_gatewayStatuses; //!< map tracking the state of gateways connected to this network server
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_STATUS_H */
