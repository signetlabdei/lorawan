/*
 * Copyright (c) 2025 ML!PA Consulting GmbH
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Marian Buschsieweke <marian.buschsieweke@posteo.net>
 */

// Include headers of classes to test
#include "ns3/log.h"
#include "ns3/loratap-header.h"

// An essential include is test.h
#include "ns3/test.h"

#include <cmath>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("PcapSerializationTestSuite");

/**
 * @ingroup lorawan
 *
 * It tests the correct functionality of serialization and deserialization of
 * LoRa frames for PCap exporting is tested
 */
class LoraTapHeaderTest : public TestCase
{
  public:
    LoraTapHeaderTest(); //!< Default constructor

  private:
    void DoRun() override;
    void RunGetterSetterTest();
    void RunSimpleSerializationDeserializationTest();
    void RunConstructionFromLoraTagTest();
};

LoraTapHeaderTest::LoraTapHeaderTest()
    : TestCase("Verify correct seralization and deserialization of LoRa Frames")
{
}

void
LoraTapHeaderTest::RunGetterSetterTest()
{
    const uint32_t frequencyHz = 868100000;
    const uint32_t bandwidthHz = 125000;
    const uint8_t spreading = 10;
    const double snrDb = -9.75;
    const double rssiMaxDbm = -30;
    const uint8_t sync = 0x34;
    const double rssiCurrentDbm = -40;
    const double rssiPacketDbm = -50;

    LoraTapHeader loratap;
    loratap.SetChannelFrequency(frequencyHz);
    loratap.SetChannelBandwidth(bandwidthHz);
    loratap.SetSpreadingFactor(spreading);
    loratap.SetSignalToNoise(snrDb);
    loratap.SetSyncWord(sync);

    // Check that HasRssiFoobar() says false when there is *NO* RSSI info
    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiPacket(), false, "No RSSI info provided yet");
    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiReceiverCurrent(), false, "No RSSI info provided yet");
    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiReceiverMax(), false, "No RSSI info provided yet");

    loratap.SetRssiPacket(rssiPacketDbm);
    loratap.SetRssiReceiverCurrent(rssiCurrentDbm);
    loratap.SetRssiReceiverMax(rssiMaxDbm);

    // Check that HasRssiFoobar() says true when there *IS* RSSI info
    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiPacket(), true, "RSSI info provided ");
    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiReceiverCurrent(), true, "RSSI info provided");
    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiReceiverMax(), true, "RSSI info provided");

    // Check if getters work as expected
    NS_TEST_EXPECT_MSG_EQ(loratap.GetChannelFrequency(), frequencyHz, "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(loratap.GetChannelBandwidth(), bandwidthHz, "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(loratap.GetSpreadingFactor(), spreading, "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(TestDoubleIsEqual(loratap.GetSignalToNoise(), snrDb),
                          true,
                          "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(loratap.GetSyncWord(), sync, "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(TestDoubleIsEqual(loratap.GetRssiPacket(), rssiPacketDbm),
                          true,
                          "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(TestDoubleIsEqual(loratap.GetRssiReceiverCurrent(), rssiCurrentDbm),
                          true,
                          "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(TestDoubleIsEqual(loratap.GetRssiReceiverMax(), rssiMaxDbm),
                          true,
                          "Getter/setter misbehaving");

    // Check that clearing RSSI info works
    loratap.ClearRssiPacket();
    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiPacket(), false, "RSSI info cleared");
    loratap.ClearRssiReceiverCurrent();
    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiReceiverCurrent(), false, "RSSI info cleared");
    loratap.ClearRssiReceiverMax();
    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiReceiverMax(), false, "RSSI info cleared");
}

void
LoraTapHeaderTest::RunSimpleSerializationDeserializationTest()
{
    const uint32_t frequencyHz = 868100000;
    const uint32_t bandwidthHz = 125000;
    const uint8_t spreading = 10;
    const double snrDb = -9.75;
    const double rssiMaxDbm = -30;
    const uint8_t sync = 0x34;
    const double rssiCurrentDbm = -40;
    const double rssiPacketDbm = -80;

    LoraTapHeader loratap;
    loratap.SetChannelFrequency(frequencyHz);
    loratap.SetChannelBandwidth(bandwidthHz);
    loratap.SetSpreadingFactor(spreading);
    loratap.SetSignalToNoise(snrDb);
    loratap.SetSyncWord(sync);
    loratap.SetRssiPacket(rssiPacketDbm);
    loratap.SetRssiReceiverCurrent(rssiCurrentDbm);
    loratap.SetRssiReceiverMax(rssiMaxDbm);

    const uint8_t expected[] =
        {0x00, 0x00, 0x00, 0x0f, 0x33, 0xbe, 0x27, 0xa0, 0x01, 0x0a, 0xec, 0x6d, 0x63, 0xd9, 0x34};
    uint8_t tmp[sizeof(expected)];
    memset(tmp, 0xff, sizeof(tmp));

    Buffer buf;
    buf.AddAtStart(sizeof(expected));
    auto tapSerialized = buf.Begin();
    loratap.Serialize(tapSerialized);

    NS_TEST_ASSERT_MSG_EQ(tapSerialized.GetSize(),
                          sizeof(expected),
                          "Expected and actual serialization must match in size");

    tapSerialized.Read(tmp, sizeof(tmp));
    NS_TEST_EXPECT_MSG_EQ(memcmp(tmp, expected, sizeof(expected)),
                          0,
                          "Expected and actual serialization need to be bitwise identical");

    // Quickly testing that packet RSSI is correctly serialized when SNR >= 0
    loratap.SetSignalToNoise(0);
    tapSerialized = buf.Begin();
    loratap.Serialize(tapSerialized);
    tapSerialized.Read(tmp, sizeof(tmp));
    const unsigned rssiPktPos = 10;
    NS_TEST_EXPECT_MSG_EQ(tmp[rssiPktPos],
                          (int32_t)rssiPacketDbm + 139,
                          "Serialization of packet RSSI needs to change depending on SNR value");
}

void
LoraTapHeaderTest::RunConstructionFromLoraTagTest()
{
    const uint32_t frequencyHz = 868100000;
    const uint32_t bandwidthHz = 125000;
    const uint8_t spreading = 10;
    const double snrDb = 0.0; // TODO: Add SNR info to LoraTag
    const uint8_t sync = 0x34;
    const double rssiDbm = -80;

    LoraTag tag;

    tag.SetFrequency(frequencyHz);
    tag.SetReceivePower(rssiDbm);
    tag.SetSpreadingFactor(spreading);

    LoraTapHeader loratap(tag);

    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiPacket(), true, "Packet RSSI info provided");
    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiReceiverCurrent(),
                          false,
                          "No receiver RSSI info provided yet");
    NS_TEST_EXPECT_MSG_EQ(loratap.HasRssiReceiverMax(),
                          false,
                          "No receiver RSSI info provided yet");
    NS_TEST_EXPECT_MSG_EQ(loratap.GetChannelFrequency(), frequencyHz, "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(loratap.GetChannelBandwidth(), bandwidthHz, "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(loratap.GetSpreadingFactor(), spreading, "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(TestDoubleIsEqual(loratap.GetSignalToNoise(), snrDb),
                          true,
                          "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(loratap.GetSyncWord(), sync, "Getter/setter misbehaving");
    NS_TEST_EXPECT_MSG_EQ(TestDoubleIsEqual(loratap.GetRssiPacket(), rssiDbm),
                          true,
                          "Getter/setter misbehaving");
}

void
LoraTapHeaderTest::DoRun()
{
    NS_LOG_DEBUG("LoraTapHeaderTest");

    RunGetterSetterTest();
    RunSimpleSerializationDeserializationTest();
    RunConstructionFromLoraTagTest();
}

/**
 * @ingroup lorawan
 *
 * This TestSuite contains the test for PCAP serialization code and related
 * helpers.
 */
class LorawanPcapSerializationTestSuite : public TestSuite
{
  public:
    LorawanPcapSerializationTestSuite(); //!< Default constructor
};

LorawanPcapSerializationTestSuite::LorawanPcapSerializationTestSuite()
    : TestSuite("lorawan-pcap-serialization", Type::UNIT)
{
    AddTestCase(new LoraTapHeaderTest, Duration::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
static LorawanPcapSerializationTestSuite lorawanPcapSerializationTestSuite;
