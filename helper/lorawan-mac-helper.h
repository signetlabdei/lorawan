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

#ifndef LORAWAN_MAC_HELPER_H
#define LORAWAN_MAC_HELPER_H

#include "ns3/lora-device-address-generator.h"
#include "ns3/lora-net-device.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

namespace ns3
{
namespace lorawan
{

class LorawanMacHelper
{
  public:
    /**
     * Define the operational region.
     */
    enum Regions
    {
        EU,
        US,
        China,
        EU433MHz,
        Australia,
        CN,
        AS923MHz,
        SouthKorea,
        SingleChannel,
        ALOHA
    };

    /**
     * Create a mac helper without any parameter set. The user must set
     * them all to be able to call Install later.
     */
    LorawanMacHelper();
    ~LorawanMacHelper();

    /**
     * Set the region in which the device is to operate.
     */
    void SetRegion(enum Regions region);

    /**
     * \tparam Args \deduced Template type parameter pack for the sequence of name-value pairs.
     * \param type the type of ns3::LorawanMac to create.
     * \param args A sequence of name-value pairs of the attributes to set.
     *
     * All the attributes specified in this method should exist
     * in the requested MAC.
     */
    template <typename... Args>
    void SetType(std::string type, Args&&... args);

    /**
     * Set the address generator to use for creation of these nodes.
     */
    void SetAddressGenerator(Ptr<LoraDeviceAddressGenerator> addrGen);

    /**
     * Create the LorawanMac instance and connect it to a device
     *
     * \param device the device within which this MAC will be created.
     * \returns a newly-created LorawanMac object.
     */
    Ptr<LorawanMac> Install(Ptr<LoraNetDevice> device) const;

    /**
     * Set up the end device's data rates with the criteria from the default ADR algortithm
     */
    static std::vector<int> SetSpreadingFactorsUp(NodeContainer endDevices,
                                                  NodeContainer gateways,
                                                  Ptr<LoraChannel> channel);

  private:
    /**
     * Perform region-specific configurations for the 868 MHz EU band.
     */
    void ConfigureForEuRegion(Ptr<LorawanMac> mac) const;

    /**
     * Perform region-specific configurations for the SINGLECHANNEL band.
     */
    void ConfigureForSingleChannelRegion(Ptr<LorawanMac> mac) const;

    /**
     * Perform region-specific configurations for the ALOHA band.
     */
    void ConfigureForAlohaRegion(Ptr<LorawanMac> mac) const;

    ObjectFactory m_mac;
    Ptr<LoraDeviceAddressGenerator> m_addrGen; //!< Pointer to the address generator to use
    enum Regions m_region;                     //!< The region in which the device will operate
};

template <typename... Args>
void
LorawanMacHelper::SetType(std::string type, Args&&... args)
{
    m_mac.SetTypeId(type);
    m_mac.Set(args...);
}

} // namespace lorawan

} // namespace ns3
#endif /* LORA_PHY_HELPER_H */
