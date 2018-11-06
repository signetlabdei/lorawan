/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

// Include headers of classes to test
#include "ns3/log.h"
#include "ns3/network-scheduler.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE ("NetworkSchedulerTestSuite");

/////////////////////////////
// NetworkStatus testing //
/////////////////////////////

class NetworkSchedulerTest : public TestCase
{
public:
  NetworkSchedulerTest ();
  virtual ~NetworkSchedulerTest ();

private:
  virtual void DoRun (void);
};

// Add some help text to this case to describe what it is intended to test
NetworkSchedulerTest::NetworkSchedulerTest ()
  : TestCase ("Verify correct behavior of the NetworkScheduler object")
{
}

// Reminder that the test case should clean up after itself
NetworkSchedulerTest::~NetworkSchedulerTest ()
{
}


// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
NetworkSchedulerTest::DoRun (void)
{
  NS_LOG_DEBUG ("NetworkSchedulerTest");

  // If a packet is received at the network server, a reply event should be
  // scheduled to happen 1 second after the reception.
}

/**************
 * Test Suite *
 **************/

// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined

class NetworkSchedulerTestSuite : public TestSuite
{
public:
  NetworkSchedulerTestSuite ();
};

NetworkSchedulerTestSuite::NetworkSchedulerTestSuite ()
  : TestSuite ("network-scheduler", UNIT)
{
  LogComponentEnable ("NetworkSchedulerTestSuite", LOG_LEVEL_DEBUG);
  // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
  AddTestCase (new NetworkSchedulerTest, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NetworkSchedulerTestSuite lorawanTestSuite;
