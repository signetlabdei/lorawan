/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LORAWAN_MAC_HELPER_H
#define LORAWAN_MAC_HELPER_H

#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/lora-channel.h"
#include "ns3/lora-device-address-generator.h"
#include "ns3/lora-phy.h"
#include "ns3/lorawan-mac.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/random-variable-stream.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Helper class for configuring and installing the LorawanMac class on devices and gateways. The
 * user must set all parameters before calling Install on nodes.
 */
class LorawanMacHelper
{
  public:
    /**
     * Define the kind of device. Can be either GW (Gateway) or ED (End Device).
     */
    enum DeviceType
    {
        GW,
        ED_A
    };

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

    LorawanMacHelper(); //!< Default constructor

    /**
     * Set an attribute of the underlying MAC object.
     *
     * @param name The name of the attribute to set.
     * @param v The value of the attribute.
     */
    void Set(std::string name, const AttributeValue& v);

    /**
     * Set the address generator to use for creation of these nodes.
     *
     * @param addrGen Pointer to the address generator object.
     */
    void SetAddressGenerator(Ptr<LoraDeviceAddressGenerator> addrGen);

    /**
     * Set the kind of MAC this helper will create.
     *
     * @param dt The device type (either gateway or end device).
     */
    void SetDeviceType(enum DeviceType dt);

    /**
     * Set the region in which the device is to operate.
     *
     * @param region The region enum value.
     */
    void SetRegion(enum Regions region);

    /**
     * Create the LorawanMac instance and connect it to a device.
     *
     * @param node The node on which we wish to create a wifi MAC.
     * @param device The device within which this MAC will be created.
     * @return A newly-created LorawanMac object.
     */
    Ptr<LorawanMac> Install(Ptr<Node> node, Ptr<NetDevice> device) const;

    /**
     * Initialize the end devices' data rate parameter.
     *
     * The Data Rate (DR) of each device is set to the maximum possible for its transmissions to be
     * correctly received by the gateway measuring the best RSSI from the device, mimicking the DR
     * maximization part of the default online LoRaWAN Adaptive Data Rate (ADR) algorithm. Please
     * note that a single RSSI measurement between each device and gateway pair is taken for the DR
     * assignment, so the assignment may be suboptimal in scenarios with a time-varying channel.
     *
     * This function uses the following convention (EU868) to compute the transmission range:
     *
     * DR5 -> SF7  \n
     * DR4 -> SF8  \n
     * DR3 -> SF9  \n
     * DR2 -> SF10 \n
     * DR1 -> SF11 \n
     * DR0 -> SF12 \n
     *
     *
     * It returns a DR distribution vector with the following counters:
     *
     * v[0] -> number of devices using DR5                                 \n
     * v[1] -> number of devices using DR4                                 \n
     * v[2] -> number of devices using DR3                                 \n
     * v[3] -> number of devices using DR2                                 \n
     * v[4] -> number of devices using DR1                                 \n
     * v[5] -> number of devices using DR0, in range of at least a gateway \n
     * v[6] -> number of devices using DR0, out of range                   \n
     *
     * @param endDevices The end devices to configure.
     * @param gateways The gateways to consider for RSSI measurements.
     * @param channel The radio channel to consider for RSSI measurements.
     * @return A vector containing the final number of devices per DR.
     */
    static std::vector<int> SetSpreadingFactorsUp(NodeContainer endDevices,
                                                  NodeContainer gateways,
                                                  Ptr<LoraChannel> channel);

    /**
     * Randomly initialize the end devices' data rate parameter according to the given
     * distribution.
     *
     * This function expects a data rate (DR) distribution vector of length 6 filled with real
     * numbers summing up to 1. The value at index \f$i\f$ is considered to be the fraction of
     * devices to be assigned DR \f$5-i\f$, for example:
     *
     * distribution[0] == 0.2 -> fraction of devices to be assigned to DR5 \n
     * distribution[1] == 0.1 -> fraction of devices to be assigned to DR4 \n
     * distribution[2] == 0.1 -> fraction of devices to be assigned to DR3 \n
     * distribution[3] == 0.1 -> fraction of devices to be assigned to DR2 \n
     * distribution[4] == 0.2 -> fraction of devices to be assigned to DR1 \n
     * distribution[5] == 0.3 -> fraction of devices to be assigned to DR0 \n
     *
     *
     * Devices are then randomly assigned a DR following the provided distribution.
     *
     * It returns a DR distribution vector with the following counters:
     *
     * v[0] -> number of devices using DR5 \n
     * v[1] -> number of devices using DR4 \n
     * v[2] -> number of devices using DR3 \n
     * v[3] -> number of devices using DR2 \n
     * v[4] -> number of devices using DR1 \n
     * v[5] -> number of devices using DR0 \n
     *
     *
     * @param endDevices The end devices to configure.
     * @param gateways The gateways in the network (this is only a placeholder parameter).
     * @param distribution The distribution (probability mass function) of DR assignment.
     * @return A vector containing the final number of devices per DR.
     *
     * @todo Remove unused parameter gateways.
     */
    static std::vector<int> SetSpreadingFactorsGivenDistribution(NodeContainer endDevices,
                                                                 NodeContainer gateways,
                                                                 std::vector<double> distribution);

  private:
    /**
     * Perform region-specific configurations for the 868 MHz EU band.
     *
     * @param edMac Pointer to the device MAC layer to configure.
     */
    void ConfigureForEuRegion(Ptr<ClassAEndDeviceLorawanMac> edMac) const;

    /**
     * Perform region-specific configurations for the 868 MHz EU band.
     *
     * @param gwMac Pointer to the gateway MAC layer to configure.
     */
    void ConfigureForEuRegion(Ptr<GatewayLorawanMac> gwMac) const;

    /**
     * Apply configurations that are common both for the GatewayLorawanMac and the
     * ClassAEndDeviceLorawanMac classes.
     *
     * @param lorawanMac Pointer to the MAC layer to configure.
     */
    void ApplyCommonEuConfigurations(Ptr<LorawanMac> lorawanMac) const;

    /**
     * Perform region-specific configurations for the SINGLECHANNEL band.
     *
     * @param edMac Pointer to the device MAC layer to configure.
     */
    void ConfigureForSingleChannelRegion(Ptr<ClassAEndDeviceLorawanMac> edMac) const;

    /**
     * Perform region-specific configurations for the SINGLECHANNEL band.
     *
     * @param gwMac Pointer to the gateway MAC layer to configure.
     */
    void ConfigureForSingleChannelRegion(Ptr<GatewayLorawanMac> gwMac) const;

    /**
     * Apply configurations that are common both for the GatewayLorawanMac and the
     * ClassAEndDeviceLorawanMac classes.
     *
     * @param lorawanMac Pointer to the MAC layer to configure.
     */
    void ApplyCommonSingleChannelConfigurations(Ptr<LorawanMac> lorawanMac) const;

    /**
     * Perform region-specific configurations for the ALOHA band.
     *
     * @param edMac Pointer to the device MAC layer to configure.
     */
    void ConfigureForAlohaRegion(Ptr<ClassAEndDeviceLorawanMac> edMac) const;

    /**
     * Perform region-specific configurations for the ALOHA band.
     *
     * @param gwMac Pointer to the gateway MAC layer to configure.
     */
    void ConfigureForAlohaRegion(Ptr<GatewayLorawanMac> gwMac) const;

    /**
     * Apply configurations that are common both for the GatewayLorawanMac and the
     * ClassAEndDeviceLorawanMac classes.
     *
     * @param lorawanMac Pointer to the MAC layer to configure.
     */
    void ApplyCommonAlohaConfigurations(Ptr<LorawanMac> lorawanMac) const;

    ObjectFactory m_mac;                       //!< MAC-layer object factory
    Ptr<LoraDeviceAddressGenerator> m_addrGen; //!< Pointer to the address generator to use
    enum DeviceType m_deviceType;              //!< The kind of device to install
    enum Regions m_region;                     //!< The region in which the device will operate
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_PHY_HELPER_H */
