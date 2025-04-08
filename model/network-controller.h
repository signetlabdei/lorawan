/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef NETWORK_CONTROLLER_H
#define NETWORK_CONTROLLER_H

#include "network-controller-components.h"
#include "network-status.h"

#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3
{
namespace lorawan
{

class NetworkStatus;
class NetworkControllerComponent;

/**
 * @ingroup lorawan
 *
 * This class collects a series of components that deal with various aspects
 * of managing the network, and queries them for action when a new packet is
 * received or other events occur in the network.
 */
class NetworkController : public Object
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    NetworkController();           //!< Default constructor
    ~NetworkController() override; //!< Destructor

    /**
     * Construct a new NetworkController object providing the NetworkStatus.
     *
     * @param networkStatus A pointer to the NetworkStatus object.
     */
    NetworkController(Ptr<NetworkStatus> networkStatus);

    /**
     * Add a new NetworkControllerComponent.
     *
     * @param component A pointer to the NetworkControllerComponent object.
     */
    void Install(Ptr<NetworkControllerComponent> component);

    /**
     * Method that is called by the NetworkServer application when a new packet is received.
     *
     * @param packet The newly received packet.
     */
    void OnNewPacket(Ptr<const Packet> packet);

    /**
     * Method that is called by the NetworkScheduler just before sending a reply
     * to a certain end device.
     *
     * @param endDeviceStatus A pointer to the EndDeviceStatus object.
     */
    void BeforeSendingReply(Ptr<EndDeviceStatus> endDeviceStatus);

  private:
    Ptr<NetworkStatus> m_status; //!< A pointer to the NetworkStatus object.
    std::list<Ptr<NetworkControllerComponent>>
        m_components; //!< List of NetworkControllerComponent objects.
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_CONTROLLER_H */
