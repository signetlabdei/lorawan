#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/gateway-lora-phy.h"
#include <cstdlib>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ReceivePathTest");

Ptr<GatewayLoraPhy> gatewayPhy;
int m_noMoreDemodulatorsCalls = 0;
int m_interferenceCalls = 0;
int m_receivedPacketCalls = 0;
int m_maxOccupiedReceptionPaths = 0;

double frequency1 = 868.1;
double frequency2 = 868.3;
double frequency3 = 868.5;
double frequency4 = 868.7;

void OccupiedReceptionPaths (int oldValue, int newValue)
{
  NS_LOG_FUNCTION (oldValue << newValue);

  if (m_maxOccupiedReceptionPaths < newValue)
    {
      m_maxOccupiedReceptionPaths = newValue;
    }
}

void NoMoreDemodulators (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_noMoreDemodulatorsCalls++;
}

void Interference (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_interferenceCalls++;
}

void ReceivedPacket (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_receivedPacketCalls++;
}

void Reset (void)
{
  m_noMoreDemodulatorsCalls = 0;
  m_interferenceCalls = 0;
  m_receivedPacketCalls = 0;
  m_maxOccupiedReceptionPaths = 0;

  gatewayPhy = CreateObject<GatewayLoraPhy> ();
  gatewayPhy->TraceConnectWithoutContext ("LostPacketBecauseNoMoreReceivers",
                                          MakeCallback (&NoMoreDemodulators));
  gatewayPhy->TraceConnectWithoutContext ("LostPacketBecauseInterference",
                                          MakeCallback (&Interference));
  gatewayPhy->TraceConnectWithoutContext ("ReceivedPacket",
                                          MakeCallback (&ReceivedPacket));
  gatewayPhy->TraceConnectWithoutContext ("OccupiedReceptionPaths",
                                          MakeCallback
                                            (&OccupiedReceptionPaths));

  // Add 3 receive paths
  gatewayPhy->AddReceptionPath (frequency1);
  gatewayPhy->AddReceptionPath (frequency1);
  gatewayPhy->AddReceptionPath (frequency2);
  gatewayPhy->AddReceptionPath (frequency2);
  gatewayPhy->AddReceptionPath (frequency3);
  gatewayPhy->AddReceptionPath (frequency3);
}

int main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);

  LogComponentEnable ("ReceivePathTest", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLoraPhy", LOG_LEVEL_ALL);

  Ptr<Packet> packet = Create<Packet> ();

  NS_LOG_INFO ("--------------");
  NS_LOG_INFO ("New simulation");
  NS_LOG_INFO ("--------------");

  Reset ();

  // If no ReceptionPath is configured to listen on a frequency, no packet is received

  Simulator::Schedule (Seconds (1), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (1), frequency4);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_noMoreDemodulatorsCalls == 1);

  NS_LOG_INFO ("--------------");
  NS_LOG_INFO ("New simulation");
  NS_LOG_INFO ("--------------");

  Reset ();

  // A ReceptionPath can receive a packet of any SF without any preconfiguration
  Simulator::Schedule (Seconds (1), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (1), frequency1);
  Simulator::Schedule (Seconds (3), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 8, Seconds (1), frequency1);
  Simulator::Schedule (Seconds (5), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 9, Seconds (1), frequency1);
  Simulator::Schedule (Seconds (7), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 10, Seconds (1), frequency1);
  Simulator::Schedule (Seconds (9), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 11, Seconds (1), frequency1);
  Simulator::Schedule (Seconds (11), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 12, Seconds (1), frequency1);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_noMoreDemodulatorsCalls == 0);
  NS_ASSERT (m_receivedPacketCalls == 6);

  NS_LOG_INFO ("--------------");
  NS_LOG_INFO ("New simulation");
  NS_LOG_INFO ("--------------");

  Reset ();

  // Schedule two reception events at the first frequency, where there are two
  // reception paths listening. Each packet should be received correctly.
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (3), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 9, Seconds (4), frequency1);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_noMoreDemodulatorsCalls == 0);
  NS_ASSERT (m_receivedPacketCalls == 2);

  NS_LOG_INFO ("--------------");
  NS_LOG_INFO ("New simulation");
  NS_LOG_INFO ("--------------");

  Reset ();

  // Interference between packets on the same frequency and different ReceptionPaths
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (3), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_noMoreDemodulatorsCalls == 0);
  NS_ASSERT (m_interferenceCalls == 2);

  NS_LOG_INFO ("--------------");
  NS_LOG_INFO ("New simulation");
  NS_LOG_INFO ("--------------");

  Reset ();

  // Three receptions where only two receivePaths are available
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (3), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_noMoreDemodulatorsCalls == 1);

  NS_LOG_INFO ("--------------");
  NS_LOG_INFO ("New simulation");
  NS_LOG_INFO ("--------------");

  Reset ();

  // Packets that are on different frequencys do not interfere
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency2);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_interferenceCalls == 0);

  NS_LOG_INFO ("--------------");
  NS_LOG_INFO ("New simulation");
  NS_LOG_INFO ("--------------");

  Reset ();

  // Full capacity
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 8, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 9, Seconds (4), frequency2);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 10, Seconds (4), frequency2);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 11, Seconds (4), frequency3);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 12, Seconds (4), frequency3);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_noMoreDemodulatorsCalls == 0);
  NS_ASSERT (m_interferenceCalls == 0);
  NS_ASSERT (m_receivedPacketCalls == 6);

  NS_LOG_INFO ("--------------");
  NS_LOG_INFO ("New simulation");
  NS_LOG_INFO ("--------------");

  Reset ();

  // Full capacity + 1
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 8, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 9, Seconds (4), frequency2);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 10, Seconds (4), frequency2);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 11, Seconds (4), frequency3);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 12, Seconds (4), frequency3);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 10, Seconds (4), frequency3);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_noMoreDemodulatorsCalls == 1);
  NS_ASSERT (m_interferenceCalls == 0);
  NS_ASSERT (m_receivedPacketCalls == 6);

  NS_LOG_INFO ("--------------");
  NS_LOG_INFO ("New simulation");
  NS_LOG_INFO ("--------------");

  Reset ();

  // Receive Paths are correctly freed
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 8, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 9, Seconds (4), frequency2);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 10, Seconds (4), frequency2);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 11, Seconds (4), frequency3);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 12, Seconds (4), frequency3);

  Simulator::Schedule (Seconds (8), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (8), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 8, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (8), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 9, Seconds (4), frequency2);
  Simulator::Schedule (Seconds (8), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 10, Seconds (4), frequency2);
  Simulator::Schedule (Seconds (8), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 11, Seconds (4), frequency3);
  Simulator::Schedule (Seconds (8), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 12, Seconds (4), frequency3);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_noMoreDemodulatorsCalls == 0);
  NS_ASSERT (m_interferenceCalls == 0);
  NS_ASSERT (m_receivedPacketCalls == 12);

  NS_LOG_INFO ("--------------");
  NS_LOG_INFO ("New simulation");
  NS_LOG_INFO ("--------------");

  Reset ();

  // Receive Paths stay occupied exactly for the necessary time
  // Occupy both ReceptionPaths centered at frequency1
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 8, Seconds (4), frequency1);

  // This packet will find no free ReceptionPaths
  Simulator::Schedule (Seconds (2+4) - NanoSeconds (1), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 9, Seconds (4), frequency1);

  // This packet will find a free ReceptionPath
  Simulator::Schedule (Seconds (2+4) + NanoSeconds (1), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 10, Seconds (4), frequency1);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_noMoreDemodulatorsCalls == 1);
  NS_ASSERT (m_interferenceCalls == 0);
  NS_ASSERT (m_receivedPacketCalls == 3);

  NS_LOG_INFO ("--------------");
  NS_LOG_INFO ("New simulation");
  NS_LOG_INFO ("--------------");

  Reset ();

  // Only one ReceivePath locks on the incoming packet
  Simulator::Schedule (Seconds (2), &GatewayLoraPhy::StartReceive, gatewayPhy,
                       packet, 14, 7, Seconds (4), frequency1);

  Simulator::Stop (Hours (2)); Simulator::Run (); Simulator::Destroy ();

  NS_ASSERT (m_noMoreDemodulatorsCalls == 0);
  NS_ASSERT (m_interferenceCalls == 0);
  NS_ASSERT (m_receivedPacketCalls == 1);
  NS_ASSERT (m_maxOccupiedReceptionPaths == 1);
  return 0;
}
