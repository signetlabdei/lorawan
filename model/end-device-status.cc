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
#include "ns3/lora-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/command-line.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/lora-tag.h"

#include <algorithm>

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("EndDeviceStatus");

  EndDeviceStatus::EndDeviceStatus ()
  {
    NS_LOG_FUNCTION (this);

    // Initialize data structure
    m_reply = EndDeviceStatus::Reply();
    m_receivedPacketList = ReceivedPacketList();
  }

  EndDeviceStatus::~EndDeviceStatus ()
  {
    NS_LOG_FUNCTION (this);
  }

  ///////////////
  //  Getters  //
  ///////////////

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

  Ptr<Packet>
  EndDeviceStatus::GetReply ()
  {
    NS_LOG_FUNCTION (this);

    // Start from reply payload
    Ptr<Packet> replyPacket;
    if (m_reply.payload) // If it has APP data to send
      {
        NS_LOG_DEBUG ("Crafting reply packet from existing payload");
        replyPacket = m_reply.payload-> Copy();
      }
    else // If no APP data needs to be sent, use an empty payload
      {
        NS_LOG_DEBUG ("Crafting reply packet using an empty payload");
        replyPacket = Create<Packet> (0);
      }

    // Add headers
    replyPacket->AddHeader (m_reply.frameHeader);
    replyPacket->AddHeader (m_reply.macHeader);

    return replyPacket;
  }


  bool
  EndDeviceStatus::NeedsReply (void)
  {
    NS_LOG_FUNCTION (this);

    return m_reply.needsReply;
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
  EndDeviceStatus::SetReplyPayload(Ptr<Packet> replyPayload)
  {
    NS_LOG_FUNCTION (this);
    m_reply.payload = replyPayload;
  }

  ///////////////////////
  //   Other methods   //
  ///////////////////////

  void
  EndDeviceStatus::InsertReceivedPacket (Ptr<Packet const> receivedPacket,
                                         const Address& gwAddress)
  {
    NS_LOG_FUNCTION (this);

    // Create a copy of the packet
    Ptr<Packet> myPacket = receivedPacket->Copy ();

    // Extract the headers
    LoraMacHeader macHdr;
    myPacket->RemoveHeader (macHdr);

    LoraFrameHeader frameHdr;
    myPacket->RemoveHeader (frameHdr);

    // Update current parameters
    LoraTag tag;
    myPacket->RemovePacketTag (tag);
    SetFirstReceiveWindowSpreadingFactor(tag.GetSpreadingFactor());
    SetFirstReceiveWindowFrequency(tag.GetFrequency());
    //TODO extract BW

    // Update Information on the received packet
    ReceivedPacketInfo info;
    info.sf = tag.GetSpreadingFactor();
    info.frequency= tag.GetFrequency();

    double rcvPower = tag.GetReceivePower();

    auto it = m_receivedPacketList.find(receivedPacket);
    if (it != m_receivedPacketList.end())
      {
        m_receivedPacketList.insert(std::pair<Ptr<Packet const>, ReceivedPacketInfo>
                                    (receivedPacket,info));
      }
    // this packet had already been received from another gateway:
    //   adding this gateway's reception information.
    else
      {
        ReceivedPacketInfo savedInfo = it -> second;
        GatewayList savedGwList = savedInfo.gwlist;
        UpdateGatewayData (savedGwList, gwAddress, rcvPower);
      }

    // Determine whether the packet requires a reply
    if (macHdr.GetMType () == LoraMacHeader::CONFIRMED_DATA_UP)
      {
        NS_LOG_DEBUG ("Scheduling a reply for this device");

        m_reply.needsReply = true;

        LoraFrameHeader replyFrameHdr = LoraFrameHeader ();
        replyFrameHdr.SetAsDownlink ();
        // replyFrameHdr.SetAddress (frameHdr.GetAddress ());
        replyFrameHdr.SetAck (true);
        m_reply.frameHeader = replyFrameHdr;

      }
  }

  void
  EndDeviceStatus::InitializeReply ()
  {
    NS_LOG_FUNCTION (this);
    m_reply = Reply ();
    m_reply.needsReply = false;
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
