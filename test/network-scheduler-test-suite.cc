/*
 * Copyright (c) 2018 University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

// Include headers of classes to test
#include "ns3/log.h"
#include "ns3/network-scheduler.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("NetworkSchedulerTestSuite");

/**
 * \ingroup lorawan
 *
 * It tests the correct functionality of the NetworkScheduler component class of the network server
 */
class NetworkSchedulerTest : public TestCase
{
  public:
    NetworkSchedulerTest();           //!< Default constructor
    ~NetworkSchedulerTest() override; //!< Destructor

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
NetworkSchedulerTest::NetworkSchedulerTest()
    : TestCase("Verify correct behavior of the NetworkScheduler object")
{
}

// Reminder that the test case should clean up after itself
NetworkSchedulerTest::~NetworkSchedulerTest()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
NetworkSchedulerTest::DoRun()
{
    NS_LOG_DEBUG("NetworkSchedulerTest");

    // If a packet is received at the network server, a reply event should be
    // scheduled to happen 1 second after the reception.
}

/**
 * \ingroup lorawan
 *
 * The TestSuite class names the TestSuite, identifies what type of TestSuite, and enables the
 * TestCases to be run. Typically, only the constructor for this class must be defined
 */
class NetworkSchedulerTestSuite : public TestSuite
{
  public:
    NetworkSchedulerTestSuite(); //!< Default constructor
};

NetworkSchedulerTestSuite::NetworkSchedulerTestSuite()
    : TestSuite("network-scheduler", Type::UNIT)
{
    LogComponentEnable("NetworkSchedulerTestSuite", LOG_LEVEL_DEBUG);
    // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
    AddTestCase(new NetworkSchedulerTest, Duration::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NetworkSchedulerTestSuite lorawanTestSuite;
