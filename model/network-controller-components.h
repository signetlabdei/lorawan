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

#ifndef NETWORK_CONTROLLER_COMPONENTS_H
#define NETWORK_CONTROLLER_COMPONENTS_H

#include "network-status.h"

#include "ns3/log.h"
#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3
{
namespace lorawan
{

class NetworkStatus;

////////////////
// Base class //
////////////////

/**
 * Generic class describing a component of the NetworkController.
 *
 * This is the class that is meant to be extended by all NetworkController
 * components, and provides a common interface for the NetworkController to
 * query available components and prompt them to act on new packet arrivals.
 */
class NetworkControllerComponent : public Object
{
  public:
    static TypeId GetTypeId();

    // Constructor and destructor
    NetworkControllerComponent();
    ~NetworkControllerComponent() override;

    // Virtual methods whose implementation is left to child classes
    /**
     * Method that is called when a new packet is received by the NetworkServer.
     *
     * \param packet The newly received packet
     * \param networkStatus A pointer to the NetworkStatus object
     */
    virtual void OnReceivedPacket(Ptr<const Packet> packet,
                                  Ptr<EndDeviceStatus> status,
                                  Ptr<NetworkStatus> networkStatus) = 0;

    virtual void BeforeSendingReply(Ptr<EndDeviceStatus> status,
                                    Ptr<NetworkStatus> networkStatus) = 0;

    /**
     * Method that is called when a packet cannot be sent in the downlink.
     *
     * \param status The EndDeviceStatus of the device to which it was
     *               impossible to send a reply.
     * \param networkStatus A pointer to the NetworkStatus object
     */
    virtual void OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) = 0;
};

///////////////////////////////
// Acknowledgment management //
///////////////////////////////

class ConfirmedMessagesComponent : public NetworkControllerComponent
{
  public:
    static TypeId GetTypeId();

    // Constructor and destructor
    ConfirmedMessagesComponent();
    ~ConfirmedMessagesComponent() override;

    /**
     * This method checks whether the received packet requires an acknowledgment
     * and sets up the appropriate reply in case it does.
     *
     * \param packet The newly received packet
     * \param networkStatus A pointer to the NetworkStatus object
     */
    void OnReceivedPacket(Ptr<const Packet> packet,
                          Ptr<EndDeviceStatus> status,
                          Ptr<NetworkStatus> networkStatus) override;

    void BeforeSendingReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;

    void OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;
};

///////////////////////////////////
// LinkCheck commands management //
///////////////////////////////////

class LinkCheckComponent : public NetworkControllerComponent
{
  public:
    static TypeId GetTypeId();

    // Constructor and destructor
    LinkCheckComponent();
    ~LinkCheckComponent() override;

    /**
     * This method checks whether the received packet requires an acknowledgment
     * and sets up the appropriate reply in case it does.
     *
     * \param packet The newly received packet
     * \param networkStatus A pointer to the NetworkStatus object
     */
    void OnReceivedPacket(Ptr<const Packet> packet,
                          Ptr<EndDeviceStatus> status,
                          Ptr<NetworkStatus> networkStatus) override;

    void BeforeSendingReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;

    void OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;

  private:
    void UpdateLinkCheckAns(Ptr<const Packet> packet, Ptr<EndDeviceStatus> status);
};
} // namespace lorawan

} // namespace ns3
#endif
