/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LORA_PHY_HELPER_H
#define LORA_PHY_HELPER_H

#include "ns3/lora-channel.h"
#include "ns3/lora-phy.h"
#include "ns3/lorawan-mac.h"
#include "ns3/net-device.h"
#include "ns3/object-factory.h"
#include "ns3/simple-end-device-lora-phy.h"
#include "ns3/simple-gateway-lora-phy.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Helper to install LoraPhy instances on multiple Nodes. The
 * user must set all parameters before calling Install on nodes.
 */
class LoraPhyHelper
{
  public:
    /**
     * Enum for the type of device: End Device (ED) or Gateway (GW).
     */
    enum DeviceType
    {
        GW,
        ED
    };

    LoraPhyHelper(); //!< Default constructor

    /**
     * Set the LoraChannel to connect the PHYs to.
     *
     * Every PHY created by a call to Install is associated to this channel.
     *
     * @param channel The channel to associate to this helper.
     */
    void SetChannel(Ptr<LoraChannel> channel);

    /**
     * Set the kind of PHY this helper will create.
     *
     * @param dt The device type.
     */
    void SetDeviceType(enum DeviceType dt);

    /**
     * Get the TypeId of the object to be created with LoraPhyHelper.
     *
     * @return The TypeId instance.
     */
    TypeId GetDeviceType() const;

    /**
     * Set an attribute of the underlying PHY object.
     *
     * @param name The name of the attribute to set.
     * @param v The value of the attribute.
     */
    void Set(std::string name, const AttributeValue& v);

    /**
     * Create a LoraPhy and connect it to a device on a node.
     *
     * @param node The node on which we wish to create a wifi PHY.
     * @param device The device within which this PHY will be created.
     * @return A newly-created PHY object.
     */
    Ptr<LoraPhy> Install(Ptr<Node> node, Ptr<NetDevice> device) const;

    /**
     * Set the maximum number of gateway receive paths.
     *
     * @param maxReceptionPaths The maximum number of reception paths at
     *  the gateway.
     */
    void SetMaxReceptionPaths(int maxReceptionPaths);

    /**
     * Set if giving priority to downlink transmission over reception at
     * the gateways.
     *
     * @param txPriority Whether gateway transmission interrupt all receptions for their duration.
     */
    void SetGatewayTransmissionPriority(bool txPriority);

  private:
    ObjectFactory m_phy;        //!< The PHY layer factory object.
    Ptr<LoraChannel> m_channel; //!< The channel instance the PHYs will be connected to.
    int m_maxReceptionPaths;    //!< The maximum number of receive paths at the gateway.
    bool m_txPriority; //!< Whether to give priority to downlink transmission over reception at the
                       //!< gateways. \todo This parameter does nothing, to be removed.
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_PHY_HELPER_H */
