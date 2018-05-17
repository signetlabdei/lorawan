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

#include "ns3/end-device-status.h"
#include "ns3/lora-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/command-line.h"
#include "ns3/simulator.h"
#include <algorithm>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("EndDeviceStatus");

EndDeviceStatus::EndDeviceStatus ()
{
  NS_LOG_FUNCTION (this);
}

EndDeviceStatus::~EndDeviceStatus ()
{
  NS_LOG_FUNCTION (this);
  m_reply = EndDeviceStatus::Reply();
  m_receivedPacketList = ReceivedPacketList();
}

  /* EndDeviceStatus::EndDeviceStatus (Ptr<Packet> receivedPacket, ReceivedPacketInfo packetInfo)
{
  NS_LOG_FUNCTION (this);
  // Basta InserReceivedPacket (compila e prova! :) )
  InsertReceivedPacket (receivedPacket, packetInfo);
}
  */

///////////////
//  Getters  //
///////////////

LoraDeviceAddress
EndDeviceStatus::GetAddress ()
{
  NS_LOG_FUNCTION (this);

  return m_address;
}

uint8_t
EndDeviceStatus::GetFirstReceiveWindowSpreadingFactor ()
{
  NS_LOG_FUNCTION (this << m_firstReceiveWindowSpreadingFactor);

  return m_firstReceiveWindowSpreadingFactor;
}

double
EndDeviceStatus::GetFirstReceiveWindowFrequency ()
{
  NS_LOG_FUNCTION (this << m_firstReceiveWindowFrequency);

  return m_firstReceiveWindowFrequency;
}

  uint8_t
  EndDeviceStatus::GetSecondReceiveWindowOffset ()
  {
    NS_LOG_FUNCTION (this << m_secondReceiveWindowOffset);

    return m_secondReceiveWindowOffset;
  }

double
EndDeviceStatus::GetSecondReceiveWindowFrequency ()
{
  NS_LOG_FUNCTION (this << m_secondReceiveWindowFrequency);
  return m_secondReceiveWindowFrequency;
}

bool
EndDeviceStatus::NeedsReply ()
{
  NS_LOG_FUNCTION (this << m_needsReply);
  return m_needsReply;
}

Ptr<Packet>
EndDeviceStatus::GetReply ()
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> replyPacket;
  if (m_hasReplyPayload)
    {
      replyPacket = m_reply.payload-> Copy();
    }
  else
    {
      replyPacket = Create<Packet> (m_payloadSize);
    }
  replyPacket -> AddHeader (m_reply.frameHeader);
  replyPacket -> AddHeader(m_reply.macHeader);
  return replyPacket;
}

LoraMacHeader
EndDeviceStatus::GetReplyMacHeader ()
{
  NS_LOG_FUNCTION (this);
  return m_reply.macHeader;
}

LoraFrameHeader
EndDeviceStatus::GetReplyFrameHeader ()
{
  NS_LOG_FUNCTION (this);
  return m_reply.frameHeader;
}

Ptr<Packet>
EndDeviceStatus::GetReplyPayload (void)
{
  NS_LOG_FUNCTION (this);
  return m_reply.payload;
}

EndDeviceStatus::ReceivedPacketList
EndDeviceStatus::GetReceivedPacketList ()
{
  NS_LOG_FUNCTION (this);
  return m_receivedPacketList;
}


  /////////////////
  //   Setters   //
  /////////////////

  void
  EndDeviceStatus::SetFirstReceiveWindowSpreadingFactor (uint8_t sf)
  {
    NS_LOG_FUNCTION (this << sf);
    m_firstReceiveWindowSpreadingFactor = sf;
  }

  void
  EndDeviceStatus::SetFirstReceiveWindowFrequency (double frequency)
  {
    NS_LOG_FUNCTION (this << frequency);
    m_firstReceiveWindowFrequency = frequency;
  }

  void
  EndDeviceStatus::SetSecondReceiveWindowSpreadingFactor (uint8_t sf)
  {
    NS_LOG_FUNCTION (this << sf);
    m_secondReceiveWindowSpreadingFactor = sf;
  }

  void
  EndDeviceStatus::SetSecondReceiveWindowFrequency (double frequency)
  {
    NS_LOG_FUNCTION (this << frequency);
    m_secondReceiveWindowFrequency = frequency;
  }

  void
  EndDeviceStatus::SetReply (struct Reply reply)
  {
    NS_LOG_FUNCTION (this);
    m_reply = reply;
  }

  void
  EndDeviceStatus::SetNeedsReply (bool needsReply)
  {
    NS_LOG_FUNCTION (this << needsReply);
    m_needsReply = needsReply;
  }

  void
  EndDeviceStatus::SetReplyMacHeader (LoraMacHeader macHeader)
  {
    NS_LOG_FUNCTION (this);
    m_reply.macHeader = macHeader;
  }

  void
  EndDeviceStatus::SetReplyFrameHeader (LoraFrameHeader frameHeader)
  {
    NS_LOG_FUNCTION (this);
    m_reply.frameHeader = frameHeader;
  }

  void
  EndDeviceStatus::SetReplyPayload (Ptr<Packet> data)
  {
    NS_LOG_FUNCTION (this);
    m_reply.payload = data;
  }

  void
  EndDeviceStatus::SetPayloadSize (uint8_t payloadSize)
  {
    NS_LOG_FUNCTION (this << payloadSize);
    m_payloadSize = payloadSize;
  }
  //////////////////////
  //   Other method   //
  //////////////////////

void
EndDeviceStatus::InsertReceivedPacket (Ptr<Packet> receivedPacket, struct
                                            ReceivedPacketInfo info)
{
  NS_LOG_FUNCTION (this);
  m_receivedPacketList.insert(std::pair<Ptr<Packet const>, ReceivedPacketInfo> (receivedPacket,info));
}

void
EndDeviceStatus::InitializeReply ()
{
  NS_LOG_FUNCTION (this);
  m_reply = Reply ();
}

  /*
Address
EndDeviceStatus::GetBestGatewayAddress (void)
{
  NS_LOG_FUNCTION (this);

  return (*(std::max_element (m_gateways.begin (), m_gateways.end (),
                              [] (const std::pair<Address, double>&p1,
                                  const std::pair<Address, double>&p2)
                              { return p1.second > p2.second; }
                              ))).first;
}

std::list<Address>
EndDeviceStatus::GetSortedGatewayAddresses (void)
{
  NS_LOG_FUNCTION (this);

  // Copy the map pairs into a vector
  std::vector<std::pair<Address, double> > pairs;
  for (auto it = m_gateways.begin (); it != m_gateways.end (); ++it)
    {
      pairs.push_back (*it);
    }

  // Sort the vector
  std::sort (pairs.begin (), pairs.end (),
             [] (const std::pair<Address, double>&p1,
                 const std::pair<Address, double>&p2)
             { return p1.second > p2.second; }
             );

  // Create a new array with only the addresses
  std::list<Address> addresses;
  for (auto it = pairs.begin (); it != pairs.end (); ++it)
    {
      addresses.push_back ((*it).first);
    }

  // Return the list
  return addresses;
}

bool
EndDeviceStatus::HasReply (void)
{
  NS_LOG_FUNCTION (this << m_reply.hasReply);

  return m_reply.hasReply;
}

void
EndDeviceStatus::SetReply (struct Reply reply)
{
  NS_LOG_FUNCTION (this);

  m_reply = reply;
}

Ptr<Packet>
EndDeviceStatus::GetReplyPacket (void)
{
  NS_LOG_FUNCTION (this);

  // Add headers to the packet
  Ptr<Packet> replyPacket = m_reply.packet->Copy ();
  replyPacket->AddHeader (m_reply.frameHeader);
  replyPacket->AddHeader (m_reply.macHeader);

  return replyPacket;
}

void
EndDeviceStatus::SetFirstReceiveWindowFrequency (double frequency)
{
  NS_LOG_FUNCTION (this << frequency);

  m_firstReceiveWindowFrequency = frequency;
}

double
EndDeviceStatus::GetFirstReceiveWindowFrequency (void)
{
  NS_LOG_FUNCTION (this);

  return m_firstReceiveWindowFrequency;
}

double
EndDeviceStatus::GetSecondReceiveWindowFrequency (void)
{
  NS_LOG_FUNCTION (this);

  return m_mac->GetSecondReceiveWindowFrequency ();
}

uint8_t
EndDeviceStatus::GetFirstReceiveWindowDataRate (void)
{
  NS_LOG_FUNCTION (this);

  return m_mac->GetFirstReceiveWindowDataRate ();
}

uint8_t
EndDeviceStatus::GetSecondReceiveWindowDataRate (void)
{
  NS_LOG_FUNCTION (this);

  return m_mac->GetSecondReceiveWindowDataRate ();
}
  */


}
