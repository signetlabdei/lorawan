/*
 * This script simulates a complex scenario with multiple gateways and end
 * devices. The metric of interest for this script is the throughput of the
 * network.
 */

#include "ns3/callback.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/node-container.h"
#include "ns3/mobility-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/double.h"
#include "ns3/random-variable-stream.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/command-line.h"
#include "ns3/network-server-helper.h"
#include "ns3/correlated-shadowing-propagation-loss-model.h"
#include "ns3/building-penetration-loss.h"
#include "ns3/building-allocator.h"
#include "ns3/buildings-helper.h"
#include "ns3/forwarder-helper.h"
#include <algorithm>
#include <ctime>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("FrameCounterUpdateExample");

// Network settings
int nGateways = 1;
double simulationTime = 3600;

void
OnPhySentPacket (Ptr<const Packet> packet, uint32_t index)
{
  Ptr<Packet> packetCopy = packet->Copy();

  LorawanMacHeader mHdr;
  packetCopy->RemoveHeader (mHdr);
  LoraFrameHeader fHdr;
  packetCopy->RemoveHeader (fHdr);

  NS_LOG_DEBUG ("Sent a packet with Frame Counter " << fHdr.GetFCnt());
  // NS_LOG_DEBUG ("MAC Header:");
  // NS_LOG_DEBUG (mHdr);
  // NS_LOG_DEBUG ("Frame Header:");
  // NS_LOG_DEBUG (fHdr);
}

void
OnMacPacketOutcome (uint8_t transmissions, bool successful, Time firstAttempt, Ptr<Packet> packet)
{
  if (successful)
  {
    NS_LOG_INFO ("Packet was successful");
  }
  else
  {
    NS_LOG_INFO ("Giving up");
  }
}

void
ChangeEndDevicePosition (Ptr<Node> endDevice, bool inRange)
{
  if (inRange)
    {
      NS_LOG_INFO ("Moving ED in range");
      endDevice->GetObject<MobilityModel> ()->SetPosition (Vector(0.0, 0.0, 0.0));
    }
  else
    {
      NS_LOG_INFO ("Moving ED out of range");
      endDevice->GetObject<MobilityModel> ()->SetPosition (Vector(10000.0, 0.0, 0.0));
    }
}

int
main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.AddValue ("simulationTime", "The time for which to simulate", simulationTime);
  cmd.AddValue ("MaxTransmissions",
                "ns3::EndDeviceLorawanMac::MaxTransmissions");
  cmd.AddValue ("MType",
                "ns3::EndDeviceLorawanMac::MType");
  cmd.Parse (argc, argv);

  // Set up logging
  LogComponentEnable ("FrameCounterUpdateExample", LOG_LEVEL_ALL);

  /***********
   *  Setup  *
   ***********/

  // Mobility
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator> ();
  // Make it so that nodes are at a certain height > 0
  allocator->Add (Vector (100000.0, 0.0, 15.0)); // ED position
  allocator->Add (Vector (0.0, 0.0, 15.0)); // GW position
  mobility.SetPositionAllocator (allocator);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  /************************
   *  Create the channel  *
   ************************/

  // Create the lora channel object
  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

  /************************
   *  Create the helpers  *
   ************************/

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LorawanMacHelper
  LorawanMacHelper macHelper = LorawanMacHelper ();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();
  helper.EnablePacketTracking (); // Output filename

  //Create the NetworkServerHelper
  NetworkServerHelper nsHelper = NetworkServerHelper ();

  //Create the ForwarderHelper
  ForwarderHelper forHelper = ForwarderHelper ();

  /************************
   *  Create End Devices  *
   ************************/

  // Create a set of nodes
  NodeContainer endDevices;
  endDevices.Create (1);

  // Assign a mobility model to each node
  mobility.Install (endDevices);

  // Make it so that nodes are at a certain height > 0
  for (NodeContainer::Iterator j = endDevices.Begin (); j != endDevices.End (); ++j)
    {
      Ptr<MobilityModel> mobility = (*j)->GetObject<MobilityModel> ();
      Vector position = mobility->GetPosition ();
      position.z = 1.2;
      mobility->SetPosition (position);
    }

  // Create the LoraNetDevices of the end devices
  uint8_t nwkId = 54;
  uint32_t nwkAddr = 1864;
  Ptr<LoraDeviceAddressGenerator> addrGen =
      CreateObject<LoraDeviceAddressGenerator> (nwkId, nwkAddr);

  // Create the LoraNetDevices of the end devices
  macHelper.SetAddressGenerator (addrGen);
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LorawanMacHelper::ED_A);
  macHelper.Set("DataRate", UintegerValue(5));
  helper.Install (phyHelper, macHelper, endDevices);

  // Connect trace sources
  for (NodeContainer::Iterator j = endDevices.Begin (); j != endDevices.End (); ++j)
    {
      Ptr<Node> node = *j;
      Ptr<LoraNetDevice> loraNetDevice = node->GetDevice (0)->GetObject<LoraNetDevice> ();
      Ptr<LoraPhy> phy = loraNetDevice->GetPhy ();
      Ptr<EndDeviceLorawanMac> mac = loraNetDevice->GetMac ()->GetObject<EndDeviceLorawanMac> ();
      phy->TraceConnectWithoutContext("StartSending", MakeCallback(&OnPhySentPacket));
      mac->TraceConnectWithoutContext("RequiredTransmissions", MakeCallback(&OnMacPacketOutcome));
    }

  // Create the gateway nodes (allocate them uniformely on the disc)
  NodeContainer gateways;
  gateways.Create (nGateways);

  // Make it so that nodes are at a certain height > 0
  mobility.SetPositionAllocator (allocator);
  mobility.Install (gateways);

  // Create a netdevice for each gateway
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LorawanMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  NS_LOG_INFO ("Completed configuration");

  /*********************************************
   *  Install applications on the end devices  *
   *********************************************/

  Time appStopTime = Seconds (simulationTime);
  OneShotSenderHelper appHelper = OneShotSenderHelper ();
  appHelper.SetSendTime (Seconds (0));
  ApplicationContainer appContainer = appHelper.Install (endDevices);
  appHelper.SetSendTime (Seconds (100));
  appContainer.Add(appHelper.Install (endDevices));
  appHelper.SetSendTime (Seconds (200));
  appContainer.Add(appHelper.Install (endDevices));

  appContainer.Start (Seconds (0));
  appContainer.Stop (appStopTime);

  Simulator::Schedule (Seconds (110), &ChangeEndDevicePosition, endDevices.Get(0), true);
  Simulator::Schedule (Seconds (201), &ChangeEndDevicePosition, endDevices.Get(0), false);
  Simulator::Schedule (Seconds (204), &ChangeEndDevicePosition, endDevices.Get(0), true);

  /**************************
   *  Create Network Server  *
   ***************************/

  // Create the NS node
  NodeContainer networkServer;
  networkServer.Create (1);

  // Create a NS for the network
  nsHelper.SetEndDevices (endDevices);
  nsHelper.SetGateways (gateways);
  nsHelper.Install (networkServer);

  //Create a forwarder for each gateway
  forHelper.Install (gateways);

  ////////////////
  // Simulation //
  ////////////////

  Simulator::Stop (appStopTime + Hours (1));

  NS_LOG_INFO ("Running simulation...");
  Simulator::Run ();

  Simulator::Destroy ();

  ///////////////////////////
  // Print results to file //
  ///////////////////////////

  LoraPacketTracker &tracker = helper.GetPacketTracker ();
  NS_LOG_INFO ("Printing total sent MAC-layer packets and successful MAC-layer packets");
  std::cout << tracker.CountMacPacketsGlobally (Seconds (0), appStopTime + Hours (1)) << std::endl;

  return 0;
}
