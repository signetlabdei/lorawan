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

#include "network-controller.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("NetworkController");

NS_OBJECT_ENSURE_REGISTERED (NetworkController);

TypeId
NetworkController::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NetworkController")
    .AddConstructor<NetworkController> ()
    .SetGroupName ("lorawan");
  return tid;
}

NetworkController::NetworkController ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

NetworkController::NetworkController (Ptr<NetworkStatus> networkStatus) :
  m_status (networkStatus)
{
  NS_LOG_FUNCTION_NOARGS ();
}

NetworkController::~NetworkController ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
NetworkController::Install (Ptr<NetworkControllerComponent> component)
{
  NS_LOG_FUNCTION (this);
  m_components.push_back (component);
}

void
NetworkController::OnNewPacket (Ptr<Packet const> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // NOTE As a future optimization, we can allow components to register their
  // callbacks and only be called in case a certain MAC command is contained.
  // For now, we call all components.

  // Inform each component about the new packet
  for (auto it = m_components.begin (); it != m_components.end (); ++it)
    {
      (*it)->OnReceivedPacket (packet,
                               m_status->GetEndDeviceStatus (packet),
                               m_status);
    }
}

void
NetworkController::BeforeSendingReply (Ptr<EndDeviceStatus> endDeviceStatus)
{
  NS_LOG_FUNCTION (this);

  // Inform each component about the imminent reply
  for (auto it = m_components.begin (); it != m_components.end (); ++it)
    {
      (*it)->BeforeSendingReply (endDeviceStatus, m_status);
    }
}

}
}
