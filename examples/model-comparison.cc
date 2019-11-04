/*
 * This program creates a simple network which replicates the
 * conditions contemplated by the mathematical model.
 */

#include "ns3/point-to-point-module.h"
#include "ns3/forwarder-helper.h"
#include "ns3/network-server-helper.h"
#include "ns3/lora-channel.h"
#include "ns3/mobility-helper.h"
#include "ns3/lora-phy-helper.h"
#include "ns3/lora-mac-helper.h"
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
#include "ns3/random-variable-stream.h"
#include "ns3/config.h"
#include "ns3/rectangle.h"
#include "ns3/hex-grid-position-allocator.h"

#include <numeric>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("ModelComparison");

int main (int argc, char *argv[])
{

  int radius = 1000;
  double lambda = 1;
  int appPeriod = 1200;

  CommandLine cmd;
  cmd.AddValue ("radius", "Radius in which to place devices", radius);
  cmd.AddValue ("lambda", "App-layer traffic intensity", lambda);
  cmd.AddValue ("MType", "ns3::EndDeviceLorawanMac::MType");
  cmd.AddValue ("MaxTransmissions",
                "ns3::EndDeviceLorawanMac::MaxTransmissions");
  cmd.Parse (argc, argv);

  int nDevices = lambda * appPeriod;

  // Logging
  //////////

  // LogComponentEnable ("ModelComparison", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraInterferenceHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraPacketTracker", LOG_LEVEL_ALL);
  // LogComponentEnable ("SimpleGatewayLoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable ("EndDeviceLorawanMac", LOG_LEVEL_ALL);
  // LogComponentEnable ("NetworkServer", LOG_LEVEL_ALL);
  // LogComponentEnable ("NetworkController", LOG_LEVEL_ALL);
  // LogComponentEnable ("GatewayLorawanMac", LOG_LEVEL_ALL);
  // LogComponentEnable ("NetworkScheduler", LOG_LEVEL_ALL);
  // LogComponentEnable ("NetworkStatus", LOG_LEVEL_ALL);
  // LogComponentEnable ("EndDeviceStatus", LOG_LEVEL_ALL);
  // LogComponentEnable ("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraMacHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraPacketTracker", LOG_LEVEL_ALL);
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
  mobilityEd.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                   "rho", DoubleValue (radius),
                                   "X", DoubleValue (0.0),
                                   "Y", DoubleValue (0.0));

  // Gateway mobility
  Ptr<ListPositionAllocator> positionAllocGw = CreateObject<ListPositionAllocator> ();
  positionAllocGw->Add (Vector (0.0, 0.0, 15.0));
  mobilityGw.SetPositionAllocator (positionAllocGw);
  mobilityGw.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LoraMacHelper
  LoraMacHelper macHelper = LoraMacHelper ();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();
  helper.EnablePacketTracking ();

  ////////////////
  // Create GWs //
  ////////////////

  NodeContainer gateways;
  gateways.Create (1);
  mobilityGw.Install (gateways);

  // Create the LoraNetDevices of the gateways
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LoraMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  ////////////////
  // Create EDs //
  ////////////////

  NodeContainer endDevices;
  endDevices.Create (nDevices);

  // Install mobility model on fixed nodes
  mobilityEd.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityEd.Install (endDevices);

  // Create a LoraDeviceAddressGenerator
  uint8_t nwkId = 54;
  uint32_t nwkAddr = 1864;
  Ptr<LoraDeviceAddressGenerator> addrGen = CreateObject<LoraDeviceAddressGenerator> (nwkId,nwkAddr);

  // Create the LoraNetDevices of the end devices
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LoraMacHelper::ED);
  macHelper.SetAddressGenerator (addrGen);
  macHelper.SetRegion (LoraMacHelper::EU);
  helper.Install (phyHelper, macHelper, endDevices);

  // Install applications in EDs
  PeriodicSenderHelper appHelper = PeriodicSenderHelper ();
  appHelper.SetPeriod (Seconds (appPeriod));
  ApplicationContainer appContainer = appHelper.Install (endDevices);

  ////////////
  // Create NS
  ////////////

  NodeContainer networkServers;
  networkServers.Create (1);

  // Install the NetworkServer application on the network server
  NetworkServerHelper networkServerHelper;
  networkServerHelper.SetGateways (gateways);
  networkServerHelper.SetEndDevices (endDevices);
  networkServerHelper.EnableAdr (false);
  networkServerHelper.Install (networkServers);

  // Install the Forwarder application on the gateways
  ForwarderHelper forwarderHelper;
  forwarderHelper.Install (gateways);

  // Initialize the spreading factors of the nodes
   // std::vector<int> quantities = macHelper.SetSpreadingFactorsUp
   //  (endDevices, gateways, channel);
  std::vector<double> distribution;
   distribution.push_back(0.25);
   distribution.push_back(0);
   distribution.push_back(0.1);
   distribution.push_back(0.25);
   distribution.push_back(0.4);
   distribution.push_back(0.5);
   std::vector<int> quantities = macHelper.SetSpreadingFactorsGivenDistribution
    (endDevices, gateways, distribution);
  std::cout << "Number of devices for each SF: (the last value stands for devices out of range)" << std::endl;
  for (auto i : quantities)
    {
      std::cout << i << " ";
    }
  std::cout << std::endl;

  helper.DoPrintDeviceStatus (endDevices, gateways, "drs.txt");

  LoraPacketTracker& tracker = helper.GetPacketTracker ();

  // Start simulation
  int transient = 0;
  Time simulationTime = Seconds(appPeriod) * (1 + 2 * transient);
  Simulator::Stop (simulationTime);
  Simulator::Run ();
  Simulator::Destroy ();

  // PHY layer packets
  std::cout << "Performance: " << std::endl;
  std::cout << tracker.PrintPhyPacketsPerGw
    (Seconds(appPeriod * transient), simulationTime -
     Seconds(appPeriod * transient), 0) << std::endl;
  std::cout << tracker.CountMacPacketsGlobally
    (Seconds(appPeriod * transient),
     simulationTime -
     Seconds(appPeriod * transient))
            << std::endl;
  std::cout << tracker.CountMacPacketsGloballyCpsr
    (Seconds(appPeriod * transient),
     simulationTime -
     Seconds(appPeriod * transient))
            << std::endl;

  return 0;
}
