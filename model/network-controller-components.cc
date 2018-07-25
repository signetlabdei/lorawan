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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "ns3/network-controller-components.h"

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("NetworkControllerComponent");

  NS_OBJECT_ENSURE_REGISTERED (NetworkControllerComponent);

  TypeId
  NetworkControllerComponent::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::NetworkControllerComponent")
      .SetParent<Object> ()
      .SetGroupName ("lorawan")
      ;
    return tid;
  }

  // Constructor and destructor
  NetworkControllerComponent::NetworkControllerComponent () {}
  NetworkControllerComponent::~NetworkControllerComponent () {}

  ////////////////////////////////
  // ConfirmedMessagesComponent //
  ////////////////////////////////
  TypeId
  ConfirmedMessagesComponent::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::ConfirmedMessagesComponent")
      .AddConstructor<ConfirmedMessagesComponent> ()
      .SetGroupName ("lorawan");
    return tid;
  }

  ConfirmedMessagesComponent::ConfirmedMessagesComponent () {}
  ConfirmedMessagesComponent::~ConfirmedMessagesComponent () {}

  void
  ConfirmedMessagesComponent::OnReceivedPacket (Ptr<const Packet> packet,
                                                Ptr<NetworkStatus> networkStatus)
  {
    NS_LOG_FUNCTION (this << packet << networkStatus);

    // Check whether the received packet requires an acknowledgment.
    LoraMacHeader mHdr;
    packet->Copy()->RemoveHeader(mHdr);
    NS_LOG_INFO ("Mac Header: " << mHdr);

    if (mHdr.GetMType() == LoraMacHeader::CONFIRMED_DATA_UP)
      {
        NS_LOG_INFO ("Packet requires confirmation");

        // Set up the ACK bit on the reply
        Ptr<EndDeviceStatus> edStatus =
          networkStatus->GetEndDeviceStatus(packet);

        edStatus->GetReply().frameHeader.SetAck (true);

      }
  }
}
