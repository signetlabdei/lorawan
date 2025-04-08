/*
 * Copyright (c) 2018 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
 * @ingroup lorawan
 *
 * This class represents the knowledge about the state of the network that is
 * available at the network server. It is essentially a collection of two maps:
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
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    NetworkStatus();           //!< Default constructor
    ~NetworkStatus() override; //!< Destructor

    /**
     * Add a device to the ones that are tracked by this NetworkStatus object.
     *
     * @param edMac Pointer to the MAC layer object of the device to be tracked.
     */
    void AddNode(Ptr<ClassAEndDeviceLorawanMac> edMac);

    /**
     * Add a new gateway to the list of gateways connected to the network.
     *
     * Each gateway is identified by its NetDevice Address in the network connecting it to the
     * network server.
     *
     * @param address The gateway's NetDevice Address.
     * @param gwStatus A pointer to a GatewayStatus object for the gateway.
     */
    void AddGateway(Address& address, Ptr<GatewayStatus> gwStatus);

    /**
     * Update network status on a received packet.
     *
     * @param packet The received packet.
     * @param gwaddress Address of the gateway this packet was received from.
     */
    void OnReceivedPacket(Ptr<const Packet> packet, const Address& gwaddress);

    /**
     * Return whether the specified device needs a reply.
     *
     * @param deviceAddress The address of the device we are interested in.
     * @return True if we need to reply to the last packet from the device, false otherwise.
     */
    bool NeedsReply(LoraDeviceAddress deviceAddress);

    /**
     * Return whether we have a gateway that is available to send a reply to the specified
     * device.
     *
     * @param deviceAddress The address of the device we are interested in.
     * @param window The device reception window we are currently targeting (1 or 2).
     * @return The Address of the gateway which measured the best RSSI of the last packet from the
     * device, selected among the gateways being currently available for downlink transmission.
     */
    Address GetBestGatewayForDevice(LoraDeviceAddress deviceAddress, int window);

    /**
     * Send a packet through a gateway.
     *
     * This function assumes that the packet is already tagged with a LoraTag
     * that will inform the gateway of the parameters to use for the
     * transmission.
     *
     * @param packet The packet.
     * @param gwAddress The address of the gateway.
     */
    void SendThroughGateway(Ptr<Packet> packet, Address gwAddress);

    /**
     * Get the reply packet prepared for a reception window of a device.
     *
     * @param edAddress The address of the device.
     * @param windowNumber The reception window number (1 or 2).
     * @return The reply packet.
     */
    Ptr<Packet> GetReplyForDevice(LoraDeviceAddress edAddress, int windowNumber);

    /**
     * Get the EndDeviceStatus of the device that sent a packet.
     *
     * @param packet The packet sent by the end device.
     * @return A pointer to the end device status.
     */
    Ptr<EndDeviceStatus> GetEndDeviceStatus(Ptr<const Packet> packet);

    /**
     * Get the EndDeviceStatus corresponding to a LoraDeviceAddress.
     *
     * @param address The LoraDeviceAddress of the end device.
     * @return A pointer to the end device status.
     */
    Ptr<EndDeviceStatus> GetEndDeviceStatus(LoraDeviceAddress address);

    /**
     * Return the number of end devices currently managed by the server.
     *
     * @return The number of end devices as an int.
     */
    int CountEndDevices();

  public:
    std::map<LoraDeviceAddress, Ptr<EndDeviceStatus>>
        m_endDeviceStatuses; //!< Map tracking the state of devices connected to this network server
    std::map<Address, Ptr<GatewayStatus>>
        m_gatewayStatuses; //!< Map tracking the state of gateways connected to this network server
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_STATUS_H */
