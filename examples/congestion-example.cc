/*
 * This program creates a network which uses congestion control.
 */

// ns3 imports
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/mobility-helper.h"
#include "ns3/okumura-hata-propagation-loss-model.h"
#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/string.h"
#include "ns3/log.h"

// lorawan imports
#include "ns3/lora-helper.h"
#include "ns3/lora-phy-helper.h"
#include "ns3/lorawan-mac-helper.h"
#include "ns3/network-server-helper.h"
#include "ns3/forwarder-helper.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/lora-channel.h"
#include "ns3/lora-device-address-generator.h"
#include "ns3/hex-grid-position-allocator.h"
#include "ns3/range-position-allocator.h"
#include "ns3/lora-interference-helper.h"
#include "example-utils.cc"

// cpp imports
#include <unordered_map>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("CongestionExample");

int
main (int argc, char *argv[])
{

  /***************************
   *  Simulation parameters  *
   ***************************/

  int periods = 24; // H * D
  int gatewayRings = 1;
  double range = 2540.25; // Max range to have coverage probability > 0.98 (with okumura)
  int nDevices = 100;
  std::string sir = "GOURSAUD";
  bool adrEnabled = false;
  bool initializeSF = true;
  bool model = false;
  int beta = 1;
  bool congest = false;
  double warmup = 2;
  double sampling = 2;
  double variance = 0.01;
  double tolerance = 0.001;
  double target = 0.95;
  std::string clusterStr;
  bool debug = false;
  bool file = false;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("periods", "Number of periods to simulate (1 period = 1 hour)", periods);
  cmd.AddValue ("rings", "Number of gateway rings in hexagonal topology", gatewayRings);
  cmd.AddValue ("range", "Radius of the device allocation disk around a gateway)", range);
  cmd.AddValue ("devices", "Number of end devices to include in the simulation", nDevices);
  cmd.AddValue ("sir", "Signal to Interference Ratio matrix used for interference", sir);
  cmd.AddValue ("initSF", "Whether to initialize the SFs", initializeSF);
  cmd.AddValue ("adr", "Whether to enable online ADR", adrEnabled);
  cmd.AddValue ("model", "Use static duty-cycle config with capacity model", model);
  cmd.AddValue ("beta", "[static ctrl] Scaling factor of the static model output", beta);
  cmd.AddValue ("congest", "Use congestion control", congest);
  cmd.AddValue ("warmup",
                "[cong ctrl] Starting delay of the congestion control algorithm for "
                "initial network warm-up (e.g. ADR) and RSSI measurements collection",
                warmup);
  cmd.AddValue ("sampling", "[cong ctrl] Duration (hours) of the PDR sampling fase", sampling);
  cmd.AddValue ("variance", "[cong ctrl] Acceptable gap around the target PDR value", variance);
  cmd.AddValue ("tolerance", "[cong ctrl] Tolerance step to declare value stagnation", tolerance);
  cmd.AddValue ("target", "[ctrl] Central PDR value targeted (single cluster)", target);
  cmd.AddValue ("clusters", "[ctrl] Clusters descriptor: \"{{share,pdr},...\"}", clusterStr);
  cmd.AddValue ("debug", "Whether or not to debug logs at various levels. ", debug);
  cmd.AddValue ("file", "Output the metrics of the simulation in a file", file);
  cmd.Parse (argc, argv);
  NS_ASSERT (!(congest & model));

  // Static configurations
  Config::SetDefault ("ns3::EndDeviceLorawanMac::DRControl", BooleanValue (true)); //!< ADR bit
  Config::SetDefault ("ns3::EndDeviceLorawanMac::MType", StringValue ("Unconfirmed"));
  Config::SetDefault ("ns3::EndDeviceLorawanMac::MaxTransmissions", IntegerValue (1));
  Config::SetDefault ("ns3::AdrComponent::MultipleGwCombiningMethod", StringValue ("max"));
  Config::SetDefault ("ns3::AdrComponent::MultiplePacketsCombiningMethod", StringValue ("avg"));
  Config::SetDefault ("ns3::AdrComponent::HistoryRange", IntegerValue (20));
  Config::SetDefault ("ns3::AdrComponent::ChangeTransmissionPower", BooleanValue (true));
  Config::SetDefault ("ns3::AdrComponent::SNRDeviceMargin",
                      DoubleValue (10 * log10 (-1 / log (0.98))));
  Config::SetDefault ("ns3::CongestionControlComponent::StartTime", TimeValue (Hours (warmup)));
  Config::SetDefault ("ns3::CongestionControlComponent::SamplingDuration",
                      TimeValue (Hours (sampling)));
  Config::SetDefault ("ns3::CongestionControlComponent::AcceptedPDRVariance",
                      DoubleValue (variance));
  Config::SetDefault ("ns3::CongestionControlComponent::ValueStagnationTolerance",
                      DoubleValue (tolerance));

  /************
   *  Logging *
   ************/

  if (debug) // This also requires to build ns3 with debug option
    {
      LogComponentEnable ("CongestionControlComponent", LOG_LEVEL_DEBUG);
      LogComponentEnable ("TrafficControlUtils", LOG_LEVEL_DEBUG);
      LogComponentEnableAll (LOG_PREFIX_FUNC);
      LogComponentEnableAll (LOG_PREFIX_NODE);
      LogComponentEnableAll (LOG_PREFIX_TIME);
    }

  /******************
   *  Radio Channel *
   ******************/
  LoraInterferenceHelper::collisionMatrix = sirMap.at (sir);

  // Delay obtained from distance and speed of light in vacuum (constant)
  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  // This one is empirical and it encompasses average loss due to distance, shadowing (i.e. obstacles), weather, height
  Ptr<OkumuraHataPropagationLossModel> loss = CreateObject<OkumuraHataPropagationLossModel> ();
  loss->SetAttribute ("Frequency", DoubleValue (868000000.0));
  loss->SetAttribute ("Environment", EnumValue (EnvironmentType::UrbanEnvironment));
  loss->SetAttribute ("CitySize", EnumValue (CitySize::LargeCity));

  // Here we can add variance to the propagation model with multipath Rayleigh fading
  Ptr<NakagamiPropagationLossModel> rayleigh = CreateObject<NakagamiPropagationLossModel> ();
  rayleigh->SetAttribute ("m0", DoubleValue (1.0));
  rayleigh->SetAttribute ("m1", DoubleValue (1.0));
  rayleigh->SetAttribute ("m2", DoubleValue (1.0));

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

  /**************
   *  Mobility  *
   **************/

  MobilityHelper mobilityEd, mobilityGw;

  // Gateway mobility
  mobilityGw.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // In hex tiling, distance = range * cos (pi/6) * 2 to have no holes
  double gatewayDistance = range * std::cos (M_PI / 6) * 2;
  Ptr<HexGridPositionAllocator> hexAllocator = CreateObject<HexGridPositionAllocator> ();
  hexAllocator->SetAttribute ("Z", DoubleValue (15.0));
  hexAllocator->SetAttribute ("distance", DoubleValue (gatewayDistance));
  mobilityGw.SetPositionAllocator (hexAllocator);

  // End Device mobility
  mobilityEd.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  // We define rho to generalize the allocation disk for any number of gateway rings
  double rho = range + 2.0 * gatewayDistance * (gatewayRings - 1);
  Ptr<RangePositionAllocator> rangeAllocator = CreateObject<RangePositionAllocator> ();
  rangeAllocator->SetAttribute ("rho", DoubleValue (rho));
  rangeAllocator->SetAttribute ("Z", DoubleValue (15.0));
  rangeAllocator->SetAttribute ("range", DoubleValue (range));
  mobilityEd.SetPositionAllocator (rangeAllocator);

  /*************
   *  Helpers  *
   *************/

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);
  phyHelper.SetDuplexMode (!model);

  // Create the LorawanMacHelper
  LorawanMacHelper macHelper = LorawanMacHelper ();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();
  helper.EnablePacketTracking ();

  /******************
   *  Create Nodes  *
   ******************/

  NodeContainer networkServer;
  networkServer.Create (1);

  NodeContainer gateways;
  int nGateways = 3 * gatewayRings * gatewayRings - 3 * gatewayRings + 1;
  gateways.Create (nGateways);
  mobilityGw.Install (gateways);
  rangeAllocator->SetNodes (gateways);

  NodeContainer endDevices;
  endDevices.Create (nDevices);
  mobilityEd.Install (endDevices);

  /************************
   *  Create Net Devices  *
   ************************/

  // Create a LoraDeviceAddressGenerator
  uint8_t nwkId = 54;
  uint32_t nwkAddr = 1864;
  Ptr<LoraDeviceAddressGenerator> addrGen =
      CreateObject<LoraDeviceAddressGenerator> (nwkId, nwkAddr);

  // Create the LoraNetDevices of the gateways
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LorawanMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  // Create the LoraNetDevices of the end devices
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LorawanMacHelper::ED_A);
  macHelper.SetAddressGenerator (addrGen);
  macHelper.SetRegion (LorawanMacHelper::EU);
  helper.Install (phyHelper, macHelper, endDevices);

  /*************************
   *  Create Applications  *
   *************************/

  // Set clusters
  cluster_t clusters =
      (clusterStr.empty ()) ? cluster_t ({{100.0, target}}) : ParseClusterInfo (clusterStr);

  // Install the NetworkServer application on the network server
  NetworkServerHelper networkServerHelper;
  networkServerHelper.SetGateways (gateways);
  networkServerHelper.SetEndDevices (endDevices);
  networkServerHelper.EnableAdr (adrEnabled);
  networkServerHelper.EnableCongestionControl (congest);
  networkServerHelper.AssignClusters (clusters); // Assignes one freq. by default
  networkServerHelper.Install (networkServer);

  // Install the Forwarder application on the gateways
  // !!!! THIS MUST REMAIN AFTER SERVER INSTALL.
  // ServerHelper.Install creates the p2p device needed by the app
  ForwarderHelper forwarderHelper;
  forwarderHelper.Install (gateways);

  // Install applications in EDs
  PeriodicSenderHelper appHelper = PeriodicSenderHelper ();
  appHelper.SetPeriodGenerator (CreateObjectWithAttributes<NormalRandomVariable> (
      "Mean", DoubleValue (600.0), "Variance", DoubleValue (300.0), "Bound", DoubleValue (600.0)));
  appHelper.SetPacketSizeGenerator (CreateObjectWithAttributes<NormalRandomVariable> (
      "Mean", DoubleValue (18), "Variance", DoubleValue (10), "Bound", DoubleValue (18)));
  appHelper.Install (endDevices);

  // Initialize SF emulating the ADR algorithm, then add variance to path loss
  std::vector<int> devPerSF (1, nDevices);
  if (initializeSF)
    devPerSF = macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);
  //! Here is the point where we allocate CHANNELS in case
  if (model)
    macHelper.SetDutyCyclesWithCapacityModel (endDevices, gateways, channel, clusters, beta);
  loss->SetNext (rayleigh);

  /***************************
   *  Simulation and metrics *
   ***************************/

  if (file)
    {
      // Activate printing of ED MAC parameters
      Time statusSamplePeriod = Seconds (1800);
      helper.EnablePeriodicDeviceStatusPrinting (endDevices, gateways, "nodeData.txt",
                                                 statusSamplePeriod);
      helper.EnablePeriodicGlobalPerformancePrinting ("globalPerformance.txt", statusSamplePeriod);
    }

  // Limit memory usage
  LoraPacketTracker &tracker = helper.GetPacketTracker ();
  tracker.EnableOldPacketsCleanup (Hours (1));

  if (debug)
    {
      // Print current configuration
      PrintConfigSetup (nDevices, range, gatewayRings, devPerSF);
      helper.EnableSimulationTimePrinting (Seconds (3600));
    }

  // Start simulation
  Time periodLenght = Hours (1);
  Simulator::Stop (periodLenght * periods);
  Simulator::Run ();

  Time trackFinalOutcomeFrom = periodLenght * periods - Hours (10);
  if (debug)
    std::cout << tracker.PrintSimulationStatistics (trackFinalOutcomeFrom);

  Simulator::Destroy ();
  return 0;
}
