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
 * Authors: Davide Magrin <magrinda@dei.unipd.it>
 *          Martina Capuzzo <capuzzom@dei.unipd.it>
 */

#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include "ns3/object.h"
#include "ns3/application.h"
#include "ns3/net-device.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/packet.h"
#include "ns3/lora-device-address.h"
#include "ns3/device-status.h"
#include "ns3/gateway-status.h"
#include "ns3/network-status.h"
#include "ns3/node-container.h"
#include "ns3/network-server-scheduler.h"
#include "ns3/network-server-controller.h"
#include "ns3/log.h"
#include "ns3/end-device-lora-mac.h"

namespace ns3 {

/**
 * The NetworkServer is an application standing on top of a node equipped with
 * links that connect it with the gateways.
 *
 * This version of the NetworkServer attempts to closely mimic an actual Network
 * Server, by providing as much functionality as possible.
 */
class NetworkServer : public Application
{
public:
  static TypeId GetTypeId (void);

  NetworkServer ();
  virtual ~NetworkServer ();

  /**
   * Start the NS application
   */
  void StartApplication (void);

  /**
   * Stop the NS application
   */
  void StopApplication (void);

  /**
   * Inform the NetworkServer that these nodes are connected to the network.
   * This method will create a DeviceStatus object for each new node, and add it
   * to the list.
   */
  void AddNodes (NodeContainer nodes);

  /**
   * Inform the NetworkServer that this node is connected to the network.
   * This method will create a DeviceStatus object for the new node (if it
   * doesn't already exist).
   */
  void AddNode (Ptr<Node> node);

  /**
   * Add this gateway to the list of gateways connected to this NS.
   * Each GW is identified by its Address in the NS-GWs network.
   */
  void AddGateway (Ptr<Node> gateway, Ptr<NetDevice> netDevice);

  /**
   * Receive a packet from a gateway.
   * \param packet the received packet
   */
  bool Receive (Ptr<NetDevice> device, Ptr<const Packet> packet,
                uint16_t protocol, const Address& address);


  /**
   * Parse packet frame header commands
   */
  void ParseCommands (LoraFrameHeader frameHeader);

/**
   * Perform the actions that need to be taken when receiving a LinkCheckAns command.
   *
   * \param margin The margin value of the command.
   * \param gwCnt The gateway count value of the command.
   */
  void OnLinkCheckAns (uint8_t margin, uint8_t gwCnt);

  /**
   * Perform the actions that need to be taken when receiving a LinkAdrReq command.
   *
   * \param dataRate The data rate value of the command.
   * \param txPower The transmission power value of the command.
   * \param enabledChannels A list of the enabled channels.
   * \param repetitions The number of repetitions prescribed by the command.
   */
  void OnLinkAdrReq (uint8_t dataRate, uint8_t txPower,
                     std::list<int> enabledChannels, int repetitions);

  /**
   * Perform the actions that need to be taken when receiving a DutyCycleReq command.
   *
   * \param dutyCycle The aggregate duty cycle prescribed by the command, in
   * fraction form.
   */
  void OnDutyCycleReq (double dutyCycle);

  /**
   * Perform the actions that need to be taken when receiving a RxParamSetupReq command.
   *
   * \param rx1DrOffset The offset to set.
   * \param rx2DataRate The data rate to use for the second receive window.
   * \param frequency The frequency to use for the second receive window.
   */
  void OnRxParamSetupReq (uint8_t rx1DrOffset, uint8_t rx2DataRate, double frequency);

  /**
   * Perform the actions that need to be taken when receiving a DevStatusReq command.
   */
  void OnDevStatusReq (void);

  /**
   * Perform the actions that need to be taken when receiving a NewChannelReq command.
   */
  void OnNewChannelReq (uint8_t chIndex, double frequency, uint8_t minDataRate,
                        uint8_t maxDataRate);







    protected:
  Ptr<NetworkServerScheduler> m_scheduler;
  Ptr<NetworkServerController> m_controller;
  Ptr<NetworkStatus> m_networkStatus;
};

} /* namespace ns3 */

#endif /* NETWORK_SERVER_H */
