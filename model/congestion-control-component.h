/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Orange SA
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
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#ifndef CONGESTION_CONTROL_COMPONENT_H
#define CONGESTION_CONTROL_COMPONENT_H

#include "ns3/network-controller-components.h"
#include "ns3/lora-device-address.h"

#include <unordered_map>
#include <limits>

namespace ns3 {
namespace lorawan {

/** 
 * Lightweight congestion control technique: according to PDR
 * measurements, duty-cycle is reconfigured to maximize traffic
 * while maintaining a certain quality level.
 */

class CongestionControlComponent : public NetworkControllerComponent
{
  // Cluster PDR targets
  using targets_t = std::vector<double>;

  /** 
   * ###################################
   * # Track useful metrics of devices #
   * ################################### 
   **/
  struct devinfo_t
  {
    // Static information
    uint8_t datarate = 0;
    uint8_t cluster = 0;
    Address bestGw = Address ();
    double maxoftraf = 0; // Useful in case we need to reorganize
    Time toa = Seconds (0); // Time on Air, useful to track disconnections

    // Changing with time
    int fCnt = 0;
    uint8_t dutycycle = 0;
    bool active = false;
  };
  using devinfomap_t = std::unordered_map<uint32_t, devinfo_t>; // 1 instance in the class

  // Timestamp map to track device disconnection (3 periods)
  using lastframemap_t = std::unordered_map<uint32_t, Time>;

  /** 
   * ##########################################
   * # Track congestion status of the network #
   * ########################################## 
   **/
  using devices_t = std::vector<std::pair<uint32_t, double>>;
  struct offtraff_t
  {
    double high = 0; // Intialized to max ot tot at beginning
    double low = 0;
    double currbest = 0; // Initialized on first iteration
    bool started = false;
    bool changed = false;
  };
  struct dataratestatus_t
  {
    devices_t devs; // Devices in this group
    offtraff_t ot; // Structure to track offered traffic convergence
    int received = 0;
    int sent = 0;
    void Reset (void); // Reset sent and received to 0
  };
  using clusterstatus_t = std::vector<dataratestatus_t>;
  using gatewaystatus_t = std::vector<clusterstatus_t>;
  using networkstatus_t = std::map<Address, gatewaystatus_t>; // 1 instance in the class

  /** 
   * ##########################################################
   * # Track configurations yet to be done + disabled devices #
   * ########################################################## 
   **/
  /*LoraDeviceAddress::Get() is hashable*/
  using configs_t = std::unordered_map<uint32_t, uint8_t>; // In each gateway/cluster
  using configsmap_t = std::map<Address, std::vector<configs_t>>; // 1 instance in the class
  // Track disabled devices
  using disabled_t = std::map<uint32_t, Ptr<EndDeviceStatus>>; // In each gateway/cluster
  using disabledmap_t = std::map<Address, std::vector<disabled_t>>; // 1 instance in the class

  /** 
   * #########################
   * # Track sampling phases #
   * ######################### 
   **/
  using samplingstatus_t = std::map<Address, std::vector<Time>>;

public:
  static TypeId GetTypeId (void);

  //Constructor
  CongestionControlComponent ();
  //Destructor
  virtual ~CongestionControlComponent ();

  void SetTargets (targets_t targets);

  void OnReceivedPacket (Ptr<const Packet> packet, Ptr<EndDeviceStatus> status,
                         Ptr<NetworkStatus> networkStatus);

  void BeforeSendingReply (Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus);

  void OnFailedReply (Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus);

  static double CapacityForPDRModel (double pdr);

private:
  void StartSampling (Address bestGw, uint8_t cluster);

  void StartReconfig (Address bestGw, uint8_t cluster);

  bool ProduceConfigScheme (dataratestatus_t &group, double target);

  bool BisectionLogic (offtraff_t &ot, double pdr, double target);

  void TrimConfigs (const devices_t &devs, configs_t &configs, disabled_t &disabled);

  void InitializeData (Ptr<NetworkStatus> status);

  void AddNewDevice (uint32_t devaddr);

  void RemoveDisconnected (dataratestatus_t &group);

  std::string PrintCongestion (Address bestGw, uint8_t cluster);

  void LoadConfigFromFile (Ptr<NetworkStatus> status);

  void SaveConfigToFile (void);

  void FastForwardConfig (Ptr<NetworkStatus> status, configs_t &configs);

  // To track network congestion
  networkstatus_t m_congestionStatus;
  // To track current status of devices
  devinfomap_t m_devStatus;
  // To track last frame reception in case of disconnections
  lastframemap_t m_lastFrame;

  // To track ongoing duty-cycle configuration
  /*LoraDeviceAddress::Get() is hashable*/
  configsmap_t m_configToDoList;
  // Failsafe for disabled devices
  disabledmap_t m_disabled;

  // Start congestion control procedure
  Time m_start;
  // Duration of the period in which we sample PDR
  Time m_samplingDuration;
  // Track sampling phases
  samplingstatus_t m_samplingStart;

  // PDR targets
  targets_t m_targets;

  // Acceptable distance from target PDR value
  double m_epsilon;
  // Minimum step between offered traffic values in a SF to declare value stagnation
  double m_tolerance;

  // Number fo channels per cluster
  // (not general right now, used with capacity model to jump-start)
  int N_CH;
  // Multiplicative constant for the capacity model
  int m_beta;

  // Constants
  static const int N_SF;

  // File path to load existing offered traffic configuration
  std::string m_inputFile;
  // File path to save updated offered traffic configurations
  std::string m_outputFile;
  // Whether to fast track convergence (skipping having to wait for uplink)
  bool m_fast;
};
} // namespace lorawan
} // namespace ns3

#endif
