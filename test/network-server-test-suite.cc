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

NS_LOG_COMPONENT_DEFINE ("NetworkServerTestSuite");

/////////////////////////////
// NetworkServer testing //
/////////////////////////////

class NetworkServerTest : public TestCase
{
public:
  NetworkServerTest ();
  virtual ~NetworkServerTest ();

  void ReceivedPacket (Ptr<Packet const> packet);
  void SendPacket (Ptr<Node> endDevice);

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
NetworkServerTest::NetworkServerTest ()
  : TestCase ("Verify correct behavior of the NetworkServer object")
{
}

// Reminder that the test case should clean up after itself
NetworkServerTest::~NetworkServerTest ()
{
}

void
NetworkServerTest::ReceivedPacket (Ptr<Packet const> packet)
{
  NS_LOG_DEBUG ("Received a packet at the NS");
}

void
NetworkServerTest::SendPacket (Ptr<Node> endDevice)
{
  endDevice->GetDevice(0)->Send(Create<Packet> (20), Address(), 0);
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
NetworkServerTest::DoRun (void)
{
  NS_LOG_DEBUG ("NetworkServerTest");

  // Create a bunch of actual devices
  NetworkComponents components = InitializeNetwork(1, 1);

  Ptr<LoraChannel> channel = components.channel;
  NodeContainer endDevices = components.endDevices;
  NodeContainer gateways = components.gateways;

  // Create the NetworkServer
  NetworkServerHelper networkServerHelper = NetworkServerHelper();
  networkServerHelper.SetEndDevices (endDevices);
  networkServerHelper.SetGateways (gateways);
  Ptr<Node> nsNode = CreateObject<Node> ();
  networkServerHelper.Install (nsNode); // This connects NS and GWs

  // Connect the trace source for received packets
  nsNode->GetApplication (0)->TraceConnectWithoutContext
    ("ReceivedPacket",
     MakeCallback
     (&NetworkServerTest::ReceivedPacket,
      this));

  // Send a packet
  Simulator::Schedule(Seconds(1), &NetworkServerTest::SendPacket, this,
                      endDevices.Get(0));

  Simulator::Stop(Seconds(5));
  Simulator::Run();
  Simulator::Destroy();

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
  LogComponentEnable ("LoraNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLoraMac", LOG_LEVEL_ALL);

  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new NetworkServerTest, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NetworkServerTestSuite lorawanTestSuite;
