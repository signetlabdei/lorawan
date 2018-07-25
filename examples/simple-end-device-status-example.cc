/*
 * This script simulates a simple network in which one end device sends one
 * packet to the gateway.
 */

#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/end-device-lora-mac.h"
#include "ns3/gateway-lora-mac.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/position-allocator.h"
#include "ns3/one-shot-sender-helper.h"
#include "ns3/end-device-status.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lora-mac-header.h"
#include "ns3/command-line.h"
#include <algorithm>
#include <ctime>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SimpleEndDeviceStatusExample");

int main (int argc, char *argv[])
{

  // Set up logging
  LogComponentEnable ("EndDeviceStatus", LOG_LEVEL_ALL);
  LogComponentEnable ("SimpleEndDeviceStatusExample", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraMacHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraFrameHeader", LOG_LEVEL_ALL);
  LogComponentEnableAll (LOG_PREFIX_FUNC);
  LogComponentEnableAll (LOG_PREFIX_NODE);
  LogComponentEnableAll (LOG_PREFIX_TIME);

  /*********************************
   *  Create the end device status  *
   *********************************/
  EndDeviceStatus edStatus;
  uint8_t sf1= 10;
  uint8_t offset= 2;
  double freq1= 868.3;
  double freq2= 868.5;
  edStatus.SetFirstReceiveWindowSpreadingFactor(sf1);
  edStatus.SetSecondReceiveWindowOffset(offset);
  edStatus.SetFirstReceiveWindowFrequency(freq1);
  edStatus.SetSecondReceiveWindowFrequency(freq2);
  NS_LOG_INFO ("The edStatus has been initialized with: \n" <<
               "SF in RX1= " << double(edStatus.GetFirstReceiveWindowSpreadingFactor()) << "\n" <<
               "Offset of RX2= " << double(edStatus.GetSecondReceiveWindowOffset()) << "\n" <<
               "RX1 frequency= " << edStatus.GetFirstReceiveWindowFrequency() << "\n" <<
               "RX2 frequency= " << edStatus.GetSecondReceiveWindowFrequency());

  // Testing the whole reply
  Ptr<Packet> replyPayload = Create<Packet> (23);
  LoraMacHeader macHdr;
  macHdr.SetMType(LoraMacHeader::CONFIRMED_DATA_DOWN);
  LoraFrameHeader frameHdr;
  frameHdr.SetAdr(true);

  edStatus.SetReplyPayload(replyPayload);
  edStatus.SetReplyFrameHeader(frameHdr);
  edStatus.SetReplyMacHeader(macHdr);
  Ptr<Packet> reply= edStatus.GetCompleteReplyPacket ();


  NS_LOG_INFO("edStatus needs reply: " << edStatus.NeedsReply());

  // packet size should be payload + 1 byte (macHdr) + 8 byes (frame header without Opts)
  NS_LOG_INFO ("Packet size is: " << double(edStatus.GetCompleteReplyPacket
                                            ()->GetSize()));

  LoraFrameHeader replyFrameheader;
  LoraMacHeader replyMacheader;
  reply->RemoveHeader(replyMacheader);
  reply->RemoveHeader(replyFrameheader);
  NS_LOG_INFO ("Ack bit of reply is (0):  " << replyFrameheader.GetAck());
  NS_LOG_INFO ("Adr bit of reply is (1): " << replyFrameheader.GetAdr());
  NS_LOG_INFO ("MType of reply is : " << unsigned(replyMacheader.GetMType()));


  // testing initialize reply and building reply payload
  edStatus.InitializeReply();
  NS_LOG_INFO ("After initialization, edStatus needsReply= " << edStatus.NeedsReply());
  NS_LOG_INFO ("Adr bit of reply after initialization is (0): " << edStatus.GetReplyFrameHeader().GetAdr());

  // testing methods changing some fields of the reply and returning headers.
  LoraFrameHeader frameHeader;
  frameHeader.SetAck(true);
  edStatus.SetReplyFrameHeader(frameHeader);
  NS_LOG_INFO ("After setting only the frame header: edStatus needsReply= " << edStatus.NeedsReply());
  NS_LOG_INFO ("Packet size is: " << double(edStatus.GetCompleteReplyPacket ()->GetSize()));
  NS_LOG_INFO ("Ack bit of reply is (1):  " << edStatus.GetReplyFrameHeader().GetAck());
  NS_LOG_INFO ("MType of reply is:  " << unsigned(edStatus.GetReplyMacHeader().GetMType()));




  Simulator::Stop (Hours (2));

  Simulator::Run ();

  Simulator::Destroy ();

  return 0;
}
