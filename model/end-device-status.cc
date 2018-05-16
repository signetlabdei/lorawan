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
  m_receivedPacketList = EndDeviceStatus::ReceivedPacketList();
}

EndDeviceStatus::EndDeviceStatus (Ptr<Packet> receivedPacket) :
{
  NS_LOG_FUNCTION (this);
  //TODO sintassi?
  this->InsertReceivedPacket (receivedPacket);
}

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

uint8_t
EndDeviceStatus::GetFirstReceiveWindowDataRate ()
{
  NS_LOG_FUNCTION (this);

  sf1 = m_firstReceiveWindowSpreadingFactor;
  if (sf1 == 7)
    return 5;
  else if (sf1 == 8)
    return 4;
  else if (sf1 == 9)
    return 3;
  else if (sf1 == 10)
    return 2;
  else if (sf1 == 11)
    return 1;
  else if (sf1 == 12)
    return 0;
}

double
EndDeviceStatus::GetFirstReceiveWindowFrequency ()
{
  NS_LOG_FUNCTION (this << m_firstReceiveWindowFrequency);

  return m_firstReceiveWindowFrequency;
}

  uint8_t
  EndDeviceStatus::GetSecondReceiveWindowSpreadingFactor ()
  {
    NS_LOG_FUNCTION (this << m_firstReceiveWindowSpreadingFactor);

    return m_secondReceiveWindowSpreadingFactor;
  }

uint8_t
EndDeviceStatus::GetSecondReceiveWindowDataRate ()
{
  NS_LOG_FUNCTION (this);

  sf2 = m_secondReceiveWindowSpreadingFactor;
  if (sf2 == 7)
    return 5;
  else if (sf2 == 8)
    return 4;
  else if (sf2 == 9)
    return 3;
  else if (sf2 == 10)
    return 2;
  else if (sf2 == 11)
    return 1;
  else if (sf2 == 12)
    return 0;
}

double
EndDeviceStatus::GetSecondReceiveWindowFrequency ()
{
  NS_LOG_FUNCTION (this << m_secondReceiveWindowFrequency);
  return m_secondReceiveWindowFrequency;
}

bool needReply ()
{
  NS_LOG_FUNCTION (this << m_needReply);
  return m_needReply;
}

EndDeviceStatus::Reply
EndDeviceStatus::GetReply ()
{
  NS_LOG_FUNCTION (this);
  return m_reply;
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
  EndDeviceStatus::SetFirstReceiveWindowSpreadingFactor (uint8_t sf);
  {
    NS_LOG_FUNCTION (this << sf);
    m_firstReceiveWindowSpreadingFactor = sf;
  }

  void
  EndDeviceStatus::SetFirstReceiveWindowFrequency (double frequency);
  {
    NS_LOG_FUNCTION (this << frequency);
    m_firstReceiveWindowFrequency = frequency;
  }

  void
  EndDeviceStatus::SetSecondReceiveWindowSpreadingFactor (uint8_t sf);
  {
    NS_LOG_FUNCTION (this << sf);
    m_secondReceiveWindowSpreadingFactor = sf;
  }

  void
  EndDeviceStatus::SetSecondReceiveWindowFrequency (double frequency);
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
  EndDeviceStatus::SetNeedReply (bool needReply)
  {
    NS_LOG_FUNCTION (this << needReply);
    m_needReply = needReply;
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


  //////////////////////
  //   Other method   //
  //////////////////////

void
EndDeviceStatus::InsertReceivedPacket (Ptr<Packet> receivedPacket, struct ReceivedPacketInfo info)
{
  NS_LOG_FUNCTION (this);
  m_receivedPacketList.insert(std::pair<Ptr<Packet> const>, ReceivedPacketInfo info));
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
