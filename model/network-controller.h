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
 * \ingroup lorawan
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
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    NetworkController();
    /**
     * \brief Construct a new NetworkController object providing the NetworkStatus.
     *
     * \param networkStatus A Ptr to the NetworkStatus object.
     */
    NetworkController(Ptr<NetworkStatus> networkStatus);
    ~NetworkController() override; //!< Destructor

    /**
     * \brief Add a new NetworkControllerComponent.
     *
     * \param component A Ptr to the NetworkControllerComponent object.
     */
    void Install(Ptr<NetworkControllerComponent> component);

    /**
     * \brief Method that is called by the NetworkServer when a new packet is received.
     *
     * \param packet The newly received packet.
     */
    void OnNewPacket(Ptr<const Packet> packet);

    /**
     * \brief Method that is called by the NetworkScheduler just before sending a reply
     * to a certain End Device.
     *
     * \param endDeviceStatus A Ptr to the EndDeviceStatus object.
     */
    void BeforeSendingReply(Ptr<EndDeviceStatus> endDeviceStatus);

  private:
    Ptr<NetworkStatus> m_status; //!< A Ptr to the NetworkStatus object.
    std::list<Ptr<NetworkControllerComponent>>
        m_components; //!< List of NetworkControllerComponent objects.
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_CONTROLLER_H */
