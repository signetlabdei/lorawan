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

#include "ns3/network-server.h"
#include "ns3/net-device.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/packet.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lora-device-address.h"
#include "ns3/network-status.h"
#include "ns3/lora-frame-header.h"
#include "ns3/node-container.h"
#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/mac-command.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("NetworkServer");

NS_OBJECT_ENSURE_REGISTERED (NetworkServer);

TypeId
NetworkServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NetworkServer")
    .SetParent<Application> ()
    .AddConstructor<NetworkServer> ()
    .AddTraceSource ("ReceivedPacket",
                     "Trace source that is fired when a packet arrives at the Network Server",
                     MakeTraceSourceAccessor (&NetworkServer::m_receivedPacket),
                     "ns3::Packet::TracedCallback")
    .SetGroupName ("lorawan");
  return tid;
}

NetworkServer::NetworkServer () :
  m_status (Create<NetworkStatus> ()),
  m_controller (Create<NetworkController> (m_status)),
  m_scheduler (Create<NetworkScheduler> (m_status, m_controller))
{
  NS_LOG_FUNCTION_NOARGS ();
}

NetworkServer::~NetworkServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
NetworkServer::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
NetworkServer::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
NetworkServer::AddGateway (Ptr<Node> gateway, Ptr<NetDevice> netDevice)
{
  NS_LOG_FUNCTION (this << gateway);

  // Get the PointToPointNetDevice
  Ptr<PointToPointNetDevice> p2pNetDevice;
  for (uint32_t i = 0; i < gateway->GetNDevices (); i++)
    {
      p2pNetDevice = gateway->GetDevice (i)->GetObject<PointToPointNetDevice> ();
      if (p2pNetDevice != 0)
        {
          // We found a p2pNetDevice on the gateway
          break;
        }
    }

  // Get the gateway's LoRa MAC layer (assumes gateway's MAC is configured as first device)
  Ptr<GatewayLorawanMac> gwMac = gateway->GetDevice (0)->GetObject<LoraNetDevice> ()->
    GetMac ()->GetObject<GatewayLorawanMac> ();
  NS_ASSERT (gwMac != 0);

  // Get the Address
  Address gatewayAddress = p2pNetDevice->GetAddress ();

  // Create new gatewayStatus
  Ptr<GatewayStatus> gwStatus = Create<GatewayStatus> (gatewayAddress,
                                                       netDevice,
                                                       gwMac);

  m_status->AddGateway (gatewayAddress, gwStatus);
}

void
NetworkServer::AddNodes (NodeContainer nodes)
{
  NS_LOG_FUNCTION_NOARGS ();

  // For each node in the container, call the function to add that single node
  NodeContainer::Iterator it;
  for (it = nodes.Begin (); it != nodes.End (); it++)
    {
      AddNode (*it);
    }
}

void
NetworkServer::AddNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);

  // Get the LoraNetDevice
  Ptr<LoraNetDevice> loraNetDevice;
  for (uint32_t i = 0; i < node->GetNDevices (); i++)
    {
      loraNetDevice = node->GetDevice (i)->GetObject<LoraNetDevice> ();
      if (loraNetDevice != 0)
        {
          // We found a LoraNetDevice on the node
          break;
        }
    }

  // Get the MAC
  Ptr<ClassAEndDeviceLorawanMac> edLorawanMac =
    loraNetDevice->GetMac ()->GetObject<ClassAEndDeviceLorawanMac> ();

  // Update the NetworkStatus about the existence of this node
  m_status->AddNode (edLorawanMac);
}

bool
NetworkServer::Receive (Ptr<NetDevice> device, Ptr<const Packet> packet,
                        uint16_t protocol, const Address& address)
{
  NS_LOG_FUNCTION (this << packet << protocol << address);

  // Create a copy of the packet
  Ptr<Packet> myPacket = packet->Copy ();

  // Fire the trace source
  m_receivedPacket (packet);

  // Inform the scheduler of the newly arrived packet
  m_scheduler->OnReceivedPacket (packet);

  // Inform the status of the newly arrived packet
  m_status->OnReceivedPacket (packet, address);

  // Inform the controller of the newly arrived packet
  m_controller->OnNewPacket (packet);

  return true;
}

void
NetworkServer::AddComponent (Ptr<NetworkControllerComponent> component)
{
  NS_LOG_FUNCTION (this << component);

  m_controller->Install (component);
}

Ptr<NetworkStatus>
NetworkServer::GetNetworkStatus (void)
{
  return m_status;
}

}
}
