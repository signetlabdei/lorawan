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

#include "ns3/network-server-helper.h"
#include "ns3/network-controller-components.h"
#include "ns3/adr-component.h"
#include "ns3/congestion-control-component.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("NetworkServerHelper");

NetworkServerHelper::NetworkServerHelper () : m_adrEnabled (false), m_ccEnabled (false)
{
  m_factory.SetTypeId ("ns3::NetworkServer");
  p2pHelper.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2pHelper.SetChannelAttribute ("Delay", StringValue ("2ms"));
  SetAdr ("ns3::AdrComponent");
  m_clusterTargets = {0.95};
}

NetworkServerHelper::~NetworkServerHelper ()
{
}

void
NetworkServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
NetworkServerHelper::SetGateways (NodeContainer gateways)
{
  m_gateways = gateways;
}

void
NetworkServerHelper::SetEndDevices (NodeContainer endDevices)
{
  m_endDevices = endDevices;
}

ApplicationContainer
NetworkServerHelper::Install (Ptr<Node> node)
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
NetworkServerHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
NetworkServerHelper::InstallPriv (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);

  Ptr<NetworkServer> app = m_factory.Create<NetworkServer> ();

  app->SetNode (node);
  node->AddApplication (app);

  // Cycle on each gateway
  for (NodeContainer::Iterator i = m_gateways.Begin ();
       i != m_gateways.End ();
       i++)
    {
      // Add the connections with the gateway
      // Create a PointToPoint link between gateway and NS
      NetDeviceContainer container = p2pHelper.Install (node, *i);

      // Add the gateway to the NS list
      app->AddGateway (*i, container.Get (0));
    }

  // Link the NetworkServer to its NetDevices
  for (uint32_t i = 0; i < node->GetNDevices (); i++)
    {
      Ptr<NetDevice> currentNetDevice = node->GetDevice (i);
      currentNetDevice->SetReceiveCallback (MakeCallback
                                              (&NetworkServer::Receive,
                                              app));
    }

  // Add the end devices
  app->AddNodes (m_endDevices);

  // Add components to the NetworkServer
  InstallComponents (app);

  return app;
}

void
NetworkServerHelper::EnableAdr (bool enableAdr)
{
  NS_LOG_FUNCTION (this << enableAdr);

  m_adrEnabled = enableAdr;
}

void
NetworkServerHelper::SetAdr (std::string type)
{
  NS_LOG_FUNCTION (this << type);

  m_adrSupportFactory = ObjectFactory ();
  m_adrSupportFactory.SetTypeId (type);
}

void
NetworkServerHelper::EnableCongestionControl (bool enableCC)
{
  NS_LOG_FUNCTION (this << enableCC);

  m_ccEnabled = enableCC;
}

void
NetworkServerHelper::AssignClusters (cluster_t clustersInfo) 
{
  int nClusters = clustersInfo.size ();
  NS_ASSERT_MSG (nClusters <= 3, 
      "For the moment only up to 3 clusters are supported.");
  NS_ASSERT_MSG (m_endDevices.GetN () > 0, 
      "Devices must be set before assigning clusters.");

  double devWeight = double (100.0) / m_endDevices.GetN ();

  uint8_t currCluster = 0;
  double totWeight = 0;
  for (NodeContainer::Iterator i = m_endDevices.Begin (); i != m_endDevices.End (); ++i)
    {
      if (clustersInfo[currCluster].first == 0.0)
        currCluster++;

      Ptr<EndDeviceLorawanMac> mac = (*i)->GetDevice (0)->GetObject<LoraNetDevice> ()
          ->GetMac ()->GetObject<EndDeviceLorawanMac> ();
      mac->SetCluster (currCluster);
      
      // Assign one frequency to each cluster
      int chid = 0;
      for (auto &ch : mac->GetLogicalLoraChannelHelper ().GetChannelList ())
        {
          if (chid == currCluster)
            ch->SetEnabledForUplink ();
          else
            ch->DisableForUplink ();
          chid++;
        }

      totWeight += devWeight;

      if (currCluster < nClusters - 1 and
          totWeight >= clustersInfo[currCluster].first - devWeight / 2)
        {
          currCluster++;
          totWeight = 0;
        }
    }
  
  m_clusterTargets.clear ();
  for (auto const &cluster : clustersInfo)
    m_clusterTargets.push_back (cluster.second);
}

void
NetworkServerHelper::InstallComponents (Ptr<NetworkServer> netServer)
{
  NS_LOG_FUNCTION (this << netServer);

  // Add Confirmed Messages support
  Ptr<ConfirmedMessagesComponent> ackSupport =
    CreateObject<ConfirmedMessagesComponent> ();
  netServer->AddComponent (ackSupport);

  // Add LinkCheck support
  Ptr<LinkCheckComponent> linkCheckSupport = CreateObject<LinkCheckComponent> ();
  netServer->AddComponent (linkCheckSupport);

  // Add Adr support
  if (m_adrEnabled)
    {
      netServer->AddComponent (m_adrSupportFactory.Create<NetworkControllerComponent> ());
    }

  // Add congestion control support
  if (m_ccEnabled)
    {
      Ptr<CongestionControlComponent> ccc = CreateObject<CongestionControlComponent> ();
      ccc->SetTargets (m_clusterTargets);
      netServer->AddComponent (ccc);
    }
}

}
} // namespace ns3
