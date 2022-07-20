/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Orange SA
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
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#include "ns3/congestion-control-component.h"
#include "ns3/periodic-sender.h"
#include "ns3/traffic-control-utils.h"

#include <boost/math/special_functions/lambert_w.hpp>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CongestionControlComponent");

NS_OBJECT_ENSURE_REGISTERED (CongestionControlComponent);

void
CongestionControlComponent::datarate_t::Reset (void)
{
  received = 0;
  sent = 0;
}

TypeId
CongestionControlComponent::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::CongestionControlComponent")
          .SetGroupName ("lorawan")
          .AddConstructor<CongestionControlComponent> ()
          .SetParent<NetworkControllerComponent> ()
          .AddAttribute ("StartTime", "Time at which we start the congestion control algorithm",
                         TimeValue (Hours (10)),
                         MakeTimeAccessor (&CongestionControlComponent::m_start),
                         MakeTimeChecker ())
          .AddAttribute ("SamplingDuration",
                         "Time duration of the post-configuration PDR sampling period",
                         TimeValue (Hours (3)),
                         MakeTimeAccessor (&CongestionControlComponent::m_samplingDuration),
                         MakeTimeChecker ())
          .AddAttribute ("AcceptedPDRVariance", "Acceptable distance from target PDR value",
                         DoubleValue (0.01),
                         MakeDoubleAccessor (&CongestionControlComponent::m_epsilon),
                         MakeDoubleChecker<double> ())
          .AddAttribute (
              "ValueStagnationTolerance",
              "Minimum step between offered traffic values in a SF to declare value stagnation",
              DoubleValue (0.001), MakeDoubleAccessor (&CongestionControlComponent::m_tolerance),
              MakeDoubleChecker<double> ());
  return tid;
}

CongestionControlComponent::CongestionControlComponent () : m_targets ({0.95}), N_CH (1)
{
}

CongestionControlComponent::~CongestionControlComponent ()
{
}

void
CongestionControlComponent::SetTargets (targets_t targets)
{
  NS_LOG_FUNCTION (targets);

  m_targets = targets;
  N_CH = 1;
}

void
CongestionControlComponent::OnReceivedPacket (Ptr<const Packet> packet, Ptr<EndDeviceStatus> status,
                                              Ptr<NetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (packet << status << networkStatus);

  // Get headers/tags
  Ptr<Packet> packetCopy = packet->Copy ();
  LorawanMacHeader mhead;
  packetCopy->RemoveHeader (mhead);
  LoraFrameHeader fhead;
  fhead.SetAsUplink (); //<! Needed by Deserialize ()
  packetCopy->RemoveHeader (fhead);
  LoraTag tag;
  packetCopy->RemovePacketTag (tag);

  // Retrieve device metrics
  uint32_t devaddr = fhead.GetAddress ().Get ();
  devinfo_t &devinfo = m_devTracking[devaddr];

  // Update frame counter
  uint16_t currFCnt = fhead.GetFCnt ();
  NS_ASSERT_MSG (
      !(currFCnt < devinfo.fCnt),
      "Frame counter can't decrease, as re-connections to the network are not implemented.");
  int prevFCnt = devinfo.fCnt; // Save previous FCnt for PDR computations later
  devinfo.fCnt = currFCnt;

  // If packet is a duplicate, exit
  if (currFCnt == prevFCnt)
    return;

  // Do no start until start time
  if (Simulator::Now () < m_start)
    return;

  /**
   * CONGESTION CONTROL PROCEDURE
   * We oscillate between sampling fase and reconfiguration fase.
   */

  // If first time, init congestion data
  if (m_congestMetrics.empty ())
    InitializeData (networkStatus);

  // If we are in reconfiguration fase, look for acknowledgement and exit
  if (!m_configToDoList.empty ())
    {
      // Wait for ack policy
      for (const auto &command : fhead.GetCommands ())
        if (command->GetCommandType () == DUTY_CYCLE_ANS)
          {
            devinfo.dutycycle = m_configToDoList[devaddr];
            // idea: add a confirmed offered traffic field that
            // is updated with acks, then use it to update objective
            m_configToDoList.erase (devaddr);
            NS_LOG_INFO ((int) m_configToDoList.size () << " remaining");
          }
      // If it was a disabled device who did not receive duty-cycle command
      if (m_disabled.count (devaddr) and !m_configToDoList.count (devaddr))
        m_configToDoList[devaddr] = 255;
      // If config has finished, start sampling fase
      if (m_configToDoList.empty ())
        {
          NS_LOG_DEBUG ("Duty-cycle configuration terminated in "
                        << (Simulator::Now () - m_samplingStart).As (Time::H));
          // Reset congestion metrics
          for (auto &gw : m_congestMetrics)
            for (auto &cl : gw.second)
              for (auto &dr : cl)
                dr.Reset ();
          // Reset timer
          m_samplingStart = Simulator::Now ();
        }
      return;
    }

  // Else, we are in sampling fase. Add sample to congestion metrics
  datarate_t &group = m_congestMetrics[devinfo.bestGw][devinfo.cluster][devinfo.datarate];
  group.received++;
  group.sent += currFCnt - prevFCnt;

  // If sampling fase expired, produce reconfiguration
  if (Simulator::Now () > m_samplingStart + m_samplingDuration)
    {
      NS_LOG_DEBUG (PrintCongestion ());
      // Produce new reconfig scheme (one SF for each gateway/cluster)
      for (auto &gw : m_congestMetrics)
        for (size_t cl = 0; cl < m_targets.size (); ++cl)
          for (auto &dr : gw.second[cl])
            if (ProduceConfigScheme (dr, m_targets[cl]))
              {
                for (auto const &d : dr.devs)
                  if (m_configToDoList.count (
                          d.first)) // Check key existence (to avoid creating it)
                    if (m_configToDoList[d.first] == m_devTracking[d.first].dutycycle)
                      m_configToDoList.erase (d.first); // Nothing changed between config
                break;
              }

      // Cheat and re-enable devices if they need to receive different config
      for (auto const &ed : m_disabled)
        if (m_configToDoList.count (ed.first))
          {
            ed.second->GetMac ()->SetAggregatedDutyCycle (1.0);
            m_devTracking[ed.first].dutycycle = 0;
          }
      m_disabled.clear ();

      // This is in case there was nothig to reconfig
      // (otherwise rewritten at end of config)
      m_samplingStart = Simulator::Now ();
    }
}

void
CongestionControlComponent::BeforeSendingReply (Ptr<EndDeviceStatus> status,
                                                Ptr<NetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (status << networkStatus);

  // Here all versions of a message have been received,
  // still no insurance whether this method will be called on reception.

  // Get address
  uint32_t devaddr = status->m_endDeviceAddress.Get ();

  // Here we just set up the reply packet with dutycycle config.

  // Early returns
  if (!m_configToDoList.count (devaddr))
    return; // No re-config instruction
  uint8_t dc = m_configToDoList[devaddr];
  NS_ASSERT (dc == 0 or (7 <= dc and dc <= 15) or dc == 255);
  // Save devices which are disabled
  if (dc == 255)
    {
      if (!m_disabled.count (devaddr))
        m_disabled[devaddr] = status;
      m_configToDoList.erase (devaddr);
    }

  NS_LOG_INFO ("Sending DutyCycleReq (" << 1.0 / pow (2, dc) << " E), "
                                        << (int) m_configToDoList.size () << " remaining");

  /*   // No ack policy
  m_devTracking[devaddr].dutycycle = m_configToDoList[devaddr];
  m_configToDoList.erase (devaddr);
  // If config has finished, start sampling fase
  if (m_configToDoList.empty ())
    {
      NS_LOG_DEBUG ("Duty-cycle configuration terminated in "
                    << (Simulator::Now () - m_samplingStart).As (Time::H));
      // Reset congestion metrics
      for (auto &gw : m_congestMetrics)
        for (auto &cl : gw.second)
          for (auto &dr : cl)
            dr.Reset ();
      // Reset timer
      m_samplingStart = Simulator::Now ();
    } */

  status->m_reply.frameHeader.AddDutyCycleReq (dc);
  status->m_reply.frameHeader.SetAsDownlink ();
  status->m_reply.macHeader.SetMType (LorawanMacHeader::UNCONFIRMED_DATA_DOWN);
  status->m_reply.needsReply = true;
}

void
CongestionControlComponent::OnFailedReply (Ptr<EndDeviceStatus> status,
                                           Ptr<NetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (this->GetTypeId () << networkStatus);
}

void
CongestionControlComponent::InitializeData (Ptr<NetworkStatus> status)
{
  NS_LOG_FUNCTION (status);

  m_samplingStart = m_start;

  for (auto const &gw : (status->m_gatewayStatuses))
    m_congestMetrics[gw.first] = gateway_t (m_targets.size (), cluster_t (N_SF));

  // Initialize device data.
  // (It's ok to cheat because this is declared on device registration)
  for (auto const &ed : (status->m_endDeviceStatuses))
    {
      // Compute offered traffic
      devinfo_t &devinfo = m_devTracking[ed.first.Get ()];
      Ptr<Node> node = ed.second->GetMac ()->GetDevice ()->GetNode ();
      Ptr<PeriodicSender> app = node->GetApplication (0)->GetObject<PeriodicSender> ();

      Ptr<Packet> tmp =
          Create<Packet> (app->GetPacketSize () + 13 /* Headers with no MAC commands */);
      LoraTxParameters params;
      params.sf = 12 - devinfo.datarate;
      params.lowDataRateOptimizationEnabled =
          LoraPhy::GetTSym (params) > MilliSeconds (16) ? true : false;
      double toa = LoraPhy::GetOnAirTime (tmp, params).GetSeconds ();
      double traffic = toa / app->GetInterval ().GetSeconds ();
      devinfo.maxoftraf = (traffic > 0.01) ? 0.01 : traffic;

      devinfo.datarate = ed.second->GetMac ()->GetDataRate ();
      devinfo.cluster = ed.second->GetMac ()->GetCluster ();

      Ptr<MobilityModel> devpos = node->GetObject<MobilityModel> ();
      double distance = std::numeric_limits<double>::max ();
      for (auto const &gw : (status->m_gatewayStatuses))
        {
          double tmp = devpos->GetDistanceFrom (
              gw.second->GetGatewayMac ()->GetDevice ()->GetNode ()->GetObject<MobilityModel> ());
          if (tmp < distance)
            {
              distance = tmp;
              devinfo.bestGw = gw.first;
            }
        }

      // Insert data in structure according to best gateway and SF
      datarate_t &group = m_congestMetrics[devinfo.bestGw][devinfo.cluster][devinfo.datarate];
      group.devs.push_back ({ed.first.Get (), traffic});
      group.ot.max += traffic;
      group.ot.high += traffic;
      group.ot.curr += traffic;
    }
}

std::string
CongestionControlComponent::PrintCongestion ()
{
  std::stringstream ss;
  ss << std::endl;
  for (size_t cl = 0; cl < m_targets.size (); ++cl)
    {
      ss << std::endl << "Cluster " << cl << ": ";
      for (int dr = N_SF - 1; dr >= 0; --dr)
        {
          double sent = 0.0;
          double rec = 0.0;
          for (auto const &gw : m_congestMetrics)
            {
              sent += gw.second[cl][dr].sent;
              rec += gw.second[cl][dr].received;
            }
          ss << "SF" << 12 - dr << " " << ((sent > 0.0) ? rec / sent : -1.0) << ", ";
        }
    }
  return ss.str ();
}

bool
CongestionControlComponent::ProduceConfigScheme (datarate_t &group, double target)
{
  if (group.devs.empty ())
    return false;
  if (group.sent <= 0.0)
    ; // Do something else?

  double pdr = (group.sent > 0.0) ? (double) group.received / group.sent : 1.0;
  bool congested = (pdr < target);
  oftraffic_t &ot = group.ot;
  bool started = (ot.curr < ot.max);

  // Early returns:
  if (!congested and !started)
    return false; //! Nothing to do
  if (abs (target - pdr) < m_epsilon)
    return false; //! We are in acceptable range
  if ((ot.high - ot.low) / 2 < m_tolerance)
    return false; //! Capacity values are stagnating
  // We are missing a failsafe in case pdr cannot be increased further due to low coverage

  // Normal behaviour
  if (congested and started)
    ot.high = ot.curr;
  else if (!congested and started)
    ot.low = ot.curr;

  ot.curr = (ot.high + ot.low) / 2;

  if (false and !started) //! Check if we can jump start with capacity model.
    {
      double cap = CapacityForPDRModel (target) * N_CH;
      ot.curr = ((ot.high - cap) / 2.0 < m_tolerance) ? ot.curr : cap;
    }

  int sf = 12 - m_devTracking[group.devs[0].first].datarate;
  NS_LOG_DEBUG ("Reconfig SF" << sf << ": " << ot.curr << " [" << ot.low << ", " << ot.high << "]");
  TrafficControlUtils::OptimizeDutyCycleMaxMin (group.devs, ot.curr, m_configToDoList);

  return true;
}

double
CongestionControlComponent::CapacityForPDRModel (double pdr)
{
  double gt = -log (0.98); //!< dB, desired thermal gain for 0.98 PDR with rayleigh fading
  double gamma = std::pow (10.0, 6.0 / 10.0);
  double a = (gamma + 1) / (1 + gamma * (1 - exp (-gt + 1 / gamma)));
  return -0.5 * (a + boost::math::lambert_w0 (-(a / exp (a)) * exp (gt) * pdr));
}

const int CongestionControlComponent::N_SF = 6;

} // namespace lorawan
} // namespace ns3
