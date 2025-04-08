/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * Modified by: Alessandro Aimi <alessandro.aimi@unibo.it>
 */

#ifndef NETWORK_SERVER_HELPER_H
#define NETWORK_SERVER_HELPER_H

#include "ns3/address.h"
#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/network-server.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/point-to-point-helper.h"

#include <stdint.h>
#include <string>

namespace ns3
{
namespace lorawan
{

/**
 * Store network server app registration details for gateway nodes having a P2P link with the
 * network server.
 *
 * For each gateway, store in a pair:
 * - The Point-to-point net device of the network server;
 * - The gateway node connected to the P2P net device.
 */
typedef std::list<std::pair<Ptr<PointToPointNetDevice>, Ptr<Node>>> P2PGwRegistration_t;

/**
 * @ingroup lorawan
 *
 * This class can install a NetworkServer application on a node.
 */
class NetworkServerHelper
{
  public:
    NetworkServerHelper();  //!< Default constructor
    ~NetworkServerHelper(); //!< Destructor

    /**
     * Record an attribute to be set in each Application after it is is created.
     *
     * @param name The name of the application attribute to set.
     * @param value The value of the application attribute to set.
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * Create one lorawan network server application on the Node.
     *
     * @param node The node on which to create the Application.
     * @return The application created.
     */
    ApplicationContainer Install(Ptr<Node> node);

    /**
     * Register gateways connected with point-to-point to this network server.
     *
     * @remark For the moment, only P2P connections are supported.
     *
     * @param registration The gateways registration data.
     *
     * @see ns3::lorawan::P2PGwRegistration_t
     */
    void SetGatewaysP2P(const P2PGwRegistration_t& registration);

    /**
     * Set which end devices will be managed by this network server.
     *
     * @param endDevices The end device nodes.
     */
    void SetEndDevices(NodeContainer endDevices);

    /**
     * Enable (true) or disable (false) the Adaptive Data Rate (ADR) component in the Network
     * Server created by this helper.
     *
     * @param enableAdr Whether to enable ADR in the network server.
     */
    void EnableAdr(bool enableAdr);

    /**
     * Set the Adaptive Data Rate (ADR) implementation to use in the network server created
     * by this helper.
     *
     * @param type The type of ADR implementation.
     */
    void SetAdr(std::string type);

  private:
    /**
     * Install the NetworkServerComponent objects onto the NetworkServer application.
     *
     * @param netServer A pointer to the NetworkServer application.
     */
    void InstallComponents(Ptr<NetworkServer> netServer);

    /**
     * Do the actual NetworkServer application installation on the Node.
     *
     * This function creates the NetworkServer application, installs it on the Node, connect the
     * gateways to the Node with a PointToPoint link, registers gateways and devices in the
     * NetworkServer application, and installs the necessary NetworkServerComponent objects.
     *
     * @param node A pointer to the Node.
     * @return A pointer to the installed NetworkServer application.
     */
    Ptr<Application> InstallPriv(Ptr<Node> node);

    ObjectFactory m_factory; //!< Factory to create the Network server application
    std::list<std::pair<Ptr<NetDevice>, Ptr<Node>>>
        m_gatewayRegistrationList; //!< List of gateway to register to this network server
    NodeContainer m_endDevices;    //!< Set of end devices to connect to this network server
    bool m_adrEnabled; //!< Whether to enable the Adaptive Data Rate (ADR) algorithm on the
                       //!< NetworkServer application
    ObjectFactory m_adrSupportFactory; //!< Factory to create the Adaptive Data Rate (ADR) component
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_SERVER_HELPER_H */
