/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Include headers of classes to test
#include "ns3/log.h"
#include "ns3/lora-helper.h"
#include "ns3/simple-end-device-lora-phy.h"
#include "ns3/simple-gateway-lora-phy.h"
#include "ns3/mobility-helper.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/constant-position-mobility-model.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("LorawanTestSuite");

/********************
 * InterferenceTest *
 ********************/

class InterferenceTest : public TestCase
{
public:
  InterferenceTest ();
  virtual ~InterferenceTest ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
InterferenceTest::InterferenceTest ()
    : TestCase ("Verify that LoraInterferenceHelper works as expected")
{
}

// Reminder that the test case should clean up after itself
InterferenceTest::~InterferenceTest ()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
InterferenceTest::DoRun (void)
{
  NS_LOG_DEBUG ("InterferenceTest");

  LoraInterferenceHelper interferenceHelper;

  double frequency = 868.1;
  double differentFrequency = 868.3;

  Ptr<LoraInterferenceHelper::Event> event;
  Ptr<LoraInterferenceHelper::Event> event1;

  // Test overlap duration
  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  event1 = interferenceHelper.Add (Seconds (1), 14, 12, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.GetOverlapTime (event, event1), Seconds (1),
                         "Overlap computation didn't give the expected result");
  interferenceHelper.ClearAllEvents ();

  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  event1 = interferenceHelper.Add (Seconds (1.5), 14, 12, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.GetOverlapTime (event, event1), Seconds (1.5),
                         "Overlap computation didn't give the expected result");
  interferenceHelper.ClearAllEvents ();

  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  event1 = interferenceHelper.Add (Seconds (3), 14, 12, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.GetOverlapTime (event, event1), Seconds (2),
                         "Overlap computation didn't give the expected result");
  interferenceHelper.ClearAllEvents ();

  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  event1 = interferenceHelper.Add (Seconds (2), 14, 12, 0, frequency);
  // Because of some strange behavior, this test would get stuck if we used the same syntax of the previous ones.
  // This works instead.
  bool retval = interferenceHelper.GetOverlapTime (event, event1) == Seconds (2);
  NS_TEST_EXPECT_MSG_EQ (retval, true, "Overlap computation didn't give the expected result");
  interferenceHelper.ClearAllEvents ();

  // Perfect overlap, packet survives
  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14, 12, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.IsDestroyedByInterference (event), 0,
                         "Packet did not survive interference as expected");
  interferenceHelper.ClearAllEvents ();

  // Perfect overlap, packet survives
  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14 - 7, 7, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.IsDestroyedByInterference (event), 0,
                         "Packet did not survive interference as expected");
  interferenceHelper.ClearAllEvents ();

  // Perfect overlap, packet destroyed
  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14 - 6, 7, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.IsDestroyedByInterference (event), 7,
                         "Packet was not destroyed by interference as expected");
  interferenceHelper.ClearAllEvents ();

  // Partial overlap, packet survives
  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  interferenceHelper.Add (Seconds (1), 14 - 6, 7, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.IsDestroyedByInterference (event), 0,
                         "Packet did not survive interference as expected");
  interferenceHelper.ClearAllEvents ();

  // Different frequencys
  // Packet would be destroyed if they were on the same frequency, but survives
  // since they are on different frequencies
  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14, 7, 0, differentFrequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.IsDestroyedByInterference (event), 0,
                         "Packet did not survive interference as expected");
  interferenceHelper.ClearAllEvents ();

  // Different SFs
  // Packet would be destroyed if they both were SF7, but survives thanks to SF
  // orthogonality
  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14 + 16, 8, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.IsDestroyedByInterference (event), 0,
                         "Packet did not survive interference as expected");
  interferenceHelper.ClearAllEvents ();

  // SF imperfect orthogonality
  // Different SFs are orthogonal only up to a point
  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14 + 17, 8, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.IsDestroyedByInterference (event), 8,
                         "Packet was not destroyed by interference as expected");
  interferenceHelper.ClearAllEvents ();

  // If a more 'distant' SF is used, isolation gets better
  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14 + 17, 10, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.IsDestroyedByInterference (event), 0,
                         "Packet was destroyed by interference while it should have survived");
  interferenceHelper.ClearAllEvents ();

  // Cumulative interference
  // Same-SF interference is cumulative
  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14 + 16, 8, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14 + 16, 8, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14 + 16, 8, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.IsDestroyedByInterference (event), 8,
                         "Packet was not destroyed by interference as expected");
  interferenceHelper.ClearAllEvents ();

  // Cumulative interference
  // Interference is not cumulative between different SFs
  event = interferenceHelper.Add (Seconds (2), 14, 7, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14 + 16, 8, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14 + 16, 9, 0, frequency);
  interferenceHelper.Add (Seconds (2), 14 + 16, 10, 0, frequency);
  NS_TEST_EXPECT_MSG_EQ (interferenceHelper.IsDestroyedByInterference (event), 0,
                         "Packet did not survive interference as expected");
  interferenceHelper.ClearAllEvents ();
}

/***************
 * AddressTest *
 ***************/

class AddressTest : public TestCase
{
public:
  AddressTest ();
  virtual ~AddressTest ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
AddressTest::AddressTest () : TestCase ("Verify that LoraDeviceAddress works as expected")
{
}

// Reminder that the test case should clean up after itself
AddressTest::~AddressTest ()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
AddressTest::DoRun (void)
{
  NS_LOG_DEBUG ("AddressTest");

  //////////////////////////////////////
  // Test the LoraDeviceAddress class //
  //////////////////////////////////////

  // Address equality
  LoraDeviceAddress firstAddress (0xFFFFFFFF);
  LoraDeviceAddress secondAddress (0xFFFFFFFF);
  NS_TEST_EXPECT_MSG_EQ ((firstAddress == secondAddress), true, "Addresses don't match");

  // Address ordering
  LoraDeviceAddress bigAddress (0xFFFFFF00);
  LoraDeviceAddress smallAddress (0xFFF00000);
  NS_TEST_EXPECT_MSG_EQ ((bigAddress > smallAddress), true,
                         "> function for addresses doesn't work correctly");

  // Setting and getting
  LoraDeviceAddress referenceAddress (0xFFFFFFFF);
  LoraDeviceAddress address (0x00000000);
  NS_TEST_EXPECT_MSG_EQ ((address != referenceAddress), true, "Different addresses match!");
  address.SetNwkAddr (0xFFFFFFF);
  address.SetNwkID (0b1111111);
  NS_TEST_EXPECT_MSG_EQ ((address == referenceAddress), true,
                         "Addresses set to be equal don't match");

  // Serialization and deserialization
  uint8_t buffer[4];
  LoraDeviceAddress toSerialize (0x0F0F0F0F);
  toSerialize.Serialize (buffer);
  LoraDeviceAddress deserialized = LoraDeviceAddress::Deserialize (buffer);
  NS_TEST_EXPECT_MSG_EQ ((toSerialize == deserialized), true,
                         "Serialization + Deserialization doesn't yield an equal address");

  ///////////////////////////////////
  // Test the address generator class
  ///////////////////////////////////

  LoraDeviceAddressGenerator addressGenerator;
  for (int i = 0; i < 200; i++)
    {
      addressGenerator.NextAddress ();
    }
  // After 200 iterations, the address should be 0xC9
  NS_TEST_EXPECT_MSG_EQ ((addressGenerator.GetNextAddress () == LoraDeviceAddress (0xC9)), true,
                         "LoraDeviceAddressGenerator doesn't increment as expected");
}

/***************
 * HeaderTest *
 ***************/

class HeaderTest : public TestCase
{
public:
  HeaderTest ();
  virtual ~HeaderTest ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
HeaderTest::HeaderTest ()
    : TestCase ("Verify that LorawanMacHeader and LoraFrameHeader work as expected")
{
}

// Reminder that the test case should clean up after itself
HeaderTest::~HeaderTest ()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
HeaderTest::DoRun (void)
{
  NS_LOG_DEBUG ("HeaderTest");

  //////////////////////////////////
  // Test the LorawanMacHeader class //
  //////////////////////////////////
  LorawanMacHeader macHdr;
  macHdr.SetMType (LorawanMacHeader::CONFIRMED_DATA_DOWN);
  macHdr.SetMajor (1);

  Buffer macBuf;
  macBuf.AddAtStart (100);
  Buffer::Iterator macSerialized = macBuf.Begin ();
  macHdr.Serialize (macSerialized);

  macHdr.Deserialize (macSerialized);

  NS_TEST_EXPECT_MSG_EQ ((macHdr.GetMType () == LorawanMacHeader::CONFIRMED_DATA_DOWN), true,
                         "MType changes in the serialization/deserialization process");
  NS_TEST_EXPECT_MSG_EQ ((macHdr.GetMajor () == 1), true,
                         "MType changes in the serialization/deserialization process");

  ////////////////////////////////////
  // Test the LoraFrameHeader class //
  ////////////////////////////////////
  LoraFrameHeader frameHdr;
  frameHdr.SetAsDownlink ();
  frameHdr.SetAck (true);
  frameHdr.SetAdr (false);
  frameHdr.SetFCnt (1);
  frameHdr.SetAddress (LoraDeviceAddress (56, 1864));
  frameHdr.AddLinkCheckAns (10, 1);

  // Serialization
  Buffer buf;
  buf.AddAtStart (100);
  Buffer::Iterator serialized = buf.Begin ();
  frameHdr.Serialize (serialized);

  // Deserialization
  frameHdr.Deserialize (serialized);

  Ptr<LinkCheckAns> command = (*(frameHdr.GetCommands ().begin ()))->GetObject<LinkCheckAns> ();
  uint8_t margin = command->GetMargin ();
  uint8_t gwCnt = command->GetGwCnt ();

  NS_TEST_EXPECT_MSG_EQ (frameHdr.GetAck (), true,
                         "Ack changes in the serialization/deserialization process");
  NS_TEST_EXPECT_MSG_EQ (frameHdr.GetAdr (), false,
                         "Adr changes in the serialization/deserialization process");
  NS_TEST_EXPECT_MSG_EQ (frameHdr.GetFCnt (), 1,
                         "FCnt changes in the serialization/deserialization process");
  NS_TEST_EXPECT_MSG_EQ ((frameHdr.GetAddress () == LoraDeviceAddress (56, 1864)), true,
                         "Address changes in the serialization/deserialization process");
  NS_TEST_EXPECT_MSG_EQ (margin, 10, "Margin changes in the serialization/deserialization process");
  NS_TEST_EXPECT_MSG_EQ (gwCnt, 1, "GwCnt changes in the serialization/deserialization process");

  /////////////////////////////////////////////////
  // Test a combination of the two above classes //
  /////////////////////////////////////////////////
  Ptr<Packet> pkt = Create<Packet> (10);
  pkt->AddHeader (frameHdr);
  pkt->AddHeader (macHdr);

  // Length = Payload + FrameHeader + MacHeader
  //        = 10 + (8+3) + 1 = 22
  NS_TEST_EXPECT_MSG_EQ ((pkt->GetSize ()), 22, "Wrong size of packet + headers");

  LorawanMacHeader macHdr1;

  pkt->RemoveHeader (macHdr1);

  NS_TEST_EXPECT_MSG_EQ ((pkt->GetSize ()), 21, "Wrong size of packet + headers - macHeader");

  LoraFrameHeader frameHdr1;
  frameHdr1.SetAsDownlink ();

  pkt->RemoveHeader (frameHdr1);
  Ptr<LinkCheckAns> linkCheckAns =
      (*(frameHdr1.GetCommands ().begin ()))->GetObject<LinkCheckAns> ();

  NS_TEST_EXPECT_MSG_EQ ((pkt->GetSize ()), 10,
                         "Wrong size of packet + headers - macHeader - frameHeader");

  // Verify contents of removed MAC header
  NS_TEST_EXPECT_MSG_EQ (macHdr1.GetMType (), macHdr.GetMType (),
                         "Removed header contents don't match");
  NS_TEST_EXPECT_MSG_EQ (macHdr1.GetMajor (), macHdr.GetMajor (),
                         "Removed header contents don't match");

  // Verify contents of removed frame header
  NS_TEST_EXPECT_MSG_EQ (frameHdr1.GetAck (), frameHdr.GetAck (),
                         "Removed header contents don't match");
  NS_TEST_EXPECT_MSG_EQ (frameHdr1.GetAdr (), frameHdr.GetAdr (),
                         "Removed header contents don't match");
  NS_TEST_EXPECT_MSG_EQ (frameHdr1.GetFCnt (), frameHdr.GetFCnt (),
                         "Removed header contents don't match");
  NS_TEST_EXPECT_MSG_EQ ((frameHdr1.GetAddress () == frameHdr.GetAddress ()), true,
                         "Removed header contents don't match");
  NS_TEST_EXPECT_MSG_EQ (linkCheckAns->GetMargin (), 10,
                         "Removed header's MAC command contents don't match");
  NS_TEST_EXPECT_MSG_EQ (linkCheckAns->GetGwCnt (), 1,
                         "Removed header's MAC command contents don't match");
}

/*******************
 * ReceivePathTest *
 *******************/

class ReceivePathTest : public TestCase
{
public:
  ReceivePathTest ();
  virtual ~ReceivePathTest ();

private:
  virtual void DoRun (void);
  void Reset (void);
  void OccupiedReceptionPaths (int oldValue, int newValue);
  void NoMoreDemodulators (Ptr<const Packet> packet, uint32_t node);
  void Interference (Ptr<const Packet> packet, uint32_t node);
  void ReceivedPacket (Ptr<const Packet> packet, uint32_t node);

  Ptr<SimpleGatewayLoraPhy> gatewayPhy;
  int m_noMoreDemodulatorsCalls = 0;
  int m_interferenceCalls = 0;
  int m_receivedPacketCalls = 0;
  int m_maxOccupiedReceptionPaths = 0;
};

// Add some help text to this case to describe what it is intended to test
ReceivePathTest::ReceivePathTest () : TestCase ("Verify that ReceivePaths work as expected")
{
}

// Reminder that the test case should clean up after itself
ReceivePathTest::~ReceivePathTest ()
{
}

void
ReceivePathTest::Reset (void)
{
  // FIXME
  // m_noMoreDemodulatorsCalls = 0;
  // m_interferenceCalls = 0;
  // m_receivedPacketCalls = 0;
  // m_maxOccupiedReceptionPaths = 0;

  // gatewayPhy = CreateObject<SimpleGatewayLoraPhy> ();
  // gatewayPhy->TraceConnectWithoutContext (
  //     "LostPacketBecauseNoMoreReceivers",
  //     MakeCallback (&ReceivePathTest::NoMoreDemodulators, this));
  // gatewayPhy->TraceConnectWithoutContext ("LostPacketBecauseInterference",
  //                                         MakeCallback (&ReceivePathTest::Interference, this));
  // gatewayPhy->TraceConnectWithoutContext ("ReceivedPacket",
  //                                         MakeCallback (&ReceivePathTest::ReceivedPacket, this));
  // gatewayPhy->TraceConnectWithoutContext (
  //     "OccupiedReceptionPaths", MakeCallback (&ReceivePathTest::OccupiedReceptionPaths, this));

  // // Add receive paths
  // gatewayPhy->AddReceptionPath ();
  // gatewayPhy->AddReceptionPath ();
  // gatewayPhy->AddReceptionPath ();
  // gatewayPhy->AddReceptionPath ();
  // gatewayPhy->AddReceptionPath ();
  // gatewayPhy->AddReceptionPath ();
}

void
ReceivePathTest::OccupiedReceptionPaths (int oldValue, int newValue)
{
  NS_LOG_FUNCTION (oldValue << newValue);

  if (m_maxOccupiedReceptionPaths < newValue)
    {
      m_maxOccupiedReceptionPaths = newValue;
    }
}

void
ReceivePathTest::NoMoreDemodulators (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_noMoreDemodulatorsCalls++;
}

void
ReceivePathTest::Interference (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_interferenceCalls++;
}

void
ReceivePathTest::ReceivedPacket (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_receivedPacketCalls++;
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
ReceivePathTest::DoRun (void)
{
  NS_LOG_DEBUG ("ReceivePathTest");

  Ptr<Packet> packet = Create<Packet> ();

  Reset ();

  // FIXME
  // //////////////////////////////////////////////////////////////////////////////////
  // // If no ReceptionPath is configured to listen on a frequency, no packet is received
  // //////////////////////////////////////////////////////////////////////////////////

  // Simulator::Schedule (Seconds (1), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (1), frequency4);

  // Simulator::Stop (Hours (2));
  // Simulator::Run ();
  // Simulator::Destroy ();

  // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 1, "Unexpected value");

  // Reset ();

  // //////////////////////////////////////////////////////////////////////////////
  // // A ReceptionPath can receive a packet of any SF without any preconfiguration
  // //////////////////////////////////////////////////////////////////////////////

  // Simulator::Schedule (Seconds (1), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (1), frequency1);
  // Simulator::Schedule (Seconds (3), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 8,
  //                      Seconds (1), frequency1);
  // Simulator::Schedule (Seconds (5), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 9,
  //                      Seconds (1), frequency1);
  // Simulator::Schedule (Seconds (7), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 10,
  //                      Seconds (1), frequency1);
  // Simulator::Schedule (Seconds (9), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 11,
  //                      Seconds (1), frequency1);
  // Simulator::Schedule (Seconds (11), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14,
  //                      12, Seconds (1), frequency1);

  // Simulator::Stop (Hours (2));
  // Simulator::Run ();
  // Simulator::Destroy ();

  // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 0, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_receivedPacketCalls, 6, "Unexpected value");

  // Reset ();

  // ///////////////////////////////////////////////////////////////////////////
  // // Schedule two reception events at the first frequency, where there are two
  // // reception paths listening. Each packet should be received correctly.
  // ///////////////////////////////////////////////////////////////////////////
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (3), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 9,
  //                      Seconds (4), frequency1);

  // Simulator::Stop (Hours (2));
  // Simulator::Run ();
  // Simulator::Destroy ();

  // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 0, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_receivedPacketCalls, 2, "Unexpected value");

  // Reset ();

  // ///////////////////////////////////////////////////////////////////////////
  // // Interference between packets on the same frequency and different ReceptionPaths
  // ///////////////////////////////////////////////////////////////////////////
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (3), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);

  // Simulator::Stop (Hours (2));
  // Simulator::Run ();
  // Simulator::Destroy ();

  // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 0, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_interferenceCalls, 2, "Unexpected value");

  // Reset ();

  // ///////////////////////////////////////////////////////////////////////////
  // // Three receptions where only two receivePaths are available
  // ///////////////////////////////////////////////////////////////////////////
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (3), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);

  // Simulator::Stop (Hours (2));
  // Simulator::Run ();
  // Simulator::Destroy ();

  // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 1, "Unexpected value");

  // Reset ();

  // ///////////////////////////////////////////////////////////////////////////
  // // Packets that are on different frequencys do not interfere
  // ///////////////////////////////////////////////////////////////////////////
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency2);

  // Simulator::Stop (Hours (2));
  // Simulator::Run ();
  // Simulator::Destroy ();

  // NS_TEST_EXPECT_MSG_EQ (m_interferenceCalls, 0, "Unexpected value");

  // Reset ();

  // ///////////////////////////////////////////////////////////////////////////
  // // Full capacity
  // ///////////////////////////////////////////////////////////////////////////
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 8,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 9,
  //                      Seconds (4), frequency2);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 10,
  //                      Seconds (4), frequency2);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 11,
  //                      Seconds (4), frequency3);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 12,
  //                      Seconds (4), frequency3);

  // Simulator::Stop (Hours (2));
  // Simulator::Run ();
  // Simulator::Destroy ();

  // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 0, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_interferenceCalls, 0, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_receivedPacketCalls, 6, "Unexpected value");

  // Reset ();

  // ///////////////////////////////////////////////////////////////////////////
  // // Full capacity + 1
  // ///////////////////////////////////////////////////////////////////////////
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 8,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 9,
  //                      Seconds (4), frequency2);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 10,
  //                      Seconds (4), frequency2);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 11,
  //                      Seconds (4), frequency3);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 12,
  //                      Seconds (4), frequency3);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 10,
  //                      Seconds (4), frequency3);

  // Simulator::Stop (Hours (2));
  // Simulator::Run ();
  // Simulator::Destroy ();

  // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 1, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_interferenceCalls, 0, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_receivedPacketCalls, 6, "Unexpected value");

  // Reset ();

  // ///////////////////////////////////////////////////////////////////////////
  // // Receive Paths are correctly freed
  // ///////////////////////////////////////////////////////////////////////////
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 8,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 9,
  //                      Seconds (4), frequency2);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 10,
  //                      Seconds (4), frequency2);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 11,
  //                      Seconds (4), frequency3);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 12,
  //                      Seconds (4), frequency3);

  // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 8,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 9,
  //                      Seconds (4), frequency2);
  // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 10,
  //                      Seconds (4), frequency2);
  // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 11,
  //                      Seconds (4), frequency3);
  // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 12,
  //                      Seconds (4), frequency3);

  // Simulator::Stop (Hours (2));
  // Simulator::Run ();
  // Simulator::Destroy ();

  // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 0, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_interferenceCalls, 0, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_receivedPacketCalls, 12, "Unexpected value");

  // Reset ();

  // ///////////////////////////////////////////////////////////////////////////
  // // Receive Paths stay occupied exactly for the necessary time
  // // Occupy both ReceptionPaths centered at frequency1
  // ///////////////////////////////////////////////////////////////////////////
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 8,
  //                      Seconds (4), frequency1);

  // // This packet will find no free ReceptionPaths
  // Simulator::Schedule (Seconds (2 + 4) - NanoSeconds (1), &SimpleGatewayLoraPhy::StartReceive,
  //                      gatewayPhy, packet, 14, 9, Seconds (4), frequency1);

  // // This packet will find a free ReceptionPath
  // Simulator::Schedule (Seconds (2 + 4) + NanoSeconds (1), &SimpleGatewayLoraPhy::StartReceive,
  //                      gatewayPhy, packet, 14, 10, Seconds (4), frequency1);

  // Simulator::Stop (Hours (2));
  // Simulator::Run ();
  // Simulator::Destroy ();

  // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 1, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_interferenceCalls, 0, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_receivedPacketCalls, 3, "Unexpected value");

  // Reset ();

  // ///////////////////////////////////////////////////////////////////////////
  // // Only one ReceivePath locks on the incoming packet
  // ///////////////////////////////////////////////////////////////////////////
  // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet, 14, 7,
  //                      Seconds (4), frequency1);

  // Simulator::Stop (Hours (2));
  // Simulator::Run ();
  // Simulator::Destroy ();

  // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 0, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_interferenceCalls, 0, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_receivedPacketCalls, 1, "Unexpected value");
  // NS_TEST_EXPECT_MSG_EQ (m_maxOccupiedReceptionPaths, 1, "Unexpected value");
}

/**************************
 * LogicalLoraChannelTest *
 **************************/

class LogicalLoraChannelTest : public TestCase
{
public:
  LogicalLoraChannelTest ();
  virtual ~LogicalLoraChannelTest ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
LogicalLoraChannelTest::LogicalLoraChannelTest ()
    : TestCase ("Verify that LogicalLoraChannel and LogicalLoraChannelHelper work as expected")
{
}

// Reminder that the test case should clean up after itself
LogicalLoraChannelTest::~LogicalLoraChannelTest ()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
LogicalLoraChannelTest::DoRun (void)
{
  NS_LOG_DEBUG ("LogicalLoraChannelTest");

  /////////////////////////////
  // Test LogicalLoraChannel //
  /////////////////////////////

  // Setup
  Ptr<LogicalLoraChannel> channel1 = CreateObject<LogicalLoraChannel> (868);
  Ptr<LogicalLoraChannel> channel2 = CreateObject<LogicalLoraChannel> (868);
  Ptr<LogicalLoraChannel> channel3 = CreateObject<LogicalLoraChannel> (868.1);
  Ptr<LogicalLoraChannel> channel4 = CreateObject<LogicalLoraChannel> (868.001);

  // Equality between channels
  // Test the == and != operators
  NS_TEST_EXPECT_MSG_EQ (channel1, channel2, "== operator doesn't work as expected");
  NS_TEST_EXPECT_MSG_NE (channel1, channel3, "!= operator doesn't work as expected");
  NS_TEST_EXPECT_MSG_NE (channel1, channel4, "!= operator doesn't work as expected");

  //////////////////
  // Test SubBand //
  //////////////////

  // Setup
  SubBand subBand (868, 868.7, 0.01, 14);
  Ptr<LogicalLoraChannel> channel5 = CreateObject<LogicalLoraChannel> (870);

  // Test BelongsToSubBand
  NS_TEST_EXPECT_MSG_EQ (subBand.BelongsToSubBand (channel3), true,
                         "BelongsToSubBand does not behave as expected");
  NS_TEST_EXPECT_MSG_EQ (subBand.BelongsToSubBand (channel3->GetFrequency ()), true,
                         "BelongsToSubBand does not behave as expected");
  NS_TEST_EXPECT_MSG_EQ (subBand.BelongsToSubBand (channel5), false,
                         "BelongsToSubBand does not behave as expected");

  ///////////////////////////////////
  // Test LogicalLoraChannelHelper //
  ///////////////////////////////////

  // Setup
  Ptr<LogicalLoraChannelHelper> channelHelper = CreateObject<LogicalLoraChannelHelper> ();
  SubBand subBand1 (869, 869.4, 0.1, 27);
  channel1 = CreateObject<LogicalLoraChannel> (868.1);
  channel2 = CreateObject<LogicalLoraChannel> (868.3);
  channel3 = CreateObject<LogicalLoraChannel> (868.5);
  channel4 = CreateObject<LogicalLoraChannel> (869.1);
  channel5 = CreateObject<LogicalLoraChannel> (869.3);

  // Channel diagram
  //
  // Channels      1      2      3                     4       5
  // SubBands  868 ----- 0.1% ----- 868.7       869 ----- 1% ----- 869.4

  // Add SubBands and LogicalLoraChannels to the helper
  channelHelper->AddSubBand (&subBand);
  channelHelper->AddSubBand (&subBand1);
  channelHelper->AddChannel (channel1);
  channelHelper->AddChannel (channel2);
  channelHelper->AddChannel (channel3);
  channelHelper->AddChannel (channel4);
  channelHelper->AddChannel (channel5);

  // Duty Cycle tests
  // (high level duty cycle behavior)
  ///////////////////////////////////

  channelHelper->AddEvent (Seconds (2), channel1);
  Time expectedTimeOff = Seconds (2 / 0.01 - 2);

  // Waiting time is computed correctly
  NS_TEST_EXPECT_MSG_EQ (channelHelper->GetWaitingTime (channel1), expectedTimeOff,
                         "Waiting time doesn't behave as expected");

  // Duty Cycle involves the whole SubBand, not just a channel
  NS_TEST_EXPECT_MSG_EQ (channelHelper->GetWaitingTime (channel2), expectedTimeOff,
                         "Waiting time doesn't behave as expected");
  NS_TEST_EXPECT_MSG_EQ (channelHelper->GetWaitingTime (channel3), expectedTimeOff,
                         "Waiting time doesn't behave as expected");

  // Other bands are not affected by this transmission
  NS_TEST_EXPECT_MSG_EQ (channelHelper->GetWaitingTime (channel4), Time (0),
                         "Waiting time affects other subbands");
  NS_TEST_EXPECT_MSG_EQ (channelHelper->GetWaitingTime (channel5), Time (0),
                         "Waiting time affects other subbands");
}

/*****************
 * TimeOnAirTest *
 *****************/

class TimeOnAirTest : public TestCase
{
public:
  TimeOnAirTest ();
  virtual ~TimeOnAirTest ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
TimeOnAirTest::TimeOnAirTest ()
    : TestCase (
          "Verify that LoraPhy's function to compute the time on air of a packet works as expected")
{
}

// Reminder that the test case should clean up after itself
TimeOnAirTest::~TimeOnAirTest ()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
TimeOnAirTest::DoRun (void)
{
  NS_LOG_DEBUG ("TimeOnAirTest");

  Ptr<Packet> packet;
  Time duration;

  // Available parameters:
  // PayloadSize, SF, HeaderDisabled, CodingRate, Bandwidth, nPreambleSyms, crcEnabled, lowDROptimization

  // Starting parameters
  packet = Create<Packet> (10);
  LoraTxParameters txParams;
  txParams.sf = 7;
  txParams.headerDisabled = false;
  txParams.codingRate = 1;
  txParams.bandwidthHz = 125000;
  txParams.nPreamble = 8;
  txParams.crcEnabled = 1;
  txParams.lowDataRateOptimizationEnabled = 0;

  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.041216, 0.0001, "Unexpected duration");

  txParams.sf = 8;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.072192, 0.0001, "Unexpected duration");

  txParams.headerDisabled = true;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.072192, 0.0001, "Unexpected duration");

  txParams.codingRate = 2;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.078336, 0.0001, "Unexpected duration");

  txParams.nPreamble = 10;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.082432, 0.0001, "Unexpected duration");

  txParams.lowDataRateOptimizationEnabled = true;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.082432, 0.0001, "Unexpected duration");

  txParams.sf = 10;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.280576, 0.0001, "Unexpected duration");

  txParams.bandwidthHz = 250000;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.14028, 0.0001, "Unexpected duration");

  txParams.bandwidthHz = 500000;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.070144, 0.0001, "Unexpected duration");

  txParams.headerDisabled = false;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.082432, 0.0001, "Unexpected duration");

  txParams.nPreamble = 8;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.078336, 0.0001, "Unexpected duration");

  txParams.sf = 12;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.264192, 0.0001, "Unexpected duration");

  packet = Create<Packet> (50);
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 0.657408, 0.0001, "Unexpected duration");

  txParams.bandwidthHz = 125000;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 2.629632, 0.0001, "Unexpected duration");

  txParams.codingRate = 1;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_TEST_EXPECT_MSG_EQ_TOL (duration.GetSeconds (), 2.301952, 0.0001, "Unexpected duration");
}

/**************************
 * PhyConnectivityTest *
 **************************/

class PhyConnectivityTest : public TestCase
{
public:
  PhyConnectivityTest ();
  virtual ~PhyConnectivityTest ();
  void Reset ();
  void ReceivedPacket (Ptr<const Packet> packet, uint32_t node);
  void UnderSensitivity (Ptr<const Packet> packet, uint32_t node);
  void Interference (Ptr<const Packet> packet, uint32_t node);
  void NoMoreDemodulators (Ptr<const Packet> packet, uint32_t node);
  void WrongFrequency (Ptr<const Packet> packet, uint32_t node);
  void WrongSf (Ptr<const Packet> packet, uint32_t node);
  bool HaveSamePacketContents (Ptr<Packet> packet1, Ptr<Packet> packet2);

private:
  virtual void DoRun (void);
  Ptr<LoraChannel> channel;
  Ptr<SimpleEndDeviceLoraPhy> edPhy1;
  Ptr<SimpleEndDeviceLoraPhy> edPhy2;
  Ptr<SimpleEndDeviceLoraPhy> edPhy3;

  Ptr<Packet> m_latestReceivedPacket;
  int m_receivedPacketCalls = 0;
  int m_underSensitivityCalls = 0;
  int m_interferenceCalls = 0;
  int m_noMoreDemodulatorsCalls = 0;
  int m_wrongSfCalls = 0;
  int m_wrongFrequencyCalls = 0;
};

// Add some help text to this case to describe what it is intended to test
PhyConnectivityTest::PhyConnectivityTest ()
    : TestCase ("Verify that PhyConnectivity works as expected")
{
}

// Reminder that the test case should clean up after itself
PhyConnectivityTest::~PhyConnectivityTest ()
{
}

void
PhyConnectivityTest::ReceivedPacket (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_receivedPacketCalls++;

  m_latestReceivedPacket = packet->Copy ();
}

void
PhyConnectivityTest::UnderSensitivity (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_underSensitivityCalls++;
}

void
PhyConnectivityTest::Interference (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_interferenceCalls++;
}

void
PhyConnectivityTest::NoMoreDemodulators (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_noMoreDemodulatorsCalls++;
}

void
PhyConnectivityTest::WrongSf (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_wrongSfCalls++;
}

void
PhyConnectivityTest::WrongFrequency (Ptr<const Packet> packet, uint32_t node)
{
  NS_LOG_FUNCTION (packet << node);

  m_wrongFrequencyCalls++;
}

bool
PhyConnectivityTest::HaveSamePacketContents (Ptr<Packet> packet1, Ptr<Packet> packet2)
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
      NS_LOG_DEBUG (unsigned (buffer1[i]) << " " << unsigned (buffer2[i]));
      if (buffer1[i] != buffer2[i])
        {
          foundADifference = true;
          break;
        }
    }

  return !foundADifference;
}

void
PhyConnectivityTest::Reset (void)
{
  m_receivedPacketCalls = 0;
  m_underSensitivityCalls = 0;
  m_interferenceCalls = 0;
  m_wrongSfCalls = 0;
  m_wrongFrequencyCalls = 0;

  Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
  loss->SetPathLossExponent (3.76);
  loss->SetReference (1, 7.7);

  Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();

  // Create the channel
  channel = CreateObject<LoraChannel> (loss, delay);

  // Connect PHYs
  edPhy1 = CreateObject<SimpleEndDeviceLoraPhy> ();
  edPhy2 = CreateObject<SimpleEndDeviceLoraPhy> ();
  edPhy3 = CreateObject<SimpleEndDeviceLoraPhy> ();

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

  // Listen for a specific SpreadingFactor
  edPhy1->SetSpreadingFactor (12);
  edPhy2->SetSpreadingFactor (12);
  edPhy3->SetSpreadingFactor (12);

  // Listen on a specific frequency
  edPhy1->SetFrequency (868.1);
  edPhy2->SetFrequency (868.1);
  edPhy3->SetFrequency (868.1);

  edPhy1->TraceConnectWithoutContext ("ReceivedPacket",
                                      MakeCallback (&PhyConnectivityTest::ReceivedPacket, this));
  edPhy2->TraceConnectWithoutContext ("ReceivedPacket",
                                      MakeCallback (&PhyConnectivityTest::ReceivedPacket, this));
  edPhy3->TraceConnectWithoutContext ("ReceivedPacket",
                                      MakeCallback (&PhyConnectivityTest::ReceivedPacket, this));

  edPhy1->TraceConnectWithoutContext ("LostPacketBecauseUnderSensitivity",
                                      MakeCallback (&PhyConnectivityTest::UnderSensitivity, this));
  edPhy2->TraceConnectWithoutContext ("LostPacketBecauseUnderSensitivity",
                                      MakeCallback (&PhyConnectivityTest::UnderSensitivity, this));
  edPhy3->TraceConnectWithoutContext ("LostPacketBecauseUnderSensitivity",
                                      MakeCallback (&PhyConnectivityTest::UnderSensitivity, this));

  edPhy1->TraceConnectWithoutContext ("LostPacketBecauseInterference",
                                      MakeCallback (&PhyConnectivityTest::Interference, this));
  edPhy2->TraceConnectWithoutContext ("LostPacketBecauseInterference",
                                      MakeCallback (&PhyConnectivityTest::Interference, this));
  edPhy3->TraceConnectWithoutContext ("LostPacketBecauseInterference",
                                      MakeCallback (&PhyConnectivityTest::Interference, this));

  edPhy1->TraceConnectWithoutContext (
      "LostPacketBecauseNoMoreReceivers",
      MakeCallback (&PhyConnectivityTest::NoMoreDemodulators, this));
  edPhy2->TraceConnectWithoutContext (
      "LostPacketBecauseNoMoreReceivers",
      MakeCallback (&PhyConnectivityTest::NoMoreDemodulators, this));
  edPhy3->TraceConnectWithoutContext (
      "LostPacketBecauseNoMoreReceivers",
      MakeCallback (&PhyConnectivityTest::NoMoreDemodulators, this));

  edPhy1->TraceConnectWithoutContext ("LostPacketBecauseWrongFrequency",
                                      MakeCallback (&PhyConnectivityTest::WrongFrequency, this));
  edPhy2->TraceConnectWithoutContext ("LostPacketBecauseWrongFrequency",
                                      MakeCallback (&PhyConnectivityTest::WrongFrequency, this));
  edPhy3->TraceConnectWithoutContext ("LostPacketBecauseWrongFrequency",
                                      MakeCallback (&PhyConnectivityTest::WrongFrequency, this));

  edPhy1->TraceConnectWithoutContext ("LostPacketBecauseWrongSpreadingFactor",
                                      MakeCallback (&PhyConnectivityTest::WrongSf, this));
  edPhy2->TraceConnectWithoutContext ("LostPacketBecauseWrongSpreadingFactor",
                                      MakeCallback (&PhyConnectivityTest::WrongSf, this));
  edPhy3->TraceConnectWithoutContext ("LostPacketBecauseWrongSpreadingFactor",
                                      MakeCallback (&PhyConnectivityTest::WrongSf, this));
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
PhyConnectivityTest::DoRun (void)
{
  NS_LOG_DEBUG ("PhyConnectivityTest");

  // Setup
  ////////

  Reset ();

  LoraTxParameters txParams;
  txParams.sf = 12;

  uint8_t buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  Ptr<Packet> packet = Create<Packet> (buffer, 10);

  // Testing
  //////////

  // Basic packet delivery test
  /////////////////////////////

  Simulator::Schedule (Seconds (2), &SimpleEndDeviceLoraPhy::Send, edPhy1, packet, txParams, 868.1,
                       14);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_EXPECT_MSG_EQ (
      m_receivedPacketCalls, 2,
      "Channel skipped some PHYs when delivering a packet"); // All PHYs except the sender

  Reset ();

  // Sleeping PHYs do not receive the packet

  edPhy2->SwitchToSleep ();

  Simulator::Schedule (Seconds (2), &SimpleEndDeviceLoraPhy::Send, edPhy1, packet, txParams, 868.1,
                       14);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_EXPECT_MSG_EQ (
      m_receivedPacketCalls, 1,
      "Packet was received by a PHY in SLEEP mode"); // All PHYs in Standby except the sender

  Reset ();

  // Packet that arrives under sensitivity is received correctly if SF increases

  txParams.sf = 7;
  edPhy2->SetSpreadingFactor (7);
  edPhy2->GetMobility ()->GetObject<ConstantPositionMobilityModel> ()->SetPosition (
      Vector (2990, 0, 0));

  Simulator::Schedule (Seconds (2), &SimpleEndDeviceLoraPhy::Send, edPhy1, packet, txParams, 868.1,
                       14);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_EXPECT_MSG_EQ (
      m_underSensitivityCalls, 1,
      "Packet that should have been lost because of low receive power was recevied");

  Reset ();

  // Try again using a packet with higher SF
  txParams.sf = 8;
  edPhy2->SetSpreadingFactor (8);
  edPhy2->GetMobility ()->GetObject<ConstantPositionMobilityModel> ()->SetPosition (
      Vector (2990, 0, 0));

  Simulator::Schedule (Seconds (2), &SimpleEndDeviceLoraPhy::Send, edPhy1, packet, txParams, 868.1,
                       14);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_EXPECT_MSG_EQ (m_underSensitivityCalls, 0,
                         "Packets that should have arrived above sensitivity were under it");

  Reset ();

  // Packets can be destroyed by interference

  txParams.sf = 12;
  Simulator::Schedule (Seconds (2), &SimpleEndDeviceLoraPhy::Send, edPhy1, packet, txParams, 868.1,
                       14);
  Simulator::Schedule (Seconds (2), &SimpleEndDeviceLoraPhy::Send, edPhy3, packet, txParams, 868.1,
                       14);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_EXPECT_MSG_EQ (m_interferenceCalls, 1,
                         "Packets that should be destroyed by interference weren't");

  Reset ();

  // Packets can be lost because the PHY is not listening on the right frequency

  Simulator::Schedule (Seconds (2), &SimpleEndDeviceLoraPhy::Send, edPhy1, packet, txParams, 868.3,
                       14);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_EXPECT_MSG_EQ (m_wrongFrequencyCalls, 2,
                         "Packets were received even though PHY was on a different frequency");

  Reset ();

  // Packets can be lost because the PHY is not listening for the right SF

  txParams.sf = 8; // Send with 8, listening for 12
  Simulator::Schedule (Seconds (2), &SimpleEndDeviceLoraPhy::Send, edPhy1, packet, txParams, 868.1,
                       14);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_EXPECT_MSG_EQ (m_wrongSfCalls, 2,
                         "Packets were received even though PHY was listening for a different SF");

  Reset ();

  // Sending of packets
  /////////////////////

  // The very same packet arrives at the other PHY
  Simulator::Schedule (Seconds (2), &SimpleEndDeviceLoraPhy::Send, edPhy1, packet, txParams, 868.1,
                       14);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_EXPECT_MSG_EQ (HaveSamePacketContents (packet, m_latestReceivedPacket), true,
                         "Packet changed contents when going through the channel");

  Reset ();

  // Correct state transitions
  ////////////////////////////

  // PHY switches to STANDBY after TX and RX

  Simulator::Schedule (Seconds (2), &SimpleEndDeviceLoraPhy::Send, edPhy1, packet, txParams, 868.1,
                       14);

  Simulator::Stop (Hours (2));
  Simulator::Run ();
  Simulator::Destroy ();

  NS_TEST_EXPECT_MSG_EQ (edPhy1->GetState (), SimpleEndDeviceLoraPhy::STANDBY,
                         "State didn't switch to STANDBY as expected");
  NS_TEST_EXPECT_MSG_EQ (edPhy2->GetState (), SimpleEndDeviceLoraPhy::STANDBY,
                         "State didn't switch to STANDBY as expected");
}

/*****************
 * LorawanMacTest *
 *****************/

class LorawanMacTest : public TestCase
{
public:
  LorawanMacTest ();
  virtual ~LorawanMacTest ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
LorawanMacTest::LorawanMacTest ()
    : TestCase ("Verify that the MAC layer of EDs behaves as expected")
{
}

// Reminder that the test case should clean up after itself
LorawanMacTest::~LorawanMacTest ()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
LorawanMacTest::DoRun (void)
{
  NS_LOG_DEBUG ("LorawanMacTest");
}

/**************
 * Test Suite *
 **************/

// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined

class LorawanTestSuite : public TestSuite
{
public:
  LorawanTestSuite ();
};

LorawanTestSuite::LorawanTestSuite () : TestSuite ("lorawan", UNIT)
{
  LogComponentEnable ("LorawanTestSuite", LOG_LEVEL_DEBUG);
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new InterferenceTest, TestCase::QUICK);
  AddTestCase (new AddressTest, TestCase::QUICK);
  AddTestCase (new HeaderTest, TestCase::QUICK);
  AddTestCase (new ReceivePathTest, TestCase::QUICK);
  AddTestCase (new LogicalLoraChannelTest, TestCase::QUICK);
  AddTestCase (new TimeOnAirTest, TestCase::QUICK);
  AddTestCase (new PhyConnectivityTest, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static LorawanTestSuite lorawanTestSuite;
