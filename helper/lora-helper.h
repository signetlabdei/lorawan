/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LORA_HELPER_H
#define LORA_HELPER_H

#include "lora-packet-tracker.h"
#include "lora-phy-helper.h"
#include "lorawan-mac-helper.h"

#include "ns3/lora-net-device.h"
#include "ns3/net-device-container.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"

#include <ctime>

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Helps to create LoraNetDevice objects.
 *
 * This class can help create a large set of similar LoraNetDevice objects and
 * configure a large set of their attributes during creation.
 */
class LoraHelper
{
  public:
    LoraHelper();          //!< Default constructor
    virtual ~LoraHelper(); //!< Destructor

    /**
     * Install LoraNetDevices on a list of nodes.
     *
     * @param phyHelper The PHY helper to create PHY objects.
     * @param macHelper The MAC helper to create MAC objects.
     * @param c The set of nodes on which a lora device will be installed.
     * @return A device container which contains all the devices created by this method.
     */
    virtual NetDeviceContainer Install(const LoraPhyHelper& phyHelper,
                                       const LorawanMacHelper& macHelper,
                                       NodeContainer c) const;

    /**
     * Install LoraNetDevice on a single node.
     *
     * @param phyHelper The PHY helper to create PHY objects.
     * @param macHelper The MAC helper to create MAC objects.
     * @param node The node on which a lora device will be installed.
     * @return A device container which contains all the devices created by this method.
     */
    virtual NetDeviceContainer Install(const LoraPhyHelper& phyHelper,
                                       const LorawanMacHelper& macHelper,
                                       Ptr<Node> node) const;

    /**
     * Enable tracking of packets via trace sources.
     *
     * This method automatically connects to trace sources to computes relevant
     * metrics.
     */
    void EnablePacketTracking();

    /**
     * Periodically prints the simulation time to the standard output.
     *
     * @param interval The time period of the interval.
     */
    void EnableSimulationTimePrinting(Time interval);

    /**
     * Periodically prints the status of devices in the network to a file.
     *
     * For each input device print the current position, data rate and transmission power settings.
     *
     * @param endDevices The devices to track.
     * @param gateways The gateways in the network (this is only a placeholder parameter).
     * @param filename The output filename.
     * @param interval The time interval for printing.
     *
     * @todo Remove unused parameter gateways.
     */
    void EnablePeriodicDeviceStatusPrinting(NodeContainer endDevices,
                                            NodeContainer gateways,
                                            std::string filename,
                                            Time interval);

    /**
     * Periodically prints PHY-level performance at every gateway in the container.
     *
     * For each input gateway print counters for totPacketsSent, receivedPackets, interferedPackets,
     * noMoreGwPackets, underSensitivityPackets and lostBecauseTxPackets.
     *
     * @param gateways The gateways to track.
     * @param filename The output filename.
     * @param interval The time interval for printing.
     */
    void EnablePeriodicPhyPerformancePrinting(NodeContainer gateways,
                                              std::string filename,
                                              Time interval);

    /**
     * Print the PHY-level performance of every gateway in the container since the last
     * performance update.
     *
     * For each input gateway print counters for totPacketsSent, receivedPackets, interferedPackets,
     * noMoreGwPackets, underSensitivityPackets and lostBecauseTxPackets.
     *
     * @param gateways The gateways to track.
     * @param filename The output filename.
     */
    void DoPrintPhyPerformance(NodeContainer gateways, std::string filename);

    /**
     * Periodically print global performance as the total number of send and received
     * packets.
     *
     * @param filename The output filename.
     * @param interval The time interval for printing.
     */
    void EnablePeriodicGlobalPerformancePrinting(std::string filename, Time interval);

    /**
     * Print global performance as the total number of send and received packets since last
     * performance update.
     *
     * @param filename The output filename.
     */
    void DoPrintGlobalPerformance(std::string filename);

    /**
     * Get a reference to the Packet Tracker object.
     *
     * @return the reference to the Packet Tracker object.
     */
    LoraPacketTracker& GetPacketTracker();

    LoraPacketTracker* m_packetTracker = nullptr; //!< Pointer to the Packet Tracker object
    time_t m_oldtime; //!< Real time (i.e., physical) of the last simulation time print

    /**
     * Print a summary of the current status of input devices.
     *
     * For each input device print the current position, data rate and transmission power settings.
     *
     * @param endDevices The devices to track.
     * @param gateways The gateways in the network (this is only a placeholder parameter).
     * @param filename The output filename.
     *
     * @todo Remove unused parameter gateways.
     */
    void DoPrintDeviceStatus(NodeContainer endDevices,
                             NodeContainer gateways,
                             std::string filename);

  private:
    /**
     * Actually print the simulation time and re-schedule execution of this
     * function.
     *
     * @param interval The delay for next printing.
     */
    void DoPrintSimulationTime(Time interval);

    Time m_lastPhyPerformanceUpdate;    //!< Timestamp of the last PHY performance update
    Time m_lastGlobalPerformanceUpdate; //!< Timestamp of the last global performance update
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_HELPER_H */
