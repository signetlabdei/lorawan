/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
 * @ingroup lorawan
 *
 * Network server component in charge of scheduling downling packets onto devices' reception windows
 *
 * @todo We should probably add getters and setters or remove default constructor
 */
class NetworkScheduler : public Object
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    NetworkScheduler();           //!< Default constructor
    ~NetworkScheduler() override; //!< Destructor

    /**
     * Construct a new NetworkScheduler providing the NetworkStatus and the NetworkController
     * objects.
     *
     * @param status A pointer to the NetworkStatus object.
     * @param controller A pointer to the NetworkController object.
     */
    NetworkScheduler(Ptr<NetworkStatus> status, Ptr<NetworkController> controller);

    /**
     * Method called by NetworkServer application to inform the Scheduler of a newly arrived uplink
     * packet.
     *
     * This function schedules the OnReceiveWindowOpportunity events 1 and 2 seconds later.
     *
     * @param packet A pointer to the new Packet instance.
     */
    void OnReceivedPacket(Ptr<const Packet> packet);

    /**
     * Method that is scheduled after packet arrival in order to take action on
     * sender's receive windows openings.
     *
     * @param deviceAddress The Address of the end device.
     * @param window The reception window number (1 or 2).
     */
    void OnReceiveWindowOpportunity(LoraDeviceAddress deviceAddress, int window);

  private:
    TracedCallback<Ptr<const Packet>>
        m_receiveWindowOpened;           //!< Trace callback source for reception windows openings.
                                         //!< \todo Never called. Place calls in the right places.
    Ptr<NetworkStatus> m_status;         //!< A pointer to the NetworkStatus object.
    Ptr<NetworkController> m_controller; //!< A pointer to the NetworkController object.
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_SCHEDULER_H */
