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

#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include "class-a-end-device-lorawan-mac.h"
#include "gateway-status.h"
#include "lora-device-address.h"
#include "network-controller.h"
#include "network-scheduler.h"
#include "network-status.h"

#include "ns3/application.h"
#include "ns3/log.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/point-to-point-net-device.h"

namespace ns3
{
namespace lorawan
{

/**
 * \ingroup lorawan
 *
 * The NetworkServer is an application standing on top of a node equipped with
 * links that connect it with the gateways.
 *
 * This version of the NetworkServer attempts to closely mimic an actual
 * Network Server, by providing as much functionality as possible.
 */
class NetworkServer : public Application
{
  public:
    /**
     *  Register this type.
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    NetworkServer();
    ~NetworkServer() override;

    /**
     * \brief Start the NS application.
     */
    void StartApplication() override;

    /**
     * \brief Stop the NS application.
     */
    void StopApplication() override;

    /**
     * \brief Inform the NetworkServer that these nodes are connected to the network.
     *
     * This method will create a DeviceStatus object for each new node, and add
     * it to the list.
     *
     * \param nodes The end device NodeContainer.
     */
    void AddNodes(NodeContainer nodes);

    /**
     * \brief Inform the NetworkServer that this node is connected to the network.
     *
     * This method will create a DeviceStatus object for the new node (if it
     * doesn't already exist).
     *
     * \param node The end device Node.
     */
    void AddNode(Ptr<Node> node);

    /**
     * \brief Add the gateway to the list of gateways connected to this NS.
     *
     * Each GW is identified by its Address in the NS-GWs network.
     *
     * \param gateway A Ptr to the gateway Node.
     * \param netDevice A Ptr to the NetDevice of the NS connected to the gateway.
     */
    void AddGateway(Ptr<Node> gateway, Ptr<NetDevice> netDevice);

    /**
     * \brief Add a NetworkControllerComponent to this NetworkServer instance.
     *
     * \param component A Ptr to the NetworkControllerComponent object.
     */
    void AddComponent(Ptr<NetworkControllerComponent> component);

    /**
     * \brief Receive a packet from a gateway.
     *
     * This function is meant to be provided to NetDevice objects as a ReceiveCallback.
     *
     * \copydoc ns3::NetDevice::ReceiveCallback
     */
    bool Receive(Ptr<NetDevice> device,
                 Ptr<const Packet> packet,
                 uint16_t protocol,
                 const Address& sender);

    /**
     * \brief Get the NetworkStatus object of this NetworkServer.
     *
     * \return A Ptr to the NetworkStatus object.
     */
    Ptr<NetworkStatus> GetNetworkStatus();

  protected:
    Ptr<NetworkStatus> m_status;         //!< Ptr to the NetworkStatus object.
    Ptr<NetworkController> m_controller; //!< Ptr to the NetworkController object.
    Ptr<NetworkScheduler> m_scheduler;   //!< Ptr to the NetworkScheduler object.

    TracedCallback<Ptr<const Packet>> m_receivedPacket; //!< The `ReceivedPacket` trace source.
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_SERVER_H */
