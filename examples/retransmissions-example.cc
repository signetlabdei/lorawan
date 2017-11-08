/*
 * This script simulates a simple network in which one end device sends two
 * confirmed packets to the gateway.
 * In the first case, the gateway does not answer with an acknowledgment, causing the
 * to retransmit the packet until it reaches the maximum number of transmissions
 * allowed (here set to 4). This simulates a scenario in which the network server
 * can not answer to the end device or there are packet losses so that the
 * ack is never received by the end device.
 * For the second case, a packet carrying an acknowledgment is manually created
 * and sent by the gateway to the end device after the second transmisson
 * attempt, in the second receive window.
 * Since the ACK is received, the end device stops the retransmission procedure.
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

NS_LOG_COMPONENT_DEFINE ("RetransmissionsExample");

int main (int argc, char *argv[])
{

  // Set up logging
  LogComponentEnable ("RetransmissionsExample", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLoraPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraMac", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLoraMac", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLoraMac", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraPhyHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraMacHelper", LOG_LEVEL_ALL);
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
  loss->SetReference (1, 7.7);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);


  /************************
  *  Create the helpers  *
  ************************/

  NS_LOG_INFO ("Setting up helpers...");

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator> ();
  // Position of the end device
  allocator->Add (Vector (500,0,0));
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


  uint32_t id = endDevices.Get (0)->GetId ();
  Vector pos = endDevices.Get (0)->GetObject<MobilityModel> ()->GetPosition ();

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
  std::vector<int> sfQuantity (6);
  sfQuantity = macHelper.SetSpreadingFactorsUp (endDevices, gateways, channel);



/***************************************************************************************************************************************
***************************************************************************************************************************************/

/*******************************
*   Building uplink packets  *
*******************************/

// First packet

  NS_LOG_INFO ("\n Creating First Packet for Uplink transmission...");

  // Setting ED's address
  LoraDeviceAddress addr = LoraDeviceAddress (2311);
  Ptr<LoraMac> edMac = endDevices.Get (0)->GetDevice (0)->GetObject<LoraNetDevice> ()->GetMac ();
  Ptr<EndDeviceLoraMac> edLoraMac = edMac->GetObject<EndDeviceLoraMac> ();
  edLoraMac->SetDeviceAddress (addr);
  edLoraMac->SetMType (LoraMacHeader::CONFIRMED_DATA_UP);  // this device will send packets requiring ack
  edLoraMac->SetMaxNumberOfTransmissions (4);    // the maximum number of transmissions performed is 4.
  // if after 4 transmissions the ack is not received, the
  // packet is dropped.

  Ptr<Packet> pkt1 = Create<Packet> (5);

  Simulator::Schedule (Seconds (2), &LoraMac::Send, edMac, pkt1);

  NS_LOG_DEBUG ("Sent first confirmed packet");



// Second packet

  NS_LOG_INFO ("\n Creating Second Packet for Uplink transmission...");

  Ptr<Packet> pkt2 = Create<Packet> (8);

  Simulator::Schedule (Seconds (35), &LoraMac::Send, edMac, pkt2);

  NS_LOG_DEBUG (" Sent second confirmed packet ");


  /*******************************
   *   Building downlink packet  *
   *******************************/

  // Creating packet for downlink transmussion
  NS_LOG_INFO ("Creating Packet for Downlink transmission...");

  Ptr<Packet> reply = Create<Packet> (5);


  // Setting frame header
  LoraFrameHeader downframeHdr;
  downframeHdr.SetAsDownlink ();
  downframeHdr.SetAddress (addr);    // indirizzo ED dst
  downframeHdr.SetAck (true);
  //frameHdr.SetFPort(0);       // FPort=0 when there are only MAC commands.
  // This instruction not necessary because it is 0 by default
  reply->AddHeader (downframeHdr);
  NS_LOG_INFO ("Added frame header of size " << downframeHdr.GetSerializedSize () << " bytes");


  // Setting Mac header
  LoraMacHeader downmacHdr;
  downmacHdr.SetMType (LoraMacHeader::UNCONFIRMED_DATA_DOWN);
  reply->AddHeader (downmacHdr);

  NS_LOG_INFO ("\n Setting parameters for Downlink Transmission...");

  // The spreading factor has been set manually, looking at the results of the previous transmision
  LoraTxParameters downparams;
  downparams.sf = 12;
  downparams.headerDisabled = 1;
  downparams.codingRate = 1;
  downparams.bandwidthHz =  125000;
  downparams.nPreamble = 8;
  downparams.crcEnabled = 1;
  downparams.lowDataRateOptimizationEnabled = 0;


  Ptr<LoraPhy> gwPhy = gateways.Get (0)->GetDevice (0)->GetObject<LoraNetDevice> ()->GetPhy ();

  // The end device open its second receive window 2 seconds after the transmission.
  // For educational purposes, we make the end device retransmit its packet.
  // Therefore, the ack reply is scheduled when the end device opens its
  // second receive window after the second transmission.

  // 2nd rx window: freq= 869.525 MHz, SF=12
  Simulator::Schedule (Seconds (44.2), &LoraPhy::Send, gwPhy, reply, downparams, 869.525, 27);


  /****************
  *  Simulation  *
  ****************/

  Simulator::Stop (Hours (1));

  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
