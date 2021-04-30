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
 * Authors: Martina Capuzzo <capuzzom@dei.unipd.it>
 *          Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef END_DEVICE_STATUS_H
#define END_DEVICE_STATUS_H

#include "ns3/object.h"
#include "ns3/lora-net-device.h"
#include "ns3/lora-device-address.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/lora-frame-header.h"
#include "ns3/pointer.h"
#include "ns3/lora-frame-header.h"
#include <iostream>

namespace ns3 {
namespace lorawan {

/**
 * This class represents the Network Server's knowledge about an End Device in
 * the LoRaWAN network it is administering.
 *
 * The Network Server's NetworkStatus component contains a list of instances of
 * this class, one for each device in the network. Each instance contains all
 * the parameters and information of the end device and the packets received
 * from it. Furthermore, this class holds the reply packet that the network
 * server will send to this device at the first available receive window. Upon
 * new packet arrivals at the Network Server, the OnReceivedPacket method is
 * called to update the information regarding the last received packet and its
 * parameters.
 *
 */

/*
 * Diagram of the end-device-status data structure. One instance of this class
 * for each ED, that will be identified by its address.
 *
 * Public Access:
 *
 *  (ED address) --- Current device parameters:
 *                   - First Receive Window SF and DRRe
 *                   - First Receive Window frequency
 *                   - Second Window SF and DR
 *                   - Second Receive Window frequency
 *               --- Reply
 *                   - Need for reply (true/false)
 *                   - Updated reply
 *               --- Received Packets
 *                   - Received packets list (see below).
 *
 *
 * Private Access:
 *
 *  (Received packets list) - List of gateways that received the packet (see below)
 *                          - SF of the received packet
 *                          - Frequency of the received packet
 *                          - Bandwidth of the received packet
 *
 *  (Gateway list) - Time at which the packet was received
 *                 - Reception power
 */

class EndDeviceStatus : public Object
{

public:
  /********************/
  /* Reply management */
  /********************/

  /**
   * Structure representing the reply that the network server will send this
   * device at the first opportunity.
   */
  struct Reply
  {
    // The Mac Header to attach to the reply packet.
    LorawanMacHeader macHeader;

    // The Frame Header to attach to the reply packet.
    LoraFrameHeader frameHeader;

    // The data packet that will be sent as a reply.
    Ptr<Packet> payload;

    // Whether or not this device needs a reply
    bool needsReply = false;
  };

  /**
   * Whether the end device needs a reply.
   *
   * This is determined by looking at headers and payload of the Reply
   * structure: if they are empty, no reply should be needed.
   *
   * \return A boolean value signaling if the end device needs a reply.
   */
  bool NeedsReply (void);

  /**
   * Get the reply packet.
   *
   * \return A pointer to the packet reply (data + headers).
   */
  Ptr<Packet> GetCompleteReplyPacket (void);

  /**
   * Get the reply packet mac header.
   *
   * \return The packet reply mac header.
   */
  LorawanMacHeader GetReplyMacHeader (void);

  /**
   * Get the reply packet frame header.
   *
   * \return The packet reply frame header.
   */
  LoraFrameHeader GetReplyFrameHeader (void);

  /**
   * Get the data of the reply packet.
   *
   * \return A pointer to the packet reply.
   */
  Ptr<Packet> GetReplyPayload (void);

  /***********************************/
  /* Received packet list management */
  /***********************************/

  /**
   * Structure saving information regarding the packet reception in
   * each gateway.
   */
  struct PacketInfoPerGw
  {
    Address gwAddress;     //!< Address of the gateway that received the packet.
    Time receivedTime;     //!< Time at which the packet was received by this gateway.
    double rxPower;        //!< Reception power of the packet at this gateway.
  };

  // List of gateways, with relative information
  typedef std::map<Address, PacketInfoPerGw> GatewayList;

  /**
   * Structure saving information regarding all packet receptions.
   */
  struct ReceivedPacketInfo
  {
    // Members
    Ptr<Packet const> packet = 0;   //!< The received packet
    GatewayList gwList;      //!< List of gateways that received this packet.
    uint8_t sf;
    double frequency;
  };

  typedef std::list<std::pair<Ptr<Packet const>, ReceivedPacketInfo> >
    ReceivedPacketList;


  /*******************************************/
  /* Proper EndDeviceStatus class definition */
  /*******************************************/

  static TypeId GetTypeId (void);

  EndDeviceStatus ();
  EndDeviceStatus (LoraDeviceAddress endDeviceAddress,
                   Ptr<ClassAEndDeviceLorawanMac> endDeviceMac);
  virtual ~EndDeviceStatus ();

  /**
   * Get the spreading factor this device is using in the first receive window.
   *
   * \return An unsigned 8-bit integer containing the spreading factor.
   */
  uint8_t GetFirstReceiveWindowSpreadingFactor (void);

  /**
   * Get the first window frequency of this device.
   */
  double GetFirstReceiveWindowFrequency (void);

  /**
   * Get the offset of spreading factor this device is using in the second
   * receive window with respect to the first receive window.
   *
   * \return An unsigned 8-bit integer containing the spreading factor.
   */
  uint8_t GetSecondReceiveWindowOffset (void);

  /**
   * Return the second window frequency of this device.
   *
   */
  double GetSecondReceiveWindowFrequency (void);

  /**
   * Get the received packet list.
   *
   * \return The received packet list.
   */
  ReceivedPacketList GetReceivedPacketList (void);

  /**
   * Set the spreading factor this device is using in the first receive window.
   */
  void SetFirstReceiveWindowSpreadingFactor (uint8_t sf);

  /**
   * Set the first window frequency of this device.
   */
  void SetFirstReceiveWindowFrequency (double frequency);

  /**
   * Set the spreading factor this device is using in the first receive window.
   */
  void SetSecondReceiveWindowOffset (uint8_t offset);

  /**
   * Set the second window frequency of this device.
   */
  void SetSecondReceiveWindowFrequency  (double frequency);

  /**
   * Set the reply packet mac header.
   */
  void SetReplyMacHeader (LorawanMacHeader macHeader);

  /**
   * Set the reply packet frame header.
   */
  void SetReplyFrameHeader (LoraFrameHeader frameHeader);

  /**
   * Set the packet reply payload.
   */
  void SetReplyPayload (Ptr<Packet> replyPayload);

  Ptr<ClassAEndDeviceLorawanMac> GetMac (void);

  //////////////////////
  //  Other methods  //
  //////////////////////

  /**
   * Insert a received packet in the packet list.
   */
  void InsertReceivedPacket (Ptr<Packet const> receivedPacket,
                             const Address& gwAddress);

  /**
   * Return the last packet that was received from this device.
   */
  Ptr<Packet const> GetLastPacketReceivedFromDevice (void);

  /**
   * Return the information about the last packet that was received from the
   * device.
   */
  EndDeviceStatus::ReceivedPacketInfo GetLastReceivedPacketInfo (void);

  /**
   * Initialize reply.
   */
  void InitializeReply (void);

  /**
   * Add MAC command to the list.
   */
  void AddMACCommand (Ptr<MacCommand> macCommand);

  /**
   * Update Gateway data when more then one gateway receive the same packet.
   */
  void UpdateGatewayData (GatewayList gwList, Address gwAddress, double rcvPower);

  /**
   * Returns whether we already decided we will schedule a transmission to this ED
   */
  bool HasReceiveWindowOpportunityScheduled ();

  void SetReceiveWindowOpportunity (EventId event);

  void RemoveReceiveWindowOpportunity (void);

  /**
   * Return an ordered list of the best gateways.
   */
  std::map<double, Address> GetPowerGatewayMap (void);

  struct Reply m_reply; //<! Next reply intended for this device

  LoraDeviceAddress m_endDeviceAddress;   //<! The address of this device

  friend std::ostream& operator<< (std::ostream& os, const EndDeviceStatus& status);

private:
  // Receive window data
  uint8_t m_firstReceiveWindowSpreadingFactor = 0;
  double m_firstReceiveWindowFrequency = 0;
  uint8_t m_secondReceiveWindowOffset = 0;
  double m_secondReceiveWindowFrequency = 869.525;
  EventId m_receiveWindowEvent;

  ReceivedPacketList m_receivedPacketList;   //<! List of received packets

  // NOTE Using this attribute is 'cheating', since we are assuming perfect
  // synchronization between the info at the device and at the network server
  Ptr<ClassAEndDeviceLorawanMac> m_mac;   //!< Pointer to the MAC layer of this device
};
}

}
#endif /* DEVICE_STATUS_H */
