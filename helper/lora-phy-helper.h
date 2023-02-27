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

#ifndef LORA_PHY_HELPER_H
#define LORA_PHY_HELPER_H

#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/lora-channel.h"
#include "ns3/lora-net-device.h"
#include "ns3/lora-phy.h"
#include "ns3/lorawan-mac.h"
#include "ns3/object-factory.h"

namespace ns3
{
namespace lorawan
{

/**
 * Helper to install LoraPhy instances on multiple Nodes.
 */
class LoraPhyHelper
{
  public:
    /**
     * Create a phy helper without any parameter set. The user must set
     * them all to be able to call Install later.
     */
    LoraPhyHelper();
    ~LoraPhyHelper();

    /**
     * Crate a LoraPhy and connect it to a device on a node.
     *
     * \param device the device within which this PHY will be created.
     * \return a newly-created PHY object.
     */
    Ptr<LoraPhy> Install(Ptr<LoraNetDevice> device) const;

    /**
     * \tparam Args \deduced Template type parameter pack for the sequence of name-value pairs.
     * \param type the type of ns3::LoraPhy to create.
     * \param args A sequence of name-value pairs of the attributes to set.
     *
     * All the attributes specified in this method should exist
     * in the requested PHY.
     */
    template <typename... Args>
    void SetType(std::string type, Args&&... args);

    /**
     * Helper function used to set the interference helper attributes.
     *
     * \tparam Args \deduced Template type parameter pack for the sequence of name-value pairs.
     * \param args A sequence of name-value pairs of the attributes to set.
     */
    template <typename... Args>
    void SetInterference(const std::string& name, const AttributeValue& value, Args&&... args);

    /**
     * Set the LoraChannel to connect the PHYs to.
     *
     * Every PHY created by a call to Install is associated to this channel.
     *
     * \param channel the channel to associate to this helper.
     */
    void SetChannel(Ptr<LoraChannel> channel);

  private:
    /**
     * The PHY layer factory object.
     */
    ObjectFactory m_phy;
    ObjectFactory m_interferenceHelper; ///< interference helper

    /**
     * The channel instance the PHYs will be connected to.
     */
    Ptr<LoraChannel> m_channel;
};

template <typename... Args>
void
LoraPhyHelper::SetType(std::string type, Args&&... args)
{
    m_phy.SetTypeId(type);
    m_phy.Set(args...);
}

template <typename... Args>
void
LoraPhyHelper::SetInterference(const std::string& name, const AttributeValue& value, Args&&... args)
{
    m_interferenceHelper.Set(name, value, args...);
}

} // namespace lorawan

} // namespace ns3
#endif /* LORA_PHY_HELPER_H */
