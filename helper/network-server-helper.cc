/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * Modified by: Alessandro Aimi <alessandro.aimi@unibo.it>
 */

#include "network-server-helper.h"

#include "ns3/adr-component.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/network-controller-components.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("NetworkServerHelper");

NetworkServerHelper::NetworkServerHelper()
    : m_adrEnabled(false)
{
    m_factory.SetTypeId("ns3::NetworkServer");
    SetAdr("ns3::AdrComponent");
}

NetworkServerHelper::~NetworkServerHelper()
{
}

void
NetworkServerHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

void
NetworkServerHelper::SetGatewaysP2P(const P2PGwRegistration_t& registration)
{
    for (const auto& [serverP2PNetDev, gwNode] : registration)
    {
        NS_ASSERT_MSG(serverP2PNetDev->GetNode()->GetId() != gwNode->GetId(),
                      "wrong P2P NetDevice detected, please provide the one on the network "
                      "server's side instead");
        m_gatewayRegistrationList.emplace_back(serverP2PNetDev, gwNode);
    }
}

void
NetworkServerHelper::SetEndDevices(NodeContainer endDevices)
{
    m_endDevices = endDevices;
}

ApplicationContainer
NetworkServerHelper::Install(Ptr<Node> node)
{
    return ApplicationContainer(InstallPriv(node));
}

Ptr<Application>
NetworkServerHelper::InstallPriv(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    NS_ASSERT_MSG(node->GetNDevices() > 0, "No gateways connected to provided node");

    Ptr<NetworkServer> app = m_factory.Create<NetworkServer>();

    app->SetNode(node);
    node->AddApplication(app);

    // Connect the net devices receive callback to the app and register the respective gateway
    for (const auto& [currentNetDevice, gwNode] : m_gatewayRegistrationList)
    {
        currentNetDevice->SetReceiveCallback(MakeCallback(&NetworkServer::Receive, app));
        app->AddGateway(gwNode, currentNetDevice);
    }

    // Add the end devices
    app->AddNodes(m_endDevices);

    // Add components to the NetworkServer
    InstallComponents(app);

    return app;
}

void
NetworkServerHelper::EnableAdr(bool enableAdr)
{
    NS_LOG_FUNCTION(this << enableAdr);

    m_adrEnabled = enableAdr;
}

void
NetworkServerHelper::SetAdr(std::string type)
{
    NS_LOG_FUNCTION(this << type);

    m_adrSupportFactory = ObjectFactory();
    m_adrSupportFactory.SetTypeId(type);
}

void
NetworkServerHelper::InstallComponents(Ptr<NetworkServer> netServer)
{
    NS_LOG_FUNCTION(this << netServer);

    // Add Confirmed Messages support
    Ptr<ConfirmedMessagesComponent> ackSupport = CreateObject<ConfirmedMessagesComponent>();
    netServer->AddComponent(ackSupport);

    // Add LinkCheck support
    Ptr<LinkCheckComponent> linkCheckSupport = CreateObject<LinkCheckComponent>();
    netServer->AddComponent(linkCheckSupport);

    // Add Adaptive Data Rate (ADR) support
    if (m_adrEnabled)
    {
        netServer->AddComponent(m_adrSupportFactory.Create<NetworkControllerComponent>());
    }
}
} // namespace lorawan
} // namespace ns3
