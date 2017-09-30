#include "ns3/lora-phy.h"
#include "ns3/log.h"
#include "ns3/command-line.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TimeOnAirTest");

int main (int argc, char *argv[])
{

  CommandLine cmd;
  cmd.Parse (argc, argv);

  LogComponentEnable ("TimeOnAirTest", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraPhy", LOG_LEVEL_ALL);

  Ptr<Packet> packet;
  Time duration;

  // Available parameters:
  // PayloadSize, SF, HeaderDisabled, CodingRate, Bandwidth, nPreambleSyms, crcEnabled, lowDROptimization

  // Starting parameters
  packet = Create<Packet> (10);
  LoraTxParameters txParams;
  txParams.sf = 7;
  txParams.headerDisabled = false;
  txParams.codingRate = 1;
  txParams.bandwidthHz = 125000;
  txParams.nPreamble = 8;
  txParams.crcEnabled = 1;
  txParams.lowDataRateOptimizationEnabled = 0;

  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_LOG_DEBUG ("Computed: " << duration.GetSeconds () << " s, expected: " << 0.041216 << " s");

  txParams.sf = 8;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_LOG_DEBUG ("Computed: " << duration.GetSeconds () << " s, expected: " << 0.072192 << " s");

  txParams.headerDisabled = true;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_LOG_DEBUG ("Computed: " << duration.GetSeconds () << " s, expected: " << 0.072192 << " s");

  txParams.codingRate = 2;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_LOG_DEBUG ("Computed: " << duration.GetSeconds () << " s, expected: " << 0.078336 << " s");

  txParams.nPreamble = 10;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_LOG_DEBUG ("Computed: " << duration.GetSeconds () << " s, expected: " << 0.082432 << " s");

  txParams.lowDataRateOptimizationEnabled = true;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_LOG_DEBUG ("Computed: " << duration.GetSeconds () << " s, expected: " << 0.082432 << " s");

  txParams.sf = 10;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_LOG_DEBUG ("Computed: " << duration.GetSeconds () << " s, expected: " << 0.280576 << " s");

  txParams.bandwidthHz = 250000;
  duration = LoraPhy::GetOnAirTime (packet, txParams);
  NS_LOG_DEBUG ("Computed: " << duration.GetSeconds () << " s, expected: " << 0.14028 << " s");
  return 0;
}
