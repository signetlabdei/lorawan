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
#ifndef LORA_PACKET_TRACKER_H
#define LORA_PACKET_TRACKER_H

#include "ns3/packet.h"
#include "ns3/nstime.h"

#include <map>
#include <string>

namespace ns3 {
enum PacketOutcome
{
  RECEIVED,
  INTERFERED,
  NO_MORE_RECEIVERS,
  UNDER_SENSITIVITY,
  LOST_BECAUSE_TX,
  UNSET
};

struct PacketStatus
{
  Ptr<Packet const> packet;
  uint32_t senderId;
  int outcomeNumber;
  std::vector<enum PacketOutcome> outcomes;
};

struct RetransmissionStatus
{
  Time firstAttempt;
  Time finishTime;
  uint8_t reTxAttempts;
  bool successful;
};

struct MacPacketStatus
{
  Time sendTime;
  Time receivedTime;
  uint32_t systemId;
};

typedef std::pair<Time, PacketOutcome> PhyOutcome;

typedef std::map<Ptr<Packet const>, MacPacketStatus> MacPacketData;
typedef std::map<Ptr<Packet const>, PacketStatus> PhyPacketData;
typedef std::map<Ptr<Packet const>, RetransmissionStatus> RetransmissionData;


class LoraPacketTracker
{
public:
  LoraPacketTracker (std::string filename);
  ~LoraPacketTracker ();

  ///////////////
  // PHY layer //
  ///////////////
  // Packet transmission callback
  void TransmissionCallback (Ptr<Packet const> packet, uint32_t systemId);
  // Packet outcome traces
  void PacketReceptionCallback (Ptr<Packet const> packet, uint32_t systemId);
  void InterferenceCallback (Ptr<Packet const> packet, uint32_t systemId);
  void NoMoreReceiversCallback (Ptr<Packet const> packet, uint32_t systemId);
  void UnderSensitivityCallback (Ptr<Packet const> packet, uint32_t systemId);
  void LostBecauseTxCallback (Ptr<Packet const> packet, uint32_t systemId);

  ///////////////
  // MAC layer //
  ///////////////
  // Packet transmission at an EndDevice
  void MacTransmissionCallback (Ptr<Packet const> packet);
  void RequiredTransmissionsCallback (uint8_t reqTx, bool success,
                                      Time firstAttempt, Ptr<Packet> packet);
  // Packet reception at the Gateway
  void MacGwReceptionCallback (Ptr<Packet const> packet);

  ////////////////////////////////
  // Packet counting facilities //
  ////////////////////////////////
  void CheckReceptionByAllGWsComplete (std::map<Ptr<Packet const>,
                                                PacketStatus>::iterator it);

  void CountRetransmissions (Time transient, Time simulationTime, MacPacketData
                             macPacketTracker, RetransmissionData reTransmissionTracker,
                             PhyPacketData packetTracker);

  void CountPhyPackets (Time startTime, Time stopTime);

  void PrintVector (std::vector<int> vector);

  void PrintSumRetransmissions (std::vector<int> reTxVector);

  ///////////////
  // Utilities //
  ///////////////
  void PrintPerformance (Time start, Time stop);

private:
  void DoCountPhyPackets (Time startTime, Time stopTime, PhyPacketData packetTracker);

  std::list<PhyOutcome> m_phyPacketOutcomes;

  std::string m_outputFilename;

  PhyPacketData m_packetTracker;
  MacPacketData m_macPacketTracker;
  RetransmissionData m_reTransmissionTracker;
};
}
#endif
