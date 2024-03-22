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

#include "class-a-end-device-lorawan-mac.h"
#include "lora-device-address.h"
#include "lora-frame-header.h"
#include "lora-net-device.h"
#include "lorawan-mac-header.h"

#include "ns3/object.h"
#include "ns3/pointer.h"

#include <iostream>

namespace ns3
{
namespace lorawan
{

/**
 * \ingroup lorawan
 *
 * This class represents the network server's knowledge about an end device in
 * the LoRaWAN network it is administering.
 *
 * The network server's NetworkStatus component contains a list of instances of
 * this class, one for each device in the network. Each instance contains all
 * the parameters and information of the end device and the packets received
 * from it. Furthermore, this class holds the reply packet that the network
 * server will send to this device at the first available receive window. Upon
 * new packet arrivals at the network server, the OnReceivedPacket method is
 * called to update the information regarding the last received packet and its
 * parameters.
 *
 * Diagram of the end-device-status data structure. One instance of this class
 * for each end device, that will be identified by its address.
 *
 * Public Access:
 *
 *  (End device address) --- Current device parameters:
 *                           - First Receive Window SF and DRRe
 *                           - First Receive Window frequency
 *                           - Second Window SF and Data Rate (DR)
 *                           - Second Receive Window frequency
 *                       --- Reply
 *                           - Need for reply (true/false)
 *                           - Updated reply
 *                       --- Received Packets
 *                           - Received packets list (see below).
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
        LorawanMacHeader macHeader;  //!< The MAC Header to attach to the reply packet.
        LoraFrameHeader frameHeader; //!< The Frame Header to attach to the reply packet.
        Ptr<Packet> payload;         //!< The data packet that will be sent as a reply.
        bool needsReply = false;     //!< Whether or not this device needs a reply
    };

    /**
     * Whether the end device needs a reply.
     *
     * This is determined by looking at headers and payload of the Reply
     * structure: if they are empty, no reply should be needed.
     *
     * \return A boolean value signaling if the end device needs a reply.
     */
    bool NeedsReply() const;

    /**
     * Get the reply packet.
     *
     * \return A pointer to the packet reply (data + headers).
     */
    Ptr<Packet> GetCompleteReplyPacket();

    /**
     * Get the reply packet mac header.
     *
     * \return The packet reply mac header.
     */
    LorawanMacHeader GetReplyMacHeader() const;

    /**
     * Get the reply packet frame header.
     *
     * \return The packet reply frame header.
     */
    LoraFrameHeader GetReplyFrameHeader() const;

    /**
     * Get the data of the reply packet.
     *
     * \return A pointer to the packet reply.
     */
    Ptr<Packet> GetReplyPayload();

    /***********************************/
    /* Received packet list management */
    /***********************************/

    /**
     * Structure saving information regarding the packet reception in
     * each gateway.
     */
    struct PacketInfoPerGw
    {
        Address gwAddress; //!< Address of the gateway that received the packet.
        Time receivedTime; //!< Time at which the packet was received by this gateway.
        double rxPower;    //!< Reception power of the packet at this gateway.
    };

    /**
     * typedef of a list of gateways with relative reception information.
     */
    typedef std::map<Address, PacketInfoPerGw> GatewayList;

    /**
     * Structure saving information regarding all packet receptions.
     */
    struct ReceivedPacketInfo
    {
        // Members
        Ptr<const Packet> packet = nullptr; //!< The received packet
        GatewayList gwList;                 //!< List of gateways that received this packet
        uint8_t sf;                         //!< Spreading factor used to send this packet
        double frequency;                   //!< Carrier frequency [MHz] used to send this packet
    };

    /**
     * typedef of a list of packets paired to their reception info.
     */
    typedef std::list<std::pair<Ptr<const Packet>, ReceivedPacketInfo>> ReceivedPacketList;

    /*******************************************/
    /* Proper EndDeviceStatus class definition */
    /*******************************************/

    /**
     *  Register this type.
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    EndDeviceStatus();           //!< Default constructor
    ~EndDeviceStatus() override; //!< Destructor

    /**
     * Constructor with initialization parameters.
     *
     * \param endDeviceAddress Address of the end device.
     * \param endDeviceMac Pointer to the MAC layer of the end device.
     */
    EndDeviceStatus(LoraDeviceAddress endDeviceAddress,
                    Ptr<ClassAEndDeviceLorawanMac> endDeviceMac);

    /**
     * Get the spreading factor this device is using in the first receive window.
     *
     * \return An unsigned 8-bit integer containing the spreading factor.
     */
    uint8_t GetFirstReceiveWindowSpreadingFactor() const;

    /**
     * Get the first window frequency of this device.
     *
     * \return The frequency [MHz].
     */
    double GetFirstReceiveWindowFrequency() const;

    /**
     * Get the spreading factor this device is using in the second
     * receive window.
     *
     * \return An unsigned 8-bit integer containing the spreading factor.
     */
    uint8_t GetSecondReceiveWindowSpreadingFactor() const;

    /**
     * Return the second window frequency of this device.
     *
     * \return The frequency [MHz].
     */
    double GetSecondReceiveWindowFrequency() const;

    /**
     * Get the received packet list.
     *
     * \return The received packet list.
     */
    ReceivedPacketList GetReceivedPacketList() const;

    /**
     * Set the spreading factor this device is using in the first receive window.
     *
     * \param sf The spreading factor.
     */
    void SetFirstReceiveWindowSpreadingFactor(uint8_t sf);

    /**
     * Set the first window frequency of this device.
     *
     * \param frequency The frequency [MHz].
     */
    void SetFirstReceiveWindowFrequency(double frequency);

    /**
     * Set the spreading factor this device is using in the second receive window.
     *
     * \param sf The spreading factor.
     */
    void SetSecondReceiveWindowSpreadingFactor(uint8_t sf);

    /**
     * Set the second window frequency of this device.
     *
     * \param frequency The frequency [MHz].
     */
    void SetSecondReceiveWindowFrequency(double frequency);

    /**
     * Set the reply packet mac header.
     *
     * \param macHeader The mac header (MHDR).
     */
    void SetReplyMacHeader(LorawanMacHeader macHeader);

    /**
     * Set the reply packet frame header.
     *
     * \param frameHeader The frame header (FHDR + FPort).
     */
    void SetReplyFrameHeader(LoraFrameHeader frameHeader);

    /**
     * Set the packet reply payload.
     *
     * \param replyPayload Packet containing the FRMPayload.
     */
    void SetReplyPayload(Ptr<Packet> replyPayload);

    /**
     * Get the MAC layer of the end device.
     *
     * \return A pointer to the MAC layer.
     */
    Ptr<ClassAEndDeviceLorawanMac> GetMac();

    //////////////////////
    //  Other methods  //
    //////////////////////

    /**
     * Insert a received packet in the packet list.
     *
     * \param receivedPacket The packet received.
     * \param gwAddress The address of the receiver gateway.
     */
    void InsertReceivedPacket(Ptr<const Packet> receivedPacket, const Address& gwAddress);

    /**
     * Return the last packet that was received from this device.
     *
     * \return The last received packet.
     */
    Ptr<const Packet> GetLastPacketReceivedFromDevice();

    /**
     * Return the information about the last packet that was received from the
     * device.
     *
     * \return The information about the last received packet.
     */
    EndDeviceStatus::ReceivedPacketInfo GetLastReceivedPacketInfo();

    /**
     * Reset the next reply state.
     */
    void InitializeReply();

    /**
     * Add MAC command to the frame header of next reply.
     *
     * \param macCommand The MAC command.
     */
    void AddMACCommand(Ptr<MacCommand> macCommand);

    /**
     * Update gateway data when more then one gateway receive the same packet.
     *
     * \param gwList List of gateways with previous receptions.
     * \param gwAddress Gateway address of the current reception.
     * \param rcvPower Reception power.
     *
     * \todo not implemented.
     */
    void UpdateGatewayData(GatewayList gwList, Address gwAddress, double rcvPower);

    /**
     * Check if there is already a running reception window event scheduled for this end device.
     *
     * \return True if a reception window event is already scheduled, false otherwise.
     */
    bool HasReceiveWindowOpportunityScheduled();

    /**
     * Store next scheduled reception window event.
     *
     * \param event The event.
     */
    void SetReceiveWindowOpportunity(EventId event);

    /**
     * Cancel next scheduled reception window event.
     */
    void RemoveReceiveWindowOpportunity();

    /**
     * Get the gateways which received the last packet from the end device. Gateways are mapped
     * to their measured reception power of the last packet, in ascending order.
     *
     * \return The ordered map of reception power values and gateways.
     */
    std::map<double, Address> GetPowerGatewayMap();

    struct Reply m_reply;                 //!< Next reply intended for this device
    LoraDeviceAddress m_endDeviceAddress; //!< The address of this device

    /**
     * Stream insertion operator.
     *
     * \param os The stream.
     * \param status The status.
     * \return A reference to the stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const EndDeviceStatus& status);

  private:
    // Receive window data
    uint8_t m_firstReceiveWindowSpreadingFactor = 0;  //!< SF for RX1 window
    double m_firstReceiveWindowFrequency = 0;         //!< Frequency [MHz] for RX1 window
    uint8_t m_secondReceiveWindowSpreadingFactor = 0; //!< SF for RX2 window.
    double m_secondReceiveWindowFrequency = 869.525;  //!< Frequency [MHz] for RX2 window
    EventId m_receiveWindowEvent; //!< Event storing the next scheduled downlink transmission

    ReceivedPacketList m_receivedPacketList; //!< List of received packets

    /// \note Using this attribute is 'cheating', since we are assuming perfect
    /// synchronization between the info at the device and at the network server
    Ptr<ClassAEndDeviceLorawanMac> m_mac; //!< Pointer to the MAC layer of this device
};
} // namespace lorawan

} // namespace ns3
#endif /* DEVICE_STATUS_H */
