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
#include "ns3/gateway-status.h"
#include "ns3/end-device-status.h"
#include "ns3/net-device.h"
#include "ns3/packet.h"
#include "ns3/lora-device-address.h"
#include "ns3/node-container.h"
#include "ns3/log.h"
#include "ns3/pointer"

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("NetworkStatus");

  NetworkStatus::NetworkStatus ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  NetworkStatus::~NetworkStatus ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }


  NetworkStatus::AddNode (Ptr<Node> node)
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
    if (m_endDeviceStatuses.find (deviceAddress) == m_endDeviceStatuses.end ())
      {
        // The device doesn't exist. Create new EndDeviceStatus
        EndDeviceStatus edStatus = EndDeviceStatus ();
        // Add it to the map
        m_endDeviceStatuses.insert (std::pair<LoraDeviceAddress, EndDeviceStatus>
                                    (deviceAddress, edStatus));
        NS_LOG_DEBUG ("Added to the list a device with address " <<
                      deviceAddress.Print ());
      }
  }

  NetworkStatus::AddGateway (Address& address, Ptr<GatewayStatus> gwStatus)
  {
    NS_LOG_FUNCTION (this);

    // Check whether this device already exists in the list
    if (m_gatewayStatuses.find (address) == m_gatewayStatuses.end ())
      {
        // The device doesn't exist. 
        // Add it to the map
        m_gatewayStatuses.insert (std::pair<Address,Ptr<GatewayStatus>>
                                  (address, gwStatus));
        NS_LOG_DEBUG ("Added to the list a gateway with address " <<
                      address.Print());
      }
  }


  //TODO che parametri prende? servirebbe la lista di GW che l'hanno ricevuto
  Receive (Ptr<NetDevice> device, Ptr<const Packet> packet,
                uint16_t protocol, const Address& gwAddress)
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
    LoraDeviceAddress edAddr= frameHdr.GetAddress();

    // Update current parameters
    LoraTag tag;
    myPacket->RemovePacketTag (tag);
    m_endDeviceStatuses.at(edAddr.SetFirstReceiveWindowSpreadingFactor(tag.GetSpreadingFactor()));
    m_endDeviceStatuses.at(edAddr.SetFirstReceiveWindowFrequency(tag.GetFrequency()));
    //TODO extract BW

    // Update Information on the received packet
    EndDeviceStatus ReceivedPacketInfo info;
    info.sf = tag.GetSpreadingFactor();
    info.frequency= tag.GetFrequency();
    // info.bw
    //info.gwlist

    m_endDeviceStatuses.at.InsertReceivedPacket(packet, info));



  }

protected:
  Ptr<NetworkStatusScheduler> m_scheduler;
  Ptr<NetworkStatusController> m_controller;
  Ptr<NetworkStatus> m_status;
};

} /* namespace ns3 */

#endif /* NETWORK_SERVER_H */

std::map<LoraDeviceAddress,EndDeviceStatus> m_endDeviceStatuses;

std::map<Address,GatewayStatus> m_gatewayStatuses;
