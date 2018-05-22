/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

// // Include headers of classes to test
#include "ns3/log.h"
#include "ns3/lora-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/end-device-status.h"
#include "end-device-status-test.h"

// An essential include is test.h
#include "ns3/test.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EndDeviceStatusTest");

// Add some help text to this case to describe what it is intended to test
EndDeviceStatusTest::EndDeviceStatusTest ()
  : TestCase ("Verify that EndDeviceStatus works as expected")
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

  EndDeviceStatus edStatus;

  double frequency = 868.1;
  double differentFrequency = 868.3;
  uint8_t sf1 = 7;
  uint8_t offset = 1;


  // Test basic setters and getters
  edStatus.SetFirstReceiveWindowFrequency(frequency);
  NS_TEST_EXPECT_MSG_EQ (edStatus.GetFirstReceiveWindowFrequency(),868.1, "Setting of first receive window frequency didn't give the expected result");

  edStatus.SetFirstReceiveWindowSpreadingFactor(sf1);
  NS_TEST_EXPECT_MSG_EQ (edStatus.GetFirstReceiveWindowSpreadingFactor(),7, "Setting of first receive window spreading factor didn't give the expected result");

  edStatus.SetSecondReceiveWindowFrequency(differentFrequency);
  NS_TEST_EXPECT_MSG_EQ (edStatus.GetSecondReceiveWindowFrequency(),868.3, "Setting of second receive window frequency didn't give the expected result");

  edStatus.SetSecondReceiveWindowOffset(offset);
  NS_TEST_EXPECT_MSG_EQ (edStatus.GetSecondReceiveWindowOffset(),1, "Setting of second receive window spreading factor didn't give the expected result");
  
  // Testing reply setters and getters
  Ptr<Packet> replyPayload = Create<Packet> (23);
  LoraMacHeader macHdr;
  macHdr.SetMType(LoraMacHeader::CONFIRMED_DATA_DOWN);
  LoraFrameHeader frameHdr;
  frameHdr.SetAdr(true);

  edStatus.SetReplyPayload(replyPayload);
  edStatus.SetReplyFrameHeader(frameHdr);
  edStatus.SetReplyMacHeader(macHdr);
  Ptr<Packet> reply= edStatus.GetReply();


  NS_TEST_EXPECT_MSG_EQ (edStatus.NeedsReply(),true, "Setting and getting the reply frame header didn't give the expected result on the boolean \"has reply\" value.");
  NS_TEST_EXPECT_MSG_EQ (reply->GetSize(),32, "Setting and getting the reply frame header didn't give the expected result on the reply size.");

  // Checking that headers have been correctly setted with methods provided by Packet class
  LoraFrameHeader replyFrameheader;
  LoraMacHeader replyMacheader;
  reply->RemoveHeader(replyMacheader);
  reply->RemoveHeader(replyFrameheader);
  NS_TEST_EXPECT_MSG_EQ (replyFrameheader.GetAck(),false, "Setting the reply frame header didn't give the expected result");
  NS_TEST_EXPECT_MSG_EQ (replyFrameheader.GetAdr(),true, "Setting the reply frame header didn't give the expected result");


  NS_TEST_EXPECT_MSG_EQ ((replyMacheader.GetMType() == LoraMacHeader::CONFIRMED_DATA_DOWN),true, "Setting the reply mac header didn't give the expected result");


  // testing initialize reply and building reply payload
  edStatus.InitializeReply();
  NS_TEST_EXPECT_MSG_EQ (edStatus.NeedsReply(),false, "Reply initialization didn't give the expected result on the boolean \"has reply\" value.");
  NS_TEST_EXPECT_MSG_EQ (edStatus.GetReplyFrameHeader().GetAdr(),false, "Reply initialization didn't give the expected result");

  // testing methods changing some fields of the reply
  LoraFrameHeader frameHeader;
  frameHeader.SetAck(true);
  edStatus.SetReplyFrameHeader(frameHeader);
  NS_TEST_EXPECT_MSG_EQ (edStatus.NeedsReply(),true, "Setting and getting the reply frame header didn't give the expected result on the boolean \"has reply\" value.");
  NS_TEST_EXPECT_MSG_EQ (edStatus.GetReplyFrameHeader().GetAck(),true, "Setting and getting the reply frame header didn't give the expected result");

  edStatus.InitializeReply();
  LoraMacHeader macHeader;
  macHeader.SetMType(LoraMacHeader::CONFIRMED_DATA_DOWN);
  NS_TEST_EXPECT_MSG_EQ ((edStatus.GetReplyMacHeader().GetMType() == LoraMacHeader::CONFIRMED_DATA_DOWN),false, "Setting and getting the reply mac header didn't give the expected result");

  edStatus.SetReplyMacHeader(macHeader);
  NS_TEST_EXPECT_MSG_EQ ((edStatus.GetReplyMacHeader().GetMType() == LoraMacHeader::CONFIRMED_DATA_DOWN),true, "Setting and getting the reply mac header didn't give the expected result");

  NS_TEST_EXPECT_MSG_EQ (edStatus.GetReply()->GetSize(),9, "Setting and getting the reply mac header didn't give the expected result on the packet size.");

  edStatus.InitializeReply();
  edStatus.SetPayloadSize(5);
  NS_TEST_EXPECT_MSG_EQ (edStatus.GetReply()->GetSize(),14, "Setting reply payload size didn't give the expected result on the packet size.");



  //TODO (using also Network status):
  // - InserReceivedPacket
  // - getReceivedPacketList and its inspection

}
