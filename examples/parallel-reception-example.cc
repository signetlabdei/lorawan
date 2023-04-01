/*
 * This script simulates a simple network in which one end device sends one
 * packet to the gateway.
 */

#include "ns3/base-end-device-lorawan-mac.h"
#include "ns3/command-line.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/log.h"
#include "ns3/lorawan-helper.h"
#include "ns3/lorawan-mac-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/simulator.h"

#include <algorithm>
#include <ctime>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("ParallelReceptionExample");

int
main(int argc, char* argv[])
{
    // Set up logging
    LogComponentEnable("ParallelReceptionExample", LOG_LEVEL_ALL);
    // LogComponentEnable ("LoraChannel", LOG_LEVEL_INFO);
    // LogComponentEnable ("LoraPhy", LOG_LEVEL_ALL);
    // LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_ALL);
    LogComponentEnable("GatewayLoraPhy", LOG_LEVEL_ALL);
    // LogComponentEnable ("LoraInterferenceHelper", LOG_LEVEL_ALL);
    // LogComponentEnable ("LorawanMac", LOG_LEVEL_ALL);
    // LogComponentEnable ("BaseEndDeviceLorawanMac", LOG_LEVEL_ALL);
    // LogComponentEnable ("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("GatewayLorawanMac", LOG_LEVEL_ALL);
    // LogComponentEnable ("LogicalChannelManager", LOG_LEVEL_ALL);
    // LogComponentEnable ("LogicalChannel", LOG_LEVEL_ALL);
    // LogComponentEnable ("LorawanHelper", LOG_LEVEL_ALL);
    // LogComponentEnable ("LoraPhyHelper", LOG_LEVEL_ALL);
    // LogComponentEnable ("LorawanMacHelper", LOG_LEVEL_ALL);
    // LogComponentEnable ("OneShotSenderHelper", LOG_LEVEL_ALL);
    // LogComponentEnable ("OneShotSender", LOG_LEVEL_ALL);
    // LogComponentEnable ("LorawanMacHeader", LOG_LEVEL_ALL);
    // LogComponentEnable ("LoraFrameHeader", LOG_LEVEL_ALL);
    LogComponentEnableAll(LOG_PREFIX_FUNC);
    LogComponentEnableAll(LOG_PREFIX_NODE);
    LogComponentEnableAll(LOG_PREFIX_TIME);

    /************************
     *  Create the channel  *
     ************************/

    NS_LOG_INFO("Creating the channel...");

    // Create the lora channel object
    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

    /************************
     *  Create the helpers  *
     ************************/

    NS_LOG_INFO("Setting up helpers...");

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator>();
    allocator->Add(Vector(0, 0, 0));
    mobility.SetPositionAllocator(allocator);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    // Create the LoraPhyHelper
    LoraPhyHelper phyHelper = LoraPhyHelper();
    phyHelper.SetChannel(channel);

    // Create the LorawanMacHelper
    LorawanMacHelper macHelper = LorawanMacHelper();

    // Create the LorawanHelper
    LorawanHelper helper = LorawanHelper();

    /************************
     *  Create End Devices  *
     ************************/

    NS_LOG_INFO("Creating the end device...");

    // Create a set of nodes
    NodeContainer endDevices;
    endDevices.Create(6);

    // Assign a mobility model to the nodes
    mobility.Install(endDevices);

    // Create the LoraNetDevices of the end devices
    phyHelper.SetType("ns3::EndDeviceLoraPhy");
    macHelper.SetType("ns3::ClassAEndDeviceLorawanMac");
    macHelper.SetRegion(LorawanMacHelper::SingleChannel);
    helper.Install(phyHelper, macHelper, endDevices);

    /*********************
     *  Create Gateways  *
     *********************/

    NS_LOG_INFO("Creating the gateway...");
    NodeContainer gateways;
    gateways.Create(1);

    mobility.Install(gateways);

    // Create a netdevice for each gateway
    phyHelper.SetType("ns3::GatewayLoraPhy");
    macHelper.SetType("ns3::GatewayLorawanMac");
    helper.Install(phyHelper, macHelper, gateways);

    /*********************************************
     *  Install applications on the end devices  *
     *********************************************/

    OneShotSenderHelper oneShotSenderHelper;

    oneShotSenderHelper.SetSendTime(Seconds(1));
    oneShotSenderHelper.Install(endDevices);

    /******************
     * Set Data Rates *
     ******************/
    for (uint32_t i = 0; i < endDevices.GetN(); i++)
    {
        DynamicCast<BaseEndDeviceLorawanMac>(
            DynamicCast<LoraNetDevice>(endDevices.Get(i)->GetDevice(0))->GetMac())
            ->SetDataRate(5 - i);
    }

    /****************
     *  Simulation  *
     ****************/

    Simulator::Stop(Hours(2));

    Simulator::Run();

    Simulator::Destroy();

    return 0;
}
