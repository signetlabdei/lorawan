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

#ifndef ONE_SHOT_SENDER_H
#define ONE_SHOT_SENDER_H

#include "lorawan-mac.h"

#include "ns3/application.h"
#include "ns3/attribute.h"
#include "ns3/nstime.h"

namespace ns3
{
namespace lorawan
{

class OneShotSender : public Application
{
  public:
    OneShotSender();
    OneShotSender(Time sendTime);
    ~OneShotSender() override;

    static TypeId GetTypeId();

    /**
     * Send a packet using the LoraNetDevice's Send method.
     */
    void SendPacket();

    /**
     * Set the time at which this app will send a packet.
     */
    void SetSendTime(Time sendTime);

    /**
     * Start the application by scheduling the first SendPacket event.
     */
    void StartApplication() override;

    /**
     * Stop the application.
     */
    void StopApplication() override;

  private:
    /**
     * The time at which to send the packet.
     */
    Time m_sendTime;

    /**
     * The sending event.
     */
    EventId m_sendEvent;

    /**
     * The MAC layer of this node.
     */
    Ptr<LorawanMac> m_mac;
};

} // namespace lorawan

} // namespace ns3
#endif /* ONE_SHOT_APPLICATION */
