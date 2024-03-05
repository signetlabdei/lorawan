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

/*
 * This file includes testing for the following components:
 * - EndDeviceStatus
 * - GatewayStatus
 * - NetworkStatus
 */

// Include headers of classes to test
#include "utilities.h"

#include "ns3/end-device-status.h"
#include "ns3/log.h"
#include "ns3/network-status.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("NetworkStatusTestSuite");

/////////////////////////////
// EndDeviceStatus testing //
/////////////////////////////

class EndDeviceStatusTest : public TestCase
{
  public:
    EndDeviceStatusTest();
    ~EndDeviceStatusTest() override;

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
EndDeviceStatusTest::EndDeviceStatusTest()
    : TestCase("Verify correct behavior of the EndDeviceStatus object")
{
}

// Reminder that the test case should clean up after itself
EndDeviceStatusTest::~EndDeviceStatusTest()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
EndDeviceStatusTest::DoRun()
{
    NS_LOG_DEBUG("EndDeviceStatusTest");

    // Create an EndDeviceStatus object
    EndDeviceStatus eds = EndDeviceStatus();
}

/////////////////////////////
// NetworkStatus testing //
/////////////////////////////

class NetworkStatusTest : public TestCase
{
  public:
    NetworkStatusTest();
    ~NetworkStatusTest() override;

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
NetworkStatusTest::NetworkStatusTest()
    : TestCase("Verify correct behavior of the NetworkStatus object")
{
}

// Reminder that the test case should clean up after itself
NetworkStatusTest::~NetworkStatusTest()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
NetworkStatusTest::DoRun()
{
    NS_LOG_DEBUG("NetworkStatusTest");

    // Create a NetworkStatus object
    NetworkStatus ns = NetworkStatus();

    // Create a bunch of actual devices
    NetworkComponents components = InitializeNetwork(1, 1);

    Ptr<LoraChannel> channel = components.channel;
    NodeContainer endDevices = components.endDevices;
    NodeContainer gateways = components.gateways;

    ns.AddNode(GetMacLayerFromNode<ClassAEndDeviceLorawanMac>(endDevices.Get(0)));
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
    NetworkStatusTestSuite();
};

NetworkStatusTestSuite::NetworkStatusTestSuite()
    : TestSuite("network-status", UNIT)
{
    LogComponentEnable("NetworkStatusTestSuite", LOG_LEVEL_DEBUG);
    // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
    AddTestCase(new EndDeviceStatusTest, TestCase::QUICK);
    AddTestCase(new NetworkStatusTest, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NetworkStatusTestSuite lorawanTestSuite;
