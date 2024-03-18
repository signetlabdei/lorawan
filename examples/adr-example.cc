/*
 * Copyright (c) 2018 University of Padova
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

/*
 * This program creates a simple network which uses an ADR algorithm to set up
 * the Spreading Factors of the devices in the Network.
 */

#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/core-module.h"
#include "ns3/forwarder-helper.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/hex-grid-position-allocator.h"
#include "ns3/log.h"
#include "ns3/lora-channel.h"
#include "ns3/lora-device-address-generator.h"
#include "ns3/lora-helper.h"
#include "ns3/lora-phy-helper.h"
#include "ns3/lorawan-mac-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/network-module.h"
#include "ns3/network-server-helper.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/periodic-sender.h"
#include "ns3/point-to-point-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/rectangle.h"
#include "ns3/string.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("AdrExample");

/**
 * \brief Record a change in the data rate setting on an end device.
 *
 * \param oldDr The previous data rate value.
 * \param newDr The updated data rate value.
 */
void
OnDataRateChange(uint8_t oldDr, uint8_t newDr)
{
    NS_LOG_DEBUG("DR" << unsigned(oldDr) << " -> DR" << unsigned(newDr));
}

/**
 * \brief Record a change in the transmission power setting on an end device.
 *
 * \param oldTxPower The previous transmission power value.
 * \param newTxPower The updated transmission power value.
 */
void
OnTxPowerChange(double oldTxPower, double newTxPower)
{
    NS_LOG_DEBUG(oldTxPower << " dBm -> " << newTxPower << " dBm");
}

int
main(int argc, char* argv[])
{
    bool verbose = false;
    bool adrEnabled = true;
    bool initializeSF = false;
    int nDevices = 400;
    int nPeriods = 20;
    double mobileNodeProbability = 0;
    double sideLength = 10000;
    int gatewayDistance = 5000;
    double maxRandomLoss = 10;
    double minSpeed = 2;
    double maxSpeed = 16;
    std::string adrType = "ns3::AdrComponent";

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Whether to print output or not", verbose);
    cmd.AddValue("MultipleGwCombiningMethod", "ns3::AdrComponent::MultipleGwCombiningMethod");
    cmd.AddValue("MultiplePacketsCombiningMethod",
                 "ns3::AdrComponent::MultiplePacketsCombiningMethod");
    cmd.AddValue("HistoryRange", "ns3::AdrComponent::HistoryRange");
    cmd.AddValue("MType", "ns3::EndDeviceLorawanMac::MType");
    cmd.AddValue("EDDRAdaptation", "ns3::EndDeviceLorawanMac::EnableEDDataRateAdaptation");
    cmd.AddValue("ChangeTransmissionPower", "ns3::AdrComponent::ChangeTransmissionPower");
    cmd.AddValue("AdrEnabled", "Whether to enable ADR", adrEnabled);
    cmd.AddValue("nDevices", "Number of devices to simulate", nDevices);
    cmd.AddValue("PeriodsToSimulate", "Number of periods to simulate", nPeriods);
    cmd.AddValue("MobileNodeProbability",
                 "Probability of a node being a mobile node",
                 mobileNodeProbability);
    cmd.AddValue("sideLength",
                 "Length of the side of the rectangle nodes will be placed in",
                 sideLength);
    cmd.AddValue("maxRandomLoss",
                 "Maximum amount in dB of the random loss component",
                 maxRandomLoss);
    cmd.AddValue("gatewayDistance", "Distance between gateways", gatewayDistance);
    cmd.AddValue("initializeSF", "Whether to initialize the SFs", initializeSF);
    cmd.AddValue("MinSpeed", "Minimum speed for mobile devices", minSpeed);
    cmd.AddValue("MaxSpeed", "Maximum speed for mobile devices", maxSpeed);
    cmd.AddValue("MaxTransmissions", "ns3::EndDeviceLorawanMac::MaxTransmissions");
    cmd.Parse(argc, argv);

    int gatewayRings = 2 + (std::sqrt(2) * sideLength) / (gatewayDistance);
    int nGateways = 3 * gatewayRings * gatewayRings - 3 * gatewayRings + 1;

    // Logging
    //////////

    LogComponentEnable("AdrExample", LOG_LEVEL_ALL);
    // LogComponentEnable ("LoraPacketTracker", LOG_LEVEL_ALL);
    // LogComponentEnable ("NetworkServer", LOG_LEVEL_ALL);
    // LogComponentEnable ("NetworkController", LOG_LEVEL_ALL);
    // LogComponentEnable ("NetworkScheduler", LOG_LEVEL_ALL);
    // LogComponentEnable ("NetworkStatus", LOG_LEVEL_ALL);
    // LogComponentEnable ("EndDeviceStatus", LOG_LEVEL_ALL);
    LogComponentEnable("AdrComponent", LOG_LEVEL_ALL);
    // LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);
    // LogComponentEnable ("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
    // LogComponentEnable ("MacCommand", LOG_LEVEL_ALL);
    // LogComponentEnable ("AdrExploraSf", LOG_LEVEL_ALL);
    // LogComponentEnable ("AdrExploraAt", LOG_LEVEL_ALL);
    // LogComponentEnable ("EndDeviceLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnableAll(LOG_PREFIX_FUNC);
    LogComponentEnableAll(LOG_PREFIX_NODE);
    LogComponentEnableAll(LOG_PREFIX_TIME);

    // Set the EDs to require Data Rate control from the NS
    Config::SetDefault("ns3::EndDeviceLorawanMac::DRControl", BooleanValue(true));

    // Create a simple wireless channel
    ///////////////////////////////////

    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);

    Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable>();
    x->SetAttribute("Min", DoubleValue(0.0));
    x->SetAttribute("Max", DoubleValue(maxRandomLoss));

    Ptr<RandomPropagationLossModel> randomLoss = CreateObject<RandomPropagationLossModel>();
    randomLoss->SetAttribute("Variable", PointerValue(x));

    loss->SetNext(randomLoss);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

    // Helpers
    //////////

    // End Device mobility
    MobilityHelper mobilityEd;
    MobilityHelper mobilityGw;
    mobilityEd.SetPositionAllocator(
        "ns3::RandomRectanglePositionAllocator",
        "X",
        PointerValue(CreateObjectWithAttributes<UniformRandomVariable>("Min",
                                                                       DoubleValue(-sideLength),
                                                                       "Max",
                                                                       DoubleValue(sideLength))),
        "Y",
        PointerValue(CreateObjectWithAttributes<UniformRandomVariable>("Min",
                                                                       DoubleValue(-sideLength),
                                                                       "Max",
                                                                       DoubleValue(sideLength))));

    // // Gateway mobility
    // Ptr<ListPositionAllocator> positionAllocGw = CreateObject<ListPositionAllocator> ();
    // positionAllocGw->Add (Vector (0.0, 0.0, 15.0));
    // positionAllocGw->Add (Vector (-5000.0, -5000.0, 15.0));
    // positionAllocGw->Add (Vector (-5000.0, 5000.0, 15.0));
    // positionAllocGw->Add (Vector (5000.0, -5000.0, 15.0));
    // positionAllocGw->Add (Vector (5000.0, 5000.0, 15.0));
    // mobilityGw.SetPositionAllocator (positionAllocGw);
    // mobilityGw.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    Ptr<HexGridPositionAllocator> hexAllocator =
        CreateObject<HexGridPositionAllocator>(gatewayDistance / 2);
    mobilityGw.SetPositionAllocator(hexAllocator);
    mobilityGw.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // Create the LoraPhyHelper
    LoraPhyHelper phyHelper = LoraPhyHelper();
    phyHelper.SetChannel(channel);

    // Create the LorawanMacHelper
    LorawanMacHelper macHelper = LorawanMacHelper();

    // Create the LoraHelper
    LoraHelper helper = LoraHelper();
    helper.EnablePacketTracking();

    ////////////////
    // Create GWs //
    ////////////////

    NodeContainer gateways;
    gateways.Create(nGateways);
    mobilityGw.Install(gateways);

    // Create the LoraNetDevices of the gateways
    phyHelper.SetDeviceType(LoraPhyHelper::GW);
    macHelper.SetDeviceType(LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, gateways);

    // Create EDs
    /////////////

    NodeContainer endDevices;
    endDevices.Create(nDevices);

    // Install mobility model on fixed nodes
    mobilityEd.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    int fixedPositionNodes = double(nDevices) * (1 - mobileNodeProbability);
    for (int i = 0; i < fixedPositionNodes; ++i)
    {
        mobilityEd.Install(endDevices.Get(i));
    }
    // Install mobility model on mobile nodes
    mobilityEd.SetMobilityModel(
        "ns3::RandomWalk2dMobilityModel",
        "Bounds",
        RectangleValue(Rectangle(-sideLength, sideLength, -sideLength, sideLength)),
        "Distance",
        DoubleValue(1000),
        "Speed",
        PointerValue(CreateObjectWithAttributes<UniformRandomVariable>("Min",
                                                                       DoubleValue(minSpeed),
                                                                       "Max",
                                                                       DoubleValue(maxSpeed))));
    for (int i = fixedPositionNodes; i < (int)endDevices.GetN(); ++i)
    {
        mobilityEd.Install(endDevices.Get(i));
    }

    // Create a LoraDeviceAddressGenerator
    uint8_t nwkId = 54;
    uint32_t nwkAddr = 1864;
    Ptr<LoraDeviceAddressGenerator> addrGen =
        CreateObject<LoraDeviceAddressGenerator>(nwkId, nwkAddr);

    // Create the LoraNetDevices of the end devices
    phyHelper.SetDeviceType(LoraPhyHelper::ED);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    macHelper.SetAddressGenerator(addrGen);
    macHelper.SetRegion(LorawanMacHelper::EU);
    helper.Install(phyHelper, macHelper, endDevices);

    // Install applications in EDs
    int appPeriodSeconds = 1200; // One packet every 20 minutes
    PeriodicSenderHelper appHelper = PeriodicSenderHelper();
    appHelper.SetPeriod(Seconds(appPeriodSeconds));
    ApplicationContainer appContainer = appHelper.Install(endDevices);

    // Do not set spreading factors up: we will wait for the NS to do this
    if (initializeSF)
    {
        LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);
    }

    ////////////
    // Create NS
    ////////////

    Ptr<Node> networkServer = CreateObject<Node>();

    // PointToPoint links between gateways and server
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    p2p.SetChannelAttribute("Delay", StringValue("2ms"));
    // Store NS app registration details for later
    P2PGwRegistration_t gwRegistration;
    for (auto gw = gateways.Begin(); gw != gateways.End(); ++gw)
    {
        auto container = p2p.Install(networkServer, *gw);
        auto serverP2PNetDev = DynamicCast<PointToPointNetDevice>(container.Get(0));
        gwRegistration.emplace_back(serverP2PNetDev, *gw);
    }

    // Install the NetworkServer application on the network server
    NetworkServerHelper networkServerHelper;
    networkServerHelper.EnableAdr(adrEnabled);
    networkServerHelper.SetAdr(adrType);
    networkServerHelper.SetGatewaysP2P(gwRegistration);
    networkServerHelper.SetEndDevices(endDevices);
    networkServerHelper.Install(networkServer);

    // Install the Forwarder application on the gateways
    ForwarderHelper forwarderHelper;
    forwarderHelper.Install(gateways);

    // Connect our traces
    Config::ConnectWithoutContext(
        "/NodeList/*/DeviceList/0/$ns3::LoraNetDevice/Mac/$ns3::EndDeviceLorawanMac/TxPower",
        MakeCallback(&OnTxPowerChange));
    Config::ConnectWithoutContext(
        "/NodeList/*/DeviceList/0/$ns3::LoraNetDevice/Mac/$ns3::EndDeviceLorawanMac/DataRate",
        MakeCallback(&OnDataRateChange));

    // Activate printing of ED MAC parameters
    Time stateSamplePeriod = Seconds(1200);
    helper.EnablePeriodicDeviceStatusPrinting(endDevices,
                                              gateways,
                                              "nodeData.txt",
                                              stateSamplePeriod);
    helper.EnablePeriodicPhyPerformancePrinting(gateways, "phyPerformance.txt", stateSamplePeriod);
    helper.EnablePeriodicGlobalPerformancePrinting("globalPerformance.txt", stateSamplePeriod);

    LoraPacketTracker& tracker = helper.GetPacketTracker();

    // Start simulation
    Time simulationTime = Seconds(1200 * nPeriods);
    Simulator::Stop(simulationTime);
    Simulator::Run();
    Simulator::Destroy();

    std::cout << tracker.CountMacPacketsGlobally(Seconds(1200 * (nPeriods - 2)),
                                                 Seconds(1200 * (nPeriods - 1)))
              << std::endl;

    return 0;
}
