/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 * 
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "ns3/logical-lora-channel-helper.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("LogicalLoraChannelHelper");

NS_OBJECT_ENSURE_REGISTERED (LogicalLoraChannelHelper);

TypeId
LogicalLoraChannelHelper::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::LogicalLoraChannelHelper").SetParent<Object> ().SetGroupName ("lorawan");
  return tid;
}

LogicalLoraChannelHelper::LogicalLoraChannelHelper () : m_lastTxDuration (0), m_lastTxStart (0)
{
  NS_LOG_FUNCTION (this);
}

LogicalLoraChannelHelper::~LogicalLoraChannelHelper ()
{
  NS_LOG_FUNCTION (this);
}

std::vector<Ptr<LogicalLoraChannel>>
LogicalLoraChannelHelper::GetChannelList (void)
{
  NS_LOG_FUNCTION (this);

  // Make a copy of the channel vector
  std::vector<Ptr<LogicalLoraChannel>> vector;
  vector.reserve (m_channelList.size ());
  std::copy (m_channelList.begin (), m_channelList.end (), std::back_inserter (vector));

  return vector;
}

std::vector<Ptr<LogicalLoraChannel>>
LogicalLoraChannelHelper::GetEnabledChannelList (void)
{
  NS_LOG_FUNCTION (this);

  // Make a copy of the channel vector
  std::vector<Ptr<LogicalLoraChannel>> vector;
  vector.reserve (m_channelList.size ());
  std::copy (m_channelList.begin (), m_channelList.end (),
             std::back_inserter (vector)); // Working on a copy

  std::vector<Ptr<LogicalLoraChannel>> channels;
  std::vector<Ptr<LogicalLoraChannel>>::iterator it;
  for (it = vector.begin (); it != vector.end (); it++)
    {
      if ((*it)->IsEnabledForUplink ())
        {
          channels.push_back (*it);
        }
    }

  return channels;
}

Ptr<SubBand>
LogicalLoraChannelHelper::GetSubBandFromChannel (Ptr<LogicalLoraChannel> channel)
{
  return GetSubBandFromFrequency (channel->GetFrequency ());
}

Ptr<SubBand>
LogicalLoraChannelHelper::GetSubBandFromFrequency (double frequency)
{
  // Get the SubBand this frequency belongs to
  std::list<Ptr<SubBand>>::iterator it;
  for (it = m_subBandList.begin (); it != m_subBandList.end (); it++)
    {
      if ((*it)->BelongsToSubBand (frequency))
        {
          return *it;
        }
    }

  NS_LOG_ERROR ("Requested frequency: " << frequency);
  NS_ABORT_MSG ("Warning: frequency is outside any known SubBand.");

  return 0; // If no SubBand is found, return 0
}

void
LogicalLoraChannelHelper::AddChannel (double frequency)
{
  NS_LOG_FUNCTION (this << frequency);

  // Create the new channel and increment the counter
  Ptr<LogicalLoraChannel> channel = Create<LogicalLoraChannel> (frequency);

  // Add it to the list
  m_channelList.push_back (channel);

  NS_LOG_DEBUG ("Added a channel. Current number of channels in list is " << m_channelList.size ());
}

void
LogicalLoraChannelHelper::AddChannel (Ptr<LogicalLoraChannel> logicalChannel)
{
  NS_LOG_FUNCTION (this << logicalChannel);

  // Add it to the list
  m_channelList.push_back (logicalChannel);
}

void
LogicalLoraChannelHelper::SetChannel (uint8_t chIndex, Ptr<LogicalLoraChannel> logicalChannel)

{
  NS_LOG_FUNCTION (this << chIndex << logicalChannel);

  m_channelList.push_back (logicalChannel);
}

void
LogicalLoraChannelHelper::AddSubBand (double firstFrequency, double lastFrequency, double dutyCycle,
                                      double maxTxPowerDbm)
{
  NS_LOG_FUNCTION (this << firstFrequency << lastFrequency);

  Ptr<SubBand> subBand = Create<SubBand> (firstFrequency, lastFrequency, dutyCycle, maxTxPowerDbm);

  m_subBandList.push_back (subBand);
}

void
LogicalLoraChannelHelper::AddSubBand (Ptr<SubBand> subBand)
{
  NS_LOG_FUNCTION (this << subBand);

  m_subBandList.push_back (subBand);
}

void
LogicalLoraChannelHelper::RemoveChannel (Ptr<LogicalLoraChannel> logicalChannel)
{
  // Search and remove the channel from the list
  std::vector<Ptr<LogicalLoraChannel>>::iterator it;
  for (it = m_channelList.begin (); it != m_channelList.end (); it++)
    {
      Ptr<LogicalLoraChannel> currentChannel = *it;
      if (currentChannel == logicalChannel)
        {
          m_channelList.erase (it);
          return;
        }
    }
}

Time
LogicalLoraChannelHelper::GetAggregatedWaitingTime (double aggregatedDutyCycle)
{
  NS_LOG_FUNCTION ("Aggregated duty-cycle: " + std::to_string (aggregatedDutyCycle));

  // Aggregate waiting time
  Time nextTransmissionTime =
      (aggregatedDutyCycle) ? m_lastTxStart + m_lastTxDuration / aggregatedDutyCycle : Time::Max ();
  Time aggregatedWaitingTime = nextTransmissionTime - Simulator::Now ();

  // Handle case in which waiting time is negative
  aggregatedWaitingTime = Max (aggregatedWaitingTime, Seconds (0));

  NS_LOG_DEBUG ("Aggregated waiting time: " << aggregatedWaitingTime.As (Time::S));

  return aggregatedWaitingTime;
}

Time
LogicalLoraChannelHelper::GetWaitingTime (Ptr<LogicalLoraChannel> channel)
{
  NS_LOG_FUNCTION (this << channel);

  // SubBand waiting time
  Time subBandWaitingTime =
      GetSubBandFromChannel (channel)->GetNextTransmissionTime () - Simulator::Now ();

  // Handle case in which waiting time is negative
  subBandWaitingTime = Seconds (std::max (subBandWaitingTime.GetSeconds (), double (0)));

  NS_LOG_DEBUG ("Waiting time: " << subBandWaitingTime.GetSeconds ());

  return subBandWaitingTime;
}

void
LogicalLoraChannelHelper::AddEvent (Time duration, Ptr<LogicalLoraChannel> channel)
{
  NS_LOG_FUNCTION (this << duration << channel);

  Ptr<SubBand> subBand = GetSubBandFromChannel (channel);

  double dutyCycle = subBand->GetDutyCycle ();
  m_lastTxDuration = duration;
  m_lastTxStart = Simulator::Now ();

  // Computation of necessary waiting time on this sub-band
  subBand->SetNextTransmissionTime (Simulator::Now () + m_lastTxDuration / dutyCycle);

  NS_LOG_DEBUG ("Time on air: " << m_lastTxDuration.As (Time::MS));
  NS_LOG_DEBUG ("Current time: " << Simulator::Now ().As (Time::S));
  NS_LOG_DEBUG ("Next transmission on this sub-band allowed at time: "
                << (subBand->GetNextTransmissionTime ()).As (Time::S));
}

double
LogicalLoraChannelHelper::GetTxPowerForChannel (Ptr<LogicalLoraChannel> logicalChannel)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Get the maxTxPowerDbm from the SubBand this channel is in
  std::list<Ptr<SubBand>>::iterator it;
  for (it = m_subBandList.begin (); it != m_subBandList.end (); it++)
    {
      // Check whether this channel is in this SubBand
      if ((*it)->BelongsToSubBand (logicalChannel->GetFrequency ()))
        {
          return (*it)->GetMaxTxPowerDbm ();
        }
    }
  NS_ABORT_MSG ("Logical channel doesn't belong to a known SubBand");

  return 0;
}

void
LogicalLoraChannelHelper::DisableChannel (int index)
{
  NS_LOG_FUNCTION (this << index);

  m_channelList.at (index)->DisableForUplink ();
}
} // namespace lorawan
} // namespace ns3
