/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This file includes testing for the following components:
 * - EndDeviceServer
 * - GatewayServer
 * - NetworkServer
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

// Include headers of classes to test
#include "ns3/log.h"
#include "utilities.h"
#include "ns3/core-module.h"
#include "ns3/callback.h"
#include "ns3/network-server.h"
#include "ns3/network-server-helper.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("NetworkServerTestSuite");

///////////////////////////
// NetworkServer testing //
///////////////////////////

class UplinkPacketTest : public TestCase
{
public:
  UplinkPacketTest ();
  virtual ~UplinkPacketTest ();

  void ReceivedPacket (Ptr<Packet const> packet);
  void SendPacket (Ptr<Node> endDevice);

private:
  virtual void DoRun (void);
  bool m_receivedPacket = false;
};

// Add some help text to this case to describe what it is intended to test
UplinkPacketTest::UplinkPacketTest ()
  : TestCase ("Verify that the NetworkServer can receive"
              " packets sent in the uplink by devices")
{
}

// Reminder that the test case should clean up after itself
UplinkPacketTest::~UplinkPacketTest ()
{
}

void
UplinkPacketTest::ReceivedPacket (Ptr<Packet const> packet)
{
  NS_LOG_DEBUG ("Received a packet at the NS");
  m_receivedPacket = true;
}

void
UplinkPacketTest::SendPacket (Ptr<Node> endDevice)
{
  endDevice->GetDevice (0)->Send (Create<Packet> (20), Address (), 0);
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
UplinkPacketTest::DoRun (void)
{
  NS_LOG_DEBUG ("UplinkPacketTest");

  // Create a bunch of actual devices
  NetworkComponents components = InitializeNetwork (1, 1);

  Ptr<LoraChannel> channel = components.channel;
  NodeContainer endDevices = components.endDevices;
  NodeContainer gateways = components.gateways;
  Ptr<Node> nsNode = components.nsNode;

  // Connect the trace source for received packets
  nsNode->GetApplication (0)->TraceConnectWithoutContext
    ("ReceivedPacket",
    MakeCallback
      (&UplinkPacketTest::ReceivedPacket,
      this));

  // Send a packet
  Simulator::Schedule (Seconds (1), &UplinkPacketTest::SendPacket, this,
                       endDevices.Get (0));

  Simulator::Stop (Seconds (5));
  Simulator::Run ();
  Simulator::Destroy ();

  // Check that we received the packet
  NS_ASSERT (m_receivedPacket == true);
}

////////////////////////
// DownlinkPacketTest //
////////////////////////

class DownlinkPacketTest : public TestCase
{
public:
  DownlinkPacketTest ();
  virtual ~DownlinkPacketTest ();

  void ReceivedPacketAtEndDevice (uint8_t requiredTransmissions, bool success,
                                  Time time, Ptr<Packet> packet);
  void SendPacket (Ptr<Node> endDevice, bool requestAck);

private:
  virtual void DoRun (void);
  bool m_receivedPacketAtEd = false;
};

// Add some help text to this case to describe what it is intended to test
DownlinkPacketTest::DownlinkPacketTest ()
  : TestCase ("Verify that devices requesting an acknowledgment receive"
              " a reply from the Network Server.")
{
}

// Reminder that the test case should clean up after itself
DownlinkPacketTest::~DownlinkPacketTest ()
{
}

void
DownlinkPacketTest::ReceivedPacketAtEndDevice (uint8_t requiredTransmissions, bool success, Time time, Ptr<Packet> packet)
{
  NS_LOG_DEBUG ("Received a packet at the ED");
  m_receivedPacketAtEd = success;
}

void
DownlinkPacketTest::SendPacket (Ptr<Node> endDevice, bool requestAck)
{
  if (requestAck)
    {
      endDevice->GetDevice (0)->GetObject<LoraNetDevice> ()->GetMac
        ()->GetObject<EndDeviceLorawanMac> ()->SetMType
        (LorawanMacHeader::CONFIRMED_DATA_UP);
    }
  endDevice->GetDevice (0)->Send (Create<Packet> (20), Address (), 0);
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
DownlinkPacketTest::DoRun (void)
{
  NS_LOG_DEBUG ("DownlinkPacketTest");

  // Create a bunch of actual devices
  NetworkComponents components = InitializeNetwork (1, 1);

  Ptr<LoraChannel> channel = components.channel;
  NodeContainer endDevices = components.endDevices;
  NodeContainer gateways = components.gateways;
  Ptr<Node> nsNode = components.nsNode;

  // Connect the ED's trace source for received packets
  endDevices.Get (0)->GetDevice (0)->GetObject<LoraNetDevice>()->GetMac ()->GetObject<EndDeviceLorawanMac>()->TraceConnectWithoutContext ("RequiredTransmissions", MakeCallback (&DownlinkPacketTest::ReceivedPacketAtEndDevice, this));

  // Send a packet in uplink
  Simulator::Schedule (Seconds (1), &DownlinkPacketTest::SendPacket, this,
                       endDevices.Get (0), true);

  Simulator::Stop (Seconds (10)); // Allow for time to receive a downlink packet
  Simulator::Run ();
  Simulator::Destroy ();

  NS_ASSERT (m_receivedPacketAtEd);
}

//////////////////////////
// LinkCheckSupportTest //
//////////////////////////

class LinkCheckTest : public TestCase
{
public:
  LinkCheckTest ();
  virtual ~LinkCheckTest ();

  void LastKnownGatewayCount (int newValue, int oldValue);

  void SendPacket (Ptr<Node> endDevice, bool requestAck);

private:
  virtual void DoRun (void);
  bool m_receivedPacketAtEd = false;
  int m_numberOfGatewaysThatReceivedPacket = 0;
};

// Add some help text to this case to describe what it is intended to test
LinkCheckTest::LinkCheckTest ()
  : TestCase ("Verify that the NetworkServer correctly responds to "
              "LinkCheck requests")
{
}

// Reminder that the test case should clean up after itself
LinkCheckTest::~LinkCheckTest ()
{
}

void
LinkCheckTest::LastKnownGatewayCount (int newValue,
                                      int oldValue)
{
  NS_LOG_DEBUG ("Updated Gateway Count");
  m_receivedPacketAtEd = true;

  m_numberOfGatewaysThatReceivedPacket = newValue;
}

void
LinkCheckTest::SendPacket (Ptr<Node> endDevice, bool requestAck)
{
  Ptr<EndDeviceLorawanMac> macLayer = endDevice->GetDevice
      (0)->GetObject<LoraNetDevice> ()->GetMac ()->GetObject<EndDeviceLorawanMac> ();

  if (requestAck)
    {
      macLayer->SetMType (LorawanMacHeader::CONFIRMED_DATA_UP);
    }

  macLayer->AddMacCommand (Create<LinkCheckReq> ());

  endDevice->GetDevice (0)->Send (Create<Packet> (20), Address (), 0);
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
LinkCheckTest::DoRun (void)
{
  NS_LOG_DEBUG ("LinkCheckTest");

  // Create a bunch of actual devices
  NetworkComponents components = InitializeNetwork (1, 1);

  Ptr<LoraChannel> channel = components.channel;
  NodeContainer endDevices = components.endDevices;
  NodeContainer gateways = components.gateways;
  Ptr<Node> nsNode = components.nsNode;

  // Connect the ED's trace source for Last known Gateway Count
  endDevices.Get (0)->GetDevice (0)->GetObject<LoraNetDevice>()->GetMac ()->GetObject<EndDeviceLorawanMac>()->TraceConnectWithoutContext ("LastKnownGatewayCount", MakeCallback (&LinkCheckTest::LastKnownGatewayCount, this));

  // Send a packet in uplink
  Simulator::Schedule (Seconds (1), &LinkCheckTest::SendPacket, this,
                       endDevices.Get (0), true);

  Simulator::Stop (Seconds (10)); // Allow for time to receive a downlink packet
  Simulator::Run ();
  Simulator::Destroy ();

  NS_ASSERT (m_receivedPacketAtEd);
}

/**************
 * Test Suite *
 **************/

// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined

class NetworkServerTestSuite : public TestSuite
{
public:
  NetworkServerTestSuite ();
};

NetworkServerTestSuite::NetworkServerTestSuite ()
  : TestSuite ("network-server", UNIT)
{
  LogComponentEnable ("NetworkServerTestSuite", LOG_LEVEL_DEBUG);

  LogComponentEnable ("NetworkServer", LOG_LEVEL_ALL);
  LogComponentEnable ("NetworkStatus", LOG_LEVEL_ALL);
  LogComponentEnable ("NetworkScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("NetworkController", LOG_LEVEL_ALL);
  LogComponentEnable ("NetworkControllerComponent", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLorawanMac", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLorawanMac", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceLoraPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("EndDeviceStatus", LOG_LEVEL_ALL);

  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new UplinkPacketTest, TestCase::QUICK);
  AddTestCase (new DownlinkPacketTest, TestCase::QUICK);
  AddTestCase (new LinkCheckTest, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NetworkServerTestSuite lorawanTestSuite;
