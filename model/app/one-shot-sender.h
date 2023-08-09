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

#ifndef ONE_SHOT_SENDER_H
#define ONE_SHOT_SENDER_H

#include "ns3/lora-application.h"

namespace ns3
{
namespace lorawan
{

class OneShotSender : public LoraApplication
{
  public:
    OneShotSender();
    OneShotSender(Time sendTime);
    ~OneShotSender() override;

    static TypeId GetTypeId();

    /**
     * Send a packet using the LoraNetDevice's Send method.
     */
    void SendPacket() override;

    /**
     * Set the time at which this app will send a packet.
     */
    void SetSendTime(Time sendTime);

    /**
     * Start the application by scheduling the first SendPacket event.
     */
    void StartApplication() override;
};

} // namespace lorawan

} // namespace ns3
#endif /* ONE_SHOT_APPLICATION */
