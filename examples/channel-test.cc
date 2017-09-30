#include "ns3/log.h"
#include "ns3/command-line.h"
#include "ns3/logical-lora-channel-helper.h"
#include <cstdlib>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ChannelTest");

int main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);

  LogComponentEnable ("ChannelTest", LOG_LEVEL_ALL);
  LogComponentEnable ("LogicalLoraChannel", LOG_LEVEL_ALL);
  LogComponentEnable ("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("SubBand", LOG_LEVEL_ALL);

  /////////////////////////////
  // Test LogicalLoraChannel //
  /////////////////////////////

  // Setup
  Ptr<LogicalLoraChannel> channel1 = CreateObject<LogicalLoraChannel> (868);
  Ptr<LogicalLoraChannel> channel2 = CreateObject<LogicalLoraChannel> (868);
  Ptr<LogicalLoraChannel> channel3 = CreateObject<LogicalLoraChannel> (868.1);
  Ptr<LogicalLoraChannel> channel4 = CreateObject<LogicalLoraChannel> (868.001);

  // Equality between channels
  // Test the == and != operators
  NS_ASSERT (channel1 == channel2);
  NS_ASSERT (channel1 != channel3);
  NS_ASSERT (channel1 != channel4);

  //////////////////
  // Test SubBand //
  //////////////////

  // Setup
  SubBand subBand (868, 868.7, 0.01, 14);
  Ptr<LogicalLoraChannel> channel5 = CreateObject<LogicalLoraChannel> (870);

  // Test BelongsToSubBand
  NS_ASSERT (subBand.BelongsToSubBand (channel3));
  NS_ASSERT (subBand.BelongsToSubBand (channel3->GetFrequency ()));
  NS_ASSERT (!subBand.BelongsToSubBand (channel5));

  ///////////////////////////////////
  // Test LogicalLoraChannelHelper //
  ///////////////////////////////////

  // Setup
  Ptr<LogicalLoraChannelHelper> channelHelper = CreateObject<LogicalLoraChannelHelper> ();
  SubBand subBand1 (869, 869.4, 0.1, 27);
  channel1 = CreateObject<LogicalLoraChannel> (868.1);
  channel2 = CreateObject<LogicalLoraChannel> (868.3);
  channel3 = CreateObject<LogicalLoraChannel> (868.5);
  channel4 = CreateObject<LogicalLoraChannel> (869.1);
  channel5 = CreateObject<LogicalLoraChannel> (869.3);

  // Channel diagram
  //
  // Channels      1      2      3                     4       5
  // SubBands  868 ----- 0.1% ----- 868.7       869 ----- 1% ----- 869.4

  // Add SubBands and LogicalLoraChannels to the helper
  channelHelper->AddSubBand (&subBand);
  channelHelper->AddSubBand (&subBand1);
  channelHelper->AddChannel (channel1);
  channelHelper->AddChannel (channel2);
  channelHelper->AddChannel (channel3);
  channelHelper->AddChannel (channel4);
  channelHelper->AddChannel (channel5);

  // Duty Cycle tests
  // (high level duty cycle behavior)
  ///////////////////////////////////

  // Waiting time is computed correctly
  channelHelper->AddEvent (Seconds (2), channel1);
  Time expectedTimeOff = Seconds (2/0.01 - 2);
  NS_ASSERT (channelHelper->GetWaitingTime (channel1) == expectedTimeOff);

  // Duty Cycle involves the whole SubBand, not just a channel
  NS_ASSERT (channelHelper->GetWaitingTime (channel2) == expectedTimeOff);
  NS_ASSERT (channelHelper->GetWaitingTime (channel3) == expectedTimeOff);

  // Other bands are not affected by this transmission
  NS_ASSERT (channelHelper->GetWaitingTime (channel4) == 0);
  NS_ASSERT (channelHelper->GetWaitingTime (channel5) == 0);

  return 0;
}
