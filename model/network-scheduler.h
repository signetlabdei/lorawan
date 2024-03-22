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

#ifndef NETWORK_SCHEDULER_H
#define NETWORK_SCHEDULER_H

#include "lora-device-address.h"
#include "lora-frame-header.h"
#include "lorawan-mac-header.h"
#include "network-controller.h"
#include "network-status.h"

#include "ns3/core-module.h"
#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3
{
namespace lorawan
{

class NetworkStatus;     // Forward declaration
class NetworkController; // Forward declaration

/**
 * \ingroup lorawan
 *
 * Network server component in charge of scheduling downling packets onto devices' reception windows
 */
class NetworkScheduler : public Object
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    /**
     * \brief Construct a new NetworkScheduler object
     *
     * \todo We should probably remove this or add getters and setters
     */
    NetworkScheduler();
    /**
     * \brief Construct a new NetworkScheduler providing the NetworkStatus and the NetworkController
     * objects.
     *
     * \param status A Ptr to the NetworkStatus object.
     * \param controller A Ptr to the NetworkController object.
     */
    NetworkScheduler(Ptr<NetworkStatus> status, Ptr<NetworkController> controller);
    ~NetworkScheduler() override;

    /**
     * \brief Method called by NetworkServer to inform the Scheduler of a newly arrived uplink
     * packet.
     *
     * This function schedules the OnReceiveWindowOpportunity events 1 and 2 seconds later.
     *
     * \param packet A Ptr to the new Packet instance
     */
    void OnReceivedPacket(Ptr<const Packet> packet);

    /**
     * \brief Method that is scheduled after packet arrival in order to take action on
     * sender's receive windows openings.
     *
     * \param deviceAddress The Address of the end device
     * \param window The reception window number (1 or 2)
     */
    void OnReceiveWindowOpportunity(LoraDeviceAddress deviceAddress, int window);

  private:
    TracedCallback<Ptr<const Packet>>
        m_receiveWindowOpened;   //!< Trace callback source for reception windows openings. \todo
                                 //!< never called
    Ptr<NetworkStatus> m_status; //!< A Ptr to the NetworkStatus object.
    Ptr<NetworkController> m_controller; //!< A Ptr to the NetworkController object.
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_SCHEDULER_H */
