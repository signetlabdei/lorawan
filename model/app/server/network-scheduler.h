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
 *
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef NETWORK_SCHEDULER_H
#define NETWORK_SCHEDULER_H

#include "ns3/core-module.h"
#include "ns3/lora-device-address.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/network-controller.h"
#include "ns3/network-status.h"
#include "ns3/object.h"
#include "ns3/packet.h"

namespace ns3
{
namespace lorawan
{

class NetworkStatus;     // Forward declaration
class NetworkController; // Forward declaration

class NetworkScheduler : public Object
{
  public:
    static TypeId GetTypeId();

    NetworkScheduler();
    NetworkScheduler(Ptr<NetworkStatus> status, Ptr<NetworkController> controller);
    ~NetworkScheduler() override;

    /**
     * Method called by NetworkServer to inform the Scheduler of a newly arrived
     * uplink packet. This function schedules the OnReceiveWindowOpportunity
     * events 1 and 2 seconds later.
     */
    void OnReceivedPacket(Ptr<const Packet> packet);

    /**
     * Method that is scheduled after packet arrivals in order to act on
     * receive windows 1 and 2 seconds later receptions.
     */
    void OnReceiveWindowOpportunity(LoraDeviceAddress deviceAddress, int window);

  protected:
    void DoDispose() override;

  private:
    TracedCallback<Ptr<const Packet>> m_receiveWindowOpened;
    Ptr<NetworkStatus> m_status;
    Ptr<NetworkController> m_controller;
};

} // namespace lorawan

} // namespace ns3
#endif /* NETWORK_SCHEDULER_H */
