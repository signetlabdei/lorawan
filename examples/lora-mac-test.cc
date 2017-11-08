#include "ns3/end-device-lora-mac.h"
#include "ns3/mobility-helper.h"
#include "ns3/lora-phy-helper.h"
#include "ns3/lora-mac-helper.h"
#include "ns3/lora-helper.h"
#include "ns3/log.h"
#include "ns3/command-line.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LoraMacTest");

int packetsAtGateway;
int packetsAtEndDevice;
double lastKnownLinkMargin;
int lastKnownGatewayCount;
uint8_t endDeviceDataRate;
double endDeviceTxPower;
double endDeviceDutyCycle;
Ptr<Packet> lastPacketReceivedAtGateway;
Ptr<Packet> packet;
Ptr<Packet> otherPacket;
LoraFrameHeader frameHeader;
LoraMacHeader macHeader;
LoraTxParameters txParams;
Ptr<GatewayLoraPhy> gwPhy;
Ptr<EndDeviceLoraPhy> edPhy;
NodeContainer endDevices;
NodeContainer gateways;

void AggregatedDutyCycleUpdate (double oldval, double newval)
{
  NS_LOG_INFO ("Updated m_aggregatedDutyCycle from " << oldval << " to " <<
               newval);

  endDeviceDutyCycle = newval;
}
void LastKnownLinkMarginUpdate (double oldval, double newval)
{
  NS_LOG_INFO ("Updated m_lastKnownLinkMargin from " << oldval << " to " <<
               newval);

  lastKnownLinkMargin = newval;
}

void LastKnownGatewayCountUpdate (int oldval, int newval)
{
  NS_LOG_INFO ("Updated m_lastKnownGatewayCount from " << oldval << " to " <<
               newval);

  lastKnownGatewayCount = newval;
}

void DataRateUpdate (uint8_t oldval, uint8_t newval)
{
  NS_LOG_INFO ("Updated m_dataRate from " << unsigned (oldval) << " to " <<
               unsigned (newval));

  endDeviceDataRate = newval;
}

void TxPowerUpdate (double oldval, double newval)
{
  NS_LOG_INFO ("Updated m_txPower from " << oldval << " to " <<
               newval);

  endDeviceTxPower = newval;
}

void ReceivedPacketAtEndDevice (Ptr<const Packet> packet)
{
  NS_LOG_INFO ("Incrementing number of received packets at End Device");

  packetsAtEndDevice++;
}

void ReceivedPacketAtGateway (Ptr<const Packet> packet)
{
  NS_LOG_INFO ("Incrementing number of received packets at Gateway");

  lastPacketReceivedAtGateway = packet->Copy ();

  // packet->EnablePrinting();
  // packet->Print (std::cout);

  packetsAtGateway++;
}

// Test, through an assert, that the packetsAtGateway variable has a certain
// value
void
CheckReceivedPacketsAtGateway (int nPackets)
{
  NS_LOG_INFO ("Expected: " << nPackets << ", got: " << packetsAtGateway);

  NS_ASSERT (packetsAtGateway == nPackets);
}

// Test, through an assert, that the packetsAtGateway variable has a certain
// value
void
CheckReceivedPacketsAtEndDevice (int nPackets)
{
  NS_LOG_INFO ("Expected: " << nPackets << ", got: " << packetsAtEndDevice);

  NS_ASSERT (packetsAtEndDevice == nPackets);
}

void Reset (void)
{
  // ns3::PacketMetadata::Enable ();

  // Reset counters
  packetsAtGateway = 0;
  packetsAtEndDevice = 0;
  lastKnownLinkMargin = 0;
  lastKnownGatewayCount = 0;
  endDeviceDutyCycle = 0;

  // Create the lora channel object
  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

  MobilityHelper mobility;
  Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator> ();
  mobility.SetPositionAllocator (allocator);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LoraMacHelper
  LoraMacHelper macHelper = LoraMacHelper ();
  Ptr<LoraDeviceAddressGenerator> addrGen = CreateObject<LoraDeviceAddressGenerator> (10,128);
  macHelper.SetRegion (LoraMacHelper::EU);
  macHelper.SetAddressGenerator (addrGen);

  // Create the LoraHelper
  LoraHelper helper = LoraHelper ();

  // Create a set of nodes
  endDevices = NodeContainer ();
  endDevices.Create (1);

  // Assign a mobility model to the node
  allocator->Add (Vector (100,0,0));
  mobility.Install (endDevices);

  // Create the LoraNetDevices of the end devices
  phyHelper.SetDeviceType (LoraPhyHelper::ED);
  macHelper.SetDeviceType (LoraMacHelper::ED);
  helper.Install (phyHelper, macHelper, endDevices);

  // Create a Gateway
  gateways = NodeContainer ();
  gateways.Create (1);

  allocator->Add (Vector (0,0,0));
  mobility.SetPositionAllocator (allocator);
  mobility.Install (gateways);

  // Create a netdevice for each gateway
  phyHelper.SetDeviceType (LoraPhyHelper::GW);
  macHelper.SetDeviceType (LoraMacHelper::GW);
  helper.Install (phyHelper, macHelper, gateways);

  // Get the Gateway's MAC to connect its trace sources
  Ptr<GatewayLoraMac> gwMac = gateways.Get (0)->GetDevice (0)->GetObject<LoraNetDevice> ()->GetMac ()->GetObject<GatewayLoraMac> ();

  gwMac->TraceConnectWithoutContext ("ReceivedPacket",
                                     MakeCallback
                                       (&ReceivedPacketAtGateway));

  // Get the EndDevice's MAC to connect its trace sources
  Ptr<EndDeviceLoraMac> edMac = endDevices.Get (0)->GetDevice (0)->GetObject<LoraNetDevice> ()->GetMac ()->GetObject<EndDeviceLoraMac> ();

  edMac->TraceConnectWithoutContext ("ReceivedPacket",
                                     MakeCallback
                                       (&ReceivedPacketAtEndDevice));
  edMac->TraceConnectWithoutContext ("LastKnownLinkMargin",
                                     MakeCallback
                                       (&LastKnownLinkMarginUpdate));
  edMac->TraceConnectWithoutContext ("LastKnownGatewayCount",
                                     MakeCallback
                                       (&LastKnownGatewayCountUpdate));
  edMac->TraceConnectWithoutContext ("DataRate",
                                     MakeCallback
                                       (&DataRateUpdate));
  edMac->TraceConnectWithoutContext ("TxPower",
                                     MakeCallback
                                       (&TxPowerUpdate));
  edMac->TraceConnectWithoutContext ("AggregatedDutyCycle",
                                     MakeCallback
                                       (&AggregatedDutyCycleUpdate));
}

int main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);

  LogComponentEnableAll (LOG_PREFIX_ALL);
  LogComponentEnable ("LoraMacTest", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraMac", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLoraMac", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_ALL);
  // LogComponentEnable ("GatewayLoraPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLoraMac", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraMacHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraPhyHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraHelper", LOG_LEVEL_ALL);
  // LogComponentEnable ("LoraChannel", LOG_LEVEL_ALL);
  LogComponentEnable ("MacCommand", LOG_LEVEL_ALL);

  // Setup scenario
  Reset ();

  NS_LOG_INFO ("---------------------------- Basic packet sending / receiving ----------------------------");

  //////////////////////////////////////
  // Basic packet sending / receiving //
  //////////////////////////////////////

  Ptr<EndDeviceLoraMac> edMac = endDevices.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetMac ()->GetObject<EndDeviceLoraMac> ();

  packet = Create<Packet> (10);
  Simulator::Schedule (Seconds (2), &EndDeviceLoraMac::Send, edMac, packet);


  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  CheckReceivedPacketsAtGateway (1);

  Reset ();

  NS_LOG_INFO ("---------------------------- Duty Cycle ---------------------------");

  ////////////////
  // Duty Cycle //
  ////////////////

  edMac = endDevices.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetMac ()->GetObject<EndDeviceLoraMac> ();

  packet = Create<Packet> (10);
  otherPacket = Create<Packet> (10);

  // Send a first packet
  Simulator::Schedule (Seconds (2), &EndDeviceLoraMac::Send, edMac, packet);

  // Check that packet went through
  Simulator::Schedule (Seconds (4), &CheckReceivedPacketsAtGateway, 1);

  // Send another packet too early
  Simulator::Schedule (Seconds (4), &EndDeviceLoraMac::Send, edMac, packet);

  // Check that packet was blocked due to duty cycle
  Simulator::Schedule (Seconds (7), &CheckReceivedPacketsAtGateway, 1);

  // Check that packet went through (duty cycle would allow it)
  Simulator::Schedule (Seconds (200), &EndDeviceLoraMac::Send, edMac, otherPacket);

  // Check that packet was blocked due to duty cycle
  Simulator::Schedule (Seconds (204), &CheckReceivedPacketsAtGateway, 2);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();
  Reset ();

  NS_LOG_INFO ("---------------------------- Receive window management ----------------------------");

  ////////////////////////////////////
  // TODO Receive window management //
  ////////////////////////////////////
  // TODO Correctly sets bits in the packet headers

  // TODO Correctly opens receive windows

  // TODO Correctly receives on first window

  // TODO Correctly receives on second window

  // TODO Correctly handles callbacks from the PHY

  // TODO Uniformely picks a channel to transmit on
  // Perform lots of packet transmissions to have a heterogeneous sample, then
  // see if the pdf of picking a certain packet is more or less uniform.

  NS_LOG_INFO ("---------------------------- Maximum App Payload Length ----------------------------");

  ///////////////////////////////////////////
  // Enforces limits on App payload length //
  ///////////////////////////////////////////

  edMac = endDevices.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetMac ()->GetObject<EndDeviceLoraMac> ();

  packet = Create<Packet> (100);
  otherPacket = Create<Packet> (20);
  Ptr<Packet> thirdPacket = Create<Packet> (200);

  // Send a first packet
  Simulator::Schedule (Seconds (2), &EndDeviceLoraMac::Send, edMac, packet);

  // Check that packet did not get through
  Simulator::Schedule (Seconds (4), &CheckReceivedPacketsAtGateway, 0);

  // Send a second packet
  Simulator::Schedule (Seconds (200), &EndDeviceLoraMac::Send, edMac, otherPacket);

  // Check that packet did get through
  Simulator::Schedule (Seconds (204), &CheckReceivedPacketsAtGateway, 1);

  // Send a third packet
  // DR5 and payload 200 bytes, should get through
  Simulator::Schedule (Seconds (400), &EndDeviceLoraMac::SetDataRate, edMac, 5);
  Simulator::Schedule (Seconds (401), &EndDeviceLoraMac::Send, edMac, thirdPacket);

  // Check that packet did get through
  Simulator::Schedule (Seconds (405), &CheckReceivedPacketsAtGateway, 2);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();
  Reset ();

  NS_LOG_INFO ("---------------------------- Addressing ----------------------------");

  // Correctly identifies packets that are addressed to it vs. ones that aren't
  /////////////////////////////////////////////////////////////////////////////

  // Create a packet addressed to this device
  Ptr<Packet> addressedPacket = Create<Packet> (10);
  LoraFrameHeader frameHeader;
  frameHeader.SetAsUplink ();
  frameHeader.SetAddress (LoraDeviceAddress (10, 128));
  LoraMacHeader macHeader;
  macHeader.SetMType (LoraMacHeader::UNCONFIRMED_DATA_DOWN);
  addressedPacket->AddHeader (frameHeader);
  addressedPacket->AddHeader (macHeader);
  LoraTxParameters txParams;
  txParams.sf = 7;

  Ptr<GatewayLoraPhy> gwPhy = gateways.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<GatewayLoraPhy> ();

  // Make sure the device is listening on the right channel and for the right SF
  Ptr<EndDeviceLoraPhy> edPhy = endDevices.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<EndDeviceLoraPhy> ();
  edPhy->SetSpreadingFactor (7);
  edPhy->SetFrequency (868.1);

  // Create a packet with the wrong address
  Ptr<Packet> wronglyAddressedPacket = Create<Packet> (10);
  frameHeader = LoraFrameHeader ();
  frameHeader.SetAsUplink ();
  frameHeader.SetAddress (LoraDeviceAddress (10, 129));
  wronglyAddressedPacket->AddHeader (frameHeader);
  wronglyAddressedPacket->AddHeader (macHeader);

  // Send the packet from the gateway
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::Send, gwPhy, addressedPacket, txParams, 868.1, 14);

  // Check that the packet was received correctly at the End Device
  Simulator::Schedule (Seconds (4), &CheckReceivedPacketsAtEndDevice, 1);

  // Send the packet from the gateway
  Simulator::Schedule (Seconds (200), &GatewayLoraPhy::Send, gwPhy, wronglyAddressedPacket, txParams, 868.1, 14);

  // Check that the packet was ignored by the EndDevice (i.e., that it wasn't received by the MAC layer)
  Simulator::Schedule (Seconds (202), &CheckReceivedPacketsAtEndDevice, 1);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();
  Reset ();

  NS_LOG_INFO ("---------------------------- MAC commands ----------------------------");

  //////////////////
  // MAC commands //
  //////////////////
  // - Correctly sends out a reply to the NS when needed
  // - Cumulative MAC commands are interpreted correctly

  // LinkCheck
  ////////////

  NS_LOG_INFO ("LinkCheck test");

  // Create a packet addressed to this device
  packet = Create<Packet> (10);
  frameHeader = LoraFrameHeader ();
  frameHeader.SetAsUplink ();
  frameHeader.SetAddress (LoraDeviceAddress (10, 128));
  frameHeader.AddLinkCheckAns (10, 3);
  macHeader = LoraMacHeader ();
  macHeader.SetMType (LoraMacHeader::UNCONFIRMED_DATA_DOWN);
  packet->AddHeader (frameHeader);
  packet->AddHeader (macHeader);

  txParams = LoraTxParameters ();
  txParams.sf = 7;

  gwPhy = gateways.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<GatewayLoraPhy> ();

  // Make sure the device is listening on the right channel and for the right SF
  edPhy = endDevices.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<EndDeviceLoraPhy> ();
  edPhy->SetSpreadingFactor (7);
  edPhy->SetFrequency (868.1);

  // Send the packet from the gateway
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::Send, gwPhy, packet, txParams, 868.1, 14);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_ASSERT (lastKnownGatewayCount == 3 && lastKnownLinkMargin == 10);

  Reset ();

  // LinkAdr
  //////////

  NS_LOG_INFO ("LinkAdr test");

  // Create a packet addressed to this device
  packet = Create<Packet> (10);
  frameHeader = LoraFrameHeader ();
  frameHeader.SetAsUplink ();
  frameHeader.SetAddress (LoraDeviceAddress (10, 128));
  // SF11, 11 dBm, on the second default channel only
  frameHeader.AddLinkAdrReq (1, 2, std::list<int> (1,1), 1);
  macHeader = LoraMacHeader ();
  macHeader.SetMType (LoraMacHeader::UNCONFIRMED_DATA_DOWN);
  packet->AddHeader (frameHeader);
  packet->AddHeader (macHeader);

  txParams = LoraTxParameters ();
  txParams.sf = 7;

  gwPhy = gateways.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<GatewayLoraPhy> ();

  // Make sure the device is listening on the right channel and for the right SF
  edPhy = endDevices.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<EndDeviceLoraPhy> ();
  edPhy->SetSpreadingFactor (7);
  edPhy->SetFrequency (868.1);

  // Send the packet from the gateway
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::Send, gwPhy, packet, txParams, 868.1, 14);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_ASSERT (endDeviceDataRate == 1 && endDeviceTxPower == 11);

  Reset ();

  // DutyCycle
  ////////////

  NS_LOG_INFO ("DutyCycleReq test");

  // Create a packet addressed to this device
  packet = Create<Packet> (10);
  frameHeader = LoraFrameHeader ();
  frameHeader.SetAsDownlink ();
  frameHeader.SetAddress (LoraDeviceAddress (10, 128));
  frameHeader.AddDutyCycleReq (2);
  macHeader = LoraMacHeader ();
  macHeader.SetMType (LoraMacHeader::UNCONFIRMED_DATA_DOWN);
  packet->AddHeader (frameHeader);
  packet->AddHeader (macHeader);

  txParams = LoraTxParameters ();
  txParams.sf = 7;

  gwPhy = gateways.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<GatewayLoraPhy> ();

  // Make sure the device is listening on the right channel and for the right SF
  edMac = endDevices.Get (0)->GetDevice (0)->GetObject<LoraNetDevice> ()->
    GetMac ()->GetObject<EndDeviceLoraMac> ();
  edPhy = endDevices.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<EndDeviceLoraPhy> ();
  edPhy->SetSpreadingFactor (7);
  edPhy->SetFrequency (868.1);

  // Send the packet from the gateway
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::Send, gwPhy, packet, txParams, 868.1, 14);

  // Schedule an opportunity for the device to answer to the command
  Simulator::Schedule (Seconds (20), &EndDeviceLoraMac::Send, edMac, Create<Packet> (10));

  Simulator::Stop (Hours (2));
  Simulator::Run ();

  NS_ASSERT (endDeviceDutyCycle == 0.25);

  // Analyze the device's reply to see whether it includes a DutyCycleAns
  LoraMacHeader replyMacHeader;
  lastPacketReceivedAtGateway->RemoveHeader (macHeader);
  LoraFrameHeader replyFrameHeader;
  replyFrameHeader.SetAsUplink ();
  lastPacketReceivedAtGateway->RemoveHeader (replyFrameHeader);

  replyFrameHeader.Print (std::cout);

  for (const auto &command : replyFrameHeader.GetCommands ())
    {
      if (command->GetObject<DutyCycleAns> () != 0)
        {
          NS_LOG_INFO ("Found a DutyCycleAns in the reply packet");
          break;
        }
      NS_ASSERT (false);
    }

  Simulator::Destroy ();

  Reset ();

  // RXParamSetupReq / RXParamSetupAns
  ////////////////////////////////////

  NS_LOG_INFO ("RxParamSetup test");

  // Create a packet addressed to this device
  packet = Create<Packet> (10);
  frameHeader = LoraFrameHeader ();
  frameHeader.SetAsDownlink ();
  frameHeader.SetAddress (LoraDeviceAddress (10, 128));
  frameHeader.AddRxParamSetupReq (3, 5, 868.3 * 1000000);
  macHeader = LoraMacHeader ();
  macHeader.SetMType (LoraMacHeader::UNCONFIRMED_DATA_DOWN);
  packet->AddHeader (frameHeader);
  packet->AddHeader (macHeader);

  txParams = LoraTxParameters ();
  txParams.sf = 7;

  gwPhy = gateways.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<GatewayLoraPhy> ();

  // Make sure the device is listening on the right channel and for the right SF
  edMac = endDevices.Get (0)->GetDevice (0)->GetObject<LoraNetDevice> ()->
    GetMac ()->GetObject<EndDeviceLoraMac> ();
  edPhy = endDevices.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<EndDeviceLoraPhy> ();
  edPhy->SetSpreadingFactor (7);
  edPhy->SetFrequency (868.1);

  // Send the packet from the gateway
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::Send, gwPhy, packet, txParams, 868.1, 14);

  // Schedule an opportunity for the device to answer to the command
  Simulator::Schedule (Seconds (20), &EndDeviceLoraMac::Send, edMac, Create<Packet> (10));

  Simulator::Stop (Hours (2));
  Simulator::Run ();

  // Analyze the device's reply to see whether it includes a DutyCycleAns
  lastPacketReceivedAtGateway->RemoveHeader (replyMacHeader);
  replyFrameHeader.SetAsUplink ();
  lastPacketReceivedAtGateway->RemoveHeader (replyFrameHeader);

  replyFrameHeader.Print (std::cout);

  for (const auto &command : replyFrameHeader.GetCommands ())
    {
      if (command->GetObject<RxParamSetupAns> () != 0)
        {
          NS_LOG_INFO ("Found a RxParamSetupAns in the reply packet");
          break;
        }
      NS_ASSERT (false);
    }

  Simulator::Destroy ();

  Reset ();

  // DevStatusAns / DevStatusReq test
  ///////////////////////////////////

  NS_LOG_INFO ("DevStatus test");

  // Create a packet addressed to this device
  packet = Create<Packet> (10);
  frameHeader = LoraFrameHeader ();
  frameHeader.SetAsDownlink ();
  frameHeader.SetAddress (LoraDeviceAddress (10, 128));
  frameHeader.AddDevStatusReq ();
  macHeader = LoraMacHeader ();
  macHeader.SetMType (LoraMacHeader::UNCONFIRMED_DATA_DOWN);
  packet->AddHeader (frameHeader);
  packet->AddHeader (macHeader);

  txParams = LoraTxParameters ();
  txParams.sf = 7;

  gwPhy = gateways.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<GatewayLoraPhy> ();

  // Make sure the device is listening on the right channel and for the right SF
  edMac = endDevices.Get (0)->GetDevice (0)->GetObject<LoraNetDevice> ()->
    GetMac ()->GetObject<EndDeviceLoraMac> ();
  edPhy = endDevices.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<EndDeviceLoraPhy> ();
  edPhy->SetSpreadingFactor (7);
  edPhy->SetFrequency (868.1);

  // Send the packet from the gateway
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::Send, gwPhy, packet, txParams, 868.1, 14);

  // Schedule an opportunity for the device to answer to the command
  Simulator::Schedule (Seconds (20), &EndDeviceLoraMac::Send, edMac, Create<Packet> (10));

  Simulator::Stop (Hours (2));
  Simulator::Run ();

  // Analyze the device's reply to see whether it includes a DevStatusAns
  lastPacketReceivedAtGateway->RemoveHeader (replyMacHeader);
  replyFrameHeader.SetAsUplink ();
  lastPacketReceivedAtGateway->RemoveHeader (replyFrameHeader);

  replyFrameHeader.Print (std::cout);

  for (const auto &command : replyFrameHeader.GetCommands ())
    {
      if (command->GetObject<DevStatusAns> () != 0)
        {
          NS_LOG_INFO ("Found a DevStatusAns in the reply packet");
          break;
        }
      NS_ASSERT (false);
    }

  Simulator::Destroy ();

  Reset ();

  // NewChannelAns / NewChannelReq test
  /////////////////////////////////////

  NS_LOG_INFO ("NewChannel test");

  // Create a packet addressed to this device
  packet = Create<Packet> (10);
  frameHeader = LoraFrameHeader ();
  frameHeader.SetAsDownlink ();
  frameHeader.SetAddress (LoraDeviceAddress (10, 128));
  frameHeader.AddNewChannelReq (4, 600, 0, 5);
  macHeader = LoraMacHeader ();
  macHeader.SetMType (LoraMacHeader::UNCONFIRMED_DATA_DOWN);
  packet->AddHeader (frameHeader);
  packet->AddHeader (macHeader);

  txParams = LoraTxParameters ();
  txParams.sf = 7;

  gwPhy = gateways.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<GatewayLoraPhy> ();

  // Make sure the device is listening on the right channel and for the right SF
  edMac = endDevices.Get (0)->GetDevice (0)->GetObject<LoraNetDevice> ()->
    GetMac ()->GetObject<EndDeviceLoraMac> ();
  edPhy = endDevices.Get (0)->GetDevice (0)->
    GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<EndDeviceLoraPhy> ();
  edPhy->SetSpreadingFactor (7);
  edPhy->SetFrequency (868.1);

  // Send the packet from the gateway
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::Send, gwPhy, packet, txParams, 868.1, 14);

  // Schedule an opportunity for the device to answer to the command
  Simulator::Schedule (Seconds (20), &EndDeviceLoraMac::Send, edMac, Create<Packet> (10));

  Simulator::Stop (Hours (2));
  Simulator::Run ();

  // Analyze the device's reply to see whether it includes a NewChannelAns
  lastPacketReceivedAtGateway->RemoveHeader (replyMacHeader);
  replyFrameHeader.SetAsUplink ();
  lastPacketReceivedAtGateway->RemoveHeader (replyFrameHeader);

  replyFrameHeader.Print (std::cout);

  for (const auto &command : replyFrameHeader.GetCommands ())
    {
      if (command->GetObject<NewChannelAns> () != 0)
        {
          NS_LOG_INFO ("Found a NewChannelAns in the reply packet");
          break;
        }
      NS_ASSERT (false);
    }

  Simulator::Destroy ();

  Reset ();

  // TODO RxTimingSetupAns / RxTimingSetupReq
  //////////////////////////////////////

  NS_LOG_INFO ("RxTimingSetup test");

  // This command is used to set a delay between the end of the ED's
  // transmission and its opening of the first receive window.
  //
  // To test the behavior of an ED that receives this command, we will:
  // - Verify that the device uses a predefined delay after transmission
  // - Send the MAC command to the device
  // - Verify that the device uses the new delay

  Simulator::Stop (Hours (2));
  Simulator::Run ();

  Simulator::Destroy ();

  Reset ();

  // TODO TxParamSetupAns / TxParamSetupReq
  /////////////////////////////////////////

  NS_LOG_INFO ("TxParamSetup test");

  Simulator::Stop (Hours (2));
  Simulator::Run ();

  Simulator::Destroy ();

  Reset ();

  // TODO DlChannelAns / DlChannelReq
  ///////////////////////////////////

  NS_LOG_INFO ("DlChannel test");

  Simulator::Stop (Hours (2));
  Simulator::Run ();

  Simulator::Destroy ();

  Reset ();

  return 0;
}
