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
 * Author: Martina Capuzzo <capuzzom@dei.unipd.it>
 */

#include "ns3/end-device-status.h"
#include "ns3/simulator.h"
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

  /*
    LoraDeviceAddress
  EndDeviceStatus::GetAddress ()
  {
    NS_LOG_FUNCTION (this);

    return m_address;
  }
  */

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
    NS_LOG_FUNCTION (this << m_hasReplyPayload);
    Ptr<Packet> replyPacket;
    if (m_hasReplyPayload)
      {
        replyPacket = m_reply.payload-> Copy();
        NS_LOG_DEBUG ("End device status reply has already a payload");
        
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
    return m_reply.payload-> Copy();
  }

  EndDeviceStatus::ReceivedPacketList
  EndDeviceStatus::GetReceivedPacketList ()
  {
    NS_LOG_FUNCTION (this);
    return m_receivedPacketList;
  }

  EndDeviceStatus::GatewayList
  EndDeviceStatus::GetGatewayList (Ptr<Packet const> packet)
  {
    ReceivedPacketInfo info= m_receivedPacketList.find(packet)-> second;
    return info.gwlist;
  }


  /////////////////
  //   Setters   //
  /////////////////

  void
  EndDeviceStatus::SetFirstReceiveWindowSpreadingFactor (uint8_t sf)
  {
    NS_LOG_FUNCTION (this << double(sf));
    m_firstReceiveWindowSpreadingFactor = sf;
  }

  void
  EndDeviceStatus::SetFirstReceiveWindowFrequency (double frequency)
  {
    NS_LOG_FUNCTION (this << frequency);
    m_firstReceiveWindowFrequency = frequency;
  }

  void
  EndDeviceStatus::SetSecondReceiveWindowOffset (uint8_t offset)
  {
    NS_LOG_FUNCTION (this << double(offset));
    m_secondReceiveWindowOffset = offset;
  }

  void
  EndDeviceStatus::SetSecondReceiveWindowFrequency (double frequency)
  {
    NS_LOG_FUNCTION (this << frequency);
    m_secondReceiveWindowFrequency = frequency;
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
    this->SetNeedsReply(true);

  }

  void
  EndDeviceStatus::SetReplyFrameHeader (LoraFrameHeader frameHeader)
  {
    NS_LOG_FUNCTION (this);
    m_reply.frameHeader = frameHeader;
    this->SetNeedsReply(true);
  }

  void
  EndDeviceStatus::SetReplyPayload(Ptr<Packet const> replyPayload)
  {
    NS_LOG_FUNCTION (this);
    m_reply.payload = replyPayload;
    m_hasReplyPayload= true;
    this->SetNeedsReply(true);
  }

  void
  EndDeviceStatus::SetPayloadSize (uint8_t payloadSize)
  {
    NS_LOG_FUNCTION (this << payloadSize);
    m_payloadSize = payloadSize;
  }
  ///////////////////////
  //   Other methods   //
  ///////////////////////

  void
  EndDeviceStatus::InsertReceivedPacket (Ptr<Packet const> receivedPacket,
                                         ReceivedPacketInfo info, const Address& gwAddress,
                                         double rcvPower)
  {
    NS_LOG_FUNCTION (this);
    std::map<Ptr<const ns3::Packet>, ns3::EndDeviceStatus::ReceivedPacketInfo>::iterator it = m_receivedPacketList.find(receivedPacket);
    if (it != m_receivedPacketList.end())
      {
        m_receivedPacketList.insert(std::pair<Ptr<Packet const>, ReceivedPacketInfo>
                                    (receivedPacket,info));
      }
    else
      {
        ReceivedPacketInfo savedInfo = it -> second;
        GatewayList savedGwList = savedInfo.gwlist;
        UpdateGatewayData (savedGwList, gwAddress, rcvPower);
      }
    m_lastReceivedPacket = receivedPacket;
  }

  void
  EndDeviceStatus::InitializeReply ()
  {
    NS_LOG_FUNCTION (this);
    m_reply = Reply ();
    m_needsReply = false;
    m_hasReplyPayload= false;
    
  }

  void
  EndDeviceStatus::AddMACCommand (Ptr<MacCommand> macCommand)
  {
    m_reply.macCommandList.push_back (macCommand);
  }

  void
  EndDeviceStatus::UpdateGatewayData (GatewayList gwList, Address gwAddress, double rcvPower)
  {  NS_LOG_FUNCTION (this << gwAddress << rcvPower);

    std::map<Address, PacketInfoPerGw>::iterator it = gwList.find (gwAddress);
    if (it != gwList.end ())
      {
        // Erase the existing entry
        gwList.erase (it);
      }
    // Create a new entry
    PacketInfoPerGw gwInfo;
    gwInfo.receivedTime = Simulator::Now();
    gwInfo.rxPower= rcvPower;
    gwList.insert (std::pair<Address, PacketInfoPerGw> (gwAddress, gwInfo));

  }

}
