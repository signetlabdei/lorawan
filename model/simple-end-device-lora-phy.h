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
 * Class representing a simple LoRa transceiver, with an error model based
 * on receiver sensitivity and a SIR table.
 */
class SimpleEndDeviceLoraPhy : public EndDeviceLoraPhy
{
  public:
    static TypeId GetTypeId();

    // Constructor and destructor
    SimpleEndDeviceLoraPhy();
    ~SimpleEndDeviceLoraPhy() override;

    // Implementation of EndDeviceLoraPhy's pure virtual functions
    void StartReceive(Ptr<Packet> packet,
                      double rxPowerDbm,
                      uint8_t sf,
                      Time duration,
                      double frequencyMHz) override;

    // Implementation of LoraPhy's pure virtual functions
    void EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event) override;

    // Implementation of LoraPhy's pure virtual functions
    void Send(Ptr<Packet> packet,
              LoraTxParameters txParams,
              double frequencyMHz,
              double txPowerDbm) override;

  private:
};

} // namespace lorawan

} // namespace ns3
#endif /* SIMPLE_END_DEVICE_LORA_PHY_H */
