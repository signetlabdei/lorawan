/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
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

/**
 * @ingroup lorawan
 *
 * Class representing the MAC layer of a LoRaWAN gateway.
 */
class GatewayLorawanMac : public LorawanMac
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    GatewayLorawanMac();           //!< Default constructor
    ~GatewayLorawanMac() override; //!< Destructor

    // Implementation of the LorawanMac interface
    void Send(Ptr<Packet> packet) override;

    /**
     * Check whether the underlying PHY layer of the gateway is currently transmitting.
     *
     * @return True if it is transmitting, false otherwise.
     */
    bool IsTransmitting();

    // Implementation of the LorawanMac interface
    void Receive(Ptr<const Packet> packet) override;

    // Implementation of the LorawanMac interface
    void FailedReception(Ptr<const Packet> packet) override;

    // Implementation of the LorawanMac interface
    void TxFinished(Ptr<const Packet> packet) override;

    /**
     * Return the next time at which we will be able to transmit on the specified frequency.
     *
     * @param frequencyHz The frequency value [Hz].
     * @return The next transmission time.
     */
    Time GetWaitTime(uint32_t frequencyHz);

  private:
  protected:
};

} // namespace lorawan

} // namespace ns3
#endif /* GATEWAY_LORAWAN_MAC_H */
