
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
 * Author: Martina Capuzzo <capuzzom@dei.unipd.it>
 */

#ifndef END_DEVICE_STATUS_H
#define END_DEVICE_STATUS_H

#include "ns3/object.h"
#include "ns3/lora-net-device.h"
#include "ns3/lora-device-address.h"
#include "ns3/lora-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/pointer.h"
#include "ns3/lora-mac-header.h"
#include "ns3/lora-frame-header.h"

namespace ns3 {

/**
 * This class represents the Network Server's knowledge about an End Device in
 * the LoRaWAN network it is administering.
 *
 * The Network Server contains a list of instances of this class, one for
 * each device in the network. Each instance contains all the parameters and
 * information of the end device aMartina Capuzzo

Department of Information Engineering (DEI)
University of Padova
Italynd the packets received from it.
 * Furthermore, this class holds the reply packet that the
 * network server will send to this device at the first available receive
 * window. Upon new packet arrivals at the
 * Network Server, the OnReceivedPacket method is called to update the
 * information regarding the last received packet and its parameters.
 */

/* Diagram of the end-device-status data structure. One instance of this class
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



class EndDeviceStatus
{
public:


  /**
   * Structure representing the reply that the network server will send this
   * device at the first opportunity.
   */
  struct Reply
  {
    Ptr<Packet const> payload;   // The packet that will be sent as a reply.
    LoraMacHeader macHeader; // The MacHeader to attach to the reply packet.
    LoraFrameHeader frameHeader; // The FrameHeader to attach to the reply packet.
    std::list<Ptr<MacCommand>> macCommandList; // list of the MAC commands that will be applied.
  };

  //TODO write methods to write the following structures.

  /**
   * Structure saving information regarding the packet reception in
   * each gateway.
   */

  struct PacketInfoPerGw
  {
    Time receivedTime;   //!< Time at which the packet was received by this gateway.
    double rxPower;      //!< value of the reception power of the packet at this gateway.
  };

  typedef std::map<Address, PacketInfoPerGw> GatewayList;

  /**
   * Structure saving information regarding the packet reception.
   */

  struct ReceivedPacketInfo
  {
    GatewayList gwlist;    //!< Pointer to the list of gateways that
    //!  received this packet.
    int sf;                //!< Spreading factor that the packet used.
    // double bw;             //!< Bandwidth that the packet used.
    double frequency;      //!< Frequency that the packet used.
  };

  typedef std::map<Ptr<Packet const>, ReceivedPacketInfo> ReceivedPacketList;

  EndDeviceStatus();
  virtual ~EndDeviceStatus();


  //////////////////
  //   Getters
  /////////////////

  //TODO questo metodo qua o nel network status?
  /**
   * Get the LoraDeviceAddress that the device represented by this DeviceStatus
   * is using.
   *
   * \return The address.
   */
  // LoraDeviceAddress GetAddress (void);

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
   * Whether the end device needs a reply.
   *
   * \return A boolean value signaling if the end device needs a reply.
   */
  bool NeedsReply (void);

  /**
   * Get the reply packet.
  *
  * \return A pointer to the packet reply (data + headers).
  */
  Ptr<Packet> GetReply (void);

  /**
   * Get the reply packet mac header.
   *
   * \return The packet reply mac header.
   */
  LoraMacHeader GetReplyMacHeader (void);

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

  /**
    * Get the received packet list.
    *
    * \return The received packet list.
    */
  ReceivedPacketList GetReceivedPacketList (void);

  /**
   * get the list of gateways that received this packet.
   */
  GatewayList GetGatewayList (Ptr<Packet const> packet);



  /////////////////
  //   Setters   //
  /////////////////

  // TODO here?
  /**
   * Set the LoraDeviceAddress that the device represented by this DeviceStatus
   * is using.
   *
   * \param address The device's address to set.
   */
  // void SetAddress (LoraDeviceAddress address);

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
   * Set whether the end device needs a reply.
   */
  void SetNeedsReply (bool needReply);

  /**
   * Set the reply packet mac header.
   */
  void SetReplyMacHeader (LoraMacHeader macHeader);

  /**
   * Set the reply packet frame header.
   */
  void SetReplyFrameHeader (LoraFrameHeader frameHeader);

  /**
   * Set the packet reply payload.
   */
  void SetReplyPayload (Ptr<Packet const> replyPayload);

  /**
   * Set the payload size of the replay packet.
   */
  void SetPayloadSize (uint8_t payloadSize);
  

  //////////////////////
  //  Other methods  //
  //////////////////////

  /**
  * Insert a received packet in the packet list.
  */
  void InsertReceivedPacket (Ptr<Packet const> receivedPacket, ReceivedPacketInfo info,
                             const Address& gwAddress, double rcvPower);

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


private:

  // LoraDeviceAddress m_address;

  uint8_t m_firstReceiveWindowSpreadingFactor = 0; //!< Spreading Factor of the first
  //!receive window

  double m_firstReceiveWindowFrequency = 0; //!< Frequency at which the device will
  //!open the first receive window

  uint8_t m_secondReceiveWindowOffset = 0; //!< Spreading Factor of the second
  //!receive window

  double m_secondReceiveWindowFrequency = 868.625; //!< Frequency at which the device will
  //!open the second receive window

  bool m_needsReply = false;  //!< Whether this end device needs a reply

  bool m_hasReplyPayload = false;  //!< Whether this end device needs a reply

  uint8_t m_payloadSize = 0;

  Ptr<Packet const> m_lastReceivedPacket = 0; //The last packet that has been received
                                        // from this device.

  struct Reply m_reply; //!< Structure containing the next reply meant for this
  //!device

  ReceivedPacketList m_receivedPacketList; //!< Structure containing the next
  //!reply meant for this device

};
}

#endif /* DEVICE_STATUS_H */
