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
 * - EndDeviceServer
 * - GatewayServer
 * - NetworkServer
 */

// Include headers of classes to test
#include "utilities.h"

#include "ns3/callback.h"
#include "ns3/core-module.h"
#include "ns3/log.h"
#include "ns3/network-server-helper.h"
#include "ns3/network-server.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("NetworkServerTestSuite");

/**
 * \ingroup lorawan
 *
 * It verifies that the NetworkServer can receive packets sent in uplink by devices
 */
class UplinkPacketTest : public TestCase
{
  public:
    UplinkPacketTest();           //!< Default constructor
    ~UplinkPacketTest() override; //!< Destructor

    /**
     * Callback for tracing ReceivedPacket.
     *
     * \param packet The packet received.
     */
    void ReceivedPacket(Ptr<const Packet> packet);

    /**
     * Send a packet from the input end device.
     *
     * \param endDevice A pointer to the end device Node.
     */
    void SendPacket(Ptr<Node> endDevice);

  private:
    void DoRun() override;

    bool m_receivedPacket = false; //!< Set to true if a packet is received by the server
};

// Add some help text to this case to describe what it is intended to test
UplinkPacketTest::UplinkPacketTest()
    : TestCase("Verify that the NetworkServer can receive"
               " packets sent in the uplink by devices")
{
}

// Reminder that the test case should clean up after itself
UplinkPacketTest::~UplinkPacketTest()
{
}

void
UplinkPacketTest::ReceivedPacket(Ptr<const Packet> packet)
{
    NS_LOG_DEBUG("Received a packet at the NS");
    m_receivedPacket = true;
}

void
UplinkPacketTest::SendPacket(Ptr<Node> endDevice)
{
    endDevice->GetDevice(0)->Send(Create<Packet>(20), Address(), 0);
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
UplinkPacketTest::DoRun()
{
    NS_LOG_DEBUG("UplinkPacketTest");

    // Create a bunch of actual devices
    NetworkComponents components = InitializeNetwork(1, 1);

    Ptr<LoraChannel> channel = components.channel;
    NodeContainer endDevices = components.endDevices;
    NodeContainer gateways = components.gateways;
    Ptr<Node> nsNode = components.nsNode;

    // Connect the trace source for received packets
    nsNode->GetApplication(0)->TraceConnectWithoutContext(
        "ReceivedPacket",
        MakeCallback(&UplinkPacketTest::ReceivedPacket, this));

    // Send a packet
    Simulator::Schedule(Seconds(1), &UplinkPacketTest::SendPacket, this, endDevices.Get(0));

    Simulator::Stop(Seconds(5));
    Simulator::Run();
    Simulator::Destroy();

    // Check that we received the packet
    NS_ASSERT(m_receivedPacket == true);
}

/**
 * \ingroup lorawan
 *
 * It verifies that devices requesting an acknowledgment receive a reply from the Network Server
 */
class DownlinkPacketTest : public TestCase
{
  public:
    DownlinkPacketTest();           //!< Default constructor
    ~DownlinkPacketTest() override; //!< Destructor

    /**
     * Record the exit status of a MAC layer packet retransmission process of an end device.
     *
     * This trace sink is only used here to determine whether an ack was received by the end device
     * after sending a package requiring an acknowledgement.
     *
     * \param requiredTransmissions Number of transmissions attempted during the process.
     * \param success Whether the retransmission procedure was successful.
     * \param time Timestamp of the initial transmission attempt.
     * \param packet The packet being retransmitted.
     */
    void ReceivedPacketAtEndDevice(uint8_t requiredTransmissions,
                                   bool success,
                                   Time time,
                                   Ptr<Packet> packet);

    /**
     * Send a packet from the input end device.
     *
     * \param endDevice A pointer to the end device Node.
     * \param requestAck Whether to require an acknowledgement from the server.
     */
    void SendPacket(Ptr<Node> endDevice, bool requestAck);

  private:
    void DoRun() override;

    bool m_receivedPacketAtEd = false; //!< Set to true if a packet is received by the end device
};

// Add some help text to this case to describe what it is intended to test
DownlinkPacketTest::DownlinkPacketTest()
    : TestCase("Verify that devices requesting an acknowledgment receive"
               " a reply from the Network Server.")
{
}

// Reminder that the test case should clean up after itself
DownlinkPacketTest::~DownlinkPacketTest()
{
}

void
DownlinkPacketTest::ReceivedPacketAtEndDevice(uint8_t requiredTransmissions,
                                              bool success,
                                              Time time,
                                              Ptr<Packet> packet)
{
    NS_LOG_DEBUG("Received a packet at the ED");
    m_receivedPacketAtEd = success;
}

void
DownlinkPacketTest::SendPacket(Ptr<Node> endDevice, bool requestAck)
{
    if (requestAck)
    {
        endDevice->GetDevice(0)
            ->GetObject<LoraNetDevice>()
            ->GetMac()
            ->GetObject<EndDeviceLorawanMac>()
            ->SetMType(LorawanMacHeader::CONFIRMED_DATA_UP);
    }
    endDevice->GetDevice(0)->Send(Create<Packet>(20), Address(), 0);
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
DownlinkPacketTest::DoRun()
{
    NS_LOG_DEBUG("DownlinkPacketTest");

    // Create a bunch of actual devices
    NetworkComponents components = InitializeNetwork(1, 1);

    Ptr<LoraChannel> channel = components.channel;
    NodeContainer endDevices = components.endDevices;
    NodeContainer gateways = components.gateways;
    Ptr<Node> nsNode = components.nsNode;

    // Connect the ED's trace source for received packets
    endDevices.Get(0)
        ->GetDevice(0)
        ->GetObject<LoraNetDevice>()
        ->GetMac()
        ->GetObject<EndDeviceLorawanMac>()
        ->TraceConnectWithoutContext(
            "RequiredTransmissions",
            MakeCallback(&DownlinkPacketTest::ReceivedPacketAtEndDevice, this));

    // Send a packet in uplink
    Simulator::Schedule(Seconds(1), &DownlinkPacketTest::SendPacket, this, endDevices.Get(0), true);

    Simulator::Stop(Seconds(10)); // Allow for time to receive a downlink packet
    Simulator::Run();
    Simulator::Destroy();

    NS_ASSERT(m_receivedPacketAtEd);
}

/**
 * \ingroup lorawan
 *
 * It verifies that the NetworkServer correctly responds to LinkCheck requests
 */
class LinkCheckTest : public TestCase
{
  public:
    LinkCheckTest();           //!< Default constructor
    ~LinkCheckTest() override; //!< Destructor

    /**
     * Trace changes in the last known gateway count variable (updated on reception of
     * LinkCheckAns MAC commands) of an end device.
     *
     * \param newValue The updated value.
     * \param oldValue The previous value.
     */
    void LastKnownGatewayCount(int newValue, int oldValue);

    /**
     * Send a packet containing a LinkCheckReq MAC command from the input end device.
     *
     * \param endDevice A pointer to the end device Node.
     * \param requestAck Whether to require an acknowledgement from the server.
     */
    void SendPacket(Ptr<Node> endDevice, bool requestAck);

  private:
    void DoRun() override;
    bool m_receivedPacketAtEd = false; //!< Set to true if a packet containing a LinkCheckAns MAC
                                       //!< command is received by the end device
    int m_numberOfGatewaysThatReceivedPacket =
        0; //!< Stores the number of gateways that received the last packet carrying a
           //!< LinkCheckReq MAC command
};

// Add some help text to this case to describe what it is intended to test
LinkCheckTest::LinkCheckTest()
    : TestCase("Verify that the NetworkServer correctly responds to "
               "LinkCheck requests")
{
}

// Reminder that the test case should clean up after itself
LinkCheckTest::~LinkCheckTest()
{
}

void
LinkCheckTest::LastKnownGatewayCount(int newValue, int oldValue)
{
    NS_LOG_DEBUG("Updated gateway count");
    m_receivedPacketAtEd = true;

    m_numberOfGatewaysThatReceivedPacket = newValue;
}

void
LinkCheckTest::SendPacket(Ptr<Node> endDevice, bool requestAck)
{
    Ptr<EndDeviceLorawanMac> macLayer = endDevice->GetDevice(0)
                                            ->GetObject<LoraNetDevice>()
                                            ->GetMac()
                                            ->GetObject<EndDeviceLorawanMac>();

    if (requestAck)
    {
        macLayer->SetMType(LorawanMacHeader::CONFIRMED_DATA_UP);
    }

    macLayer->AddMacCommand(Create<LinkCheckReq>());

    endDevice->GetDevice(0)->Send(Create<Packet>(20), Address(), 0);
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
LinkCheckTest::DoRun()
{
    NS_LOG_DEBUG("LinkCheckTest");

    // Create a bunch of actual devices
    NetworkComponents components = InitializeNetwork(1, 1);

    Ptr<LoraChannel> channel = components.channel;
    NodeContainer endDevices = components.endDevices;
    NodeContainer gateways = components.gateways;
    Ptr<Node> nsNode = components.nsNode;

    // Connect the ED's trace source for last known gateway count
    endDevices.Get(0)
        ->GetDevice(0)
        ->GetObject<LoraNetDevice>()
        ->GetMac()
        ->GetObject<EndDeviceLorawanMac>()
        ->TraceConnectWithoutContext("LastKnownGatewayCount",
                                     MakeCallback(&LinkCheckTest::LastKnownGatewayCount, this));

    // Send a packet in uplink
    Simulator::Schedule(Seconds(1), &LinkCheckTest::SendPacket, this, endDevices.Get(0), true);

    Simulator::Stop(Seconds(10)); // Allow for time to receive a downlink packet
    Simulator::Run();
    Simulator::Destroy();

    NS_ASSERT(m_receivedPacketAtEd);
}

/**
 * \ingroup lorawan
 *
 * The TestSuite class names the TestSuite, identifies what type of TestSuite, and enables the
 * TestCases to be run. Typically, only the constructor for this class must be defined
 */
class NetworkServerTestSuite : public TestSuite
{
  public:
    NetworkServerTestSuite(); //!< Default constructor
};

NetworkServerTestSuite::NetworkServerTestSuite()
    : TestSuite("network-server", UNIT)
{
    LogComponentEnable("NetworkServerTestSuite", LOG_LEVEL_DEBUG);

    LogComponentEnable("NetworkServer", LOG_LEVEL_ALL);
    LogComponentEnable("NetworkStatus", LOG_LEVEL_ALL);
    LogComponentEnable("NetworkScheduler", LOG_LEVEL_ALL);
    LogComponentEnable("NetworkController", LOG_LEVEL_ALL);
    LogComponentEnable("NetworkControllerComponent", LOG_LEVEL_ALL);
    LogComponentEnable("LoraNetDevice", LOG_LEVEL_ALL);
    LogComponentEnable("GatewayLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("EndDeviceLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("EndDeviceLoraPhy", LOG_LEVEL_ALL);
    LogComponentEnable("EndDeviceStatus", LOG_LEVEL_ALL);

    LogComponentEnableAll(LOG_PREFIX_FUNC);
    LogComponentEnableAll(LOG_PREFIX_NODE);
    LogComponentEnableAll(LOG_PREFIX_TIME);

    // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
    AddTestCase(new UplinkPacketTest, TestCase::QUICK);
    AddTestCase(new DownlinkPacketTest, TestCase::QUICK);
    AddTestCase(new LinkCheckTest, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static NetworkServerTestSuite lorawanTestSuite;
