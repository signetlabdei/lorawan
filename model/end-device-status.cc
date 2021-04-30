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
 * Authors: Martina Capuzzo <capuzzom@dei.unipd.it>
 *          Davide Magrin <magrinda@dei.unipd.it>
 */

#include "ns3/end-device-status.h"
#include "ns3/simulator.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/command-line.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/lora-tag.h"

#include <algorithm>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("EndDeviceStatus");

TypeId
EndDeviceStatus::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EndDeviceStatus")
                          .SetParent<Object> ()
                          .AddConstructor<EndDeviceStatus> ()
                          .SetGroupName ("lorawan");
  return tid;
}

EndDeviceStatus::EndDeviceStatus (LoraDeviceAddress endDeviceAddress,
                                  Ptr<ClassAEndDeviceLorawanMac> endDeviceMac)
    : m_reply (EndDeviceStatus::Reply ()),
      m_endDeviceAddress (endDeviceAddress),
      m_receivedPacketList (ReceivedPacketList ()),
      m_mac (endDeviceMac)
{
  NS_LOG_FUNCTION (endDeviceAddress);
}

EndDeviceStatus::EndDeviceStatus ()
{
  NS_LOG_FUNCTION_NOARGS ();

  // Initialize data structure
  m_reply = EndDeviceStatus::Reply ();
  m_receivedPacketList = ReceivedPacketList ();
}

EndDeviceStatus::~EndDeviceStatus ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

///////////////
//  Getters  //
///////////////

uint8_t
EndDeviceStatus::GetFirstReceiveWindowSpreadingFactor ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_firstReceiveWindowSpreadingFactor;
}

double
EndDeviceStatus::GetFirstReceiveWindowFrequency ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_firstReceiveWindowFrequency;
}

uint8_t
EndDeviceStatus::GetSecondReceiveWindowOffset ()
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_secondReceiveWindowOffset;
}

double
EndDeviceStatus::GetSecondReceiveWindowFrequency ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_secondReceiveWindowFrequency;
}

Ptr<Packet>
EndDeviceStatus::GetCompleteReplyPacket (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Start from reply payload
  Ptr<Packet> replyPacket;
  if (m_reply.payload) // If it has APP data to send
    {
      NS_LOG_DEBUG ("Crafting reply packet from existing payload");
      replyPacket = m_reply.payload->Copy ();
    }
  else // If no APP data needs to be sent, use an empty payload
    {
      NS_LOG_DEBUG ("Crafting reply packet using an empty payload");
      replyPacket = Create<Packet> (0);
    }

  // Add headers
  m_reply.frameHeader.SetAddress (m_endDeviceAddress);
  Ptr<Packet> lastPacket = GetLastPacketReceivedFromDevice ()->Copy ();
  LorawanMacHeader mHdr;
  LoraFrameHeader fHdr;
  fHdr.SetAsUplink ();
  lastPacket->RemoveHeader (mHdr);
  lastPacket->RemoveHeader (fHdr);
  m_reply.frameHeader.SetFCnt (fHdr.GetFCnt ());
  m_reply.macHeader.SetMType (LorawanMacHeader::UNCONFIRMED_DATA_DOWN);
  replyPacket->AddHeader (m_reply.frameHeader);
  replyPacket->AddHeader (m_reply.macHeader);

  NS_LOG_DEBUG ("Added MAC header" << m_reply.macHeader);
  NS_LOG_DEBUG ("Added frame header" << m_reply.frameHeader);

  return replyPacket;
}

bool
EndDeviceStatus::NeedsReply (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_reply.needsReply;
}

LorawanMacHeader
EndDeviceStatus::GetReplyMacHeader ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_reply.macHeader;
}

LoraFrameHeader
EndDeviceStatus::GetReplyFrameHeader ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_reply.frameHeader;
}

Ptr<Packet>
EndDeviceStatus::GetReplyPayload (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_reply.payload->Copy ();
}

Ptr<ClassAEndDeviceLorawanMac>
EndDeviceStatus::GetMac (void)
{
  return m_mac;
}

EndDeviceStatus::ReceivedPacketList
EndDeviceStatus::GetReceivedPacketList ()
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_receivedPacketList;
}

void
EndDeviceStatus::SetFirstReceiveWindowSpreadingFactor (uint8_t sf)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_firstReceiveWindowSpreadingFactor = sf;
}

void
EndDeviceStatus::SetFirstReceiveWindowFrequency (double frequency)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_firstReceiveWindowFrequency = frequency;
}

void
EndDeviceStatus::SetSecondReceiveWindowOffset (uint8_t offset)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_secondReceiveWindowOffset = offset;
}

void
EndDeviceStatus::SetSecondReceiveWindowFrequency (double frequency)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_secondReceiveWindowFrequency = frequency;
}

void
EndDeviceStatus::SetReplyMacHeader (LorawanMacHeader macHeader)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_reply.macHeader = macHeader;
}

void
EndDeviceStatus::SetReplyFrameHeader (LoraFrameHeader frameHeader)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_reply.frameHeader = frameHeader;
}

void
EndDeviceStatus::SetReplyPayload (Ptr<Packet> replyPayload)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_reply.payload = replyPayload;
}

///////////////////////
//   Other methods   //
///////////////////////

void
EndDeviceStatus::InsertReceivedPacket (Ptr<Packet const> receivedPacket, const Address &gwAddress)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Create a copy of the packet
  Ptr<Packet> myPacket = receivedPacket->Copy ();

  // Extract the headers
  LorawanMacHeader macHdr;
  myPacket->RemoveHeader (macHdr);

  LoraFrameHeader frameHdr;
  frameHdr.SetAsUplink ();
  myPacket->RemoveHeader (frameHdr);

  // Update current parameters
  LoraTag tag;
  myPacket->RemovePacketTag (tag);
  SetFirstReceiveWindowSpreadingFactor (tag.GetSpreadingFactor ());
  SetFirstReceiveWindowFrequency (tag.GetFrequency ());

  // Update Information on the received packet
  ReceivedPacketInfo info;
  info.sf = tag.GetSpreadingFactor ();
  info.frequency = tag.GetFrequency ();
  info.packet = receivedPacket;

  double rcvPower = tag.GetReceivePower ();

  // Perform insertion in list, also checking that the packet isn't already in
  // the list (it could have been received by another GW already)

  // Start searching from the end
  auto it = m_receivedPacketList.rbegin ();
  for (; it != m_receivedPacketList.rend (); it++)
    {
      // Get the frame counter of the current packet to compare it with the
      // newly received one
      Ptr<Packet> packetCopy = ((*it).first)->Copy ();
      LorawanMacHeader currentMacHdr;
      packetCopy->RemoveHeader (currentMacHdr);
      LoraFrameHeader currentFrameHdr;
      frameHdr.SetAsUplink ();
      packetCopy->RemoveHeader (currentFrameHdr);

      NS_LOG_DEBUG ("Received packet's frame counter: " << unsigned(frameHdr.GetFCnt ())
                                                        << "\nCurrent packet's frame counter: "
                                                        << unsigned(currentFrameHdr.GetFCnt ()));

      if (frameHdr.GetFCnt () == currentFrameHdr.GetFCnt ())
        {
          NS_LOG_INFO ("Packet was already received by another gateway");

          // This packet had already been received from another gateway:
          // add this gateway's reception information.
          GatewayList &gwList = it->second.gwList;

          PacketInfoPerGw gwInfo;
          gwInfo.receivedTime = Simulator::Now ();
          gwInfo.rxPower = rcvPower;
          gwInfo.gwAddress = gwAddress;
          gwList.insert (std::pair<Address, PacketInfoPerGw> (gwAddress, gwInfo));

          NS_LOG_DEBUG ("Size of gateway list: " << gwList.size ());

          break; // Exit from the cycle
        }
    }
  if (it == m_receivedPacketList.rend ())
    {
      NS_LOG_INFO ("Packet was received for the first time");
      PacketInfoPerGw gwInfo;
      gwInfo.receivedTime = Simulator::Now ();
      gwInfo.rxPower = rcvPower;
      gwInfo.gwAddress = gwAddress;
      info.gwList.insert (std::pair<Address, PacketInfoPerGw> (gwAddress, gwInfo));
      m_receivedPacketList.push_back (
          std::pair<Ptr<Packet const>, ReceivedPacketInfo> (receivedPacket, info));
    }
  NS_LOG_DEBUG (*this);
}

EndDeviceStatus::ReceivedPacketInfo
EndDeviceStatus::GetLastReceivedPacketInfo (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  auto it = m_receivedPacketList.rbegin ();
  if (it != m_receivedPacketList.rend ())
    {
      return it->second;
    }
  else
    {
      return EndDeviceStatus::ReceivedPacketInfo ();
    }
}

Ptr<Packet const>
EndDeviceStatus::GetLastPacketReceivedFromDevice (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  auto it = m_receivedPacketList.rbegin ();
  if (it != m_receivedPacketList.rend ())
    {
      return it->first;
    }
  else
    {
      return 0;
    }
}

void
EndDeviceStatus::InitializeReply ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_reply = Reply ();
  m_reply.needsReply = false;
}

void
EndDeviceStatus::AddMACCommand (Ptr<MacCommand> macCommand)
{
  m_reply.frameHeader.AddCommand (macCommand);
}

bool
EndDeviceStatus::HasReceiveWindowOpportunityScheduled ()
{
  return m_receiveWindowEvent.IsRunning();
}

void
EndDeviceStatus::SetReceiveWindowOpportunity (EventId event)
{
  m_receiveWindowEvent = event;
}

void
EndDeviceStatus::RemoveReceiveWindowOpportunity (void)
{
  Simulator::Cancel(m_receiveWindowEvent);
}

std::map<double, Address>
EndDeviceStatus::GetPowerGatewayMap (void)
{
  // Create a map of the gateways
  // Key: received power
  // Value: address of the corresponding gateway
  ReceivedPacketInfo info = m_receivedPacketList.back ().second;
  GatewayList gwList = info.gwList;

  std::map<double, Address> gatewayPowers;

  for (auto it = gwList.begin (); it != gwList.end (); it++)
    {
      Address currentGwAddress = (*it).first;
      double currentRxPower = (*it).second.rxPower;
      gatewayPowers.insert (std::pair<double, Address> (currentRxPower, currentGwAddress));
    }

  return gatewayPowers;
}

std::ostream &
operator<< (std::ostream &os, const EndDeviceStatus &status)
{
  os << "Total packets received: " << status.m_receivedPacketList.size () << std::endl;

  for (auto j = status.m_receivedPacketList.begin (); j != status.m_receivedPacketList.end (); j++)
    {
      EndDeviceStatus::ReceivedPacketInfo info = (*j).second;
      EndDeviceStatus::GatewayList gatewayList = info.gwList;
      Ptr<Packet const> pkt = (*j).first;
      os << pkt << " " << gatewayList.size () << std::endl;
      for (EndDeviceStatus::GatewayList::iterator k = gatewayList.begin (); k != gatewayList.end ();
           k++)
        {
          EndDeviceStatus::PacketInfoPerGw infoPerGw = (*k).second;
          os << "  " << infoPerGw.gwAddress << " " << infoPerGw.rxPower << std::endl;
        }
    }

  return os;
}
} // namespace lorawan
} // namespace ns3
