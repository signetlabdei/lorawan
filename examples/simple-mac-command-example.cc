/*
 * This script simulates a simple network in which one end device sends one
 * packet to the gateway. 
 * Then, a packet with a MAC command is manually created and sent by the gateway
 * to the end device, in order to set a different channel for uplink transmission. 
 * Its effect is then verified by making the end device send another packet after some seconds.
 */

#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/end-device-lora-mac.h"
#include "ns3/gateway-lora-mac.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/position-allocator.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/command-line.h"
#include <algorithm>
#include <ctime>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimpleLorawanNetworkExample");

int main (int argc, char *argv[])
{

  // Set up logging
  LogComponentEnable ("SimpleLorawanNetworkExample", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraChannel", LOG_LEVEL_INFO);
  LogComponentEnable ("LoraPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLoraPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraInterferenceHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraMac", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLoraMac", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLoraMac", LOG_LEVEL_ALL);
  LogComponentEnable ("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("LogicalLoraChannel", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraPhyHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraMacHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("OneShotSenderHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("OneShotSender", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraMacHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraFrameHeader", LOG_LEVEL_ALL);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);


  /************************
  *  Create the channel  *
  ************************/

  NS_LOG_INFO ("Creating the channel...");

  // Create the lora channel object
  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 8.1);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);


  /************************
  *  Create the helpers  *
  ************************/

  NS_LOG_INFO ("Setting up helpers...");

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator> ();
  // Position of the end device
  allocator->Add (Vector (5,0,0)); 
  // Position of the gateway 
  allocator->Add (Vector (0,0,0));
  mobility.SetPositionAllocator (allocator);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LoraMacHelper
  LoraMacHelper macHelper = LoraMacHelper ();

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();


  /************************
  *  Create End Devices  *
  ************************/

  NS_LOG_INFO ("Creating the end device...");

  // Create a set of nodes
  NodeContainer endDevices;
  endDevices.Create (1);

  // Assign a mobility model to the node
  mobility.Install (endDevices);

  // Create the LoraNetDevices of the end devices
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LoraMacHelper::ED);
  helper.Install (phyHelper, macHelper, endDevices);

  
  uint32_t id= endDevices.Get(0)->GetId();
  Vector pos= endDevices.Get(0)->GetObject<MobilityModel>()->GetPosition();

  NS_LOG_DEBUG ("End device id: " << id);
  NS_LOG_DEBUG ("End device position: " << pos);

  NS_LOG_DEBUG ("End device successfully created with PHY, MAC, mobility model. \n ");


  /*********************
  *  Create Gateways  *
  *********************/

  NS_LOG_INFO ("Creating the gateway...");
  NodeContainer gateways;
  gateways.Create (1);

  mobility.Install (gateways);

  // Create a netdevice for each gateway
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LoraMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  NS_LOG_DEBUG ("Gateway successfully created with PHY, MAC, mobility model. \n ");


  /***************************************
  *  Set DataRate according to rx power  *
  ****************************************/

  macHelper.SetSpreadingFactorsUp(endDevices, gateways, channel);


  /*********************************************
  *  Install applications on the end devices   *
  **********************************************/

  OneShotSenderHelper oneShotSenderHelper;
  // Setting the time for the first packet
  oneShotSenderHelper.SetSendTime (Seconds (2));
  oneShotSenderHelper.Install (endDevices);
  // Setting the time for the second packet
  oneShotSenderHelper.SetSendTime (Seconds (8));
  oneShotSenderHelper.Install (endDevices);



  /*******************************
   *   Building downlink packet  *    
   *******************************/
 
  // Setting ED's address
  LoraDeviceAddress addr= LoraDeviceAddress(123);   
  Ptr<LoraMac> edMac= endDevices.Get(0)->GetDevice(0)->GetObject<LoraNetDevice>()->GetMac();
  Ptr<EndDeviceLoraMac> edLoraMac = edMac->GetObject<EndDeviceLoraMac>();
  edLoraMac-> SetDeviceAddress(addr);

  // Creating packet for downlink transmussion
  NS_LOG_INFO ("Creating Packet for Downlink transmission...");

  Ptr<Packet> reply= Create<Packet>(5);


  // Setting frame header
  LoraFrameHeader frameHdr;
  frameHdr.SetAsDownlink();
  frameHdr.SetAddress(addr);    // indirizzo ED dst
  frameHdr.SetAdr(true);        // ADR flag
  //frameHdr.SetFPort(0);       // FPort=0 when there are only MAC commands. 
                                // This instruction not necessary because it is 0 by default
  
  // Parameters of the Link ADR Request
  uint8_t dataRate = 0;
  uint8_t txPower = 1;
  int repetitions = 1; 

  // List of the enabled channel.
  // Channels are called by indexes; the mandatory first three channels are 
  // implemented in this code (indexes 0, 1, 2)
  std::list<int> enabled_channels;        
  //enabled_channels.push_back(0);
  enabled_channels.push_back(1);
  //enabled_channels.push_back(2);

  frameHdr.AddLinkAdrReq(dataRate, txPower, enabled_channels, repetitions);
  reply->AddHeader(frameHdr);
  NS_LOG_INFO ("Added frame header of size " << frameHdr.GetSerializedSize () << " bytes");


  // Setting Mac header
  LoraMacHeader macHdr;
  macHdr.SetMType(LoraMacHeader::UNCONFIRMED_DATA_DOWN);
  reply->AddHeader(macHdr);

  NS_LOG_INFO ("\n Setting parameters for Downlink Transmission...");

  // The spreading factor has been set manually, looking at the results of the previous transmision
  LoraTxParameters params;
  params.sf = 7;
  params.headerDisabled = 1;
  params.codingRate = 1;
  params.bandwidthHz =  125000; 
  params.nPreamble = 8;
  params.crcEnabled = 1;
  params.lowDataRateOptimizationEnabled = 0;


  Ptr<LoraPhy> gwPhy = gateways.Get(0)->GetDevice(0)->GetObject<LoraNetDevice>()->GetPhy();

  // In this implementation the end device open its first receive window 1 second after the transmission.
  // Scheduling sending of the reply packet after and giving the inputs for function "Send", The frequency has
  // been set looking at the frequency of the previous uplink transmission.
  Simulator::Schedule(Seconds(3.1), &LoraPhy::Send, gwPhy, reply, params, 868.1, 27);


  /****************
  *  Simulation  *
  ****************/

  Simulator::Stop (Hours (1));

  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
