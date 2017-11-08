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

#include "ns3/simple-network-server.h"
#include "ns3/simulator.h"
#include "ns3/end-device-lora-mac.h"
#include "ns3/lora-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lora-tag.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimpleNetworkServer");

NS_OBJECT_ENSURE_REGISTERED (SimpleNetworkServer);

TypeId
SimpleNetworkServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SimpleNetworkServer")
    .SetParent<Application> ()
    .AddConstructor<SimpleNetworkServer> ()
    .SetGroupName ("lorawan");
  return tid;
}

SimpleNetworkServer::SimpleNetworkServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

SimpleNetworkServer::~SimpleNetworkServer ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
SimpleNetworkServer::StartApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
SimpleNetworkServer::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
SimpleNetworkServer::AddGateway (Ptr<Node> gateway, Ptr<NetDevice> netDevice)
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
  Ptr<GatewayLoraMac> gwMac = gateway->GetDevice (0)->GetObject<LoraNetDevice> ()->
    GetMac ()->GetObject<GatewayLoraMac> ();
  NS_ASSERT (gwMac != 0);

  // Get the Address
  Address gatewayAddress = p2pNetDevice->GetAddress ();

  // Check whether this device already exists
  if (m_gatewayStatuses.find (gatewayAddress) == m_gatewayStatuses.end ())
    {
      // The device doesn't exist

      // Create new gatewayStatus
      GatewayStatus gwStatus = GatewayStatus (gatewayAddress, netDevice, gwMac);
      // Add it to the map
      m_gatewayStatuses.insert (std::pair<Address, GatewayStatus>
                                  (gatewayAddress, gwStatus));
      NS_LOG_DEBUG ("Added a gateway to the list");
    }
}

void
SimpleNetworkServer::AddNodes (NodeContainer nodes)
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
SimpleNetworkServer::AddNode (Ptr<Node> node)
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
  Ptr<EndDeviceLoraMac> edLoraMac = loraNetDevice->GetMac ()->GetObject<EndDeviceLoraMac> ();

  // Get the Address
  LoraDeviceAddress deviceAddress = edLoraMac->GetDeviceAddress ();
  // Check whether this device already exists
  if (m_deviceStatuses.find (deviceAddress) == m_deviceStatuses.end ())
    {
      // The device doesn't exist

      // Create new DeviceStatus
      DeviceStatus devStatus = DeviceStatus (edLoraMac);
      // Add it to the map
      m_deviceStatuses.insert (std::pair<LoraDeviceAddress, DeviceStatus>
                                 (deviceAddress, devStatus));
      NS_LOG_DEBUG ("Added to the list a device with address " <<
                    deviceAddress.Print ());
    }
}

bool
SimpleNetworkServer::Receive (Ptr<NetDevice> device, Ptr<const Packet> packet,
                              uint16_t protocol, const Address& address)
{
  NS_LOG_FUNCTION (this << packet << protocol << address);

  // Create a copy of the packet
  Ptr<Packet> myPacket = packet->Copy ();

  // Extract the headers
  LoraMacHeader macHdr;
  myPacket->RemoveHeader (macHdr);
  LoraFrameHeader frameHdr;
  frameHdr.SetAsUplink ();
  myPacket->RemoveHeader (frameHdr);
  LoraTag tag;
  myPacket->RemovePacketTag (tag);

  // Register which gateway this packet came from
  double rcvPower = tag.GetReceivePower ();
  m_deviceStatuses.at (frameHdr.GetAddress ()).UpdateGatewayData (address,
                                                                  rcvPower);

  // Determine whether the packet requires a reply
  if (macHdr.GetMType () == LoraMacHeader::CONFIRMED_DATA_UP
      && !m_deviceStatuses.at (frameHdr.GetAddress ()).HasReply ())
    {
      NS_LOG_DEBUG ("Scheduling a reply for this device");

      DeviceStatus::Reply reply;
      reply.hasReply = true;

      LoraMacHeader replyMacHdr = LoraMacHeader ();
      replyMacHdr.SetMajor (0);
      replyMacHdr.SetMType (LoraMacHeader::UNCONFIRMED_DATA_DOWN);
      reply.macHeader = replyMacHdr;

      LoraFrameHeader replyFrameHdr = LoraFrameHeader ();
      replyFrameHdr.SetAsDownlink ();
      replyFrameHdr.SetAddress (frameHdr.GetAddress ());
      replyFrameHdr.SetAck (true);
      reply.frameHeader = replyFrameHdr;

      // The downlink packet carries 0 bytes of payload
      Ptr<Packet> replyPacket = Create<Packet> (0);
      reply.packet = replyPacket;

      m_deviceStatuses.at (frameHdr.GetAddress ()).SetReply (reply);
      m_deviceStatuses.at (frameHdr.GetAddress ()).SetFirstReceiveWindowFrequency (tag.GetFrequency ());

      // Schedule a reply on the first receive window
      Simulator::Schedule (Seconds (1), &SimpleNetworkServer::SendOnFirstWindow,
                           this, frameHdr.GetAddress ());
    }

  else if (macHdr.GetMType () == LoraMacHeader::CONFIRMED_DATA_UP
           && m_deviceStatuses.at (frameHdr.GetAddress ()).HasReply ())
    {
      NS_LOG_DEBUG ("There is already a reply for this device. Scheduling it and update frequency");

      m_deviceStatuses.at (frameHdr.GetAddress ()).SetFirstReceiveWindowFrequency (tag.GetFrequency ());

      // Schedule a reply on the first receive window
      Simulator::Schedule (Seconds (1), &SimpleNetworkServer::SendOnFirstWindow,
                           this, frameHdr.GetAddress ());

    }

  return true;
}

void
SimpleNetworkServer::SendOnFirstWindow (LoraDeviceAddress address)
{
  NS_LOG_FUNCTION (this << address);

  // Decide on which gateway we'll transmit our reply
  double firstReceiveWindowFrequency = m_deviceStatuses.at
      (address).GetFirstReceiveWindowFrequency ();

  Address gatewayForReply = GetGatewayForReply (address,
                                                firstReceiveWindowFrequency);

  if (gatewayForReply != Address ())
    {
      NS_LOG_FUNCTION ("Found a suitable GW!");

      // Get the packet to use in the reply
      Ptr<Packet> replyPacket = m_deviceStatuses.at (address).GetReplyPacket ();
      NS_LOG_DEBUG ("Packet size: " << replyPacket->GetSize ());

      // Tag the packet so that the Gateway sends it according to the first
      // receive window parameters
      LoraTag replyPacketTag;
      uint8_t dataRate = m_deviceStatuses.at (address).GetFirstReceiveWindowDataRate ();
      double frequency = m_deviceStatuses.at (address).GetFirstReceiveWindowFrequency ();
      replyPacketTag.SetDataRate (dataRate);
      replyPacketTag.SetFrequency (frequency);

      replyPacket->AddPacketTag (replyPacketTag);

      NS_LOG_INFO ("Sending reply through the gateway with address " << gatewayForReply << " and initialize the reply.");

      InitializeReply (address, false);

      // Inform the gateway of the transmission
      m_gatewayStatuses.find (gatewayForReply)->second.GetNetDevice ()->
      Send (replyPacket, gatewayForReply, 0x0800);
    }
  else
    {
      NS_LOG_FUNCTION ("No suitable GW found, scheduling second window reply");

      // Schedule a reply on the second receive window
      Simulator::Schedule (Seconds (1), &SimpleNetworkServer::SendOnSecondWindow, this,
                           address);
    }
}

void
SimpleNetworkServer::SendOnSecondWindow (LoraDeviceAddress address)
{
  NS_LOG_FUNCTION (this << address);

  double secondReceiveWindowFrequency = m_deviceStatuses.at
      (address).GetSecondReceiveWindowFrequency ();

  // Decide on which gateway we'll transmit our reply
  Address gatewayForReply = GetGatewayForReply (address, secondReceiveWindowFrequency);

  if (gatewayForReply != Address ())
    {
      // Get the packet to use in the reply
      Ptr<Packet> replyPacket = m_deviceStatuses.at (address).GetReplyPacket ();
      NS_LOG_DEBUG ("Packet size: " << replyPacket->GetSize ());

      // Tag the packet so that the Gateway sends it according to the second
      // receive window parameters
      LoraTag replyPacketTag;
      uint8_t dataRate = m_deviceStatuses.at (address).GetSecondReceiveWindowDataRate ();
      double frequency = m_deviceStatuses.at (address).GetSecondReceiveWindowFrequency ();
      replyPacketTag.SetDataRate (dataRate);
      replyPacketTag.SetFrequency (frequency);

      replyPacket->AddPacketTag (replyPacketTag);

      NS_LOG_INFO ("Sending reply through the gateway with address " <<
                   gatewayForReply << " and initialize reply.");

      InitializeReply (address, false);

      // Inform the gateway of the transmission
      m_gatewayStatuses.find (gatewayForReply)->second.GetNetDevice ()->
      Send (replyPacket, gatewayForReply, 0x0800);
    }
  else
    {
      // Schedule a reply on the second receive window
      NS_LOG_INFO ("Giving up on this reply, no GW available for second window");
    }
}

Address
SimpleNetworkServer::GetGatewayForReply (LoraDeviceAddress deviceAddress,
                                         double frequency)
{
  NS_LOG_FUNCTION (this);

  // Check which gateways can send this reply
  // Go in the order suggested by the DeviceStatus
  std::list<Address> addresses = m_deviceStatuses.at
      (deviceAddress).GetSortedGatewayAddresses ();

  for (auto it = addresses.begin (); it != addresses.end (); ++it)
    {
      if (m_gatewayStatuses.at (*it).IsAvailableForTransmission (frequency))
        {
          m_gatewayStatuses.at (*it).SetNextTransmissionTime (Simulator::Now ());
          return *it;
        }
    }

  return Address ();
}

void
SimpleNetworkServer::InitializeReply (LoraDeviceAddress addr, bool hasRep)
{
  DeviceStatus::Reply reply;
  reply.hasReply = hasRep;

  m_deviceStatuses.at (addr).SetReply (reply);
}

}
