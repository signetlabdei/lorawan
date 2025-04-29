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

#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/traced-value.h"

#include <list>

namespace ns3
{
namespace lorawan
{

class LoraChannel;

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

    void StartReceive(Ptr<Packet> packet,
                      double rxPowerDbm,
                      uint8_t sf,
                      Time duration,
                      uint32_t frequencyHz) override;

    void EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event) override;

    void Send(Ptr<Packet> packet,
              LoraTxParameters txParams,
              uint32_t frequencyHz,
              double txPowerDbm) override;

  private:
};

} // namespace lorawan

} // namespace ns3
#endif /* SIMPLE_GATEWAY_LORA_PHY_H */
