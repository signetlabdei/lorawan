
// Include headers of classes to test
#include "ns3/constant-position-mobility-model.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/log.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lorawan-helper.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/mobility-helper.h"
#include "ns3/one-shot-sender-helper.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("LorawanTestSuite");

/********************
 * InterferenceTest *
 ********************/

class InterferenceTest : public TestCase
{
  public:
    InterferenceTest();
    ~InterferenceTest() override;

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

    // The following tests are designed around GOURSAUD signal-to-interference matrix
    auto interference = CreateObject<LoraInterferenceHelper>();
    interference->SetIsolationMatrix(LoraInterferenceHelper::GOURSAUD);

    double frequency = 868100000;
    double differentFrequency = 868300000;

    Ptr<LoraInterferenceHelper::Event> event;
    Ptr<LoraInterferenceHelper::Event> event1;

    // Test overlap duration
    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    event1 = interference->Add(Seconds(1), 14, 12, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->GetOverlapTime(event, event1),
                          Seconds(1),
                          "Overlap computation didn't give the expected result");

    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    event1 = interference->Add(Seconds(1.5), 14, 12, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->GetOverlapTime(event, event1),
                          Seconds(1.5),
                          "Overlap computation didn't give the expected result");

    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    event1 = interference->Add(Seconds(3), 14, 12, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->GetOverlapTime(event, event1),
                          Seconds(2),
                          "Overlap computation didn't give the expected result");

    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    event1 = interference->Add(Seconds(2), 14, 12, nullptr, frequency);
    // Because of some strange behavior, this test would get stuck if we used the same syntax of the
    // previous ones. This works instead.
    bool retval = interference->GetOverlapTime(event, event1) == Seconds(2);
    NS_TEST_EXPECT_MSG_EQ(retval, true, "Overlap computation didn't give the expected result");

    // Perfect overlap, packet survives
    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    interference->Add(Seconds(2), 14, 12, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");

    // Perfect overlap, packet survives
    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    interference->Add(Seconds(2), 14 - 7, 7, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");

    // Perfect overlap, packet destroyed
    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    interference->Add(Seconds(2), 14 - 6, 7, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->IsDestroyedByInterference(event),
                          7,
                          "Packet was not destroyed by interference as expected");

    // Partial overlap, packet survives
    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    interference->Add(Seconds(1), 14 - 6, 7, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");

    // Different frequencys
    // Packet would be destroyed if they were on the same frequency, but survives
    // since they are on different frequencies
    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    interference->Add(Seconds(2), 14, 7, nullptr, differentFrequency);
    NS_TEST_EXPECT_MSG_EQ(interference->IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");

    // Different SFs
    // Packet would be destroyed if they both were SF7, but survives thanks to SF
    // orthogonality
    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    interference->Add(Seconds(2), 14 + 16, 8, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");

    // SF imperfect orthogonality
    // Different SFs are orthogonal only up to a point
    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    interference->Add(Seconds(2), 14 + 17, 8, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->IsDestroyedByInterference(event),
                          8,
                          "Packet was not destroyed by interference as expected");

    // If a more 'distant' SF is used, isolation gets better
    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    interference->Add(Seconds(2), 14 + 17, 10, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->IsDestroyedByInterference(event),
                          0,
                          "Packet was destroyed by interference while it should have survived");

    // Cumulative interference
    // Same-SF interference is cumulative
    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    interference->Add(Seconds(2), 14 + 16, 8, nullptr, frequency);
    interference->Add(Seconds(2), 14 + 16, 8, nullptr, frequency);
    interference->Add(Seconds(2), 14 + 16, 8, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->IsDestroyedByInterference(event),
                          8,
                          "Packet was not destroyed by interference as expected");

    // Cumulative interference
    // Interference is not cumulative between different SFs
    interference->ClearAllEvents();
    event = interference->Add(Seconds(2), 14, 7, nullptr, frequency);
    interference->Add(Seconds(2), 14 + 16, 8, nullptr, frequency);
    interference->Add(Seconds(2), 14 + 16, 9, nullptr, frequency);
    interference->Add(Seconds(2), 14 + 16, 10, nullptr, frequency);
    NS_TEST_EXPECT_MSG_EQ(interference->IsDestroyedByInterference(event),
                          0,
                          "Packet did not survive interference as expected");
}

/***************
 * AddressTest *
 ***************/

class AddressTest : public TestCase
{
  public:
    AddressTest();
    ~AddressTest() override;

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

/***************
 * HeaderTest *
 ***************/

class HeaderTest : public TestCase
{
  public:
    HeaderTest();
    ~HeaderTest() override;

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
    LorawanMacHeader mHdr;
    mHdr.SetFType(LorawanMacHeader::CONFIRMED_DATA_DOWN);
    mHdr.SetMajor(1);

    Buffer macBuf;
    macBuf.AddAtStart(100);
    Buffer::Iterator macSerialized = macBuf.Begin();
    mHdr.Serialize(macSerialized);

    mHdr.Deserialize(macSerialized);

    NS_TEST_EXPECT_MSG_EQ((mHdr.GetFType() == LorawanMacHeader::CONFIRMED_DATA_DOWN),
                          true,
                          "FType changes in the serialization/deserialization process");
    NS_TEST_EXPECT_MSG_EQ((mHdr.GetMajor() == 1),
                          true,
                          "FType changes in the serialization/deserialization process");

    ////////////////////////////////////
    // Test the LoraFrameHeader class //
    ////////////////////////////////////
    LoraFrameHeader fHdr;
    fHdr.SetAsDownlink();
    fHdr.SetAck(true);
    fHdr.SetAdr(false);
    fHdr.SetFCnt(1);
    fHdr.SetAddress(LoraDeviceAddress(56, 1864));
    fHdr.AddLinkCheckAns(10, 1);

    // Serialization
    Buffer buf;
    buf.AddAtStart(100);
    Buffer::Iterator serialized = buf.Begin();
    fHdr.Serialize(serialized);

    // Deserialization
    fHdr.Deserialize(serialized);

    Ptr<LinkCheckAns> command = DynamicCast<LinkCheckAns>(*(fHdr.GetCommands().begin()));
    uint8_t margin = command->GetMargin();
    uint8_t gwCnt = command->GetGwCnt();

    NS_TEST_EXPECT_MSG_EQ(fHdr.GetAck(),
                          true,
                          "Ack changes in the serialization/deserialization process");
    NS_TEST_EXPECT_MSG_EQ(fHdr.GetAdr(),
                          false,
                          "Adr changes in the serialization/deserialization process");
    NS_TEST_EXPECT_MSG_EQ(fHdr.GetFCnt(),
                          1,
                          "FCnt changes in the serialization/deserialization process");
    NS_TEST_EXPECT_MSG_EQ((fHdr.GetAddress() == LoraDeviceAddress(56, 1864)),
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
    pkt->AddHeader(fHdr);
    pkt->AddHeader(mHdr);

    // Length = Payload + FrameHeader + MacHeader
    //        = 10 + (8+3) + 1 = 22
    NS_TEST_EXPECT_MSG_EQ((pkt->GetSize()), 22, "Wrong size of packet + headers");

    LorawanMacHeader mHdr1;

    pkt->RemoveHeader(mHdr1);

    NS_TEST_EXPECT_MSG_EQ((pkt->GetSize()), 21, "Wrong size of packet + headers - macHeader");

    LoraFrameHeader fHdr1;
    fHdr1.SetAsDownlink();

    pkt->RemoveHeader(fHdr1);
    Ptr<LinkCheckAns> linkCheckAns = DynamicCast<LinkCheckAns>(*(fHdr1.GetCommands().begin()));

    NS_TEST_EXPECT_MSG_EQ((pkt->GetSize()),
                          10,
                          "Wrong size of packet + headers - macHeader - frameHeader");

    // Verify contents of removed MAC header
    NS_TEST_EXPECT_MSG_EQ(mHdr1.GetFType(), mHdr.GetFType(), "Removed header contents don't match");
    NS_TEST_EXPECT_MSG_EQ(mHdr1.GetMajor(), mHdr.GetMajor(), "Removed header contents don't match");

    // Verify contents of removed frame header
    NS_TEST_EXPECT_MSG_EQ(fHdr1.GetAck(), fHdr.GetAck(), "Removed header contents don't match");
    NS_TEST_EXPECT_MSG_EQ(fHdr1.GetAdr(), fHdr.GetAdr(), "Removed header contents don't match");
    NS_TEST_EXPECT_MSG_EQ(fHdr1.GetFCnt(), fHdr.GetFCnt(), "Removed header contents don't match");
    NS_TEST_EXPECT_MSG_EQ((fHdr1.GetAddress() == fHdr.GetAddress()),
                          true,
                          "Removed header contents don't match");
    NS_TEST_EXPECT_MSG_EQ(linkCheckAns->GetMargin(),
                          10,
                          "Removed header's MAC command contents don't match");
    NS_TEST_EXPECT_MSG_EQ(linkCheckAns->GetGwCnt(),
                          1,
                          "Removed header's MAC command contents don't match");
}

/*******************
 * ReceivePathTest *
 *******************/

class ReceivePathTest : public TestCase
{
  public:
    ReceivePathTest();
    ~ReceivePathTest() override;

  private:
    void DoRun() override;
    void Reset();
    void OccupiedReceptionPaths(int oldValue, int newValue);
    void NoMoreDemodulators(Ptr<const Packet> packet, uint32_t node);
    void Interference(Ptr<const Packet> packet, uint32_t node);
    void ReceivedPacket(Ptr<const Packet> packet, uint32_t node);

    Ptr<GatewayLoraPhy> gatewayPhy;
    int m_noMoreDemodulatorsCalls = 0;
    int m_interferenceCalls = 0;
    int m_receivedPacketCalls = 0;
    int m_maxOccupiedReceptionPaths = 0;
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
    m_noMoreDemodulatorsCalls = 0;
    m_interferenceCalls = 0;
    m_receivedPacketCalls = 0;
    m_maxOccupiedReceptionPaths = 0;

    // The following tests are designed around GOURSAUD signal-to-interference matrix
    auto interference = CreateObject<LoraInterferenceHelper>();
    interference->SetIsolationMatrix(LoraInterferenceHelper::GOURSAUD);

    gatewayPhy = CreateObject<GatewayLoraPhy>();
    gatewayPhy->SetInterferenceHelper(interference);
    gatewayPhy->SetReceptionPaths(6);

    // From GatewayLoraPhy
    gatewayPhy->TraceConnectWithoutContext(
        "LostPacketBecauseNoMoreReceivers",
        MakeCallback(&ReceivePathTest::NoMoreDemodulators, this));
    gatewayPhy->TraceConnectWithoutContext(
        "OccupiedReceptionPaths",
        MakeCallback(&ReceivePathTest::OccupiedReceptionPaths, this));

    // From LoraPhy
    gatewayPhy->TraceConnectWithoutContext("LostPacketBecauseInterference",
                                           MakeCallback(&ReceivePathTest::Interference, this));
    gatewayPhy->TraceConnectWithoutContext("ReceivedPacket",
                                           MakeCallback(&ReceivePathTest::ReceivedPacket, this));
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

    ///////////////////////////////////////////////////////////
    // If no ReceptionPath is configured, no packet is received
    ///////////////////////////////////////////////////////////

    Reset();
    gatewayPhy->SetReceptionPaths(0);

    Simulator::Schedule(Seconds(1),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(1),
                        868100000);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_noMoreDemodulatorsCalls, 1, "Unexpected value");

    //////////////////////////////////////////////////////////////////////////////
    // A ReceptionPath can receive a packet of any SF without any preconfiguration
    //////////////////////////////////////////////////////////////////////////////

    Reset();
    gatewayPhy->SetReceptionPaths(1);

    Simulator::Schedule(Seconds(1),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(1),
                        868100000);
    Simulator::Schedule(Seconds(3),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        8,
                        Seconds(1),
                        868100000);
    Simulator::Schedule(Seconds(5),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        9,
                        Seconds(1),
                        868100000);
    Simulator::Schedule(Seconds(7),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        10,
                        Seconds(1),
                        868100000);
    Simulator::Schedule(Seconds(9),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        11,
                        Seconds(1),
                        868100000);
    Simulator::Schedule(Seconds(11),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        12,
                        Seconds(1),
                        868100000);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_noMoreDemodulatorsCalls, 0, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketCalls, 6, "Unexpected value");

    ///////////////////////////////////////////////////////////////////////////////////////
    // Schedule two overlapping reception events. Each packet should be received correctly.
    ///////////////////////////////////////////////////////////////////////////////////////

    Reset();
    gatewayPhy->SetReceptionPaths(2);

    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(3),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        9,
                        Seconds(4),
                        868100000);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_noMoreDemodulatorsCalls, 0, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketCalls, 2, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_maxOccupiedReceptionPaths, 2, "Unexpected value");

    //////////////////////////////////////////////////////////////////////////////////
    // Interference between packets on the same frequency and different ReceptionPaths
    //////////////////////////////////////////////////////////////////////////////////

    Reset();
    gatewayPhy->SetReceptionPaths(2);

    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(3),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_noMoreDemodulatorsCalls, 0, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_interferenceCalls, 2, "Unexpected value");

    /////////////////////////////////////////////////////////////
    // Three receptions where only two receivePaths are available
    /////////////////////////////////////////////////////////////

    Reset();
    gatewayPhy->SetReceptionPaths(2);

    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(3),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_noMoreDemodulatorsCalls, 1, "Unexpected value");

    ///////////////////////////////////////////////////////////////////////////
    // Packets that are on different frequencys do not interfere
    ///////////////////////////////////////////////////////////////////////////

    Reset();
    gatewayPhy->SetReceptionPaths(2);

    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868300000);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_interferenceCalls, 0, "Unexpected value");

    ///////////////////////////////////////////////////////////////////////////
    // Full capacity (siw packets, on six SFs, distributed over 3 frequencies)
    ///////////////////////////////////////////////////////////////////////////

    Reset();

    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        8,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        9,
                        Seconds(4),
                        868300000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        10,
                        Seconds(4),
                        868300000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        11,
                        Seconds(4),
                        868500000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        12,
                        Seconds(4),
                        868500000);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_noMoreDemodulatorsCalls, 0, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_interferenceCalls, 0, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketCalls, 6, "Unexpected value");

    ///////////////////////////////////////////////////////////////////////////
    // Full capacity + 1
    ///////////////////////////////////////////////////////////////////////////

    Reset();

    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        8,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        9,
                        Seconds(4),
                        868300000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        10,
                        Seconds(4),
                        868300000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        11,
                        Seconds(4),
                        868500000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        12,
                        Seconds(4),
                        868500000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        10,
                        Seconds(4),
                        868500000);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_noMoreDemodulatorsCalls, 1, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_interferenceCalls, 0, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketCalls, 6, "Unexpected value");

    ////////////////////////////////////
    // Receive Paths are correctly freed
    ////////////////////////////////////

    Reset();

    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        8,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        9,
                        Seconds(4),
                        868300000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        10,
                        Seconds(4),
                        868300000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        11,
                        Seconds(4),
                        868500000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        12,
                        Seconds(4),
                        868500000);

    Simulator::Schedule(Seconds(8),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(8),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        8,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(8),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        9,
                        Seconds(4),
                        868300000);
    Simulator::Schedule(Seconds(8),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        10,
                        Seconds(4),
                        868300000);
    Simulator::Schedule(Seconds(8),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        11,
                        Seconds(4),
                        868500000);
    Simulator::Schedule(Seconds(8),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        12,
                        Seconds(4),
                        868500000);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_noMoreDemodulatorsCalls, 0, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_interferenceCalls, 0, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketCalls, 12, "Unexpected value");

    /////////////////////////////////////////////////////////////
    // Receive Paths stay occupied exactly for the necessary time
    /////////////////////////////////////////////////////////////

    Reset();
    gatewayPhy->SetReceptionPaths(2);

    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);
    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        8,
                        Seconds(4),
                        868100000);

    // This packet will find no free ReceptionPaths
    Simulator::Schedule(Seconds(2 + 4) - NanoSeconds(1),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        9,
                        Seconds(4),
                        868100000);

    // This packet will find a free ReceptionPath
    Simulator::Schedule(Seconds(2 + 4) + NanoSeconds(1),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        10,
                        Seconds(4),
                        868100000);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_noMoreDemodulatorsCalls, 1, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_interferenceCalls, 0, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketCalls, 3, "Unexpected value");

    ////////////////////////////////////////////////////
    // Only one ReceivePath locks on the incoming packet
    ////////////////////////////////////////////////////

    Reset();

    Simulator::Schedule(Seconds(2),
                        &GatewayLoraPhy::StartReceive,
                        gatewayPhy,
                        packet,
                        14,
                        7,
                        Seconds(4),
                        868100000);

    Simulator::Stop(Hours(2));
    Simulator::Run();
    Simulator::Destroy();

    NS_TEST_EXPECT_MSG_EQ(m_noMoreDemodulatorsCalls, 0, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_interferenceCalls, 0, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketCalls, 1, "Unexpected value");
    NS_TEST_EXPECT_MSG_EQ(m_maxOccupiedReceptionPaths, 1, "Unexpected value");
}

/**************************
 * LogicalChannelTest *
 **************************/

class LogicalChannelTest : public TestCase
{
  public:
    LogicalChannelTest();
    ~LogicalChannelTest() override;

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
LogicalChannelTest::LogicalChannelTest()
    : TestCase("Verify that LogicalChannel and LogicalChannelManager work as expected")
{
}

// Reminder that the test case should clean up after itself
LogicalChannelTest::~LogicalChannelTest()
{
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
LogicalChannelTest::DoRun()
{
    NS_LOG_DEBUG("LogicalChannelTest");

    /////////////////////////////
    // Test LogicalChannel //
    /////////////////////////////

    // Setup
    Ptr<LogicalChannel> channel0 = Create<LogicalChannel>(868000000);
    Ptr<LogicalChannel> channel1 = Create<LogicalChannel>(868000000);
    Ptr<LogicalChannel> channel2 = Create<LogicalChannel>(868100000);
    Ptr<LogicalChannel> channel3 = Create<LogicalChannel>(868001000);

    // Equality between channels
    // Test the == and != operators
    NS_TEST_EXPECT_MSG_EQ(channel0, channel1, "== operator doesn't work as expected");
    NS_TEST_EXPECT_MSG_NE(channel0, channel2, "!= operator doesn't work as expected");
    NS_TEST_EXPECT_MSG_NE(channel0, channel3, "!= operator doesn't work as expected");

    //////////////////
    // Test SubBand //
    //////////////////

    // Setup
    SubBand subBand0(868000000, 868700000, 0.01, 14);
    Ptr<LogicalChannel> channel4 = Create<LogicalChannel>(870000000);

    // Test BelongsToSubBand
    NS_TEST_EXPECT_MSG_EQ(subBand0.BelongsToSubBand(channel2),
                          true,
                          "BelongsToSubBand does not behave as expected");
    NS_TEST_EXPECT_MSG_EQ(subBand0.BelongsToSubBand(channel2->GetFrequency()),
                          true,
                          "BelongsToSubBand does not behave as expected");
    NS_TEST_EXPECT_MSG_EQ(subBand0.BelongsToSubBand(channel4),
                          false,
                          "BelongsToSubBand does not behave as expected");

    ///////////////////////////////////
    // Test LogicalChannelManager //
    ///////////////////////////////////

    // Setup
    Ptr<LogicalChannelManager> channelHelper = CreateObject<LogicalChannelManager>();
    SubBand subBand1(869000000, 869400000, 0.1, 27);
    channel0 = Create<LogicalChannel>(868100000);
    channel1 = Create<LogicalChannel>(868300000);
    channel2 = Create<LogicalChannel>(868500000);
    channel3 = Create<LogicalChannel>(869100000);
    channel4 = Create<LogicalChannel>(869300000);

    // Channel diagram
    //
    // Channels      0      1      2                     3       4
    // SubBands  868 ----- 0.1% ----- 868.7       869 ----- 1% ----- 869.4

    // Add SubBands and LogicalChannels to the helper
    channelHelper->AddSubBand(&subBand0);
    channelHelper->AddSubBand(&subBand1);
    channelHelper->AddChannel(0, channel0);
    channelHelper->AddChannel(1, channel1);
    channelHelper->AddChannel(2, channel2);
    channelHelper->AddChannel(3, channel3);
    channelHelper->AddChannel(4, channel4);

    // Duty Cycle tests
    // (high level duty cycle behavior)
    ///////////////////////////////////

    channelHelper->AddEvent(Seconds(2), channel1);
    Time expectedTimeOff = Seconds(2 / 0.01);

    // Waiting time is computed correctly
    NS_TEST_EXPECT_MSG_EQ(channelHelper->GetWaitingTime(channel0),
                          expectedTimeOff,
                          "Waiting time doesn't behave as expected");

    // Duty Cycle involves the whole SubBand, not just a channel
    NS_TEST_EXPECT_MSG_EQ(channelHelper->GetWaitingTime(channel1),
                          expectedTimeOff,
                          "Waiting time doesn't behave as expected");
    NS_TEST_EXPECT_MSG_EQ(channelHelper->GetWaitingTime(channel2),
                          expectedTimeOff,
                          "Waiting time doesn't behave as expected");

    // Other bands are not affected by this transmission
    NS_TEST_EXPECT_MSG_EQ(channelHelper->GetWaitingTime(channel3),
                          Time(0),
                          "Waiting time affects other subbands");
    NS_TEST_EXPECT_MSG_EQ(channelHelper->GetWaitingTime(channel4),
                          Time(0),
                          "Waiting time affects other subbands");
}

/*****************
 * TimeOnAirTest *
 *****************/

class TimeOnAirTest : public TestCase
{
  public:
    TimeOnAirTest();
    ~TimeOnAirTest() override;

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
    LoraPhyTxParameters txParams;
    txParams.sf = 7;
    txParams.headerDisabled = false;
    txParams.codingRate = 1;
    txParams.bandwidthHz = 125000;
    txParams.nPreamble = 8;
    txParams.crcEnabled = 1;
    txParams.lowDataRateOptimizationEnabled = false;

    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.041216, 0.0001, "Unexpected duration");

    txParams.sf = 8;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.072192, 0.0001, "Unexpected duration");

    txParams.headerDisabled = true;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.072192, 0.0001, "Unexpected duration");

    txParams.codingRate = 2;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.078336, 0.0001, "Unexpected duration");

    txParams.nPreamble = 10;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.082432, 0.0001, "Unexpected duration");

    txParams.lowDataRateOptimizationEnabled = true;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.082432, 0.0001, "Unexpected duration");

    txParams.sf = 10;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.280576, 0.0001, "Unexpected duration");

    txParams.bandwidthHz = 250000;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.14028, 0.0001, "Unexpected duration");

    txParams.bandwidthHz = 500000;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.070144, 0.0001, "Unexpected duration");

    txParams.headerDisabled = false;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.082432, 0.0001, "Unexpected duration");

    txParams.nPreamble = 8;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.078336, 0.0001, "Unexpected duration");

    txParams.sf = 12;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.264192, 0.0001, "Unexpected duration");

    packet = Create<Packet>(50);
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 0.657408, 0.0001, "Unexpected duration");

    txParams.bandwidthHz = 125000;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 2.629632, 0.0001, "Unexpected duration");

    txParams.codingRate = 1;
    duration = LoraPhy::GetTimeOnAir(packet, txParams);
    NS_TEST_EXPECT_MSG_EQ_TOL(duration.GetSeconds(), 2.301952, 0.0001, "Unexpected duration");
}

/**************************
 * PhyConnectivityTest *
 **************************/

class PhyConnectivityTest : public TestCase
{
  public:
    PhyConnectivityTest();
    ~PhyConnectivityTest() override;
    void Reset();
    void ReceivedPacket(Ptr<const Packet> packet, uint32_t node);
    void UnderSensitivity(Ptr<const Packet> packet, uint32_t node);
    void Interference(Ptr<const Packet> packet, uint32_t node);
    void WrongFrequency(Ptr<const Packet> packet, uint32_t node);
    void WrongSf(Ptr<const Packet> packet, uint32_t node);
    bool HaveSamePacketContents(Ptr<Packet> packet1, Ptr<Packet> packet2);

  private:
    void DoRun() override;
    Ptr<LoraChannel> channel;
    Ptr<EndDeviceLoraPhy> edPhy1;
    Ptr<EndDeviceLoraPhy> edPhy2;
    Ptr<GatewayLoraPhy> gwPhy1;
    Ptr<GatewayLoraPhy> gwPhy2;

    Ptr<Packet> m_latestReceivedPacket;
    int m_receivedPacketCalls = 0;
    int m_underSensitivityCalls = 0;
    int m_interferenceCalls = 0;
    int m_wrongSfCalls = 0;
    int m_wrongFrequencyCalls = 0;
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
    NS_LOG_FUNCTION(packet << (unsigned)node);

    m_receivedPacketCalls++;

    m_latestReceivedPacket = packet->Copy();
}

void
PhyConnectivityTest::UnderSensitivity(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << (unsigned)node);

    m_underSensitivityCalls++;
}

void
PhyConnectivityTest::Interference(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << (unsigned)node);

    m_interferenceCalls++;
}

void
PhyConnectivityTest::WrongSf(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << (unsigned)node);

    m_wrongSfCalls++;
}

void
PhyConnectivityTest::WrongFrequency(Ptr<const Packet> packet, uint32_t node)
{
    NS_LOG_FUNCTION(packet << (unsigned)node);

    m_wrongFrequencyCalls++;
}

bool
PhyConnectivityTest::HaveSamePacketContents(Ptr<Packet> packet1, Ptr<Packet> packet2)
{
    NS_LOG_FUNCTION(packet1 << packet2);

    uint32_t size1 = packet1->GetSerializedSize();
    uint8_t buffer1[size1];
    packet1->Serialize(buffer1, size1);

    uint32_t size2 = packet2->GetSerializedSize();
    uint8_t buffer2[size2];
    packet2->Serialize(buffer2, size2);

    NS_ASSERT(size1 == size2);

    bool foundADifference = false;
    for (uint32_t i = 0; i < size1; i++)
    {
        NS_LOG_DEBUG(unsigned(buffer1[i]) << " " << unsigned(buffer2[i]));
        if (buffer1[i] != buffer2[i])
        {
            foundADifference = true;
            break;
        }
    }

    return !foundADifference;
}

void
PhyConnectivityTest::Reset()
{
    m_latestReceivedPacket = nullptr;
    m_receivedPacketCalls = 0;
    m_underSensitivityCalls = 0;
    m_interferenceCalls = 0;
    m_wrongSfCalls = 0;
    m_wrongFrequencyCalls = 0;

    auto loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);

    auto delay = CreateObject<ConstantSpeedPropagationDelayModel>();

    // Create the channel
    channel = CreateObject<LoraChannel>(loss, delay);

    // Connect PHYs
    edPhy1 = CreateObject<EndDeviceLoraPhy>();
    edPhy2 = CreateObject<EndDeviceLoraPhy>();
    gwPhy1 = CreateObject<GatewayLoraPhy>();
    gwPhy2 = CreateObject<GatewayLoraPhy>();

    /**
     * Positions:
     *             ed2
     *             0,10
     *
     *    gw1      ed1      gw2
     *  -10,0      0,0      10,0
     *
     */
    auto mob1 = CreateObject<ConstantPositionMobilityModel>();
    mob1->SetPosition(Vector(0, 0, 0));
    edPhy1->SetMobility(mob1);
    auto mob2 = CreateObject<ConstantPositionMobilityModel>();
    mob2->SetPosition(Vector(0, 10, 0));
    edPhy2->SetMobility(mob2);
    auto mob3 = CreateObject<ConstantPositionMobilityModel>();
    mob3->SetPosition(Vector(-10, 0, 0));
    gwPhy1->SetMobility(mob3);
    auto mob4 = CreateObject<ConstantPositionMobilityModel>();
    mob4->SetPosition(Vector(10, 0, 0));
    gwPhy2->SetMobility(mob4);

    edPhy1->SetChannel(channel);
    edPhy2->SetChannel(channel);
    gwPhy1->SetChannel(channel);
    gwPhy2->SetChannel(channel);

    // LoraPhy
    edPhy1->TraceConnectWithoutContext("ReceivedPacket",
                                       MakeCallback(&PhyConnectivityTest::ReceivedPacket, this));
    edPhy2->TraceConnectWithoutContext("ReceivedPacket",
                                       MakeCallback(&PhyConnectivityTest::ReceivedPacket, this));
    gwPhy1->TraceConnectWithoutContext("ReceivedPacket",
                                       MakeCallback(&PhyConnectivityTest::ReceivedPacket, this));
    gwPhy2->TraceConnectWithoutContext("ReceivedPacket",
                                       MakeCallback(&PhyConnectivityTest::ReceivedPacket, this));

    edPhy1->TraceConnectWithoutContext("LostPacketBecauseUnderSensitivity",
                                       MakeCallback(&PhyConnectivityTest::UnderSensitivity, this));
    edPhy2->TraceConnectWithoutContext("LostPacketBecauseUnderSensitivity",
                                       MakeCallback(&PhyConnectivityTest::UnderSensitivity, this));
    gwPhy1->TraceConnectWithoutContext("LostPacketBecauseUnderSensitivity",
                                       MakeCallback(&PhyConnectivityTest::UnderSensitivity, this));
    gwPhy2->TraceConnectWithoutContext("LostPacketBecauseUnderSensitivity",
                                       MakeCallback(&PhyConnectivityTest::UnderSensitivity, this));

    edPhy1->TraceConnectWithoutContext("LostPacketBecauseInterference",
                                       MakeCallback(&PhyConnectivityTest::Interference, this));
    edPhy2->TraceConnectWithoutContext("LostPacketBecauseInterference",
                                       MakeCallback(&PhyConnectivityTest::Interference, this));
    gwPhy1->TraceConnectWithoutContext("LostPacketBecauseInterference",
                                       MakeCallback(&PhyConnectivityTest::Interference, this));
    gwPhy2->TraceConnectWithoutContext("LostPacketBecauseInterference",
                                       MakeCallback(&PhyConnectivityTest::Interference, this));

    // End device only
    edPhy1->TraceConnectWithoutContext("LostPacketBecauseWrongFrequency",
                                       MakeCallback(&PhyConnectivityTest::WrongFrequency, this));
    edPhy2->TraceConnectWithoutContext("LostPacketBecauseWrongFrequency",
                                       MakeCallback(&PhyConnectivityTest::WrongFrequency, this));

    edPhy1->TraceConnectWithoutContext("LostPacketBecauseWrongSpreadingFactor",
                                       MakeCallback(&PhyConnectivityTest::WrongSf, this));
    edPhy2->TraceConnectWithoutContext("LostPacketBecauseWrongSpreadingFactor",
                                       MakeCallback(&PhyConnectivityTest::WrongSf, this));

    // Listen for a specific SpreadingFactor
    edPhy1->SetRxSpreadingFactor(12);
    edPhy2->SetRxSpreadingFactor(12);
    // Listen on a specific frequency
    edPhy1->SetRxFrequency(868100000);
    edPhy2->SetRxFrequency(868100000);

    edPhy1->SwitchToStandby();
    edPhy2->SwitchToStandby();

    edPhy1->Initialize();
    edPhy2->Initialize();
    gwPhy1->Initialize();
    gwPhy2->Initialize();
}

// This method is the pure virtual method from class TestCase that every
// TestCase must implement
void
PhyConnectivityTest::DoRun()
{
    NS_LOG_DEBUG("PhyConnectivityTest");

    // Setup
    ////////

    LoraPhyTxParameters txParams;
    // The following packet is used to test both uplink & downlink connectivity
    uint8_t buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    Ptr<Packet> packet = Create<Packet>(buffer, 10);
    LoraFrameHeader fHdr;
    packet->AddHeader(fHdr); // Default address is accepred by devices
    LorawanMacHeader mHdr;
    mHdr.SetFType(LorawanMacHeader::UNCONFIRMED_DATA_DOWN);
    packet->AddHeader(mHdr); // Currently, gateways don't care about UL/DL

    // Testing
    //////////

    // Basic packet delivery test
    /////////////////////////////

    // Both gateways receive packet

    Reset();
    txParams.sf = 12;

    Simulator::Schedule(Seconds(2),
                        &EndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(m_receivedPacketCalls,
                          2,
                          "Channel skipped some GW PHYs when delivering a packet"); // All GW PHYs

    Simulator::Destroy();

    // Sleeping PHYs do not receive downlink packet

    Reset();
    edPhy2->SwitchToSleep();
    Simulator::Schedule(Seconds(2), &GatewayLoraPhy::Send, gwPhy1, packet, txParams, 868100000, 14);

    Simulator::Stop(Hours(2));
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(
        m_receivedPacketCalls,
        1,
        "Packet was received by a ED PHY in SLEEP mode"); // All ED PHYs in Standby except one

    Simulator::Destroy();

    // Packet that arrives under sensitivity is received correctly if SF increases

    Reset();
    txParams.sf = 7;
    DynamicCast<ConstantPositionMobilityModel>(gwPhy2->GetMobility())
        ->SetPosition(Vector(3410, 0, 0));

    Simulator::Schedule(Seconds(2),
                        &EndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(
        m_underSensitivityCalls,
        1,
        "Packet that should have been lost because of low receive power was recevied");

    Simulator::Destroy();

    // Try again using a packet with higher SF

    Reset();
    txParams.sf = 8;
    edPhy2->SetRxSpreadingFactor(8);
    DynamicCast<ConstantPositionMobilityModel>(gwPhy2->GetMobility())
        ->SetPosition(Vector(3410, 0, 0));

    Simulator::Schedule(Seconds(2),
                        &EndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(m_underSensitivityCalls,
                          0,
                          "Packets that should have arrived above sensitivity were under it");

    Simulator::Destroy();

    // Packets can be destroyed by interference

    Reset();
    txParams.sf = 12;
    DynamicCast<ConstantPositionMobilityModel>(edPhy2->GetMobility())
        ->SetPosition(Vector(19.5, 0, 0));

    Simulator::Schedule(Seconds(2),
                        &EndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);
    Simulator::Schedule(Seconds(2),
                        &EndDeviceLoraPhy::Send,
                        edPhy2,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();

    // gwPhy1: both packets are lost because similar power
    // gwPhy2: able to capture the packet sent by closest device, lose other
    NS_TEST_EXPECT_MSG_EQ(m_interferenceCalls,
                          3,
                          "Packets that should be destroyed by interference weren't");

    Simulator::Destroy();

    // Packets can be lost because the PHY is not listening on the right frequency

    Reset();
    Simulator::Schedule(Seconds(2), &GatewayLoraPhy::Send, gwPhy1, packet, txParams, 868300000, 14);

    Simulator::Stop(Hours(2));
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(m_wrongFrequencyCalls,
                          2,
                          "Packets were received even though PHY was on a different frequency");

    Simulator::Destroy();

    // Packets can be lost because the PHY is not listening for the right SF

    Reset();
    txParams.sf = 8; // Send with 8, listening for 12
    Simulator::Schedule(Seconds(2), &GatewayLoraPhy::Send, gwPhy1, packet, txParams, 868100000, 14);

    Simulator::Stop(Hours(2));
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(m_wrongSfCalls,
                          2,
                          "Packets were received even though PHY was listening for a different SF");

    Simulator::Destroy();

    // Sending of packets
    /////////////////////

    // The very same packet arrives to the receiving PHYs

    Reset();
    Simulator::Schedule(Seconds(2),
                        &EndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(HaveSamePacketContents(packet, m_latestReceivedPacket),
                          true,
                          "Packet changed contents when going through the channel");

    Simulator::Destroy();

    // Correct state transitions
    ////////////////////////////

    // PHY switches to STANDBY after TX

    Reset();
    Simulator::Schedule(Seconds(2),
                        &EndDeviceLoraPhy::Send,
                        edPhy1,
                        packet,
                        txParams,
                        868100000,
                        14);

    Simulator::Stop(Hours(2));
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(edPhy1->GetState(),
                          EndDeviceLoraPhy::STANDBY,
                          "State didn't switch to STANDBY as expected");
    NS_TEST_EXPECT_MSG_EQ(edPhy2->GetState(),
                          EndDeviceLoraPhy::STANDBY,
                          "State didn't switch to STANDBY as expected");

    Simulator::Destroy();

    // PHY switches to STANDBY after RX

    Reset();
    Simulator::Schedule(Seconds(2), &GatewayLoraPhy::Send, gwPhy1, packet, txParams, 868100000, 14);

    Simulator::Stop(Hours(2));
    Simulator::Run();

    NS_TEST_EXPECT_MSG_EQ(edPhy1->GetState(),
                          EndDeviceLoraPhy::STANDBY,
                          "State didn't switch to STANDBY as expected");
    NS_TEST_EXPECT_MSG_EQ(edPhy2->GetState(),
                          EndDeviceLoraPhy::STANDBY,
                          "State didn't switch to STANDBY as expected");

    Simulator::Destroy();
}

/*****************
 * LorawanMacTest *
 *****************/

class LorawanMacTest : public TestCase
{
  public:
    LorawanMacTest();
    ~LorawanMacTest() override;

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
LorawanMacTest::LorawanMacTest()
    : TestCase("Verify that the MAC layer of EDs behaves as expected")
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

/**************
 * Test Suite *
 **************/

// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run. Typically, only the constructor for
// this class must be defined

class LorawanTestSuite : public TestSuite
{
  public:
    LorawanTestSuite();
};

LorawanTestSuite::LorawanTestSuite()
    : TestSuite("lorawan", UNIT)
{
    // LogComponentEnable("LorawanTestSuite", LOG_LEVEL_DEBUG);
    //  TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
    AddTestCase(new InterferenceTest, TestCase::QUICK);
    AddTestCase(new AddressTest, TestCase::QUICK);
    AddTestCase(new HeaderTest, TestCase::QUICK);
    AddTestCase(new ReceivePathTest, TestCase::QUICK);
    AddTestCase(new LogicalChannelTest, TestCase::QUICK);
    AddTestCase(new TimeOnAirTest, TestCase::QUICK);
    AddTestCase(new PhyConnectivityTest, TestCase::QUICK);
    AddTestCase(new LorawanMacTest, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static LorawanTestSuite lorawanTestSuite;
