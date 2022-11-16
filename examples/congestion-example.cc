/*
 * This program creates a network which uses congestion control.
 */

// ns3 imports
#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/okumura-hata-propagation-loss-model.h"
#include "ns3/mobility-helper.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/application-container.h"
#include "ns3/propagation-delay-model.h"

// lorawan imports
#include "ns3/lora-channel.h"
#include "ns3/hex-grid-position-allocator.h"
#include "ns3/range-position-allocator.h"
#include "ns3/lora-helper.h"
#include "ns3/network-server-helper.h"
#include "ns3/forwarder-helper.h"
#include "ns3/urban-traffic-helper.h"
#include "utilities.cc"

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
  double range = 2426.85; // Max range for downlink (!) coverage probability > 0.98 (with okumura)
  int nDevices = 100;
  std::string sir = "GOURSAUD";
  bool adrEnabled = false;
  bool initializeSF = true;
  bool model = false;
  int beta = 1;
  bool congest = false;
  double warmup = 0;
  double sampling = 2;
  double target = 0.95;
  std::string clusterStr = "None";
  bool file = false;

  /* Expose parameters to command line */
  {
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
                  "Starting delay of the congestion control algorithm for "
                  "initial network warm-up (e.g. ADR) and RSSI measurements collection "
                  "(ns3::CongestionControlComponent::StartTime)",
                  warmup);
    cmd.AddValue ("sampling",
                  "Duration (hours) of the PDR sampling fase "
                  "(ns3::CongestionControlComponent::SamplingDuration)",
                  sampling);
    cmd.AddValue ("variance", "ns3::CongestionControlComponent::AcceptedPDRVariance");
    cmd.AddValue ("tolerance", "ns3::CongestionControlComponent::ValueStagnationTolerance");
    cmd.AddValue ("cfgfile", "ns3::CongestionControlComponent::LoadConfigFromFile");
    cmd.AddValue ("target", "Central PDR value targeted (single cluster)", target);
    cmd.AddValue ("clusters", "Clusters descriptor: \"{{share,pdr},...\"}", clusterStr);
    cmd.AddValue ("file", "Output the metrics of the simulation in a file", file);
    cmd.Parse (argc, argv);
    NS_ASSERT (!(congest & model));
  }

  /* Apply global configurations */
  {
    Config::SetDefault ("ns3::EndDeviceLorawanMac::DRControl",
                        BooleanValue (adrEnabled)); //!< ADR bit
    Config::SetDefault ("ns3::AdrComponent::SNRDeviceMargin",
                        DoubleValue (10 * log10 (-1 / log (0.98))));
    Config::SetDefault ("ns3::CongestionControlComponent::StartTime", TimeValue (Hours (warmup)));
    Config::SetDefault ("ns3::CongestionControlComponent::SamplingDuration",
                        TimeValue (Hours (sampling)));
  }

  /* Logging options */
  {
    //!> Requirement: build ns3 with debug option
    LogComponentEnable ("CongestionControlComponent", LOG_LEVEL_DEBUG);
    LogComponentEnable ("TrafficControlUtils", LOG_LEVEL_DEBUG);
    LogComponentEnable ("EndDeviceLorawanMac", LOG_LEVEL_WARN);
    //LogComponentEnable ("UrbanTrafficHelper", LOG_LEVEL_DEBUG);
    LogComponentEnableAll (LOG_PREFIX_FUNC);
    LogComponentEnableAll (LOG_PREFIX_NODE);
    LogComponentEnableAll (LOG_PREFIX_TIME);
  }

  /******************
   *  Radio Channel *
   ******************/

  Ptr<OkumuraHataPropagationLossModel> loss;
  Ptr<NakagamiPropagationLossModel> rayleigh;
  Ptr<LoraChannel> channel;
  {
    LoraInterferenceHelper::collisionMatrix = sirMap.at (sir);

    // Delay obtained from distance and speed of light in vacuum (constant)
    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

    // This one is empirical and it encompasses average loss due to distance, shadowing (i.e. obstacles), weather, height
    loss = CreateObject<OkumuraHataPropagationLossModel> ();
    loss->SetAttribute ("Frequency", DoubleValue (868100000.0));
    loss->SetAttribute ("Environment", EnumValue (EnvironmentType::UrbanEnvironment));
    loss->SetAttribute ("CitySize", EnumValue (CitySize::LargeCity));

    // Here we can add variance to the propagation model with multipath Rayleigh fading
    rayleigh = CreateObject<NakagamiPropagationLossModel> ();
    rayleigh->SetAttribute ("m0", DoubleValue (1.0));
    rayleigh->SetAttribute ("m1", DoubleValue (1.0));
    rayleigh->SetAttribute ("m2", DoubleValue (1.0));

    channel = CreateObject<LoraChannel> (loss, delay);
  }

  /**************
   *  Mobility  *
   **************/

  MobilityHelper mobilityEd, mobilityGw;
  Ptr<RangePositionAllocator> rangeAllocator;
  {
    // Gateway mobility
    mobilityGw.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    // In hex tiling, distance = range * cos (pi/6) * 2 to have no holes
    double gatewayDistance = range * std::cos (M_PI / 6) * 2;
    auto hexAllocator = CreateObject<HexGridPositionAllocator> ();
    hexAllocator->SetAttribute ("Z", DoubleValue (30.0));
    hexAllocator->SetAttribute ("distance", DoubleValue (gatewayDistance));
    mobilityGw.SetPositionAllocator (hexAllocator);

    // End Device mobility
    mobilityEd.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    // We define rho to generalize the allocation disk for any number of gateway rings
    double rho = range + 2.0 * gatewayDistance * (gatewayRings - 1);
    rangeAllocator = CreateObject<RangePositionAllocator> ();
    rangeAllocator->SetAttribute ("rho", DoubleValue (rho));
    rangeAllocator->SetAttribute ("ZRV", StringValue ("ns3::UniformRandomVariable[Min=1|Max=10]"));
    rangeAllocator->SetAttribute ("range", DoubleValue (range));
    mobilityEd.SetPositionAllocator (rangeAllocator);
  }

  /******************
   *  Create Nodes  *
   ******************/

  Ptr<Node> server;
  NodeContainer gateways;
  NodeContainer endDevices;
  {
    server = CreateObject<Node> ();

    int nGateways = 3 * gatewayRings * gatewayRings - 3 * gatewayRings + 1;
    gateways.Create (nGateways);
    mobilityGw.Install (gateways);
    rangeAllocator->SetNodes (gateways);

    endDevices.Create (nDevices);
    mobilityEd.Install (endDevices);
  }

  /************************
   *  Create Net Devices  *
   ************************/

  LoraHelper loraHelper;
  LorawanMacHelper macHelper;
  {
    // PointToPoint links between gateways and server
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
    for (auto gw = gateways.Begin (); gw != gateways.End (); ++gw)
      p2p.Install (server, *gw);

    /**
     *  LoRa/LoRaWAN layers 
     */

    // General LoRa settings
    loraHelper.EnablePacketTracking ();

    // Create a LoraDeviceAddressGenerator
    uint8_t nwkId = 54;
    uint32_t nwkAddr = 1864;
    auto addrGen = CreateObject<LoraDeviceAddressGenerator> (nwkId, nwkAddr);

    // Mac layer settings
    macHelper.SetRegion (LorawanMacHelper::EU);
    macHelper.SetAddressGenerator (addrGen);

    // Physiscal layer settings
    LoraPhyHelper phyHelper;
    phyHelper.SetChannel (channel);
    phyHelper.SetDuplexMode (!model);

    // Create the LoraNetDevices of the gateways
    phyHelper.SetDeviceType (LoraPhyHelper::GW);
    macHelper.SetDeviceType (LorawanMacHelper::GW);
    loraHelper.Install (phyHelper, macHelper, gateways);

    // Create the LoraNetDevices of the end devices
    phyHelper.SetDeviceType (LoraPhyHelper::ED);
    macHelper.SetDeviceType (LorawanMacHelper::ED_A);
    loraHelper.Install (phyHelper, macHelper, endDevices);
  }

  /*************************
   *  Create Applications  *
   *************************/

  cluster_t clusters;
  {
    // Set clusters
    clusters =
        (clusterStr == "None") ? cluster_t ({{100.0, target}}) : ParseClusterInfo (clusterStr);

    // Install the NetworkServer application on the network server
    NetworkServerHelper serverHelper;
    serverHelper.SetEndDevices (endDevices); // Registering devices (saves mac layer)
    serverHelper.EnableAdr (adrEnabled);
    serverHelper.EnableCongestionControl (congest);
    serverHelper.AssignClusters (clusters); // Assignes one freq. by default
    serverHelper.Install (server);

    // Install the Forwarder application on the gateways
    ForwarderHelper forwarderHelper;
    forwarderHelper.Install (gateways);

    // Install applications in EDs
    UrbanTrafficHelper appHelper = UrbanTrafficHelper ();
    ApplicationContainer apps = appHelper.Install (endDevices);
    /*   int j = 0; // Late (dis)activation of 100 devices
  for (ApplicationContainer::Iterator i = apps.Begin (); i != apps.End (); ++i)
    {
      if (j >= 50)
        break;
      //(*i)->SetStopTime (Days (4));
      (*i)->SetStartTime (Days (5));
      ++j;
    } */
  }

  /***************************
   *  Simulation and metrics *
   ***************************/

  // Initialize SF emulating the ADR algorithm, then add variance to path loss
  std::vector<int> devPerSF (1, nDevices);
  if (initializeSF)
    devPerSF = macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);
  //! Here is the point where we allocate CHANNELS in case
  if (model)
    macHelper.SetDutyCyclesWithCapacityModel (endDevices, gateways, channel, clusters, beta);
  loss->SetNext (rayleigh);

  if (file)
    {
      // Activate printing of ED MAC parameters
      Time statusSamplePeriod = Minutes (30);
      loraHelper.EnablePeriodicSFStatusPrinting (endDevices, gateways, "sfData.txt",
                                                 statusSamplePeriod);
      loraHelper.EnablePeriodicGlobalPerformancePrinting ("globalPerformance.txt",
                                                          statusSamplePeriod);
    }

  // Limit memory usage
  LoraPacketTracker &tracker = loraHelper.GetPacketTracker ();
  tracker.EnableOldPacketsCleanup (Hours (1));

#ifdef NS3_LOG_ENABLE
  // Print current configuration
  PrintConfigSetup (nDevices, range, gatewayRings, devPerSF);
  loraHelper.EnableSimulationTimePrinting (Hours (2));
#endif // NS3_LOG_ENABLE

  // Start simulation
  Time duration = Hours (1) * periods;
  Simulator::Stop (duration);
  Simulator::Run ();

#ifdef NS3_LOG_ENABLE
  std::cout << tracker.PrintSimulationStatistics (duration - Hours (24));
#endif // NS3_LOG_ENABLE

  Simulator::Destroy ();

  return 0;
}
