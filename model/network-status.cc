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

#include "ns3/network-status.h"
#include "ns3/end-device-status.h"
#include "ns3/gateway-status.h"

#include "ns3/net-device.h"
#include "ns3/packet.h"
#include "ns3/lora-device-address.h"
#include "ns3/node-container.h"
#include "ns3/log.h"
#include "ns3/pointer.h"

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("NetworkStatus");

  NS_OBJECT_ENSURE_REGISTERED (NetworkStatus);

  TypeId
  NetworkStatus::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::NetworkStatus")
      .AddConstructor<NetworkStatus> ()
      .SetGroupName ("lorawan");
    return tid;
  }

  NetworkStatus::NetworkStatus ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  NetworkStatus::~NetworkStatus ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  void
  NetworkStatus::AddNode (Ptr<EndDeviceLoraMac> edMac)
  {
    NS_LOG_FUNCTION (this << edMac);

    // Check whether this device already exists in our list
    LoraDeviceAddress edAddress = edMac->GetDeviceAddress ();
    if (m_endDeviceStatuses.find (edAddress) == m_endDeviceStatuses.end ())
      {
        // The device doesn't exist. Create new EndDeviceStatus
        EndDeviceStatus edStatus = EndDeviceStatus ();

        // Add it to the map
        m_endDeviceStatuses.insert (std::pair<LoraDeviceAddress, EndDeviceStatus>
                                    (edAddress, edStatus));
        NS_LOG_DEBUG ("Added to the list a device with address " <<
                      edAddress.Print ());
      }
  }

  void
  NetworkStatus::AddGateway (Address& address, GatewayStatus gwStatus)
  {
    NS_LOG_FUNCTION (this);

    // Check whether this device already exists in the list
    if (m_gatewayStatuses.find (address) == m_gatewayStatuses.end ())
      {
        // The device doesn't exist.

        // Add it to the map
        m_gatewayStatuses.insert (std::pair<Address,GatewayStatus>
                                  (address, gwStatus));
        NS_LOG_DEBUG ("Added to the list a gateway with address " << address);
      }
  }

  void
  NetworkStatus::OnReceivedPacket (Ptr<const Packet> packet,
                                   const Address& gwAddress)
  {
    // NS_LOG_FUNCTION (this << packet << protocol << address);

    // Create a copy of the packet
    Ptr<Packet> myPacket = packet->Copy ();

    // Extract the headers
    LoraMacHeader macHdr;
    myPacket->RemoveHeader (macHdr);

    LoraFrameHeader frameHdr;
    myPacket->RemoveHeader (frameHdr);
    LoraDeviceAddress edAddr= frameHdr.GetAddress();

    m_endDeviceStatuses.at(edAddr).InsertReceivedPacket(packet, gwAddress);

  }

}
