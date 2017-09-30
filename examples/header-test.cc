#include "ns3/packet.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lora-mac-header.h"
#include "ns3/log.h"
#include "ns3/command-line.h"
#include <bitset>
#include <cstdlib>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LoraPacketTest");

void OnLinkCheckAns (uint8_t margin, uint8_t gwCnt)
{
  NS_LOG_INFO ("OnLinkCheckAns callback called, margin: " << unsigned(margin) << " dB, gwCnt: " << unsigned(gwCnt));
}

int main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);

  LogComponentEnable ("LoraPacketTest", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraFrameHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraMacHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("Packet", LOG_LEVEL_ALL);

  // Test the LoraMacHeader class
  /////////////////////////////////
  NS_LOG_INFO ("Testing MacHeader");

  LoraMacHeader macHdr;
  macHdr.SetMType (LoraMacHeader::CONFIRMED_DATA_DOWN);
  macHdr.SetMajor (1);

  // Serialization
  Buffer macBuf;
  macBuf.AddAtStart (100);
  Buffer::Iterator macSerialized = macBuf.Begin ();
  macHdr.Serialize (macSerialized);

  // Deserialization
  macHdr.Deserialize (macSerialized);

  NS_ASSERT (macHdr.GetMType () == LoraMacHeader::CONFIRMED_DATA_DOWN);
  NS_ASSERT (macHdr.GetMajor () == 1);

  NS_LOG_INFO ("Testing FrameHeader");
  LoraFrameHeader frameHdr;
  frameHdr.SetAsDownlink ();
  frameHdr.SetAck (true);
  frameHdr.SetAdr (false);
  frameHdr.SetFCnt (1);
  frameHdr.SetAddress (LoraDeviceAddress (56,1864));
  frameHdr.AddLinkCheckAns (10,1);

  // Serialization
  Buffer buf;
  buf.AddAtStart (100);
  Buffer::Iterator serialized = buf.Begin ();
  frameHdr.Serialize (serialized);

  // Deserialization
  frameHdr.Deserialize (serialized);

  Ptr<LinkCheckAns> command = (*(frameHdr.GetCommands ().begin ()))->GetObject<LinkCheckAns> ();

  /////////////////////////////////////////////////
  // Test a combination of the two above classes //
  /////////////////////////////////////////////////
  Ptr<Packet> pkt = Create<Packet> (10);
  pkt->AddHeader (frameHdr);
  pkt->AddHeader (macHdr);

  // Length = Payload + FrameHeader + MacHeader
  //        = 10 + (8+3) + 1 = 22
  LoraMacHeader macHdr1;
  pkt->RemoveHeader (macHdr1);

  LoraFrameHeader frameHdr1;
  frameHdr1.SetAsDownlink ();
  pkt->RemoveHeader (frameHdr1);

  // Verify contents of removed MAC header
  std::cout << unsigned( macHdr1.GetMType ()) << " " << unsigned( macHdr.GetMType ()) << std::endl;
  std::cout << unsigned( macHdr1.GetMajor ()) << " " << unsigned( macHdr.GetMajor ()) << std::endl;

  // Verify contents of removed frame header
  std::cout << unsigned( frameHdr1.GetAck ()) << " " << unsigned( frameHdr.GetAck ()) << std::endl;
  std::cout << unsigned( frameHdr1.GetAdr ()) << " " << unsigned( frameHdr.GetAdr ()) << std::endl;
  std::cout << unsigned( frameHdr1.GetFCnt ()) << " " << unsigned( frameHdr.GetFCnt ()) << std::endl;
  std::cout << (frameHdr1.GetAddress () == frameHdr.GetAddress ()) << std::endl;
  return 0;
}
