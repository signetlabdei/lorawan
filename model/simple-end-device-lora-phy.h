/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef SIMPLE_END_DEVICE_LORA_PHY_H
#define SIMPLE_END_DEVICE_LORA_PHY_H

#include "end-device-lora-phy.h"

#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/traced-value.h"

namespace ns3
{
namespace lorawan
{

class LoraChannel;

/**
 * @ingroup lorawan
 *
 * Class representing a simple LoRa transceiver, with an error model based
 * on receiver sensitivity and a SIR table.
 */
class SimpleEndDeviceLoraPhy : public EndDeviceLoraPhy
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    SimpleEndDeviceLoraPhy();           //!< Default constructor
    ~SimpleEndDeviceLoraPhy() override; //!< Destructor

    // Implementation of EndDeviceLoraPhy's pure virtual functions
    void StartReceive(Ptr<Packet> packet,
                      double rxPowerDbm,
                      uint8_t sf,
                      Time duration,
                      uint32_t frequencyHz) override;

    // Implementation of LoraPhy's pure virtual functions
    void EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event) override;

    // Implementation of LoraPhy's pure virtual functions
    void Send(Ptr<Packet> packet,
              LoraTxParameters txParams,
              uint32_t frequencyHz,
              double txPowerDbm) override;

  private:
};

} // namespace lorawan

} // namespace ns3
#endif /* SIMPLE_END_DEVICE_LORA_PHY_H */
