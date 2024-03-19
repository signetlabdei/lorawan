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
 * \ingroup lorawan
 *
 * \brief Helps to create LoraNetDevice objects
 *
 * This class can help create a large set of similar LoraNetDevice objects and
 * configure a large set of their attributes during creation.
 */
class LoraHelper
{
  public:
    virtual ~LoraHelper(); //!< Destructor

    LoraHelper();

    /**
     * \brief Install LoraNetDevices on a list of nodes
     *
     * \param phyHelper the PHY helper to create PHY objects
     * \param macHelper the MAC helper to create MAC objects
     * \param c the set of nodes on which a lora device must be created
     * \returns a device container which contains all the devices created by this
     * method.
     */
    virtual NetDeviceContainer Install(const LoraPhyHelper& phyHelper,
                                       const LorawanMacHelper& macHelper,
                                       NodeContainer c) const;

    /**
     * \brief Install LoraNetDevice on a single node
     *
     * \param phyHelper the PHY helper to create PHY objects
     * \param macHelper the MAC helper to create MAC objects
     * \param node the node on which a lora device must be created
     * \returns a device container which contains all the devices created by this
     * method.
     */
    virtual NetDeviceContainer Install(const LoraPhyHelper& phyHelper,
                                       const LorawanMacHelper& macHelper,
                                       Ptr<Node> node) const;

    /**
     * \brief Enable tracking of packets via trace sources.
     *
     * This method automatically connects to trace sources to computes relevant
     * metrics.
     */
    void EnablePacketTracking();

    /**
     * \brief Periodically prints the simulation time to the standard output.
     *
     * \param interval the time period of the interval
     */
    void EnableSimulationTimePrinting(Time interval);

    /**
     * \brief Periodically prints the status of devices in the network to a file.
     *
     * For each input device print the current position, data rate and transmission power settings
     *
     * \param endDevices the devices to track
     * \param gateways \todo unused parameter
     * \param filename the output filename
     * \param interval the time interval for printing
     */
    void EnablePeriodicDeviceStatusPrinting(NodeContainer endDevices,
                                            NodeContainer gateways,
                                            std::string filename,
                                            Time interval);

    /**
     * \brief Periodically prints PHY-level performance at every gateway in the container.
     *
     * For each input gateway print counters for totPacketsSent, receivedPackets, interferedPackets,
     * noMoreGwPackets, underSensitivityPackets and lostBecauseTxPackets
     *
     * \param gateways the gateways to track
     * \param filename the output filename
     * \param interval the time interval for printing
     */
    void EnablePeriodicPhyPerformancePrinting(NodeContainer gateways,
                                              std::string filename,
                                              Time interval);

    /**
     * \brief Print PHY-level performance at every gateway in the container since last
     * performance update
     *
     * For each input gateway print counters for totPacketsSent, receivedPackets, interferedPackets,
     * noMoreGwPackets, underSensitivityPackets and lostBecauseTxPackets
     *
     * \param gateways the gateways to track
     * \param filename the output filename
     */
    void DoPrintPhyPerformance(NodeContainer gateways, std::string filename);

    /**
     * \brief Periodically print global performance as the total number of send and received
     * packets.
     *
     * \param filename the output filename
     * \param interval the time interval for printing
     */
    void EnablePeriodicGlobalPerformancePrinting(std::string filename, Time interval);

    /**
     * \brief Print global performance as the total number of send and received packets since last
     * performance update
     *
     * \param filename the output filename
     */
    void DoPrintGlobalPerformance(std::string filename);

    /**
     * \brief Get a reference to the Packet Tracker object
     *
     * \return the reference to the Packet Tracker object
     */
    LoraPacketTracker& GetPacketTracker();

    LoraPacketTracker* m_packetTracker = nullptr; //!< Pointer to the Packet Tracker object
    time_t m_oldtime;                             //!< Real time of the last simulation time print

    /**
     * \brief Print a summary of the current status of input devices.
     *
     * For each input device print the current position, data rate and transmission power settings
     *
     * \param endDevices the devices to track
     * \param gateways \todo unused parameter
     * \param filename the output filename
     */
    void DoPrintDeviceStatus(NodeContainer endDevices,
                             NodeContainer gateways,
                             std::string filename);

  private:
    /**
     * \brief Actually print the simulation time and re-schedule execution of this
     * function.
     *
     * \param interval the delay for next printing
     */
    void DoPrintSimulationTime(Time interval);

    Time m_lastPhyPerformanceUpdate;    //!< timestamp of the last PHY performance update
    Time m_lastGlobalPerformanceUpdate; //!< timestamp of the last global performance update
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_HELPER_H */
