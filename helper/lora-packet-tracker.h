/*
 * Copyright (c) 2018 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
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

/**
 * @ingroup lorawan
 *
 * Stores PHY-layer uplink packet metrics of sender/receivers.
 */
struct PacketStatus
{
    Ptr<const Packet> packet;                      //!< Packet being tracked
    uint32_t senderId;                             //!< Node id of the packet sender
    Time sendTime;                                 //!< Timestamp of pkt radio tx start
    std::map<int, enum PhyPacketOutcome> outcomes; //!< Reception outcome of this pkt at the end of
                                                   //!< the tx, mapped by gateway's node id
};

/**
 * @ingroup lorawan
 *
 * Stores MAC-layer packet metrics of sender/receivers.
 *
 * @remark Can be used for both uplink and downlink packets.
 */
struct MacPacketStatus
{
    Ptr<const Packet> packet; //!< Packet being tracked
    uint32_t senderId;        //!< Node id of the packet sender
    Time sendTime;     //!< Timestamp of the pkt leaving MAC layer to go down the stack of sender
    Time receivedTime; //!< Time of first reception (placeholder field)
                       //!< \todo Field set to max and not used
    std::map<int, Time> receptionTimes; //!< Timestamp of the pkt leaving MAC layer to go up the
                                        //!< stack, mapped by receiver's node id
};

/**
 * @ingroup lorawan
 *
 * Stores (optionally enabled) MAC layer packet retransmission process metrics of end devices.
 */
struct RetransmissionStatus
{
    Time firstAttempt;    //!< Timestamp of the first transmission attempt
    Time finishTime;      //!< Timestamp of the conclusion of the retransmission process
    uint8_t reTxAttempts; //!< Number of transmissions attempted during the process
    bool successful;      //!< Whether the retransmission procedure was successful
};

typedef std::map<Ptr<const Packet>, MacPacketStatus> MacPacketData;
typedef std::map<Ptr<const Packet>, PacketStatus> PhyPacketData;
typedef std::map<Ptr<const Packet>, RetransmissionStatus> RetransmissionData;

/**
 * @ingroup lorawan
 *
 * Tracks and stores packets sent in the simulation and provides aggregation functionality
 */
class LoraPacketTracker
{
  public:
    LoraPacketTracker();  //!< Default constructor
    ~LoraPacketTracker(); //!< Destructor

    ///////////////////////////
    // PHY layer trace sinks //
    ///////////////////////////

    // Packet transmission by end devices

    /**
     * Trace a packet TX start by the PHY layer of an end device.
     *
     * @param packet The packet being transmitted.
     * @param systemId Id of end device transmitting the packet.
     */
    void TransmissionCallback(Ptr<const Packet> packet, uint32_t systemId);

    // Packet reception outcome at gateways

    /**
     * Trace a correct packet RX by the PHY layer of a gateway.
     *
     * @param packet The packet being received.
     * @param systemId Id of the gateway receiving the packet.
     */
    void PacketReceptionCallback(Ptr<const Packet> packet, uint32_t systemId);
    /**
     * Trace a gateway packet loss caused by interference.
     *
     * @param packet The packet being lost.
     * @param systemId Id of the gateway losing the packet.
     */
    void InterferenceCallback(Ptr<const Packet> packet, uint32_t systemId);
    /**
     * Trace a gateway packet loss caused by lack of free reception paths.
     *
     * @param packet The packet being lost.
     * @param systemId Id of the gateway losing the packet.
     */
    void NoMoreReceiversCallback(Ptr<const Packet> packet, uint32_t systemId);
    /**
     * Trace a gateway packet loss caused by signal strength under sensitivity.
     *
     * @param packet The packet being lost.
     * @param systemId Id of the gateway losing the packet.
     */
    void UnderSensitivityCallback(Ptr<const Packet> packet, uint32_t systemId);
    /**
     * Trace a gateway packet loss caused by concurrent downlink transmission.
     *
     * @param packet The packet being lost.
     * @param systemId Id of the gateway losing the packet.
     */
    void LostBecauseTxCallback(Ptr<const Packet> packet, uint32_t systemId);

    ///////////////////////////
    // MAC layer trace sinks //
    ///////////////////////////

    // Packet send

    /**
     * Trace a packet leaving a node's MAC layer to go down the stack and be sent by the PHY layer.
     *
     * @remark This trace sink is normally connected to both end devices and gateways.
     *
     * @param packet The packet being sent.
     */
    void MacTransmissionCallback(Ptr<const Packet> packet);
    /**
     * Trace the exit status of a MAC layer packet retransmission process of an end device.
     *
     * @param reqTx Number of transmissions attempted during the process.
     * @param success Whether the retransmission procedure was successful.
     * @param firstAttempt Timestamp of the initial transmission attempt.
     * @param packet The packet being retransmitted.
     */
    void RequiredTransmissionsCallback(uint8_t reqTx,
                                       bool success,
                                       Time firstAttempt,
                                       Ptr<Packet> packet);

    // Packet reception by gateways

    /**
     * Trace a packet leaving a gateway's MAC layer to go up the stack and be delivered to the
     * node's application.
     *
     * @param packet The packet being received.
     */
    void MacGwReceptionCallback(Ptr<const Packet> packet);

    ///////////////////////////////
    // Packet counting functions //
    ///////////////////////////////

    /**
     * Check whether a packet is uplink.
     *
     * @param packet The packet to be checked.
     * @return True if the packet is uplink, false otherwise.
     */
    bool IsUplink(Ptr<const Packet> packet);

    // void CountRetransmissions (Time transient, Time simulationTime, MacPacketData
    //                            macPacketTracker, RetransmissionData reTransmissionTracker,
    //                            PhyPacketData packetTracker);

    /**
     * Count packets in a time interval to evaluate the performance at PHY level of a specific
     * gateway. It counts the total number of uplink packets sent over the radio medium, and - from
     * the perspective of the specified gateway - the number of such packets correctly received,
     * lost to interference, lost to unavailability of the gateway's reception paths, lost for being
     * under the RSSI sensitivity threshold, and lost due to concurrent downlink transmission of the
     * gateway.
     *
     * @param startTime Timestamp of the start of the measurement.
     * @param stopTime Timestamp of the end of the measurement.
     * @param systemId Node id of the gateway.
     * @return A vector comprised of the following fields: [totPacketsSent, receivedPackets,
     * interferedPackets, noMoreGwPackets, underSensitivityPackets, lostBecauseTxPackets].
     */
    std::vector<int> CountPhyPacketsPerGw(Time startTime, Time stopTime, int systemId);
    /**
     * @copydoc ns3::lorawan::LoraPacketTracker::CountPhyPacketsPerGw
     * @return Values in the output vector are formatted into a space-separated string.
     */
    std::string PrintPhyPacketsPerGw(Time startTime, Time stopTime, int systemId);

    /**
     * Count packets in a time interval to evaluate the performance at MAC level of a specific
     * gateway.
     *
     * @param startTime Timestamp of the start of the measurement.
     * @param stopTime Timestamp of the end of the measurement.
     * @param systemId Node id of the gateway.
     * @return String of output values.
     *
     * @todo Not implemented, this is a placeholder for future implementation.
     */
    std::string CountMacPacketsPerGw(Time startTime, Time stopTime, int systemId);
    /** @copydoc ns3::lorawan::LoraPacketTracker::CountMacPacketsPerGw */
    std::string PrintMacPacketsPerGw(Time startTime, Time stopTime, int systemId);

    /**
     * In a time interval, count the number of retransmissions that were needed to correctly deliver
     * a packet and receive the corresponding acknowledgment.
     *
     * @param startTime Timestamp of the start of the measurement.
     * @param stopTime Timestamp of the end of the measurement.
     * @return String of output values.
     *
     * @todo Not implemented, this is a placeholder for future implementation.
     */
    std::string CountRetransmissions(Time startTime, Time stopTime);

    /**
     * In a time interval, count packets to evaluate the global performance at MAC level of the
     * whole network. In this case, a MAC layer packet is labeled as successful if it was successful
     * at at least one of the available gateways.
     *
     * @param startTime Timestamp of the start of the measurement.
     * @param stopTime Timestamp of the end of the measurement.
     * @return Space-separated string containing two metrics: the number of sent packets and the
     * number of packets that were received by at least one gateway.
     */
    std::string CountMacPacketsGlobally(Time startTime, Time stopTime);

    /**
     * In a time interval, count packets to evaluate the performance at MAC level of the whole
     * network. In this case, a MAC layer packet is labeled as successful if it was successful at at
     * least one of the available gateways, and if the corresponding acknowledgment was correctly
     * delivered at the device.
     *
     * @param startTime Timestamp of the start of the measurement.
     * @param stopTime Timestamp of the end of the measurement.
     * @return Space-separated string containing two metrics: the number of sent packets and the
     * number of packets that generated a successful acknowledgment.
     */
    std::string CountMacPacketsGloballyCpsr(Time startTime, Time stopTime);

  private:
    PhyPacketData m_packetTracker;              //!< Packet map of PHY layer metrics
    MacPacketData m_macPacketTracker;           //!< Packet map of MAC layer metrics
    RetransmissionData m_reTransmissionTracker; //!< Packet map of retransmission process metrics
};
} // namespace lorawan
} // namespace ns3
#endif
