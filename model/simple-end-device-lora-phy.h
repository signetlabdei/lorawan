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

namespace ns3
{
namespace lorawan
{

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

    // Implementation of EndDeviceLoraPhy's pure virtual function
    void Send(Ptr<Packet> packet,
              uint32_t frequencyHz,
              IQPolarity iqPolarity,
              const LoraTxParameters& txParams,
              double txPowerDbm) override;

    // Implementation of EndDeviceLoraPhy's pure virtual function
    void StartReceive(Ptr<Packet> packet,
                      uint32_t frequencyHz,
                      IQPolarity iqPolarity,
                      uint8_t spreadingFactor,
                      double rxPowerDbm,
                      Time duration) override;

  private:
    // Implementation of EndDeviceLoraPhy's pure virtual function
    void EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event) override;
};

} // namespace lorawan
} // namespace ns3

#endif /* SIMPLE_END_DEVICE_LORA_PHY_H */
