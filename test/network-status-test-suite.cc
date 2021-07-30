/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This file includes testing for the following components:
 * - EndDeviceStatus
 * - GatewayStatus
 * - NetworkStatus
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
*/

// Include headers of classes to test
#include "ns3/log.h"
#include "ns3/end-device-status.h"
#include "ns3/network-status.h"
#include "utilities.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("NetworkStatusTestSuite");

/////////////////////////////
// EndDeviceStatus testing //
/////////////////////////////

class EndDeviceStatusTest : public TestCase
{
public:
  EndDeviceStatusTest ();
  virtual ~EndDeviceStatusTest ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
EndDeviceStatusTest::EndDeviceStatusTest ()
  : TestCase ("Verify correct behavior of the EndDeviceStatus object")
{
}

// Reminder that the test case should clean up after itself
EndDeviceStatusTest::~EndDeviceStatusTest ()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
EndDeviceStatusTest::DoRun (void)
{
  NS_LOG_DEBUG ("EndDeviceStatusTest");

  // Create an EndDeviceStatus object
  EndDeviceStatus eds = EndDeviceStatus ();
}

/////////////////////////////
// NetworkStatus testing //
/////////////////////////////

class NetworkStatusTest : public TestCase
{
public:
  NetworkStatusTest ();
  virtual ~NetworkStatusTest ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
NetworkStatusTest::NetworkStatusTest ()
  : TestCase ("Verify correct behavior of the NetworkStatus object")
{
}

// Reminder that the test case should clean up after itself
NetworkStatusTest::~NetworkStatusTest ()
{
}


// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
NetworkStatusTest::DoRun (void)
{
  NS_LOG_DEBUG ("NetworkStatusTest");

  // Create a NetworkStatus object
  NetworkStatus ns = NetworkStatus ();

  // Create a bunch of actual devices
  NetworkComponents components = InitializeNetwork (1, 1);

  Ptr<LoraChannel> channel = components.channel;
  NodeContainer endDevices = components.endDevices;
  NodeContainer gateways = components.gateways;

  ns.AddNode (GetMacLayerFromNode<ClassAEndDeviceLorawanMac> (endDevices.Get (0)));
}

/**************
 * Test Suite *
 **************/

// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined

class NetworkStatusTestSuite : public TestSuite
{
public:
  NetworkStatusTestSuite ();
};

NetworkStatusTestSuite::NetworkStatusTestSuite ()
  : TestSuite ("network-status", UNIT)
{
  LogComponentEnable ("NetworkStatusTestSuite", LOG_LEVEL_DEBUG);
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new EndDeviceStatusTest, TestCase::QUICK);
  AddTestCase (new NetworkStatusTest, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NetworkStatusTestSuite lorawanTestSuite;
