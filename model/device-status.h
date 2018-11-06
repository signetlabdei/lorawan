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

#ifndef DEVICE_STATUS_H
#define DEVICE_STATUS_H

#include "ns3/object.h"
#include "ns3/lora-net-device.h"
#include "ns3/lora-device-address.h"
#include "ns3/lora-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/end-device-lora-mac.h"

namespace ns3 {
namespace lorawan {

/**
 * This class represents the Network Server's knowledge about an End Device in
 * the LoRaWAN network it is administering.
 *
 * The Network Server contains a list of instances of this class, once for
 * each device in the network. This class holds the reply packet that the
 * network server will send to this device at the first available receive
 * window. Furthermore, this class is used to keep track of all gateways that
 * are able to receive the device's packets. On new packet arrivals at the
 * Network Server, the UpdateGatewayData method is called to update the
 * m_gateways map, that associates a Gateway's PointToPointNetDevice address
 * to the power it received this ED's last packet. This information is then
 * used in the GetSortedGatewayAddresses method to return a list of the
 * preferred gateways through which to reply to this device.
 */
class DeviceStatus
{
public:
  /**
   * Structure representing the reply that the network server will send this
   * device at the first opportunity.
   */
  struct Reply
  {
    bool hasReply = false;   // Whether this device already has a reply.
    Ptr<Packet> packet;   // The packet that will be sent as a reply.
    LoraMacHeader macHeader; // The MacHeader to attach to the reply packet.
    LoraFrameHeader frameHeader; // The FrameHeader to attach to the reply packet.
  };

  DeviceStatus ();
  virtual ~DeviceStatus ();

  DeviceStatus (Ptr<EndDeviceLoraMac> endDeviceMac);

  /**
   * Get the data rate this device is using
   *
   * \return An unsigned 8-bit integer containing the data rate.
   */
  uint8_t GetDataRate ();

  /**
   * Get the LoraDeviceAddress that the device represented by this DeviceStatus
   * is using.
   *
   * \return The address.
   */
  LoraDeviceAddress GetAddress ();

  /**
   * Set the LoraDeviceAddress that the device represented by this DeviceStatus
   * is using.
   *
   * \param address The device's address to set.
   */
  void SetAddress (LoraDeviceAddress address);

  /**
   * Update the DeviceStatus to take into account the power with which a
   * packet was received by the gateway with this P2P address.
   *
   * \param gwAddress The gateway's P2P interface address.
   * \param rcvPower The receive power, in dBm, with which the gateway received
   * the device's last packet.
   */
  void UpdateGatewayData (Address gwAddress, double rcvPower);

  /**
   * Return the address of the gateway that received this device's last packet
   * with the highest power.
   *
   * \return The best gateway's P2P link address.
   */
  Address GetBestGatewayAddress (void);

  /**
   * Return an iterator to the gateway addresses that received a packet by this
   * device, in order from best to worst (i.e., from highest receive power to
   * lowest receive power).
   *
   * \return A list of addresses.
   */
  std::list<Address> GetSortedGatewayAddresses (void);

  /**
   * Set the reply to send to this device.
   *
   * \param reply The reply structure to use for the next downlink transmission.
   */
  void SetReply (struct Reply reply);

  /**
   * Check whether this device already has a reply packet.
   *
   * \return True if there's already a reply for this device, false otherwise.
   */
  bool HasReply (void);

  /**
   * Return this device's next downlink packet.
   *
   * This method returns a full packet, to which headers are already added.
   *
   * \return The full packet for reply.
   */
  Ptr<Packet> GetReplyPacket (void);

  /**
   * Set the first window frequency of this device.
   */
  void SetFirstReceiveWindowFrequency (double frequency);

  /**
   * Get the first window frequency of this device.
   */
  double GetFirstReceiveWindowFrequency (void);

  /**
   * Return the second window frequency of this device.
   *
   * This value is _not_ memorized in this object, and instead it's queried
   * using the pointer to the device's MAC layer.
   */
  double GetSecondReceiveWindowFrequency (void);

  /**
   * Return the data rate this device expects on the first receive window.
   *
   * This value is memorized in this object, based on the dataRate used by the
   * uplink packet.
   */
  uint8_t GetFirstReceiveWindowDataRate (void);

  /**
   * Return the data rate this device expects on the second receive window.
   *
   * This value is _not_ memorized in this object, and instead it's queried
   * using the pointer to the device's MAC layer.
   */
  uint8_t GetSecondReceiveWindowDataRate (void);

private:
  Ptr<EndDeviceLoraMac> m_mac;   //!< Pointer to the device

  LoraDeviceAddress m_address;   //!< The address of this device

  std::map<Address, double> m_gateways;   //!< The gateways that received a packet from the device
  //! represented by this DeviceStatus.
  //! Address= address of the gateway,
  //! double value= power with which the packet has been received from that gateway

  struct Reply m_reply; //!< Structure containing the next reply meant for this
  //!device

  double m_firstReceiveWindowFrequency; //!< Frequency at which the device will
  //!open the first receive window

  // TODO Add missing information:
  // - Up/Down frame counters
};
}

}
#endif /* DEVICE_STATUS_H */
