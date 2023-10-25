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

#ifndef GATEWAY_LORAWAN_MAC_H
#define GATEWAY_LORAWAN_MAC_H

#include "lora-tag.h"
#include "lorawan-mac.h"

namespace ns3
{
namespace lorawan
{

class GatewayLorawanMac : public LorawanMac
{
  public:
    static TypeId GetTypeId();

    GatewayLorawanMac();
    ~GatewayLorawanMac() override;

    // Implementation of the LorawanMac interface
    void Send(Ptr<Packet> packet) override;

    // Implementation of the LorawanMac interface
    bool IsTransmitting();

    // Implementation of the LorawanMac interface
    void Receive(Ptr<const Packet> packet) override;

    // Implementation of the LorawanMac interface
    void FailedReception(Ptr<const Packet> packet) override;

    // Implementation of the LorawanMac interface
    void TxFinished(Ptr<const Packet> packet) override;

    /**
     * Return the next time at which we will be able to transmit.
     *
     * \return The next transmission time.
     */
    Time GetWaitingTime(double frequency);

  private:
  protected:
};

} // namespace lorawan

} // namespace ns3
#endif /* GATEWAY_LORAWAN_MAC_H */
