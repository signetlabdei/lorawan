/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
 * @ingroup lorawan
 *
 * Generic class describing a component of the NetworkController.
 *
 * This is the class that is meant to be extended by all NetworkController
 * components, and provides a common interface for the NetworkController to
 * query available components and prompt them to act on new packet arrivals.
 */
class NetworkControllerComponent : public Object
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    NetworkControllerComponent();           //!< Default constructor
    ~NetworkControllerComponent() override; //!< Destructor

    // Virtual methods whose implementation is left to child classes
    /**
     * Function called as a new uplink packet is received by the NetworkServer application.
     *
     * @param packet The newly received packet.
     * @param status A pointer to the status of the end device that sent the packet.
     * @param networkStatus A pointer to the NetworkStatus object.
     */
    virtual void OnReceivedPacket(Ptr<const Packet> packet,
                                  Ptr<EndDeviceStatus> status,
                                  Ptr<NetworkStatus> networkStatus) = 0;
    /**
     * Function called as a downlink reply is about to leave the NetworkServer application.
     *
     * @param status A pointer to the status of the end device which we are sending the reply to.
     * @param networkStatus A pointer to the NetworkStatus object.
     */
    virtual void BeforeSendingReply(Ptr<EndDeviceStatus> status,
                                    Ptr<NetworkStatus> networkStatus) = 0;
    /**
     * Method that is called when a packet cannot be sent in the downlink.
     *
     * @param status The EndDeviceStatus of the device to which it was impossible to send a reply.
     * @param networkStatus A pointer to the NetworkStatus object.
     */
    virtual void OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) = 0;
};

/**
 * @ingroup lorawan
 *
 * Network controller component for acknowledgments management.
 */
class ConfirmedMessagesComponent : public NetworkControllerComponent
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    ConfirmedMessagesComponent();           //!< Default constructor
    ~ConfirmedMessagesComponent() override; //!< Destructor

    /**
     * This method checks whether the received packet requires an acknowledgment
     * and sets up the appropriate reply in case it does.
     *
     * @param packet The newly received packet.
     * @param status A pointer to the EndDeviceStatus object of the sender.
     * @param networkStatus A pointer to the NetworkStatus object.
     */
    void OnReceivedPacket(Ptr<const Packet> packet,
                          Ptr<EndDeviceStatus> status,
                          Ptr<NetworkStatus> networkStatus) override;

    void BeforeSendingReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;

    void OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;
};

/**
 * @ingroup lorawan
 *
 * Network controller component for LinkCheck commands management.
 */
class LinkCheckComponent : public NetworkControllerComponent
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    LinkCheckComponent();           //!< Default constructor
    ~LinkCheckComponent() override; //!< Destructor

    /**
     * This method checks whether the received packet requires an acknowledgment
     * and sets up the appropriate reply in case it does.
     *
     * @param packet The newly received packet.
     * @param status A pointer to the EndDeviceStatus object of the sender.
     * @param networkStatus A pointer to the NetworkStatus object.
     */
    void OnReceivedPacket(Ptr<const Packet> packet,
                          Ptr<EndDeviceStatus> status,
                          Ptr<NetworkStatus> networkStatus) override;

    void BeforeSendingReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;

    void OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus) override;

  private:
};
} // namespace lorawan

} // namespace ns3
#endif
