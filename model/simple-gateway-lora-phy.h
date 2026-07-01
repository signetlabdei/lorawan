/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef SIMPLE_GATEWAY_LORA_PHY_H
#define SIMPLE_GATEWAY_LORA_PHY_H

#include "gateway-lora-phy.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Class modeling a Lora SX1301 chip.
 */
class SimpleGatewayLoraPhy : public GatewayLoraPhy
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    SimpleGatewayLoraPhy();           //!< Default constructor
    ~SimpleGatewayLoraPhy() override; //!< Destructor

    // Implementation of GatewayLoraPhy's pure virtual function
    void Send(Ptr<Packet> packet,
              uint32_t frequencyHz,
              const LoraTxParameters& txParams,
              double txPowerDbm) override;

    // Implementation of GatewayLoraPhy's pure virtual function
    void StartReceive(Ptr<Packet> packet,
                      uint32_t frequencyHz,
                      uint8_t spreadingFactor,
                      double rxPowerDbm,
                      Time duration) override;

  private:
    // Implementation of GatewayLoraPhy's pure virtual function
    void EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event) override;
};

} // namespace lorawan
} // namespace ns3

#endif /* SIMPLE_GATEWAY_LORA_PHY_H */
