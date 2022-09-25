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
CongestionControlComponent::dataratestatus_t::Reset (void)
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

CongestionControlComponent::CongestionControlComponent ()
    : m_targets ({0.95}), N_CH (1), m_beta (16)
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
  devinfo_t &devinfo = m_devStatus[devaddr];

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
  if (m_congestionStatus.empty ())
    InitializeData (networkStatus);
  // If first transmission of the device (late activation)
  if (prevFCnt == 0)
    {
      dataratestatus_t &group =
          m_congestionStatus[devinfo.bestGw][devinfo.cluster][devinfo.datarate];
      group.devs.push_back ({devaddr, devinfo.maxoftraf});
      if (!group.ot.started) // Reconfiguration not started yet
        group.ot.currbest += devinfo.maxoftraf;
      else
        group.ot.changed = true;
    }

  // If we are in reconfiguration fase, look for acknowledgement and exit
  if (!m_configToDoList[devinfo.bestGw][devinfo.cluster].empty ())
    {
      /*       // Wait for ack policy
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
          for (auto &gw : m_congestionStatus)
            for (auto &cl : gw.second)
              for (auto &dr : cl)
                dr.Reset ();
          // Reset timer
          m_samplingStart = Simulator::Now ();
        } */
      return;
    }

  // Else, we are in sampling fase. Add sample to congestion metrics
  {
    dataratestatus_t &group = m_congestionStatus[devinfo.bestGw][devinfo.cluster][devinfo.datarate];
    group.received++;
    group.sent += currFCnt - prevFCnt;
  }
  // If sampling fase expired, produce reconfiguration
  if (Simulator::Now () > m_samplingStart[devinfo.bestGw][devinfo.cluster] + m_samplingDuration)
    {
      NS_LOG_DEBUG (PrintCongestion (devinfo.bestGw, devinfo.cluster));
      configs_t &configs = m_configToDoList[devinfo.bestGw][devinfo.cluster];
      disabled_t &disabled = m_disabled[devinfo.bestGw][devinfo.cluster];
      // Produce new reconfig scheme
      for (auto &dr : m_congestionStatus[devinfo.bestGw][devinfo.cluster])
        if (ProduceConfigScheme (dr, m_targets[devinfo.cluster]))
          {
            for (auto const &d : dr.devs)
              if (configs.count (d.first)) // Check key existence (to avoid creating it)
                {
                  if (configs[d.first] ==
                      m_devStatus[d.first].dutycycle) // Nothing changed between configs
                    {
                      configs.erase (d.first);
                      continue;
                    }
                  if (!disabled.count (d.first)) // Doesn't need re-enabling
                    continue;
                  disabled[d.first]->GetMac ()->SetAggregatedDutyCycle (1.0); // CHEAT and re-enable
                  m_devStatus[d.first].dutycycle = 0;
                  disabled.erase (d.first);
                }
            //break; //(uncomment to enforce one SF at a time per gateway/cluster)
          }

      // This is in case there was nothig to reconfig
      // (otherwise always rewritten at end of config)
      m_samplingStart[devinfo.bestGw][devinfo.cluster] = Simulator::Now ();
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
  if (m_congestionStatus.empty ())
    return;
  devinfo_t &devinfo = m_devStatus[devaddr];
  if (!m_configToDoList[devinfo.bestGw][devinfo.cluster].count (devaddr))
    return; // No re-config instruction
  uint8_t dc = m_configToDoList[devinfo.bestGw][devinfo.cluster][devaddr];
  NS_ASSERT (dc == 0 or (7 <= dc and dc <= 15) or dc == 255);
  /*   // Save devices which are disabled, ack policy
  if (dc == 255)
    {
      if (!m_disabled.count (devaddr))
        m_disabled[devaddr] = status;
      m_configToDoList.erase (devaddr);
    } */

  NS_LOG_INFO ("Sending DutyCycleReq (" << 1.0 / pow (2, dc) << " E), "
                                        << (int) m_configToDoList.size () << " remaining");

  // No ack policy
  m_devStatus[devaddr].dutycycle = dc;
  m_configToDoList[devinfo.bestGw][devinfo.cluster].erase (devaddr);
  if (dc == 255 and !m_disabled[devinfo.bestGw][devinfo.cluster].count (devaddr))
    m_disabled[devinfo.bestGw][devinfo.cluster][devaddr] = status; // Add to disabled

  // If config has finished, start sampling fase
  if (m_configToDoList[devinfo.bestGw][devinfo.cluster].empty ())
    {
      NS_LOG_DEBUG (
          "Duty-cycle configuration terminated in "
          << (Simulator::Now () - m_samplingStart[devinfo.bestGw][devinfo.cluster]).As (Time::H));
      // Reset congestion metrics
      for (auto &dr : m_congestionStatus[devinfo.bestGw][devinfo.cluster])
        dr.Reset ();
      // Reset timer
      m_samplingStart[devinfo.bestGw][devinfo.cluster] = Simulator::Now ();
    }

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

  for (auto const &gw : (status->m_gatewayStatuses))
    {
      m_disabled[gw.first] = std::vector<disabled_t> (m_targets.size ());
      m_configToDoList[gw.first] = std::vector<configs_t> (m_targets.size ());
      m_samplingStart[gw.first] = std::vector<Time> (m_targets.size (), m_start);
      m_congestionStatus[gw.first] = gatewaystatus_t (m_targets.size (), clusterstatus_t (N_SF));
    }
  // Initialize device data.
  // (It's ok to cheat because this is declared on device registration)
  for (auto const &ed : (status->m_endDeviceStatuses))
    {
      // Compute offered traffic
      devinfo_t &devinfo = m_devStatus[ed.first.Get ()];
      Ptr<Node> node = ed.second->GetMac ()->GetDevice ()->GetNode ();
      Ptr<PeriodicSender> app = node->GetApplication (0)->GetObject<PeriodicSender> ();

      devinfo.datarate = ed.second->GetMac ()->GetDataRate ();
      devinfo.cluster = ed.second->GetMac ()->GetCluster ();

      Ptr<Packet> tmp =
          Create<Packet> (app->GetPacketSize () + 13 /* Headers with no MAC commands */);
      LoraTxParameters params;
      params.sf = 12 - devinfo.datarate;
      params.lowDataRateOptimizationEnabled =
          LoraPhy::GetTSym (params) > MilliSeconds (16) ? true : false;
      double toa = LoraPhy::GetOnAirTime (tmp, params).GetSeconds ();
      double traffic = toa / app->GetInterval ().GetSeconds ();
      devinfo.maxoftraf = (traffic > 0.01) ? 0.01 : traffic;

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
      if (devinfo.fCnt == 0)
        continue; // Do not insert and add it on first reception
      dataratestatus_t &group =
          m_congestionStatus[devinfo.bestGw][devinfo.cluster][devinfo.datarate];
      group.devs.push_back ({ed.first.Get (), traffic});
      group.ot.high += traffic;
      group.ot.currbest += traffic;
    }
}

std::string
CongestionControlComponent::PrintCongestion (Address bestGw, uint8_t cluster)
{
  std::stringstream ss;
  ss << "Cluster " << unsigned (cluster) << ", Gateway " << bestGw << ":\n\t";
  clusterstatus_t &cl = m_congestionStatus[bestGw][cluster];
  double totsent = 0;
  double totrec = 0;
  for (int dr = N_SF - 1; dr >= 0; --dr)
    {
      double sent = cl[dr].sent;
      totsent += sent;
      double rec = cl[dr].received;
      totrec += rec;
      ss << "SF" << 12 - dr << " " << ((sent > 0.0) ? rec / sent : -1.0) << ", ";
    }
  ss << "All " << ((totsent > 0.0) ? totrec / totsent : -1.0);
  return ss.str ();
}

bool
CongestionControlComponent::ProduceConfigScheme (dataratestatus_t &group, double target)
{
  if (group.devs.empty ())
    return false;

  offtraff_t &ot = group.ot;
  double pdr = (group.sent > 0.0) ? (double) group.received / group.sent : 1.0;
  bool congested = (pdr < target);

  // Early returns:
  if (!ot.started and !congested) //! Nothing to do
    return false;
  if (abs (target - pdr) < m_epsilon) //! We are in acceptable range
    return false;
  if ((ot.high - ot.low) / 2 < m_tolerance and !ot.changed) //! Capacity values are stagnating
    return false;
  // We are missing a failsafe in case pdr cannot be increased further due to low coverage

  // Bisection algorithm on offered traffic
  if (!ot.changed)
    {
      if (!ot.started) //! First iteration (check if we can jump start with capacity model)
        {
          ot.currbest = (ot.high + ot.low) / 2;
          double cap = CapacityForPDRModel (target) * N_CH * m_beta;
          ot.currbest = ((ot.high - cap) / 2.0 < m_tolerance) ? ot.currbest : cap;
          ot.started = true;
        }
      else //! Normal behaviour during convergence
        {
          if (congested)
            ot.high = ot.currbest;
          else
            ot.low = ot.currbest;
          ot.currbest = (ot.high + ot.low) / 2;
        }
    }

  // Produce reconfiguration scheme
  devinfo_t &context = m_devStatus[group.devs[0].first];
  int sf = 12 - context.datarate;
  NS_LOG_DEBUG ("Reconfig SF" << sf << ": " << ot.currbest << " [" << ot.low << ", " << ot.high
                              << "], New devices? " << ot.changed);
  TrafficControlUtils::OptimizeDutyCycleMaxMin (group.devs, ot.currbest,
                                                m_configToDoList[context.bestGw][context.cluster]);

  ot.changed = false;
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
