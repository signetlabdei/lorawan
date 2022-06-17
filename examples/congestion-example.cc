/*
 * This program creates a network which uses a ADR and congestion control.
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
#include "ns3/random-variable-stream.h"
#include "ns3/config.h"
#include "ns3/rectangle.h"
#include "ns3/hex-grid-position-allocator.h"
#include "ns3/okumura-hata-propagation-loss-model.h"
#include "ns3/range-position-allocator.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("CongestionExample");

// Trace sources that are called when a node changes its DR or TX power
void
OnDataRateChange (uint8_t oldDr, uint8_t newDr)
{
  NS_LOG_DEBUG ("DR" << unsigned (oldDr) << " -> DR" << unsigned (newDr));
}
void
OnTxPowerChange (double oldTxPower, double newTxPower)
{
  NS_LOG_DEBUG (oldTxPower << " dBm -> " << newTxPower << " dBm");
}

int
main (int argc, char *argv[])
{

  /***************************
   *  Simulation parameters  *
   ***************************/

  int periods = 24; // H * D
  int gatewayRings = 1;
  double range = 2540.25; // Max range to have coverage probability > 0.98 (with okumura)
  int nDevices = 1;

  bool adrEnabled = true;
  bool initializeSF = false;
  bool congest = false;

  bool debug = false;
  bool file = false;
  int run_number = 0;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("periods", "Number of periods to simulate", periods);
  cmd.AddValue ("rings", "Number of gateway rings in hexagonal topology", gatewayRings);
  cmd.AddValue ("range", "Radius of the device allocation disk around a gateway)", range);
  cmd.AddValue ("devices", "Number of end devices to include in the simulation", nDevices);
  cmd.AddValue ("initSF", "Whether to initialize the SFs", initializeSF);
  cmd.AddValue ("adr", "Whether to enable ADR", adrEnabled);
  cmd.AddValue ("congest", "Use congestion control", congest);
  cmd.AddValue ("debug", "Whether or not to debug logs at various levels. ", debug);
  cmd.AddValue ("file", "Output a metrics of the simulation in a file", file);
  cmd.AddValue ("run_number", "Run number for repeated simulations", run_number);
  cmd.Parse (argc, argv);

  // Static configurations
  Time periodLenght = Hours (1);
  std::string adrType = "ns3::AdrComponent";

  Config::SetDefault ("ns3::EndDeviceLorawanMac::DRControl", BooleanValue (true)); //!< ADR bit
  Config::SetDefault ("ns3::EndDeviceLorawanMac::EnableEDDataRateAdaptation",
                      BooleanValue (true)); //!< ADR backoff
  Config::SetDefault ("ns3::EndDeviceLorawanMac::MType", StringValue ("Unconfirmed"));
  Config::SetDefault ("ns3::EndDeviceLorawanMac::MaxTransmissions", IntegerValue (1));

  Config::SetDefault ("ns3::AdrComponent::MultipleGwCombiningMethod", StringValue ("max"));
  Config::SetDefault ("ns3::AdrComponent::MultiplePacketsCombiningMethod", StringValue ("avg"));
  Config::SetDefault ("ns3::AdrComponent::HistoryRange", IntegerValue (20));
  Config::SetDefault ("ns3::AdrComponent::ChangeTransmissionPower", BooleanValue (true));
  Config::SetDefault ("ns3::AdrComponent::SNRDeviceMargin",
                      DoubleValue (10 * log10 (-1 / log (0.98))));

  /************
   *  Logging *
   ************/

  //LogComponentEnable ("CongestionExample", LOG_LEVEL_ALL);
  //LogComponentEnable ("PeriodicSenderHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("HexGridPositionAllocator", LOG_LEVEL_ALL);
  //LogComponentEnable ("PeriodicSender", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraPacketTracker", LOG_LEVEL_ALL);
  //LogComponentEnable ("Forwarder", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraChannel", LOG_LEVEL_ALL);
  //LogComponentEnable ("LoraInterferenceHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("NetworkServer", LOG_LEVEL_ALL);
  //LogComponentEnable ("NetworkController", LOG_LEVEL_ALL);
  //LogComponentEnable ("NetworkScheduler", LOG_LEVEL_ALL);
  //LogComponentEnable ("NetworkStatus", LOG_LEVEL_ALL);
  //LogComponentEnable ("EndDeviceStatus", LOG_LEVEL_ALL);
  //LogComponentEnable ("AdrComponent", LOG_LEVEL_ALL);
  //LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);
  //LogComponentEnable ("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
  //LogComponentEnable ("MacCommand", LOG_LEVEL_ALL);
  //LogComponentEnable ("AdrExploraSf", LOG_LEVEL_ALL);
  //LogComponentEnable ("AdrExploraAt", LOG_LEVEL_ALL);
  //LogComponentEnable ("EndDeviceLorawanMac", LOG_LEVEL_ALL);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  /******************
   *  Radio Channel *
   ******************/

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

  loss->SetNext (rayleigh);

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

  /********************
   *  Create NetDevs  *
   ********************/

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

  // Install the NetworkServer application on the network server
  NetworkServerHelper networkServerHelper;
  networkServerHelper.SetGateways (gateways);
  networkServerHelper.SetEndDevices (endDevices);
  networkServerHelper.EnableAdr (adrEnabled);
  networkServerHelper.SetAdr (adrType);
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
      "Mean", DoubleValue (31), "Variance", DoubleValue (10), "Bound", DoubleValue (19)));
  appHelper.Install (endDevices);

  // Connect our traces
  Config::ConnectWithoutContext (
      "/NodeList/*/DeviceList/0/$ns3::LoraNetDevice/Mac/$ns3::EndDeviceLorawanMac/TxPower",
      MakeCallback (&OnTxPowerChange));
  Config::ConnectWithoutContext (
      "/NodeList/*/DeviceList/0/$ns3::LoraNetDevice/Mac/$ns3::EndDeviceLorawanMac/DataRate",
      MakeCallback (&OnDataRateChange));

  if (file)
    {
      // Activate printing of ED MAC parameters
      Time stateSamplePeriod = Seconds (1200);
      helper.EnablePeriodicDeviceStatusPrinting (endDevices, gateways, "nodeData.txt",
                                                 stateSamplePeriod);
      helper.EnablePeriodicPhyPerformancePrinting (gateways, "phyPerformance.txt",
                                                   stateSamplePeriod);
      helper.EnablePeriodicGlobalPerformancePrinting ("globalPerformance.txt", stateSamplePeriod);
    }

  LoraPacketTracker &tracker = helper.GetPacketTracker ();

  // Start simulation
  Simulator::Stop (periodLenght * periods);
  Simulator::Run ();

  std::cout << tracker.CountMacPacketsGlobally (Seconds (0), Simulator::Now ()) << std::endl;

  Simulator::Destroy ();
  return 0;
}
