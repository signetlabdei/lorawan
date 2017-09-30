#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/lora-channel.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/constant-position-mobility-model.h"
#include <cstdlib>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PhyCommunicationTest");

Ptr<LoraChannel> channel;
Ptr<EndDeviceLoraPhy> edPhy1;
Ptr<EndDeviceLoraPhy> edPhy2;
Ptr<EndDeviceLoraPhy> edPhy3;

Ptr<Packet> m_latestReceivedPacket;
int m_receivedPacketCalls = 0;
int m_underSensitivityCalls = 0;
int m_interferenceCalls = 0;
int m_noMoreDemodulatorsCalls = 0;

void ReceivedPacket (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_receivedPacketCalls++;

  m_latestReceivedPacket = packet->Copy ();
}

void UnderSensitivity (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_underSensitivityCalls++;
}

void Interference (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_interferenceCalls++;
}

void NoMoreDemodulators (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_noMoreDemodulatorsCalls++;
}

bool HaveSamePacketContents (Ptr<Packet> packet1, Ptr<Packet> packet2)
{
  uint32_t size1 = packet1->GetSerializedSize ();
  uint8_t buffer1[size1];
  packet1->Serialize (buffer1, size1);

  uint32_t size2 = packet2->GetSerializedSize ();
  uint8_t buffer2[size2];
  packet2->Serialize (buffer2, size2);

  NS_ASSERT (size1 == size2);

  bool foundADifference = false;
  for (uint32_t i = 0; i < size1; i++)
    {
      if (buffer1[i] != buffer2[i])
        {
          foundADifference = true;
          break;
        }
    }

  return !foundADifference;
}

void Reset (void)
{
  m_receivedPacketCalls = 0;
  m_underSensitivityCalls = 0;
  m_interferenceCalls = 0;

  Ptr<LogDistancePropagationLossModel> loss =
    CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);

  Ptr<PropagationDelayModel> delay =
    CreateObject<ConstantSpeedPropagationDelayModel> ();

  // Create the channel
  channel = CreateObject<LoraChannel> (loss, delay);

  // Connect PHYs
  edPhy1 = CreateObject<EndDeviceLoraPhy> ();
  edPhy2 = CreateObject<EndDeviceLoraPhy> ();
  edPhy3 = CreateObject<EndDeviceLoraPhy> ();

  edPhy1->SetFrequency (868.1);
  edPhy2->SetFrequency (868.1);
  edPhy3->SetFrequency (868.1);

  Ptr<ConstantPositionMobilityModel> mob1 = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> mob2 = CreateObject<ConstantPositionMobilityModel> ();
  Ptr<ConstantPositionMobilityModel> mob3 = CreateObject<ConstantPositionMobilityModel> ();

  mob1->SetPosition (Vector (0.0, 0.0, 0.0));
  mob2->SetPosition (Vector (10.0, 0.0, 0.0));
  mob3->SetPosition (Vector (20.0, 0.0, 0.0));

  edPhy1->SetMobility (mob1);
  edPhy2->SetMobility (mob2);
  edPhy3->SetMobility (mob3);

  edPhy1->SwitchToStandby ();
  edPhy2->SwitchToStandby ();
  edPhy3->SwitchToStandby ();

  channel->Add (edPhy1);
  channel->Add (edPhy2);
  channel->Add (edPhy3);

  edPhy1->SetChannel (channel);
  edPhy2->SetChannel (channel);
  edPhy3->SetChannel (channel);

  edPhy1->TraceConnectWithoutContext ("ReceivedPacket", MakeCallback (&ReceivedPacket));
  edPhy2->TraceConnectWithoutContext ("ReceivedPacket", MakeCallback (&ReceivedPacket));
  edPhy3->TraceConnectWithoutContext ("ReceivedPacket", MakeCallback (&ReceivedPacket));

  edPhy1->TraceConnectWithoutContext ("LostPacketBecauseUnderSensitivity", MakeCallback (&UnderSensitivity));
  edPhy2->TraceConnectWithoutContext ("LostPacketBecauseUnderSensitivity", MakeCallback (&UnderSensitivity));
  edPhy3->TraceConnectWithoutContext ("LostPacketBecauseUnderSensitivity", MakeCallback (&UnderSensitivity));

  edPhy1->TraceConnectWithoutContext ("LostPacketBecauseInterference", MakeCallback (&Interference));
  edPhy2->TraceConnectWithoutContext ("LostPacketBecauseInterference", MakeCallback (&Interference));
  edPhy3->TraceConnectWithoutContext ("LostPacketBecauseInterference", MakeCallback (&Interference));

  edPhy1->TraceConnectWithoutContext ("LostPacketBecauseNoMoreReceivers", MakeCallback (&NoMoreDemodulators));
  edPhy2->TraceConnectWithoutContext ("LostPacketBecauseNoMoreReceivers", MakeCallback (&NoMoreDemodulators));
  edPhy3->TraceConnectWithoutContext ("LostPacketBecauseNoMoreReceivers", MakeCallback (&NoMoreDemodulators));
}

int main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);

  LogComponentEnable ("PhyCommunicationTest", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraChannel", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_ALL);

  // Setup
  ////////

  Reset ();

  LoraTxParameters txParams;
  txParams.sf = 12;

  Ptr<Packet> packet = Create<Packet> (10);

  // Testing
  //////////

  // Basic packet delivery test
  /////////////////////////////

  Simulator::Schedule (Seconds (2), &EndDeviceLoraPhy::Send, edPhy1, packet,
                       txParams, 868.1, 14);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_receivedPacketCalls == 2); // All PHYs except the sender

  Reset ();

  // Sleeping PHYs do not receive the packet

  edPhy2->SwitchToSleep ();

  Simulator::Schedule (Seconds (2), &EndDeviceLoraPhy::Send, edPhy1, packet,
                       txParams, 868.1, 14);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_receivedPacketCalls == 1); // All PHYs in Standby except the sender

  Reset ();

  // Packet that arrives under sensitivity is received correctly if SF increases

  txParams.sf = 7;
  edPhy2->GetMobility ()->GetObject<ConstantPositionMobilityModel>
    ()->SetPosition (Vector (2990, 0, 0));

  Simulator::Schedule (Seconds (2), &EndDeviceLoraPhy::Send, edPhy1, packet,
                       txParams, 868.1, 14);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_underSensitivityCalls == 1);

  Reset ();

  // Try again using a packet with higher SF
  txParams.sf = 8;
  edPhy2->GetMobility ()->GetObject<ConstantPositionMobilityModel> ()->SetPosition (Vector (2990, 0, 0));

  Simulator::Schedule (Seconds (2), &EndDeviceLoraPhy::Send, edPhy1, packet,
                       txParams, 868.1, 14);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_underSensitivityCalls == 0);

  Reset ();

  // Packets can be destroyed by interference

  txParams.sf = 8;

  Simulator::Schedule (Seconds (2), &EndDeviceLoraPhy::Send, edPhy1, packet,
                       txParams, 868.1, 14);
  Simulator::Schedule (Seconds (2), &EndDeviceLoraPhy::Send, edPhy3, packet,
                       txParams, 868.1, 14);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_interferenceCalls == 1);

  Reset ();

  // Packets can be lost because the PHY is not listening on the right frequency

  Simulator::Schedule (Seconds (2), &EndDeviceLoraPhy::Send, edPhy1, packet,
                       txParams, 868.3, 14);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_noMoreDemodulatorsCalls == 2);

  Reset ();

  // Sending of packets
  /////////////////////

  // The very same packet arrives at the other PHY
  Simulator::Schedule (Seconds (2), &EndDeviceLoraPhy::Send, edPhy1, packet,
                       txParams, 868.1, 14);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (HaveSamePacketContents (packet, m_latestReceivedPacket));

  Reset ();

  // Correct state transitions
  ////////////////////////////

  // PHY switches to STANDBY after TX and RX

  Simulator::Schedule (Seconds (2), &EndDeviceLoraPhy::Send, edPhy1, packet,
                       txParams, 868.1, 14);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (edPhy1->GetState () == EndDeviceLoraPhy::STANDBY);
  NS_ASSERT (edPhy2->GetState () == EndDeviceLoraPhy::STANDBY);

  return 0;
}
