/*
 * Copyright (c) 2018 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
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

/**
 * @ingroup lorawan
 *
 * It tests the constructor of the EndDeviceStatus class
 */
class EndDeviceStatusTest : public TestCase
{
  public:
    EndDeviceStatusTest();           //!< Default constructor
    ~EndDeviceStatusTest() override; //!< Destructor

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

/**
 * @ingroup lorawan
 *
 * It tests the function NetworkStatus::AddNode
 */
class NetworkStatusTest : public TestCase
{
  public:
    NetworkStatusTest();           //!< Default constructor
    ~NetworkStatusTest() override; //!< Destructor

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

/**
 * @ingroup lorawan
 *
 * The TestSuite class names the TestSuite, identifies what type of TestSuite, and enables the
 * TestCases to be run. Typically, only the constructor for this class must be defined
 */
class NetworkStatusTestSuite : public TestSuite
{
  public:
    NetworkStatusTestSuite(); //!< Default constructor
};

NetworkStatusTestSuite::NetworkStatusTestSuite()
    : TestSuite("network-status", Type::UNIT)
{
    // LogComponentEnable("NetworkStatusTestSuite", LOG_LEVEL_DEBUG);

    AddTestCase(new EndDeviceStatusTest, Duration::QUICK);
    AddTestCase(new NetworkStatusTest, Duration::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NetworkStatusTestSuite lorawanTestSuite;
