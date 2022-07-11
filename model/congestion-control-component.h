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

  // To track congestion status of the network
  using devices_t = std::vector<std::pair<uint32_t, double>>;

  struct oftraffic_t
  {
    double high = 0;
    double low = 0;
    double curr = 0;
    double max = 0;
  };

  struct gateway_t
  {
    devices_t devs;
    int received = 0;
    int sent = 0;
    oftraffic_t ot;
    void Reset (void);
  };

  using datarate_t = std::map<Address, gateway_t>;

  // To track useful metrics of devices
  class rssi_t
  {
    int num = 0;
    double avg = 0;

  public:
    void Add (double rxpow);
    double Avg (void);
  };

  struct devinfo_t
  {
    int fCnt = 0;
    uint8_t datarate = 0;

    uint8_t cluster = 0;

    // Just in case we need to reorganize
    double maxoftraf = 0;
    uint8_t dutycycle = 0;

    std::map<Address, rssi_t> rssimap;

    Address GetBestGw (void);
  };

public:
  static TypeId GetTypeId (void);

  //Constructor
  CongestionControlComponent ();
  //Destructor
  virtual ~CongestionControlComponent ();

  void OnReceivedPacket (Ptr<const Packet> packet, Ptr<EndDeviceStatus> status,
                         Ptr<NetworkStatus> networkStatus);

  void BeforeSendingReply (Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus);

  void OnFailedReply (Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus);

  static double CapacityForPDRModel (double pdr);

private:
  void InitializeData (Ptr<NetworkStatus> status);

  std::string PrintCongestion (void);

  bool ProduceConfigScheme (gateway_t &group);

  // To track network congestion
  std::vector<datarate_t> m_congestMetrics;

  // To track current status of devices
  /*LoraDeviceAddress::Get() is hashable*/
  std::unordered_map<uint32_t, devinfo_t> m_devTracking;

  // To track ongoing duty-cycle configuration
  /*LoraDeviceAddress::Get() is hashable*/
  std::unordered_map<uint32_t, uint8_t> m_configToDoList;

  // Duration of the period in which we sample PDR
  Time m_samplingDuration;
  Time m_samplingStart;

  // Start congestion control procedure
  Time m_start;

  // PDR target
  double m_targetPDR;

  // Acceptable distance from target PDR value
  double m_epsilon;

  // Minimum step between offered traffic values in a SF to declare value stagnation
  double m_tolerance;

  // Constants
  static const int N_SF;

  static const int N_CH;

  // Failsafe for disabled devices
  std::vector<Ptr<EndDeviceStatus>> m_disabled;
};
} // namespace lorawan
} // namespace ns3

#endif
