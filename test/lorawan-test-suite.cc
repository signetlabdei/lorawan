/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

// Include headers of classes to test
#include "ns3/constant-position-mobility-model.h"
#include "ns3/log.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/simple-end-device-lora-phy.h"
#include "ns3/simple-gateway-lora-phy.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("LorawanTestSuite");

/**
 * @ingroup lorawan
 *
 * It tests interference computations in a number of possible scenarios using the
 * LoraInterferenceHelper class
 */
class InterferenceTest : public TestCase
{
  public:
    InterferenceTest();           //!< Default constructor
    ~InterferenceTest() override; //!< Destructor

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
InterferenceTest::InterferenceTest()
    : TestCase("Verify that LoraInterferenceHelper works as expected")
{
}

// Reminder that the test case should clean up after itself
InterferenceTest::~InterferenceTest()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
InterferenceTest::DoRun()
{
    NS_LOG_DEBUG("InterferenceTest");

    LoraInterferenceHelper interferenceHelper;

    uint32_t frequencyHz = 868100000;
    uint32_t differentFrequencyHz = 868300000;

    Ptr<LoraInterferenceHelper::Event> event;
    Ptr<LoraInterferenceHelper::Event> event1;

    // Test overlap duration
    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    event1 = interferenceHelper.Add(Seconds(1), 14, 12, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.GetOverlapTime(event, event1),
                          Seconds(1),
                          "Overlap computation didn't give the expected result");
    interferenceHelper.ClearAllEvents();

    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    event1 = interferenceHelper.Add(Seconds(1.5), 14, 12, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.GetOverlapTime(event, event1),
                          Seconds(1.5),
                          "Overlap computation didn't give the expected result");
    interferenceHelper.ClearAllEvents();

    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    event1 = interferenceHelper.Add(Seconds(3), 14, 12, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.GetOverlapTime(event, event1),
                          Seconds(2),
                          "Overlap computation didn't give the expected result");
    interferenceHelper.ClearAllEvents();

    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    event1 = interferenceHelper.Add(Seconds(2), 14, 12, nullptr, frequencyHz);
    // Because of some strange behavior, this test would get stuck if we used the same syntax of the
    // previous ones. This works instead.
    bool retval = interferenceHelper.GetOverlapTime(event, event1) == Seconds(2);
    NS_TEST_EXPECT_MSG_EQ(retval, true, "Overlap computation didn't give the expected result");
    interferenceHelper.ClearAllEvents();

    // Perfect overlap, packet survives
    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14, 12, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");
    interferenceHelper.ClearAllEvents();

    // Perfect overlap, packet survives
    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14 - 7, 7, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");
    interferenceHelper.ClearAllEvents();

    // Perfect overlap, packet destroyed
    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14 - 6, 7, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.IsDestroyedByInterference(event),
                          7,
                          "Packet was not destroyed by interference as expected");
    interferenceHelper.ClearAllEvents();

    // Partial overlap, packet survives
    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(1), 14 - 6, 7, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");
    interferenceHelper.ClearAllEvents();

    // Different frequencys
    // Packet would be destroyed if they were on the same frequency, but survives
    // since they are on different frequencies
    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14, 7, nullptr, differentFrequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");
    interferenceHelper.ClearAllEvents();

    // Different SFs
    // Packet would be destroyed if they both were SF7, but survives thanks to spreading factor
    // semi-orthogonality
    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14 + 16, 8, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");
    interferenceHelper.ClearAllEvents();

    // Spreading factor imperfect orthogonality
    // Different SFs are orthogonal only up to a point
    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14 + 17, 8, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.IsDestroyedByInterference(event),
                          8,
                          "Packet was not destroyed by interference as expected");
    interferenceHelper.ClearAllEvents();

    // If a more 'distant' spreading factor is used, isolation gets better
    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14 + 17, 10, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.IsDestroyedByInterference(event),
                          0,
                          "Packet was destroyed by interference while it should have survived");
    interferenceHelper.ClearAllEvents();

    // Cumulative interference
    // Same spreading factor interference is cumulative
    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14 + 16, 8, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14 + 16, 8, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14 + 16, 8, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.IsDestroyedByInterference(event),
                          8,
                          "Packet was not destroyed by interference as expected");
    interferenceHelper.ClearAllEvents();

    // Cumulative interference
    // Interference is not cumulative between different SFs
    event = interferenceHelper.Add(Seconds(2), 14, 7, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14 + 16, 8, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14 + 16, 9, nullptr, frequencyHz);
    interferenceHelper.Add(Seconds(2), 14 + 16, 10, nullptr, frequencyHz);
    NS_TEST_EXPECT_MSG_EQ(interferenceHelper.IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");
    interferenceHelper.ClearAllEvents();
}

/**
 * @ingroup lorawan
 *
 * It tests LoraDeviceAddress comparison operators overrides and generation of new addresses with
 * LoraDeviceAddressGenerator
 */
class AddressTest : public TestCase
{
  public:
    AddressTest();           //!< Default constructor
    ~AddressTest() override; //!< Destructor

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
AddressTest::AddressTest()
    : TestCase("Verify that LoraDeviceAddress works as expected")
{
}

// Reminder that the test case should clean up after itself
AddressTest::~AddressTest()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
AddressTest::DoRun()
{
    NS_LOG_DEBUG("AddressTest");

    //////////////////////////////////////
    // Test the LoraDeviceAddress class //
    //////////////////////////////////////

    // Address equality
    LoraDeviceAddress firstAddress(0xFFFFFFFF);
    LoraDeviceAddress secondAddress(0xFFFFFFFF);
    NS_TEST_EXPECT_MSG_EQ((firstAddress == secondAddress), true, "Addresses don't match");

    // Address ordering
    LoraDeviceAddress bigAddress(0xFFFFFF00);
    LoraDeviceAddress smallAddress(0xFFF00000);
    NS_TEST_EXPECT_MSG_EQ((bigAddress > smallAddress),
                          true,
                          "> function for addresses doesn't work correctly");

    // Setting and getting
    LoraDeviceAddress referenceAddress(0xFFFFFFFF);
    LoraDeviceAddress address(0x00000000);
    NS_TEST_EXPECT_MSG_EQ((address != referenceAddress), true, "Different addresses match!");
    address.SetNwkAddr(0xFFFFFFF);
    address.SetNwkID(0b1111111);
    NS_TEST_EXPECT_MSG_EQ((address == referenceAddress),
                          true,
                          "Addresses set to be equal don't match");

    // Serialization and deserialization
    uint8_t buffer[4];
    LoraDeviceAddress toSerialize(0x0F0F0F0F);
    toSerialize.Serialize(buffer);
    LoraDeviceAddress deserialized = LoraDeviceAddress::Deserialize(buffer);
    NS_TEST_EXPECT_MSG_EQ((toSerialize == deserialized),
                          true,
                          "Serialization + Deserialization doesn't yield an equal address");

    ///////////////////////////////////
    // Test the address generator class
    ///////////////////////////////////

    LoraDeviceAddressGenerator addressGenerator;
    for (int i = 0; i < 200; i++)
    {
        addressGenerator.NextAddress();
    }
    // After 200 iterations, the address should be 0xC9
    NS_TEST_EXPECT_MSG_EQ((addressGenerator.GetNextAddress() == LoraDeviceAddress(0xC9)),
                          true,
                          "LoraDeviceAddressGenerator doesn't increment as expected");
}

/**
 * @ingroup lorawan
 *
 * It tests serialization/deserialization of LoRaWAN headers (the LorawanMacHeader and
 * LoraFrameHeader classes) on packets
 */
class HeaderTest : public TestCase
{
  public:
    HeaderTest();           //!< Default constructor
    ~HeaderTest() override; //!< Destructor

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
HeaderTest::HeaderTest()
    : TestCase("Verify that LorawanMacHeader and LoraFrameHeader work as expected")
{
}

// Reminder that the test case should clean up after itself
HeaderTest::~HeaderTest()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
HeaderTest::DoRun()
{
    NS_LOG_DEBUG("HeaderTest");

    //////////////////////////////////
    // Test the LorawanMacHeader class //
    //////////////////////////////////
    LorawanMacHeader macHdr;
    macHdr.SetMType(LorawanMacHeader::CONFIRMED_DATA_DOWN);
    macHdr.SetMajor(1);

    Buffer macBuf;
    macBuf.AddAtStart(100);
    Buffer::Iterator macSerialized = macBuf.Begin();
    macHdr.Serialize(macSerialized);

    macHdr.Deserialize(macSerialized);

    NS_TEST_EXPECT_MSG_EQ((macHdr.GetMType() == LorawanMacHeader::CONFIRMED_DATA_DOWN),
                          true,
                          "MType changes in the serialization/deserialization process");
    NS_TEST_EXPECT_MSG_EQ((macHdr.GetMajor() == 1),
                          true,
                          "MType changes in the serialization/deserialization process");

    ////////////////////////////////////
    // Test the LoraFrameHeader class //
    ////////////////////////////////////
    LoraFrameHeader frameHdr;
    frameHdr.SetAsDownlink();
    frameHdr.SetAck(true);
    frameHdr.SetAdr(false);
    frameHdr.SetFCnt(1);
    frameHdr.SetAddress(LoraDeviceAddress(56, 1864));
    frameHdr.AddLinkCheckAns(10, 1);

    // Serialization
    Buffer buf;
    buf.AddAtStart(100);
    Buffer::Iterator serialized = buf.Begin();
    frameHdr.Serialize(serialized);

    // Deserialization
    frameHdr.Deserialize(serialized);

    Ptr<LinkCheckAns> command = DynamicCast<LinkCheckAns>(frameHdr.GetCommands().at(0));
    uint8_t margin = command->GetMargin();
    uint8_t gwCnt = command->GetGwCnt();

    NS_TEST_EXPECT_MSG_EQ(frameHdr.GetAck(),
                          true,
                          "ACK bit changes in the serialization/deserialization process");
    NS_TEST_EXPECT_MSG_EQ(frameHdr.GetAdr(),
                          false,
                          "ADR bit changes in the serialization/deserialization process");
    NS_TEST_EXPECT_MSG_EQ(frameHdr.GetFCnt(),
                          1,
                          "FCnt changes in the serialization/deserialization process");
    NS_TEST_EXPECT_MSG_EQ((frameHdr.GetAddress() == LoraDeviceAddress(56, 1864)),
                          true,
                          "Address changes in the serialization/deserialization process");
    NS_TEST_EXPECT_MSG_EQ(margin,
                          10,
                          "Margin changes in the serialization/deserialization process");
    NS_TEST_EXPECT_MSG_EQ(gwCnt, 1, "GwCnt changes in the serialization/deserialization process");

    /////////////////////////////////////////////////
    // Test a combination of the two above classes //
    /////////////////////////////////////////////////
    Ptr<Packet> pkt = Create<Packet>(10);
    pkt->AddHeader(frameHdr);
    pkt->AddHeader(macHdr);

    // Length = Payload + FrameHeader + MacHeader
    //        = 10 + (8+3) + 1 = 22
    NS_TEST_EXPECT_MSG_EQ((pkt->GetSize()), 22, "Wrong size of packet + headers");

    LorawanMacHeader macHdr1;

    pkt->RemoveHeader(macHdr1);

    NS_TEST_EXPECT_MSG_EQ((pkt->GetSize()), 21, "Wrong size of packet + headers - macHeader");

    LoraFrameHeader frameHdr1;
    frameHdr1.SetAsDownlink();

    pkt->RemoveHeader(frameHdr1);
    Ptr<LinkCheckAns> linkCheckAns = DynamicCast<LinkCheckAns>(frameHdr1.GetCommands().at(0));

    NS_TEST_EXPECT_MSG_EQ((pkt->GetSize()),
                          10,
                          "Wrong size of packet + headers - macHeader - frameHeader");

    // Verify contents of removed MAC header
    NS_TEST_EXPECT_MSG_EQ(macHdr1.GetMType(),
                          macHdr.GetMType(),
                          "Removed header contents don't match");
    NS_TEST_EXPECT_MSG_EQ(macHdr1.GetMajor(),
                          macHdr.GetMajor(),
                          "Removed header contents don't match");

    // Verify contents of removed frame header
    NS_TEST_EXPECT_MSG_EQ(frameHdr1.GetAck(),
                          frameHdr.GetAck(),
                          "Removed header contents don't match");
    NS_TEST_EXPECT_MSG_EQ(frameHdr1.GetAdr(),
                          frameHdr.GetAdr(),
                          "Removed header contents don't match");
    NS_TEST_EXPECT_MSG_EQ(frameHdr1.GetFCnt(),
                          frameHdr.GetFCnt(),
                          "Removed header contents don't match");
    NS_TEST_EXPECT_MSG_EQ((frameHdr1.GetAddress() == frameHdr.GetAddress()),
                          true,
                          "Removed header contents don't match");
    NS_TEST_EXPECT_MSG_EQ(linkCheckAns->GetMargin(),
                          10,
                          "Removed header's MAC command contents don't match");
    NS_TEST_EXPECT_MSG_EQ(linkCheckAns->GetGwCnt(),
                          1,
                          "Removed header's MAC command contents don't match");
}

/**
 * @ingroup lorawan
 *
 * It tests a number of cases related to SimpleGatewayLoraPhy's parallel reception paths
 *
 * @todo The test is commented out. To be fixed.
 */
class ReceivePathTest : public TestCase
{
  public:
    ReceivePathTest();           //!< Default constructor
    ~ReceivePathTest() override; //!< Destructor

  private:
    void DoRun() override;
    /**
     * Reset counters and gateway PHY for new sub test case.
     */
    void Reset();
    /**
     * Callback for tracing OccupiedReceptionPaths.
     *
     * @param oldValue The old value.
     * @param newValue The new value.
     */
    void OccupiedReceptionPaths(int oldValue, int newValue);
    /**
     * Callback for tracing LostPacketBecauseNoMoreReceivers.
     *
     * @param packet The packet lost.
     * @param node The receiver node id if any, 0 otherwise.
     */
    void NoMoreDemodulators(Ptr<const Packet> packet, uint32_t node);
    /**
     * Callback for tracing LostPacketBecauseInterference.
     *
     * @param packet The packet lost.
     * @param node The receiver node id if any, 0 otherwise.
     */
    void Interference(Ptr<const Packet> packet, uint32_t node);
    /**
     * Callback for tracing ReceivedPacket.
     *
     * @param packet The packet received.
     * @param node The receiver node id if any, 0 otherwise.
     */
    void ReceivedPacket(Ptr<const Packet> packet, uint32_t node);

    Ptr<SimpleGatewayLoraPhy> gatewayPhy; //!< PHY layer of a gateway to be tested

    int m_noMoreDemodulatorsCalls = 0;   //!< Counter for LostPacketBecauseNoMoreReceivers calls
    int m_interferenceCalls = 0;         //!< Counter for LostPacketBecauseInterference calls
    int m_receivedPacketCalls = 0;       //!< Counter for ReceivedPacket calls
    int m_maxOccupiedReceptionPaths = 0; //!< Max number of concurrent OccupiedReceptionPaths
};

// Add some help text to this case to describe what it is intended to test
ReceivePathTest::ReceivePathTest()
    : TestCase("Verify that ReceivePaths work as expected")
{
}

// Reminder that the test case should clean up after itself
ReceivePathTest::~ReceivePathTest()
{
}

void
ReceivePathTest::Reset()
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
    //                                         MakeCallback (&ReceivePathTest::ReceivedPacket,
    //                                         this));
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
ReceivePathTest::OccupiedReceptionPaths(int oldValue, int newValue)
{
    NS_LOG_FUNCTION(oldValue << newValue);

    if (m_maxOccupiedReceptionPaths < newValue)
    {
        m_maxOccupiedReceptionPaths = newValue;
    }
}

void
ReceivePathTest::NoMoreDemodulators(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << node);

    m_noMoreDemodulatorsCalls++;
}

void
ReceivePathTest::Interference(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << node);

    m_interferenceCalls++;
}

void
ReceivePathTest::ReceivedPacket(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << node);

    m_receivedPacketCalls++;
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
ReceivePathTest::DoRun()
{
    NS_LOG_DEBUG("ReceivePathTest");

    Ptr<Packet> packet = Create<Packet>();

    Reset();

    // FIXME
    // //////////////////////////////////////////////////////////////////////////////////
    // // If no ReceptionPath is configured to listen on a frequency, no packet is received
    // //////////////////////////////////////////////////////////////////////////////////

    // Simulator::Schedule (Seconds (1), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (1), frequency4);

    // Simulator::Stop (Hours (2));
    // Simulator::Run ();
    // Simulator::Destroy ();

    // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 1, "Unexpected value");

    // Reset ();

    // //////////////////////////////////////////////////////////////////////////////
    // // A ReceptionPath can receive a packet of any spreading factor without any preconfiguration
    // //////////////////////////////////////////////////////////////////////////////

    // Simulator::Schedule (Seconds (1), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (1), frequency1);
    // Simulator::Schedule (Seconds (3), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 8,
    //                      Seconds (1), frequency1);
    // Simulator::Schedule (Seconds (5), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 9,
    //                      Seconds (1), frequency1);
    // Simulator::Schedule (Seconds (7), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 10,
    //                      Seconds (1), frequency1);
    // Simulator::Schedule (Seconds (9), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 11,
    //                      Seconds (1), frequency1);
    // Simulator::Schedule (Seconds (11), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14,
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
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (3), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 9,
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
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (3), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
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
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (3), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);

    // Simulator::Stop (Hours (2));
    // Simulator::Run ();
    // Simulator::Destroy ();

    // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 1, "Unexpected value");

    // Reset ();

    // ///////////////////////////////////////////////////////////////////////////
    // // Packets that are on different frequencys do not interfere
    // ///////////////////////////////////////////////////////////////////////////
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency2);

    // Simulator::Stop (Hours (2));
    // Simulator::Run ();
    // Simulator::Destroy ();

    // NS_TEST_EXPECT_MSG_EQ (m_interferenceCalls, 0, "Unexpected value");

    // Reset ();

    // ///////////////////////////////////////////////////////////////////////////
    // // Full capacity
    // ///////////////////////////////////////////////////////////////////////////
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 8,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 9,
    //                      Seconds (4), frequency2);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 10,
    //                      Seconds (4), frequency2);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 11,
    //                      Seconds (4), frequency3);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 12,
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
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 8,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 9,
    //                      Seconds (4), frequency2);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 10,
    //                      Seconds (4), frequency2);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 11,
    //                      Seconds (4), frequency3);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 12,
    //                      Seconds (4), frequency3);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 10,
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
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 8,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 9,
    //                      Seconds (4), frequency2);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 10,
    //                      Seconds (4), frequency2);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 11,
    //                      Seconds (4), frequency3);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 12,
    //                      Seconds (4), frequency3);

    // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 8,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 9,
    //                      Seconds (4), frequency2);
    // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 10,
    //                      Seconds (4), frequency2);
    // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 11,
    //                      Seconds (4), frequency3);
    // Simulator::Schedule (Seconds (8), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 12,
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
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 8,
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
    // Simulator::Schedule (Seconds (2), &SimpleGatewayLoraPhy::StartReceive, gatewayPhy, packet,
    // 14, 7,
    //                      Seconds (4), frequency1);

    // Simulator::Stop (Hours (2));
    // Simulator::Run ();
    // Simulator::Destroy ();

    // NS_TEST_EXPECT_MSG_EQ (m_noMoreDemodulatorsCalls, 0, "Unexpected value");
    // NS_TEST_EXPECT_MSG_EQ (m_interferenceCalls, 0, "Unexpected value");
    // NS_TEST_EXPECT_MSG_EQ (m_receivedPacketCalls, 1, "Unexpected value");
    // NS_TEST_EXPECT_MSG_EQ (m_maxOccupiedReceptionPaths, 1, "Unexpected value");
}

/**
 * @ingroup lorawan
 *
 * It tests functionality of the LogicalLoraChannel, SubBand and LogicalLoraChannelHelper classes
 */
class LogicalLoraChannelTest : public TestCase
{
  public:
    LogicalLoraChannelTest();           //!< Default constructor
    ~LogicalLoraChannelTest() override; //!< Destructor

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
LogicalLoraChannelTest::LogicalLoraChannelTest()
    : TestCase("Verify that LogicalLoraChannel and LogicalLoraChannelHelper work as expected")
{
}

// Reminder that the test case should clean up after itself
LogicalLoraChannelTest::~LogicalLoraChannelTest()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
LogicalLoraChannelTest::DoRun()
{
    NS_LOG_DEBUG("LogicalLoraChannelTest");

    /////////////////////////////
    // Test LogicalLoraChannel //
    /////////////////////////////

    // Setup
    Ptr<LogicalLoraChannel> channel1 = Create<LogicalLoraChannel>(868000000, 0, 5);
    Ptr<LogicalLoraChannel> channel2 = Create<LogicalLoraChannel>(868000000, 0, 5);
    Ptr<LogicalLoraChannel> channel3 = Create<LogicalLoraChannel>(868100000, 0, 5);
    Ptr<LogicalLoraChannel> channel4 = Create<LogicalLoraChannel>(868001000, 0, 5);

    // Equality between channels
    // Test the == and != operators
    NS_TEST_EXPECT_MSG_EQ(channel1, channel2, "== operator doesn't work as expected");
    NS_TEST_EXPECT_MSG_NE(channel1, channel3, "!= operator doesn't work as expected");
    NS_TEST_EXPECT_MSG_NE(channel1, channel4, "!= operator doesn't work as expected");

    //////////////////
    // Test SubBand //
    //////////////////

    // Setup
    auto subBand = Create<SubBand>(868000000, 868600000, 0.01, 14);
    Ptr<LogicalLoraChannel> channel5 = Create<LogicalLoraChannel>(870000000, 0, 5);

    // Test Contains
    NS_TEST_EXPECT_MSG_EQ(subBand->Contains(channel3),
                          true,
                          "Contains does not behave as expected");
    NS_TEST_EXPECT_MSG_EQ(subBand->Contains(channel3->GetFrequency()),
                          true,
                          "Contains does not behave as expected");
    NS_TEST_EXPECT_MSG_EQ(subBand->Contains(channel5),
                          false,
                          "Contains does not behave as expected");

    ///////////////////////////////////
    // Test LogicalLoraChannelHelper //
    ///////////////////////////////////

    // Setup
    auto channelHelper = Create<LogicalLoraChannelHelper>(16);
    auto subBand1 = Create<SubBand>(869400000, 869650000, 0.10, 27);
    channel1 = Create<LogicalLoraChannel>(868100000, 0, 5);
    channel2 = Create<LogicalLoraChannel>(868300000, 0, 5);
    channel3 = Create<LogicalLoraChannel>(869525000, 0, 5);

    // Channel diagram
    //
    // Channels      1     2                              3
    // SubBands  868 ----- 1% ----- 868.6      869 ----- 10% ----- 869.4

    // Add SubBands and LogicalLoraChannels to the helper
    channelHelper->AddSubBand(subBand);
    channelHelper->AddSubBand(subBand1);
    channelHelper->SetChannel(0, channel1);
    channelHelper->SetChannel(1, channel2);
    channelHelper->SetChannel(2, channel3);

    // Duty Cycle tests
    // (high level duty cycle behavior)
    ///////////////////////////////////

    channelHelper->AddEvent(Seconds(2), channel1);
    Time expectedTimeOff = Seconds(2 / 0.01);

    // Wait time is computed correctly
    NS_TEST_EXPECT_MSG_EQ(channelHelper->GetWaitTime(channel1),
                          expectedTimeOff,
                          "Wait time doesn't behave as expected");

    // Duty Cycle involves the whole SubBand, not just a channel
    NS_TEST_EXPECT_MSG_EQ(channelHelper->GetWaitTime(channel2),
                          expectedTimeOff,
                          "Wait time doesn't behave as expected");

    // Other bands are not affected by this transmission
    NS_TEST_EXPECT_MSG_EQ(channelHelper->GetWaitTime(channel3),
                          Time(0),
                          "Wait time affects other subbands");
}

/**
 * @ingroup lorawan
 *
 * It tests the correctness of the LoraPhy::GetOnAirTime calculator against a number of pre-sourced
 * time values of known scenarios
 */
class TimeOnAirTest : public TestCase
{
  public:
    TimeOnAirTest();           //!< Default constructor
    ~TimeOnAirTest() override; //!< Destructor

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
TimeOnAirTest::TimeOnAirTest()
    : TestCase(
          "Verify that LoraPhy's function to compute the time on air of a packet works as expected")
{
}

// Reminder that the test case should clean up after itself
TimeOnAirTest::~TimeOnAirTest()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
TimeOnAirTest::DoRun()
{
    NS_LOG_DEBUG("TimeOnAirTest");

    Ptr<Packet> packet;
    Time duration;

    // Available parameters:
    // PayloadSize, SF, HeaderDisabled, CodingRate, Bandwidth, nPreambleSyms, crcEnabled,
    // lowDROptimization

    // Starting parameters
    packet = Create<Packet>(10);
    LoraTxParameters txParams;
    txParams.sf = 7;
    txParams.headerDisabled = false;
    txParams.codingRate = 1;
    txParams.bandwidthHz = 125000;
    txParams.nPreamble = 8;
    txParams.crcEnabled = true;
    txParams.lowDataRateOptimizationEnabled = false;

    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.041216, 0.0001, "Unexpected duration");

    txParams.sf = 8;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.072192, 0.0001, "Unexpected duration");

    txParams.headerDisabled = true;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.072192, 0.0001, "Unexpected duration");

    txParams.codingRate = 2;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.078336, 0.0001, "Unexpected duration");

    txParams.nPreamble = 10;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.082432, 0.0001, "Unexpected duration");

    txParams.lowDataRateOptimizationEnabled = true;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.082432, 0.0001, "Unexpected duration");

    txParams.sf = 10;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.280576, 0.0001, "Unexpected duration");

    txParams.bandwidthHz = 250000;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.14028, 0.0001, "Unexpected duration");

    txParams.bandwidthHz = 500000;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.070144, 0.0001, "Unexpected duration");

    txParams.headerDisabled = false;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.082432, 0.0001, "Unexpected duration");

    txParams.nPreamble = 8;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.078336, 0.0001, "Unexpected duration");

    txParams.sf = 12;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.264192, 0.0001, "Unexpected duration");

    packet = Create<Packet>(50);
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.657408, 0.0001, "Unexpected duration");

    txParams.bandwidthHz = 125000;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 2.629632, 0.0001, "Unexpected duration");

    txParams.codingRate = 1;
    duration = LoraPhy::GetOnAirTime(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 2.301952, 0.0001, "Unexpected duration");
}

/**
 * @ingroup lorawan
 *
 * It tests sending packets over a LoRa physical channel between multiple devices and the resulting
 * possible outcomes
 */
class PhyConnectivityTest : public TestCase
{
  public:
    PhyConnectivityTest();           //!< Default constructor
    ~PhyConnectivityTest() override; //!< Destructor

    /**
     * Reset counters and end devices' PHYs for new sub test case.
     */
    void Reset();

    /**
     * Callback for tracing ReceivedPacket.
     *
     * @param packet The packet received.
     * @param node The receiver node id if any, 0 otherwise.
     */
    void ReceivedPacket(Ptr<const Packet> packet, uint32_t node);

    /**
     * Callback for tracing LostPacketBecauseUnderSensitivity.
     *
     * @param packet The packet lost.
     * @param node The receiver node id if any, 0 otherwise.
     */
    void UnderSensitivity(Ptr<const Packet> packet, uint32_t node);

    /**
     * Callback for tracing LostPacketBecauseInterference.
     *
     * @param packet The packet lost.
     * @param node The receiver node id if any, 0 otherwise.
     */
    void Interference(Ptr<const Packet> packet, uint32_t node);

    /**
     * Callback for tracing LostPacketBecauseWrongFrequency.
     *
     * @param packet The packet lost.
     * @param node The receiver node id if any, 0 otherwise.
     */
    void WrongFrequency(Ptr<const Packet> packet, uint32_t node);

    /**
     * Callback for tracing LostPacketBecauseWrongSpreadingFactor.
     *
     * @param packet The packet lost.
     * @param node The receiver node id if any, 0 otherwise.
     */
    void WrongSf(Ptr<const Packet> packet, uint32_t node);

    /**
     * Compare two packets to check if they are equal.
     *
     * @param packet1 A first packet.
     * @param packet2 A second packet.
     * @return True if their unique identifiers are equal,
     * @return false otherwise.
     */
    bool IsSamePacket(Ptr<Packet> packet1, Ptr<Packet> packet2);

  private:
    void DoRun() override;

    Ptr<LoraChannel> channel;           //!< The LoRa channel used for tests
    Ptr<SimpleEndDeviceLoraPhy> edPhy1; //!< The first end device's PHY layer used in tests
    Ptr<SimpleEndDeviceLoraPhy> edPhy2; //!< The second end device's PHY layer used in tests
    Ptr<SimpleEndDeviceLoraPhy> edPhy3; //!< The third end device's PHY layer used in tests

    Ptr<Packet> m_latestReceivedPacket; //!< Pointer to track the last received packet
    int m_receivedPacketCalls = 0;      //!< Counter for ReceivedPacket calls
    int m_underSensitivityCalls = 0;    //!< Counter for LostPacketBecauseUnderSensitivity calls
    int m_interferenceCalls = 0;        //!< Counter for LostPacketBecauseInterference calls
    int m_wrongSfCalls = 0;             //!< Counter for LostPacketBecauseWrongSpreadingFactor calls
    int m_wrongFrequencyCalls = 0;      //!< Counter for LostPacketBecauseWrongFrequency calls
};

// Add some help text to this case to describe what it is intended to test
PhyConnectivityTest::PhyConnectivityTest()
    : TestCase("Verify that PhyConnectivity works as expected")
{
}

// Reminder that the test case should clean up after itself
PhyConnectivityTest::~PhyConnectivityTest()
{
}

void
PhyConnectivityTest::ReceivedPacket(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << node);

    m_receivedPacketCalls++;

    m_latestReceivedPacket = packet->Copy();
}

void
PhyConnectivityTest::UnderSensitivity(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << node);

    m_underSensitivityCalls++;
}

void
PhyConnectivityTest::Interference(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << node);

    m_interferenceCalls++;
}

void
PhyConnectivityTest::WrongSf(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << node);

    m_wrongSfCalls++;
}

void
PhyConnectivityTest::WrongFrequency(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << node);

    m_wrongFrequencyCalls++;
}

bool
PhyConnectivityTest::IsSamePacket(Ptr<Packet> packet1, Ptr<Packet> packet2)
{
    return packet1->GetUid() == packet2->GetUid();
}

void
PhyConnectivityTest::Reset()
{
    m_receivedPacketCalls = 0;
    m_underSensitivityCalls = 0;
    m_interferenceCalls = 0;
    m_wrongSfCalls = 0;
    m_wrongFrequencyCalls = 0;

    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

    // Create the channel
    channel = CreateObject<LoraChannel>(loss, delay);

    // Connect PHYs
    edPhy1 = CreateObject<SimpleEndDeviceLoraPhy>();
    edPhy2 = CreateObject<SimpleEndDeviceLoraPhy>();
    edPhy3 = CreateObject<SimpleEndDeviceLoraPhy>();

    edPhy1->SetFrequency(868100000);
    edPhy2->SetFrequency(868100000);
    edPhy3->SetFrequency(868100000);

    Ptr<ConstantPositionMobilityModel> mob1 = CreateObject<ConstantPositionMobilityModel>();
    Ptr<ConstantPositionMobilityModel> mob2 = CreateObject<ConstantPositionMobilityModel>();
    Ptr<ConstantPositionMobilityModel> mob3 = CreateObject<ConstantPositionMobilityModel>();

    mob1->SetPosition(Vector(0.0, 0.0, 0.0));
    mob2->SetPosition(Vector(10.0, 0.0, 0.0));
    mob3->SetPosition(Vector(20.0, 0.0, 0.0));

    edPhy1->SetMobility(mob1);
    edPhy2->SetMobility(mob2);
    edPhy3->SetMobility(mob3);

    edPhy1->SwitchToStandby();
    edPhy2->SwitchToStandby();
    edPhy3->SwitchToStandby();

    channel->Add(edPhy1);
    channel->Add(edPhy2);
    channel->Add(edPhy3);

    edPhy1->SetChannel(channel);
    edPhy2->SetChannel(channel);
    edPhy3->SetChannel(channel);

    // Listen for a specific SpreadingFactor
    edPhy1->SetSpreadingFactor(12);
    edPhy2->SetSpreadingFactor(12);
    edPhy3->SetSpreadingFactor(12);

    // Listen on a specific frequency
    edPhy1->SetFrequency(868100000);
    edPhy2->SetFrequency(868100000);
    edPhy3->SetFrequency(868100000);

    edPhy1->TraceConnectWithoutContext("ReceivedPacket",
                                       MakeCallback(&PhyConnectivityTest::ReceivedPacket, this));
    edPhy2->TraceConnectWithoutContext("ReceivedPacket",
                                       MakeCallback(&PhyConnectivityTest::ReceivedPacket, this));
    edPhy3->TraceConnectWithoutContext("ReceivedPacket",
                                       MakeCallback(&PhyConnectivityTest::ReceivedPacket, this));

    edPhy1->TraceConnectWithoutContext("LostPacketBecauseUnderSensitivity",
                                       MakeCallback(&PhyConnectivityTest::UnderSensitivity, this));
    edPhy2->TraceConnectWithoutContext("LostPacketBecauseUnderSensitivity",
                                       MakeCallback(&PhyConnectivityTest::UnderSensitivity, this));
    edPhy3->TraceConnectWithoutContext("LostPacketBecauseUnderSensitivity",
                                       MakeCallback(&PhyConnectivityTest::UnderSensitivity, this));

    edPhy1->TraceConnectWithoutContext("LostPacketBecauseInterference",
                                       MakeCallback(&PhyConnectivityTest::Interference, this));
    edPhy2->TraceConnectWithoutContext("LostPacketBecauseInterference",
                                       MakeCallback(&PhyConnectivityTest::Interference, this));
    edPhy3->TraceConnectWithoutContext("LostPacketBecauseInterference",
                                       MakeCallback(&PhyConnectivityTest::Interference, this));

    edPhy1->TraceConnectWithoutContext("LostPacketBecauseWrongFrequency",
                                       MakeCallback(&PhyConnectivityTest::WrongFrequency, this));
    edPhy2->TraceConnectWithoutContext("LostPacketBecauseWrongFrequency",
                                       MakeCallback(&PhyConnectivityTest::WrongFrequency, this));
    edPhy3->TraceConnectWithoutContext("LostPacketBecauseWrongFrequency",
                                       MakeCallback(&PhyConnectivityTest::WrongFrequency, this));

    edPhy1->TraceConnectWithoutContext("LostPacketBecauseWrongSpreadingFactor",
                                       MakeCallback(&PhyConnectivityTest::WrongSf, this));
    edPhy2->TraceConnectWithoutContext("LostPacketBecauseWrongSpreadingFactor",
                                       MakeCallback(&PhyConnectivityTest::WrongSf, this));
    edPhy3->TraceConnectWithoutContext("LostPacketBecauseWrongSpreadingFactor",
                                       MakeCallback(&PhyConnectivityTest::WrongSf, this));
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
PhyConnectivityTest::DoRun()
{
    NS_LOG_DEBUG("PhyConnectivityTest");

    // Setup
    ////////

    Reset();

    LoraTxParameters txParams;
    txParams.sf = 12;

    uint8_t buffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    Ptr<Packet> packet = Create<Packet>(buffer, 10);

    // Testing
    //////////

    // Basic packet delivery test
    /////////////////////////////

    Simulator::Schedule(Seconds(2),
                        &SimpleEndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(
        m_receivedPacketCalls,
        2,
        "Channel skipped some PHYs when delivering a packet"); // All PHYs except the sender

    Reset();

    // Sleeping PHYs do not receive the packet

    edPhy2->SwitchToSleep();

    Simulator::Schedule(Seconds(2),
                        &SimpleEndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(
        m_receivedPacketCalls,
        1,
        "Packet was received by a PHY in SLEEP mode"); // All PHYs in Standby except the sender

    Reset();

    // Packet that arrives under sensitivity is received correctly if the spreading factor increases

    txParams.sf = 7;
    edPhy2->SetSpreadingFactor(7);
    DynamicCast<ConstantPositionMobilityModel>(edPhy2->GetMobility())
        ->SetPosition(Vector(2990, 0, 0));

    Simulator::Schedule(Seconds(2),
                        &SimpleEndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(
        m_underSensitivityCalls,
        1,
        "Packet that should have been lost because of low receive power was received");

    Reset();

    // Try again using a packet with higher spreading factor
    txParams.sf = 8;
    edPhy2->SetSpreadingFactor(8);
    DynamicCast<ConstantPositionMobilityModel>(edPhy2->GetMobility())
        ->SetPosition(Vector(2990, 0, 0));

    Simulator::Schedule(Seconds(2),
                        &SimpleEndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_underSensitivityCalls,
                          0,
                          "Packets that should have arrived above sensitivity were under it");

    Reset();

    // Packets can be destroyed by interference

    txParams.sf = 12;
    Simulator::Schedule(Seconds(2),
                        &SimpleEndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);
    Simulator::Schedule(Seconds(2),
                        &SimpleEndDeviceLoraPhy::Send,
                        edPhy3,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_interferenceCalls,
                          1,
                          "Packets that should be destroyed by interference weren't");

    Reset();

    // Packets can be lost because the PHY is not listening on the right frequency

    Simulator::Schedule(Seconds(2),
                        &SimpleEndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868300000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_wrongFrequencyCalls,
                          2,
                          "Packets were received even though PHY was on a different frequency");

    Reset();

    // Packets can be lost because the PHY is not listening for the right spreading factor

    txParams.sf = 8; // Send with 8, listening for 12
    Simulator::Schedule(Seconds(2),
                        &SimpleEndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(
        m_wrongSfCalls,
        2,
        "Packets were received even though PHY was listening for a different spreading factor.");

    Reset();

    // Sending of packets
    /////////////////////

    // The very same packet arrives at the other PHY
    Simulator::Schedule(Seconds(2),
                        &SimpleEndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(IsSamePacket(packet, m_latestReceivedPacket),
                          true,
                          "Packet changed contents when going through the channel");

    Reset();

    // Correct state transitions
    ////////////////////////////

    // PHY switches to STANDBY after TX and RX

    Simulator::Schedule(Seconds(2),
                        &SimpleEndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(edPhy1->GetState(),
                          SimpleEndDeviceLoraPhy::STANDBY,
                          "State didn't switch to STANDBY as expected");
    NS_TEST_EXPECT_MSG_EQ(edPhy2->GetState(),
                          SimpleEndDeviceLoraPhy::STANDBY,
                          "State didn't switch to STANDBY as expected");
}

/**
 * @ingroup lorawan
 *
 * It tests the functionalities of the MAC layer of LoRaWAN devices
 *
 * @todo Not implemented yet.
 */
class LorawanMacTest : public TestCase
{
  public:
    LorawanMacTest();           //!< Default constructor
    ~LorawanMacTest() override; //!< Destructor

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
LorawanMacTest::LorawanMacTest()
    : TestCase("Verify that the MAC layer of end devices behaves as expected")
{
}

// Reminder that the test case should clean up after itself
LorawanMacTest::~LorawanMacTest()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
LorawanMacTest::DoRun()
{
    NS_LOG_DEBUG("LorawanMacTest");
}

/**
 * @ingroup lorawan
 *
 * It tests the functionalities of LoRaWAN MAC commands received by devices.
 *
 * This means testing that (i) settings in the downlink MAC commands are correctly applied/rejected
 * by the device, and that (ii) the correct answer (if expected) is produced by the device.
 */
class MacCommandTest : public TestCase
{
  public:
    MacCommandTest();           //!< Default constructor
    ~MacCommandTest() override; //!< Destructor

  private:
    /**
     * Have this class' MAC layer receive a downlink packet carrying the input MAC command. After,
     * trigger a new empty uplink packet send that can then be used to examine the MAC command
     * answers in the header.
     *
     * @tparam T  \explicit The type of MAC command to create.
     * @tparam Ts \deduced Types of the constructor arguments.
     * @param  [in] args MAC command constructor arguments.
     * @return The list of MAC commands produced by the device as an answer.
     */
    template <typename T, typename... Ts>
    std::vector<Ptr<MacCommand>> RunMacCommand(Ts&&... args);

    /**
     * This function resets the state of the MAC layer used for tests. Use it before each call of
     * RunMacCommand. Otherwise, on consecutive calls the MAC layer will not send due to duty-cycle
     * limitations.
     */
    void Reset();

    void DoRun() override;

    Ptr<ClassAEndDeviceLorawanMac> m_mac; //!< The end device's MAC layer used in tests.
};

MacCommandTest::MacCommandTest()
    : TestCase("Test functionality of MAC commands when received by a device")
{
}

MacCommandTest::~MacCommandTest()
{
    m_mac = nullptr;
}

template <typename T, typename... Ts>
std::vector<Ptr<MacCommand>>
MacCommandTest::RunMacCommand(Ts&&... args)
{
    Ptr<Packet> pkt;
    LoraFrameHeader fhdr;
    LorawanMacHeader mhdr;
    // Prepare DL packet with input command
    pkt = Create<Packet>(0);
    fhdr.SetAsDownlink();
    auto cmd = Create<T>(args...);
    fhdr.AddCommand(cmd);
    pkt->AddHeader(fhdr);
    mhdr.SetMType(LorawanMacHeader::UNCONFIRMED_DATA_DOWN);
    pkt->AddHeader(mhdr);
    // Trigger MAC layer reception
    DynamicCast<EndDeviceLoraPhy>(m_mac->GetPhy())
        ->SwitchToStandby(); // usually done as we open Rx windows
    m_mac->Receive(pkt);
    // Trigger MAC layer send
    pkt = Create<Packet>(0);
    m_mac->Send(pkt);
    // Retrieve uplink MAC commands
    pkt->RemoveHeader(mhdr);
    fhdr.SetAsUplink();
    pkt->RemoveHeader(fhdr);
    return fhdr.GetCommands();
}

void
MacCommandTest::Reset()
{
    // Reset MAC state
    LorawanMacHelper macHelper;
    macHelper.SetRegion(LorawanMacHelper::EU);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    /// @todo Create should not require a node in input.
    m_mac = DynamicCast<ClassAEndDeviceLorawanMac>(macHelper.Install(nullptr, nullptr));
    NS_TEST_EXPECT_MSG_NE(m_mac, nullptr, "Failed to initialize MAC layer object.");
    auto phy = CreateObject<SimpleEndDeviceLoraPhy>();
    phy->SetChannel(CreateObject<LoraChannel>());
    phy->SetMobility(CreateObject<ConstantPositionMobilityModel>());
    m_mac->SetPhy(phy);
}

void
MacCommandTest::DoRun()
{
    NS_LOG_DEBUG("MacCommandTest");

    Reset();
    // LinkCheckAns: get connectivity metrics of last uplink LinkCheckReq command
    {
        uint8_t margin = 20; // best reception margin [dB] from demodulation floor
        uint8_t gwCnt = 3;   // number of gateways that received last uplink
        auto answers = RunMacCommand<LinkCheckAns>(margin, gwCnt);
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetLastKnownLinkMarginDb()),
                              unsigned(margin),
                              "m_lastKnownMarginDb differs from Margin field of LinkCheckAns");
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetLastKnownGatewayCount()),
                              unsigned(gwCnt),
                              "m_lastKnownGatewayCount differs GwCnt field of LinkCheckAns");
        NS_TEST_EXPECT_MSG_EQ(answers.size(),
                              0,
                              "Unexpected uplink MAC command answer(s) to LinkCheckAns");
    }

    Reset();
    // LinkAdrReq: change data rate, TX power, redundancy, or channel mask
    {
        uint8_t dataRate = 5;
        uint8_t txPower = 2;
        uint16_t chMask = 0b101;
        uint8_t chMaskCntl = 0;
        uint8_t nbTrans = 13;
        auto answers = RunMacCommand<LinkAdrReq>(dataRate, txPower, chMask, chMaskCntl, nbTrans);
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetDataRate()),
                              unsigned(dataRate),
                              "m_dataRate does not match DataRate field of LinkAdrReq");
        NS_TEST_EXPECT_MSG_EQ(m_mac->GetTransmissionPowerDbm(),
                              14 - txPower * 2,
                              "m_txPowerDbm does not match txPower field of LinkAdrReq");
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetMaxNumberOfTransmissions()),
                              unsigned(nbTrans),
                              "m_nbTrans does not match nbTrans field of LinkAdrReq");
        auto channels = m_mac->GetLogicalLoraChannelHelper()->GetRawChannelArray();
        for (size_t i = 0; i < channels.size(); i++)
        {
            const auto& c = channels.at(i + 16 * chMaskCntl);
            bool actual = (c) ? c->IsEnabledForUplink() : false;
            bool expected = (chMask & 0b1 << i);
            NS_TEST_EXPECT_MSG_EQ(actual, expected, "Channel " << i << " state != chMask");
        }
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        auto laa = DynamicCast<LinkAdrAns>(answers.at(0));
        NS_TEST_ASSERT_MSG_NE(laa, nullptr, "LinkAdrAns was expected, cmd type cast failed");
        NS_TEST_EXPECT_MSG_EQ(laa->GetChannelMaskAck(), true, "ChannelMaskAck expected to be true");
        NS_TEST_EXPECT_MSG_EQ(laa->GetDataRateAck(), true, "DataRateAck expected to be true");
        NS_TEST_EXPECT_MSG_EQ(laa->GetPowerAck(), true, "PowerAck expected to be true");
    }

    Reset();
    // LinkAdrReq: ADR bit off, only change channel mask
    {
        uint8_t dataRate = 5;
        uint8_t txPower = 2;
        uint16_t chMask = 0b010;
        uint8_t chMaskCntl = 0;
        uint8_t nbTrans = 13;
        m_mac->SetUplinkAdrBit(false);
        auto answers = RunMacCommand<LinkAdrReq>(dataRate, txPower, chMask, chMaskCntl, nbTrans);
        NS_TEST_EXPECT_MSG_NE(unsigned(m_mac->GetDataRate()),
                              unsigned(dataRate),
                              "m_dataRate expected to differ from DataRate field of LinkAdrReq");
        NS_TEST_EXPECT_MSG_NE(m_mac->GetTransmissionPowerDbm(),
                              14 - txPower * 2,
                              "m_txPowerDbm expected to not match txPower field of LinkAdrReq");
        NS_TEST_EXPECT_MSG_NE(unsigned(m_mac->GetMaxNumberOfTransmissions()),
                              unsigned(nbTrans),
                              "m_nbTrans expected to differ from nbTrans field of LinkAdrReq");
        auto channels = m_mac->GetLogicalLoraChannelHelper()->GetRawChannelArray();
        for (size_t i = 0; i < channels.size(); i++)
        {
            const auto& c = channels.at(i + 16 * chMaskCntl);
            bool actual = (c) ? c->IsEnabledForUplink() : false;
            bool expected = (chMask & 0b1 << i);
            NS_TEST_EXPECT_MSG_EQ(actual, expected, "Channel " << i << " state != chMask");
        }
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        auto laa = DynamicCast<LinkAdrAns>(answers.at(0));
        NS_TEST_ASSERT_MSG_NE(laa, nullptr, "LinkAdrAns was expected, cmd type cast failed");
        NS_TEST_EXPECT_MSG_EQ(laa->GetChannelMaskAck(), true, "ChannelMaskAck expected to be true");
        NS_TEST_EXPECT_MSG_EQ(laa->GetDataRateAck(), false, "DataRateAck expected to be false");
        NS_TEST_EXPECT_MSG_EQ(laa->GetPowerAck(), false, "PowerAck expected to be false");
    }

    Reset();
    // LinkAdrReq: invalid chMask, data rate and power
    { // WARNING: default values are manually set here
        uint8_t dataRate = 12;
        uint8_t txPower = 8;
        uint16_t chMask = 0b0;
        uint8_t chMaskCntl = 0;
        uint8_t nbTrans = 6;
        auto answers = RunMacCommand<LinkAdrReq>(dataRate, txPower, chMask, chMaskCntl, nbTrans);
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetDataRate()),
                              0,
                              "m_dataRate expected to be default value");
        NS_TEST_EXPECT_MSG_EQ(m_mac->GetTransmissionPowerDbm(),
                              14,
                              "m_txPowerDbm expected to be default value");
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetMaxNumberOfTransmissions()),
                              1,
                              "m_nbTrans expected to be default value");
        auto channels = m_mac->GetLogicalLoraChannelHelper()->GetRawChannelArray();
        for (size_t i = 0; i < channels.size(); i++)
        {
            const auto& c = channels.at(i + 16 * chMaskCntl);
            bool actual = (c) ? c->IsEnabledForUplink() : false;
            bool expected = (uint16_t(0b111) & 0b1 << i);
            NS_TEST_EXPECT_MSG_EQ(actual, expected, "Channel " << i << " state != default");
        }
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        auto laa = DynamicCast<LinkAdrAns>(answers.at(0));
        NS_TEST_ASSERT_MSG_NE(laa, nullptr, "LinkAdrAns was expected, cmd type cast failed");
        NS_TEST_EXPECT_MSG_EQ(laa->GetChannelMaskAck(), false, "ChannelMaskAck != false");
        NS_TEST_EXPECT_MSG_EQ(laa->GetDataRateAck(), false, "DataRateAck expected to be false");
        NS_TEST_EXPECT_MSG_EQ(laa->GetPowerAck(), false, "PowerAck expected to be false");
    }

    Reset();
    // LinkAdrReq: invalid chMask, valid data rate and power
    { // WARNING: default values are manually set here
        uint8_t dataRate = 1;
        uint8_t txPower = 7;
        uint16_t chMask = 0b1000; // enable only non-exisitng channel
        uint8_t chMaskCntl = 0;
        uint8_t nbTrans = 3;
        auto answers = RunMacCommand<LinkAdrReq>(dataRate, txPower, chMask, chMaskCntl, nbTrans);
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetDataRate()),
                              0,
                              "m_dataRate expected to be default value");
        NS_TEST_EXPECT_MSG_EQ(m_mac->GetTransmissionPowerDbm(),
                              14,
                              "m_txPowerDbm expected to be default value");
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetMaxNumberOfTransmissions()),
                              1,
                              "m_nbTrans expected to be default value");
        auto channels = m_mac->GetLogicalLoraChannelHelper()->GetRawChannelArray();
        for (size_t i = 0; i < channels.size(); i++)
        {
            const auto& c = channels.at(i + 16 * chMaskCntl);
            bool actual = (c) ? c->IsEnabledForUplink() : false;
            bool expected = (uint16_t(0b111) & 0b1 << i);
            NS_TEST_EXPECT_MSG_EQ(actual, expected, "Channel " << i << " state != default");
        }
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        auto laa = DynamicCast<LinkAdrAns>(answers.at(0));
        NS_TEST_ASSERT_MSG_NE(laa, nullptr, "LinkAdrAns was expected, cmd type cast failed");
        NS_TEST_EXPECT_MSG_EQ(laa->GetChannelMaskAck(), false, "ChannelMaskAck != false");
        NS_TEST_EXPECT_MSG_EQ(laa->GetDataRateAck(), true, "DataRateAck expected to be true");
        NS_TEST_EXPECT_MSG_EQ(laa->GetPowerAck(), true, "PowerAck expected to be true");
    }

    Reset();
    // LinkAdrReq: fringe parameter values
    { // WARNING: default values are manually set here
        uint8_t dataRate = 0xF;
        uint8_t txPower = 0xF;  // 0x0F ignores config
        uint16_t chMask = 0b0;  // should be ignored because chMaskCntl is 6
        uint8_t chMaskCntl = 6; // all channels on
        uint8_t nbTrans = 0;    // restore default 1
        // Set device params to values different from default
        m_mac->SetDataRate(3);
        m_mac->SetTransmissionPowerDbm(12);
        m_mac->SetMaxNumberOfTransmissions(15);
        auto channels = m_mac->GetLogicalLoraChannelHelper()->GetRawChannelArray();
        channels.at(0)->DisableForUplink();
        auto answers = RunMacCommand<LinkAdrReq>(dataRate, txPower, chMask, chMaskCntl, nbTrans);
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetDataRate()),
                              3,
                              "m_dataRate expected to be default value");
        NS_TEST_EXPECT_MSG_EQ(m_mac->GetTransmissionPowerDbm(),
                              12,
                              "m_txPowerDbm expected to be default value");
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetMaxNumberOfTransmissions()),
                              1,
                              "m_nbTrans expected to be default value");
        for (size_t i = 0; i < channels.size(); i++)
        {
            const auto& c = channels.at(i);
            bool actual = (c) ? c->IsEnabledForUplink() : false;
            bool expected = (uint16_t(0b111) & 0b1 << i);
            NS_TEST_EXPECT_MSG_EQ(actual, expected, "Channel " << i << " state != default");
        }
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        auto laa = DynamicCast<LinkAdrAns>(answers.at(0));
        NS_TEST_ASSERT_MSG_NE(laa, nullptr, "LinkAdrAns was expected, cmd type cast failed");
        NS_TEST_EXPECT_MSG_EQ(laa->GetChannelMaskAck(), true, "ChannelMaskAck != true");
        NS_TEST_EXPECT_MSG_EQ(laa->GetDataRateAck(), true, "DataRateAck expected to be true");
        NS_TEST_EXPECT_MSG_EQ(laa->GetPowerAck(), true, "PowerAck expected to be true");
    }

    Reset();
    // DutyCycleReq: duty cycle to 100%
    {
        uint8_t maxDutyCycle = 0;
        auto answers = RunMacCommand<DutyCycleReq>(maxDutyCycle);
        NS_TEST_EXPECT_MSG_EQ(m_mac->GetAggregatedDutyCycle(),
                              1 / std::pow(2, maxDutyCycle),
                              "m_aggregatedDutyCycle != 1");
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        auto dca = DynamicCast<DutyCycleAns>(answers.at(0));
        NS_TEST_EXPECT_MSG_NE(dca, nullptr, "DutyCycleAns was expected, cmd type cast failed");
    }

    Reset();
    // DutyCycleReq: duty cycle to 12.5%
    {
        uint8_t maxDutyCycle = 3;
        auto answers = RunMacCommand<DutyCycleReq>(maxDutyCycle);
        NS_TEST_EXPECT_MSG_EQ(m_mac->GetAggregatedDutyCycle(),
                              1 / std::pow(2, maxDutyCycle),
                              "m_aggregatedDutyCycle != 1");
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        auto dca = DynamicCast<DutyCycleAns>(answers.at(0));
        NS_TEST_EXPECT_MSG_NE(dca, nullptr, "DutyCycleAns was expected, cmd type cast failed");
    }

    Reset();
    // RxParamSetupReq: set rx1Dr, rx2Dr, frequency
    {
        uint8_t rx1DrOffset = 5;
        uint8_t rx2DataRate = 5;
        double frequencyHz = 863500000;
        m_mac->SetDataRate(5);
        auto answers = RunMacCommand<RxParamSetupReq>(rx1DrOffset, rx2DataRate, frequencyHz);
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetFirstReceiveWindowDataRate()),
                              unsigned(5 - rx1DrOffset),
                              "Rx1DataRate does not match rx1DrOffset from RxParamSetupReq");
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetSecondReceiveWindowDataRate()),
                              unsigned(rx2DataRate),
                              "Rx2DataRate does not match rx2DataRate from RxParamSetupReq");
        NS_TEST_EXPECT_MSG_EQ(m_mac->GetSecondReceiveWindowFrequency(),
                              frequencyHz,
                              "Rx2 frequency does not match frequency from RxParamSetupReq");
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        auto rpsa = DynamicCast<RxParamSetupAns>(answers.at(0));
        NS_TEST_ASSERT_MSG_NE(rpsa, nullptr, "RxParamSetupAns was expected, cmd type cast failed");
        NS_TEST_EXPECT_MSG_EQ(rpsa->GetRx1DrOffsetAck(), true, "Rx1DrOffsetAck != true");
        NS_TEST_EXPECT_MSG_EQ(rpsa->GetRx2DataRateAck(), true, "Rx2DataRateAck != true");
        NS_TEST_EXPECT_MSG_EQ(rpsa->GetChannelAck(), true, "ChannelAck expected to be true");
    }

    Reset();
    // RxParamSetupReq: invalid rx1Dr, rx2Dr, frequency
    { // WARNING: default values are manually set here
        uint8_t rx1DrOffset = 6;
        uint8_t rx2DataRate = 12;
        double frequencyHz = 871000000;
        m_mac->SetDataRate(5);
        auto answers = RunMacCommand<RxParamSetupReq>(rx1DrOffset, rx2DataRate, frequencyHz);
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetFirstReceiveWindowDataRate()),
                              5,
                              "Rx1DataRate expected to be default value");
        NS_TEST_EXPECT_MSG_EQ(unsigned(m_mac->GetSecondReceiveWindowDataRate()),
                              0,
                              "Rx2DataRate expected to be default value");
        NS_TEST_EXPECT_MSG_EQ(m_mac->GetSecondReceiveWindowFrequency(),
                              869525000,
                              "Rx2 frequency expected to be default value");
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        auto rpsa = DynamicCast<RxParamSetupAns>(answers.at(0));
        NS_TEST_ASSERT_MSG_NE(rpsa, nullptr, "RxParamSetupAns was expected, cmd type cast failed");
        NS_TEST_EXPECT_MSG_EQ(rpsa->GetRx1DrOffsetAck(), false, "Rx1DrOffsetAck != false");
        NS_TEST_EXPECT_MSG_EQ(rpsa->GetRx2DataRateAck(), false, "Rx2DataRateAck != false");
        NS_TEST_EXPECT_MSG_EQ(rpsa->GetChannelAck(), false, "ChannelAck expected to be false");
    }

    Reset();
    // DevStatusReq: get default values
    { // WARNING: default values are manually set here
        auto answers = RunMacCommand<DevStatusReq>();
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        auto dsa = DynamicCast<DevStatusAns>(answers.at(0));
        NS_TEST_ASSERT_MSG_NE(dsa, nullptr, "DevStatusAns was expected, cmd type cast failed");
        NS_TEST_EXPECT_MSG_EQ(unsigned(dsa->GetBattery()), 0, "Battery expected == 0 (ext power)");
        NS_TEST_EXPECT_MSG_EQ(unsigned(dsa->GetMargin()), 31, "Margin expected to be 31 (default)");
    }

    Reset();
    // NewChannelReq: add a new channel
    {
        uint8_t chIndex = 4;
        double frequencyHz = 865100000;
        uint8_t minDataRate = 1;
        uint8_t maxDataRate = 4;
        auto answers = RunMacCommand<NewChannelReq>(chIndex, frequencyHz, minDataRate, maxDataRate);
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        auto c = m_mac->GetLogicalLoraChannelHelper()->GetRawChannelArray().at(chIndex);
        NS_TEST_ASSERT_MSG_NE(c, nullptr, "Channel at chIndex slot expected not to be nullptr");
        NS_TEST_EXPECT_MSG_EQ(c->GetFrequency(),
                              frequencyHz,
                              "Channel frequency expected to equal NewChannelReq frequency");
        NS_TEST_EXPECT_MSG_EQ(c->GetMinimumDataRate(),
                              unsigned(minDataRate),
                              "Channel minDataRate expected to equal NewChannelReq minDataRate");
        NS_TEST_EXPECT_MSG_EQ(c->GetMaximumDataRate(),
                              unsigned(maxDataRate),
                              "Channel maxDataRate expected to equal NewChannelReq maxDataRate");
        auto nca = DynamicCast<NewChannelAns>(answers.at(0));
        NS_TEST_ASSERT_MSG_NE(nca, nullptr, "NewChannelAns was expected, cmd type cast failed");
        NS_TEST_EXPECT_MSG_EQ(nca->GetDataRateRangeOk(), true, "DataRateRangeOk != true");
        NS_TEST_EXPECT_MSG_EQ(nca->GetChannelFrequencyOk(), true, "ChannelFrequencyOk != true");
    }

    Reset();
    // NewChannelReq: invalid new channel
    { // WARNING: default values are manually set here
        uint8_t chIndex = 1;
        double frequencyHz = 862000000;
        uint8_t minDataRate = 14;
        uint8_t maxDataRate = 13;
        auto answers = RunMacCommand<NewChannelReq>(chIndex, frequencyHz, minDataRate, maxDataRate);
        NS_TEST_ASSERT_MSG_EQ(answers.size(), 1, "1 answer cmd was expected, found 0 or >1");
        double defaultFrequenciesHz[3] = {868100000, 868300000, 868500000};
        auto channels = m_mac->GetLogicalLoraChannelHelper()->GetRawChannelArray();
        for (size_t i = 0; i < channels.size(); i++)
        {
            const auto& c = channels.at(i);
            if (i > 2)
            {
                NS_TEST_ASSERT_MSG_EQ(c, nullptr, "Channel " << i << "expected to be nullptr");
                continue;
            }
            NS_TEST_EXPECT_MSG_EQ(c->GetFrequency(),
                                  defaultFrequenciesHz[i],
                                  "Channel frequency expected to equal NewChannelReq frequency");
            NS_TEST_EXPECT_MSG_EQ(unsigned(c->GetMinimumDataRate()),
                                  0,
                                  "Channel " << i << " minDataRate expected to be default");
            NS_TEST_EXPECT_MSG_EQ(unsigned(c->GetMaximumDataRate()),
                                  5,
                                  "Channel " << i << " maxDataRate expected to be default");
            NS_TEST_EXPECT_MSG_EQ(c->IsEnabledForUplink(),
                                  true,
                                  "Channel " << i << " state expected to be active by default");
        }
        auto nca = DynamicCast<NewChannelAns>(answers.at(0));
        NS_TEST_ASSERT_MSG_NE(nca, nullptr, "NewChannelAns was expected, cmd type cast failed");
        NS_TEST_EXPECT_MSG_EQ(nca->GetDataRateRangeOk(), false, "DataRateRangeOk != false");
        NS_TEST_EXPECT_MSG_EQ(nca->GetChannelFrequencyOk(), false, "ChannelFrequencyOk != false");
    }
}

/**
 * @ingroup lorawan
 *
 * The TestSuite class names the TestSuite, identifies what type of TestSuite, and enables the
 * TestCases to be run. Typically, only the constructor for this class must be defined
 */
class LorawanTestSuite : public TestSuite
{
  public:
    LorawanTestSuite(); //!< Default constructor
};

LorawanTestSuite::LorawanTestSuite()
    : TestSuite("lorawan", Type::UNIT)
{
    // LogComponentEnable("LorawanTestSuite", LOG_LEVEL_DEBUG);
    // LogComponentEnable("LorawanMac", LOG_LEVEL_DEBUG);
    // LogComponentEnable("EndDeviceLorawanMac", LOG_LEVEL_DEBUG);
    // LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_DEBUG);
    // LogComponentEnable("SimpleEndDeviceLoraPhy", LOG_LEVEL_DEBUG);
    // LogComponentEnable("EndDeviceLoraPhy", LOG_LEVEL_DEBUG);
    // LogComponentEnable("LoraPhy", LOG_LEVEL_DEBUG);
    // LogComponentEnable("LoraChannel", LOG_LEVEL_DEBUG);
    // LogComponentEnable("LoraFrameHeader", LOG_LEVEL_DEBUG);
    // LogComponentEnableAll(LOG_PREFIX_FUNC);
    // LogComponentEnableAll(LOG_PREFIX_NODE);
    // LogComponentEnableAll(LOG_PREFIX_TIME);

    AddTestCase(new InterferenceTest, Duration::QUICK);
    AddTestCase(new AddressTest, Duration::QUICK);
    AddTestCase(new HeaderTest, Duration::QUICK);
    AddTestCase(new ReceivePathTest, Duration::QUICK);
    AddTestCase(new LogicalLoraChannelTest, Duration::QUICK);
    AddTestCase(new TimeOnAirTest, Duration::QUICK);
    AddTestCase(new PhyConnectivityTest, Duration::QUICK);
    AddTestCase(new MacCommandTest, Duration::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static LorawanTestSuite lorawanTestSuite;
