/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// Include headers of classes to test
#include "ns3/log.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("NetworkServerTest");

class NetworkServerTest : public TestCase
{
public:
  NetworkServerTest ();
  virtual ~NetworkServerTest ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
NetworkServerTest::NetworkServerTest ()
  : TestCase ("Verify correct initialization of the Network Server")
{
}

// Reminder that the test case should clean up after itself
NetworkServerTest::~NetworkServerTest ()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
NetworkServerTest::DoRun (void)
{
  NS_LOG_DEBUG ("NetworkServerTest");

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
  : TestSuite ("lorawan", UNIT)
{
  LogComponentEnable ("NetworkServerTest", LOG_LEVEL_DEBUG);
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new NetworkServerTest, TestCase::QUICK); // TODO
}

// Do not forget to allocate an instance of this TestSuite
static NetworkServerTestSuite networkServerTestSuite;
