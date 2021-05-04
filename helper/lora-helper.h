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

#ifndef LORA_HELPER_H
#define LORA_HELPER_H

#include "ns3/lora-phy-helper.h"
#include "ns3/lorawan-mac-helper.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/net-device.h"
#include "ns3/lora-net-device.h"
#include "ns3/lora-packet-tracker.h"
#include "ns3/trace-helper.h"

#include <ctime>

namespace ns3 {
namespace lorawan {

/**
 * Helps to create LoraNetDevice objects
 *
 * This class can help create a large set of similar LoraNetDevice objects and
 * configure a large set of their attributes during creation.
 */
class LoraHelper: public PcapHelperForDevice
{
public:
  virtual ~LoraHelper ();

  LoraHelper ();

  /**
   * Install LoraNetDevices on a list of nodes
   *
   * \param phy the PHY helper to create PHY objects
   * \param mac the MAC helper to create MAC objects
   * \param c the set of nodes on which a lora device must be created
   * \returns a device container which contains all the devices created by this
   * method.
   */
  virtual NetDeviceContainer Install (const LoraPhyHelper &phyHelper,
                                      const LorawanMacHelper &macHelper,
                                      NodeContainer c) const;

  /**
   * Install LoraNetDevice on a single node
   *
   * \param phy the PHY helper to create PHY objects
   * \param mac the MAC helper to create MAC objects
   * \param node the node on which a lora device must be created
   * \returns a device container which contains all the devices created by this
   * method.
   */
  virtual NetDeviceContainer Install (const LoraPhyHelper &phyHelper,
                                      const LorawanMacHelper &macHelper,
                                      Ptr<Node> node) const;

  /**
   * Enable tracking of packets via trace sources.
   *
   * This method automatically connects to trace sources to computes relevant
   * metrics.
   */
  void EnablePacketTracking (void);

  /**
   * Periodically prints the simulation time to the standard output.
   */
  void EnableSimulationTimePrinting (Time interval);

  /**
   * Periodically prints the status of devices in the network to a file.
   */
  void EnablePeriodicDeviceStatusPrinting (NodeContainer endDevices,
                                           NodeContainer gateways,
                                           std::string filename,
                                           Time interval);

  /**
   * Periodically prints PHY-level performance at every gateway in the container.
   */
  void EnablePeriodicPhyPerformancePrinting (NodeContainer gateways,
                                             std::string filename,
                                             Time interval);

  void DoPrintPhyPerformance (NodeContainer gateways, std::string filename);

  /**
   * Periodically prints global performance.
   */
  void EnablePeriodicGlobalPerformancePrinting (std::string filename,
                                                Time interval);

  void DoPrintGlobalPerformance (std::string filename);

  LoraPacketTracker& GetPacketTracker (void);

  LoraPacketTracker* m_packetTracker = 0;

  time_t m_oldtime;

  /**
   * Print a summary of the status of all devices in the network.
   */
  void DoPrintDeviceStatus (NodeContainer endDevices, NodeContainer gateways,
                            std::string filename);

private:
  /**
   * Actually print the simulation time and re-schedule execution of this
   * function.
   */
  void DoPrintSimulationTime (Time interval);
  
  virtual void EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool promiscuous, bool explicitFilename);

  Time m_lastPhyPerformanceUpdate;
  Time m_lastGlobalPerformanceUpdate;
};

} //namespace ns3

}
#endif /* LORA_HELPER_H */
