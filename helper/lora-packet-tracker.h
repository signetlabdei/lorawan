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
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef LORA_PACKET_TRACKER_H
#define LORA_PACKET_TRACKER_H

#include "ns3/nstime.h"
#include "ns3/packet.h"

#include <map>
#include <string>

namespace ns3
{
namespace lorawan
{

enum PhyPacketOutcome
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
    Ptr<const Packet> packet;
    uint32_t senderId;
    Time sendTime;
    std::map<int, enum PhyPacketOutcome> outcomes;
};

struct MacPacketStatus
{
    Ptr<const Packet> packet;
    uint32_t senderId;
    Time sendTime;
    Time receivedTime;
    std::map<int, Time> receptionTimes;
};

struct RetransmissionStatus
{
    Time firstAttempt;
    Time finishTime;
    uint8_t reTxAttempts;
    bool successful;
};

typedef std::map<Ptr<const Packet>, MacPacketStatus> MacPacketData;
typedef std::map<Ptr<const Packet>, PacketStatus> PhyPacketData;
typedef std::map<Ptr<const Packet>, RetransmissionStatus> RetransmissionData;

struct devCount_t
{
    int sent = 0;
    int received = 0;
};

using DevPktCount = std::unordered_map<uint32_t, devCount_t>;

struct phyCount_t
{
    std::vector<int> v = std::vector<int>(6, 0);
};

using GwsPhyPktCount = std::map<uint32_t, phyCount_t>;

struct phyPrint_t
{
    std::string s = "0 0 0 0 0 0";
};

using GwsPhyPktPrint = std::unordered_map<uint32_t, phyPrint_t>;

class LoraPacketTracker
{
  public:
    LoraPacketTracker();
    ~LoraPacketTracker();

    /////////////////////////
    // PHY layer callbacks //
    /////////////////////////
    // Packet transmission callback
    void TransmissionCallback(Ptr<const Packet> packet, uint32_t systemId);
    // Packet outcome traces
    void PacketReceptionCallback(Ptr<const Packet> packet, uint32_t systemId);
    void InterferenceCallback(Ptr<const Packet> packet, uint32_t systemId);
    void NoMoreReceiversCallback(Ptr<const Packet> packet, uint32_t systemId);
    void UnderSensitivityCallback(Ptr<const Packet> packet, uint32_t systemId);
    void LostBecauseTxCallback(Ptr<const Packet> packet, uint32_t systemId);

    /////////////////////////
    // MAC layer callbacks //
    /////////////////////////
    // Packet transmission at an EndDevice
    void MacTransmissionCallback(Ptr<const Packet> packet);
    void RequiredTransmissionsCallback(uint8_t reqTx,
                                       bool success,
                                       Time firstAttempt,
                                       Ptr<Packet> packet);
    // Packet reception at the Gateway
    void MacGwReceptionCallback(Ptr<const Packet> packet);

    ///////////////////////////////
    // Packet counting functions //
    ///////////////////////////////
    bool IsUplink(Ptr<const Packet> packet);

    // void CountRetransmissions (Time transient, Time simulationTime, MacPacketData
    //                            macPacketTracker, RetransmissionData reTransmissionTracker,
    //                            PhyPacketData packetTracker);

    /**
     * Count packets to evaluate the performance at PHY level of a specific
     * gateway.
     */
    std::vector<int> CountPhyPacketsPerGw(Time startTime, Time stopTime, int systemId);
    std::string PrintPhyPacketsPerGw(Time startTime, Time stopTime, int systemId);

    void CountPhyPacketsAllGws(Time startTime, Time stopTime, GwsPhyPktCount& output);
    void PrintPhyPacketsAllGws(Time startTime, Time stopTime, GwsPhyPktPrint& output);

    std::string PrintPhyPacketsGlobally(Time startTime, Time stopTime);

    /**
     * Count packets to evaluate the performance at MAC level of a specific
     * gateway.
     */
    std::string CountMacPacketsPerGw(Time startTime, Time stopTime, int systemId);
    std::string PrintMacPacketsPerGw(Time startTime, Time stopTime, int systemId);

    /**
     * Count the number of retransmissions that were needed to correctly deliver a
     * packet and receive the corresponding acknowledgment.
     */
    std::string CountRetransmissions(Time startTime, Time stopTime);

    /**
     * Count packets to evaluate the global performance at MAC level of the whole
     * network. In this case, a MAC layer packet is labeled as successful if it
     * was successful at at least one of the available gateways.
     *
     * This returns a string containing the number of sent packets and the number
     * of packets that were received by at least one gateway.
     */
    std::string CountMacPacketsGlobally(Time startTime, Time stopTime);

    /**
     * Count packets to evaluate the global performance at MAC level of the whole
     * network. In this case, a MAC layer packet is labeled as successful if it
     * was successful at at least one of the available gateways, and if
     * the corresponding acknowledgment was correctly delivered at the device.
     *
     * This returns a string containing the number of sent packets and the number
     * of packets that generated a successful acknowledgment.
     */
    std::string CountMacPacketsGloballyCpsr(Time startTime, Time stopTime);

    std::string PrintDevicePackets(Time startTime, Time stopTime, uint32_t devId);
    void CountAllDevicesPackets(Time startTime, Time stopTime, DevPktCount& out);

    std::string PrintSimulationStatistics(Time startTime = Seconds(0));

    void EnableOldPacketsCleanup(Time oldPacketThreshold = Hours(12));

  private:
    void CleanupOldPackets();

    PhyPacketData m_packetTracker;
    MacPacketData m_macPacketTracker;
    RetransmissionData m_reTransmissionTracker;

    Time m_oldPacketThreshold;
    Time m_lastPacketCleanup;
};
} // namespace lorawan
} // namespace ns3
#endif
