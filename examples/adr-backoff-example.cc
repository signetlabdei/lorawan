/*
 * Copyright (c) 2025 University of Bologna
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Alessandro Aimi <alessandro.aimi@unibo.it>
 */

/*
 * This minimal example has the purpose of showcasing and testing the Adaptive Data Rate (ADR)
 * backoff feture of the MAC layer of LoRaWAN devices.
 */

#include "ns3/core-module.h"
#include "ns3/lorawan-module.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("AdrBackoffExample");

/**
 * Record a change in the data rate setting on an end device.
 *
 * @param oldDr The previous data rate value.
 * @param newDr The updated data rate value.
 */
void
OnDataRateChange(uint8_t oldDr, uint8_t newDr)
{
    NS_LOG_DEBUG("DR" << unsigned(oldDr) << " -> DR" << unsigned(newDr));
}

/**
 * Record a change in the transmission power setting on an end device.
 *
 * @param oldTxPower The previous transmission power value.
 * @param newTxPower The updated transmission power value.
 */
void
OnTxPowerChange(double oldTxPower, double newTxPower)
{
    NS_LOG_DEBUG(oldTxPower << " dBm -> " << newTxPower << " dBm");
}

int
main(int argc, char* argv[])
{
    /* Logging */
    LogComponentEnable("AdrBackoffExample", LOG_LEVEL_ALL);
    LogComponentEnable("AdrComponent", LOG_LEVEL_ALL);
    LogComponentEnable("NetworkControllerComponent", LOG_LEVEL_ALL);
    LogComponentEnable("EndDeviceLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnableAll(LOG_PREFIX_FUNC);
    LogComponentEnableAll(LOG_PREFIX_NODE);
    LogComponentEnableAll(LOG_PREFIX_TIME);

    /* Create nodes */
    auto endDevice = CreateObject<Node>();
    auto gateway = CreateObject<Node>();
    auto networkServer = CreateObject<Node>();

    /* Configure mobility */
    {
        MobilityHelper mobilityHelper;
        mobilityHelper.Install(NodeContainer(endDevice, gateway));
        endDevice->GetObject<MobilityModel>()->SetPosition(Vector(1000, 0, 0)); // Device position
        gateway->GetObject<MobilityModel>()->SetPosition(Vector(0, 0, 0));      // Gateway position
    }

    /* Configure simple radio channel */
    Ptr<LoraChannel> channel;
    {
        // Delay obtained from distance and speed of light in vacuum (constant)
        auto delay = CreateObject<ConstantSpeedPropagationDelayModel>();
        // https://doi.org/10.1109/VTC2021-Fall52928.2021.9625531
        auto loss = CreateObject<LogDistancePropagationLossModel>();
        loss->SetPathLossExponent(1.58);
        loss->SetReference(1, 85.01);
        auto shadowing = CreateObject<RandomPropagationLossModel>();
        auto rv = "ns3::NormalRandomVariable[Variance=" + std::to_string(pow(9.9, 2)) + "]";
        shadowing->SetAttribute("Variable", StringValue(rv));
        loss->SetNext(shadowing);
        channel = CreateObject<LoraChannel>(loss, delay);
    }

    /* Configure network devices */
    P2PGwRegistration_t gwRegistration;
    {
        /* LoRaWAN side (between end device and gateway) */
        LoraHelper lorawanHelper;
        LoraPhyHelper phyHelper;
        phyHelper.SetChannel(channel);
        LorawanMacHelper macHelper;
        // Create the LoraNetDevice of the end device
        phyHelper.SetDeviceType(LoraPhyHelper::ED);
        macHelper.SetDeviceType(LorawanMacHelper::ED_A);
        lorawanHelper.Install(phyHelper, macHelper, endDevice);
        // Create the LoraNetDevice of the gateway
        phyHelper.SetDeviceType(LoraPhyHelper::GW);
        macHelper.SetDeviceType(LorawanMacHelper::GW);
        lorawanHelper.Install(phyHelper, macHelper, gateway);

        /* PointToPoint links between gateway and server */
        PointToPointHelper p2pHelper;
        p2pHelper.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
        p2pHelper.SetChannelAttribute("Delay", StringValue("2ms"));
        auto p2pNetDevs = p2pHelper.Install(gateway, networkServer);
        // Store gateway registration details
        auto serverNetDev = DynamicCast<PointToPointNetDevice>(p2pNetDevs.Get(1));
        gwRegistration = {{serverNetDev, gateway}};
    }

    /* Install applications */
    {
        /* End device */
        PeriodicSenderHelper edAppHelper;
        edAppHelper.SetPeriod(Seconds(1200)); // 20 minutes
        edAppHelper.SetPacketSize(20);        // 20B app payload
        edAppHelper.Install(endDevice);

        /* Gateway */
        ForwarderHelper forwarderHelper;
        forwarderHelper.Install(gateway);

        /* Network server */
        NetworkServerHelper networkServerHelper;
        networkServerHelper.SetGatewaysP2P(gwRegistration);
        networkServerHelper.SetEndDevices(NodeContainer(endDevice));
        networkServerHelper.Install(networkServer);
    }

    // Connect trace sources for logging purposes
    Config::ConnectWithoutContext(
        "/NodeList/*/DeviceList/0/$ns3::LoraNetDevice/Mac/$ns3::EndDeviceLorawanMac/TxPower",
        MakeCallback(&OnTxPowerChange));
    Config::ConnectWithoutContext(
        "/NodeList/*/DeviceList/0/$ns3::LoraNetDevice/Mac/$ns3::EndDeviceLorawanMac/DataRate",
        MakeCallback(&OnDataRateChange));

    // Start simulation
    Simulator::Stop(Seconds(1200 * 1000));
    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
