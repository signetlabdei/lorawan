/*
 * This example creates a simple network in which all LoRaWAN components are
 * simulated: End Devices, some Gateways and a Network Server.
 * Two end devices are already configured to send unconfirmed and confirmed messages respectively.
 */

#include "ns3/point-to-point-module.h"
#include "ns3/forwarder-helper.h"
#include "ns3/network-server-helper.h"
#include "ns3/lora-channel.h"
#include "ns3/mobility-helper.h"
#include "ns3/lora-phy-helper.h"
#include "ns3/lorawan-mac-helper.h"
#include "ns3/lora-helper.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/periodic-sender.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/command-line.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/lora-device-address-generator.h"
#include "ns3/one-shot-sender-helper.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("NetworkServerExample");

int main (int argc, char *argv[])
{

  bool verbose = false;

  CommandLine cmd;
  cmd.AddValue ("verbose", "Whether to print output or not", verbose);
  cmd.Parse (argc, argv);

  // Logging
  //////////

  LogComponentEnable ("NetworkServerExample", LOG_LEVEL_ALL);
  LogComponentEnable ("NetworkServer", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLorawanMac", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraFrameHeader", LOG_LEVEL_ALL);
  // LogComponentEnable("LorawanMacHeader", LOG_LEVEL_ALL);
  // LogComponentEnable("MacCommand", LOG_LEVEL_ALL);
  // LogComponentEnable("GatewayLoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable("LoraChannel", LOG_LEVEL_ALL);
  // LogComponentEnable("EndDeviceLoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLorawanMac", LOG_LEVEL_ALL);
  LogComponentEnable ("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);
  // LogComponentEnable ("OneShotSender", LOG_LEVEL_ALL);
  // LogComponentEnable("PointToPointNetDevice", LOG_LEVEL_ALL);
  // LogComponentEnable ("Forwarder", LOG_LEVEL_ALL);
  // LogComponentEnable ("OneShotSender", LOG_LEVEL_ALL);
  // LogComponentEnable ("DeviceStatus", LOG_LEVEL_ALL);
  // LogComponentEnable ("GatewayStatus", LOG_LEVEL_ALL);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  // Create a simple wireless channel
  ///////////////////////////////////

  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

  // Helpers
  //////////

  // End Device mobility
  MobilityHelper mobilityEd, mobilityGw;
  Ptr<ListPositionAllocator> positionAllocEd = CreateObject<ListPositionAllocator> ();
  positionAllocEd->Add (Vector (6000.0, 0.0, 0.0));
  positionAllocEd->Add (Vector (0.0, 100.0, 0.0));
  mobilityEd.SetPositionAllocator (positionAllocEd);
  mobilityEd.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Gateway mobility
  Ptr<ListPositionAllocator> positionAllocGw = CreateObject<ListPositionAllocator> ();
  positionAllocGw->Add (Vector (0.0, 0.0, 0.0));
  positionAllocGw->Add (Vector (-2000.0, 0.0, 0.0));
  positionAllocGw->Add (Vector (500.0, 0.0, 0.0));
  mobilityGw.SetPositionAllocator (positionAllocGw);
  mobilityGw.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LorawanMacHelper
  LorawanMacHelper macHelper = LorawanMacHelper ();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();

  // Create EDs
  /////////////

  NodeContainer endDevices;
  endDevices.Create (2);
  mobilityEd.Install (endDevices);

  // Create a LoraDeviceAddressGenerator
  uint8_t nwkId = 54;
  uint32_t nwkAddr = 1864;
  Ptr<LoraDeviceAddressGenerator> addrGen = CreateObject<LoraDeviceAddressGenerator> (nwkId,nwkAddr);

  // Create the LoraNetDevices of the end devices
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LorawanMacHelper::ED_A);
  macHelper.SetAddressGenerator (addrGen);
  macHelper.SetRegion (LorawanMacHelper::EU);
  helper.Install (phyHelper, macHelper, endDevices);

  // Set message type (Default is unconfirmed)
  Ptr<LorawanMac> edMac1 = endDevices.Get (1)->GetDevice (0)->GetObject<LoraNetDevice> ()->GetMac ();
  Ptr<ClassAEndDeviceLorawanMac> edLorawanMac1 = edMac1->GetObject<ClassAEndDeviceLorawanMac> ();
  edLorawanMac1->SetMType (LorawanMacHeader::CONFIRMED_DATA_UP);


  // Install applications in EDs
  OneShotSenderHelper oneShotHelper = OneShotSenderHelper ();
  oneShotHelper.SetSendTime (Seconds (4));
  oneShotHelper.Install (endDevices.Get (0));
  oneShotHelper.SetSendTime (Seconds (10));
  oneShotHelper.Install (endDevices.Get (1));
  // oneShotHelper.SetSendTime (Seconds (8));
  // oneShotHelper.Install(endDevices.Get (1));
  // oneShotHelper.SetSendTime (Seconds (12));
  // oneShotHelper.Install(endDevices.Get (2));

  ////////////////
  // Create GWs //
  ////////////////

  NodeContainer gateways;
  gateways.Create (1);
  mobilityGw.Install (gateways);

  // Create the LoraNetDevices of the gateways
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LorawanMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  // Set spreading factors up
  macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);

  ////////////
  // Create NS
  ////////////

  NodeContainer networkServers;
  networkServers.Create (1);

  // Install the NetworkServer application on the network server
  NetworkServerHelper networkServerHelper;
  networkServerHelper.SetGateways (gateways);
  networkServerHelper.SetEndDevices (endDevices);
  networkServerHelper.Install (networkServers);

  // Install the Forwarder application on the gateways
  ForwarderHelper forwarderHelper;
  forwarderHelper.Install (gateways);

  // Start simulation
  Simulator::Stop (Seconds (800));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
