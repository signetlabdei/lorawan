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

#include "ns3/lora-net-device.h"
#include "ns3/pointer.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/abort.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("LoraNetDevice");

NS_OBJECT_ENSURE_REGISTERED (LoraNetDevice);

TypeId
LoraNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<LoraNetDevice> ()
    .SetGroupName ("lorawan")
    .AddAttribute ("Channel", "The channel attached to this device",
                   PointerValue (),
                   MakePointerAccessor (&LoraNetDevice::DoGetChannel),
                   MakePointerChecker<LoraChannel> ())
    .AddAttribute ("Phy", "The PHY layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&LoraNetDevice::GetPhy,
                                        &LoraNetDevice::SetPhy),
                   MakePointerChecker<LoraPhy> ())
    .AddAttribute ("Mac", "The MAC layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&LoraNetDevice::GetMac,
                                        &LoraNetDevice::SetMac),
                   MakePointerChecker<LorawanMac> ())
    .AddTraceSource ("PromiscSniffer", 
                      "Trace source simulating a promiscuous packet sniffer "
                      "attached to the device",
                      MakeTraceSourceAccessor (&LoraNetDevice::m_promiscSnifferTrace),
                      "ns3::Packet::TracedCallback");
  return tid;
}

LoraNetDevice::LoraNetDevice () :
  m_node (0),
  m_phy (0),
  m_mac (0),
  m_configComplete (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

LoraNetDevice::~LoraNetDevice ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
LoraNetDevice::SetMac (Ptr<LorawanMac> mac)
{
  m_mac = mac;
}

Ptr<LorawanMac>
LoraNetDevice::GetMac (void) const
{
  return m_mac;
}

void
LoraNetDevice::SetPhy (Ptr<LoraPhy> phy)
{
  m_phy = phy;
}

Ptr<LoraPhy>
LoraNetDevice::GetPhy (void) const
{
  return m_phy;
}

void
LoraNetDevice::CompleteConfig (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Verify we have all the necessary pieces
  if (m_mac == 0 || m_phy == 0 || m_node == 0 || m_configComplete)
    {
      return;
    }

  m_mac->SetPhy (m_phy);
  m_configComplete = true;
}

void
LoraNetDevice::Send (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // Send the packet to the MAC layer, if it exists
  NS_ASSERT (m_mac != 0);
  m_mac->Send (packet);
  m_promiscSnifferTrace (packet);
}

void
LoraNetDevice::Receive (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // Fill protocol and address with empty stuff
  NS_LOG_DEBUG ("Calling receiveCallback");
  m_receiveCallback (this, packet, 0, Address ());
  m_promiscSnifferTrace (packet);
}

/******************************************
 *    Methods inherited from NetDevice    *
 ******************************************/

Ptr<Channel>
LoraNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy->GetChannel ();
}

Ptr<LoraChannel>
LoraNetDevice::DoGetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_phy->GetChannel ();
}

void
LoraNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this << index);
}

uint32_t
LoraNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION (this);

  return 0;
}

void
LoraNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this);
}

Address
LoraNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION (this);

  return Address ();
}

bool
LoraNetDevice::SetMtu (const uint16_t mtu)
{
  NS_ABORT_MSG ("Unsupported");

  return false;
}

uint16_t
LoraNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);

  return 0;
}

bool
LoraNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);

  return m_phy != 0;
}

void
LoraNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
}

bool
LoraNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);

  return true;
}

Address
LoraNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);

  return Address ();
}

bool
LoraNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);

  return true;
}

Address
LoraNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_ABORT_MSG ("Unsupported");

  return Address ();
}

Address
LoraNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this);

  return Address ();
}

bool
LoraNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);

  return false;
}

bool
LoraNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);

  return false;
}

bool
LoraNetDevice::Send (Ptr<Packet> packet, const Address& dest,
                     uint16_t protocolNumber)

{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);

  // Fallback to the vanilla Send method
  Send (packet);

  return true;
}

bool
LoraNetDevice::SendFrom (Ptr<Packet> packet, const Address& source,
                         const Address& dest, uint16_t protocolNumber)

{
  NS_ABORT_MSG ("Unsupported");

  return false;
}

Ptr<Node>
LoraNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION (this);

  return m_node;
}

void
LoraNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);

  m_node = node;
  CompleteConfig ();
}

bool
LoraNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);

  return true;
}

void
LoraNetDevice::SetReceiveCallback (ReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_receiveCallback = cb;
}

void
LoraNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION_NOARGS ();
}

bool
LoraNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  return false;
}

}
}
