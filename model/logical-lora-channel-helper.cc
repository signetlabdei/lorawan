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
  static TypeId tid = TypeId ("ns3::LogicalLoraChannelHelper")
    .SetParent<Object> ()
    .SetGroupName ("lorawan");
  return tid;
}

LogicalLoraChannelHelper::LogicalLoraChannelHelper () :
  m_nextAggregatedTransmissionTime (Seconds (0)),
  m_aggregatedDutyCycle (1)
{
  NS_LOG_FUNCTION (this);
}

LogicalLoraChannelHelper::~LogicalLoraChannelHelper ()
{
  NS_LOG_FUNCTION (this);
}

std::vector<Ptr <LogicalLoraChannel> >
LogicalLoraChannelHelper::GetChannelList (void)
{
  NS_LOG_FUNCTION (this);

  // Make a copy of the channel vector
  std::vector<Ptr<LogicalLoraChannel> > vector;
  vector.reserve (m_channelList.size ());
  std::copy (m_channelList.begin (), m_channelList.end (), std::back_inserter
               (vector));

  return vector;
}


std::vector<Ptr <LogicalLoraChannel> >
LogicalLoraChannelHelper::GetEnabledChannelList (void)
{
  NS_LOG_FUNCTION (this);

  // Make a copy of the channel vector
  std::vector<Ptr<LogicalLoraChannel> > vector;
  vector.reserve (m_channelList.size ());
  std::copy (m_channelList.begin (), m_channelList.end (), std::back_inserter
               (vector));     // Working on a copy

  std::vector<Ptr <LogicalLoraChannel> > channels;
  std::vector<Ptr <LogicalLoraChannel> >::iterator it;
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
LogicalLoraChannelHelper::GetSubBandFromChannel (Ptr<LogicalLoraChannel>
                                                 channel)
{
  return GetSubBandFromFrequency (channel->GetFrequency ());
}

Ptr<SubBand>
LogicalLoraChannelHelper::GetSubBandFromFrequency (double frequency)
{
  // Get the SubBand this frequency belongs to
  std::list< Ptr< SubBand > >::iterator it;
  for (it = m_subBandList.begin (); it != m_subBandList.end (); it++)
    {
      if ((*it)->BelongsToSubBand (frequency))
        {
          return *it;
        }
    }

  NS_LOG_ERROR ("Requested frequency: " << frequency);
  NS_ABORT_MSG ("Warning: frequency is outside any known SubBand.");

  return 0;     // If no SubBand is found, return 0
}

void
LogicalLoraChannelHelper::AddChannel (double frequency)
{
  NS_LOG_FUNCTION (this << frequency);

  // Create the new channel and increment the counter
  Ptr<LogicalLoraChannel> channel = Create<LogicalLoraChannel> (frequency);

  // Add it to the list
  m_channelList.push_back (channel);

  NS_LOG_DEBUG ("Added a channel. Current number of channels in list is " <<
                m_channelList.size ());
}

void
LogicalLoraChannelHelper::AddChannel (Ptr<LogicalLoraChannel> logicalChannel)
{
  NS_LOG_FUNCTION (this << logicalChannel);

  // Add it to the list
  m_channelList.push_back (logicalChannel);
}

void
LogicalLoraChannelHelper::SetChannel (uint8_t chIndex,
                                      Ptr<LogicalLoraChannel> logicalChannel)

{
  NS_LOG_FUNCTION (this << chIndex << logicalChannel);

  m_channelList.at (chIndex) = logicalChannel;
}

void
LogicalLoraChannelHelper::AddSubBand (double firstFrequency,
                                      double lastFrequency, double dutyCycle,
                                      double maxTxPowerDbm)
{
  NS_LOG_FUNCTION (this << firstFrequency << lastFrequency);

  Ptr<SubBand> subBand = Create<SubBand> (firstFrequency, lastFrequency,
                                          dutyCycle, maxTxPowerDbm);

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
  std::vector<Ptr<LogicalLoraChannel> >::iterator it;
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
LogicalLoraChannelHelper::GetAggregatedWaitingTime (void)
{
  // Aggregate waiting time
  Time aggregatedWaitingTime = m_nextAggregatedTransmissionTime - Simulator::Now ();

  // Handle case in which waiting time is negative
  aggregatedWaitingTime = Seconds (std::max (aggregatedWaitingTime.GetSeconds (),
                                             double(0)));

  NS_LOG_DEBUG ("Aggregated waiting time: " << aggregatedWaitingTime.GetSeconds ());

  return aggregatedWaitingTime;
}

Time
LogicalLoraChannelHelper::GetWaitingTime (Ptr<LogicalLoraChannel> channel)
{
  NS_LOG_FUNCTION (this << channel);

  // SubBand waiting time
  Time subBandWaitingTime = GetSubBandFromChannel (channel)->
    GetNextTransmissionTime () -
    Simulator::Now ();

  // Handle case in which waiting time is negative
  subBandWaitingTime = Seconds (std::max (subBandWaitingTime.GetSeconds (),
                                          double(0)));

  NS_LOG_DEBUG ("Waiting time: " << subBandWaitingTime.GetSeconds ());

  return subBandWaitingTime;
}

void
LogicalLoraChannelHelper::AddEvent (Time duration,
                                    Ptr<LogicalLoraChannel> channel)
{
  NS_LOG_FUNCTION (this << duration << channel);

  Ptr<SubBand> subBand = GetSubBandFromChannel (channel);

  double dutyCycle = subBand->GetDutyCycle ();
  double timeOnAir = duration.GetSeconds ();

  // Computation of necessary waiting time on this sub-band
  subBand->SetNextTransmissionTime (Simulator::Now () + Seconds
                                      (timeOnAir / dutyCycle - timeOnAir));

  // Computation of necessary aggregate waiting time
  m_nextAggregatedTransmissionTime = Simulator::Now () + Seconds
      (timeOnAir / m_aggregatedDutyCycle - timeOnAir);

  NS_LOG_DEBUG ("Time on air: " << timeOnAir);
  NS_LOG_DEBUG ("m_aggregatedDutyCycle: " << m_aggregatedDutyCycle);
  NS_LOG_DEBUG ("Current time: " << Simulator::Now ().GetSeconds ());
  NS_LOG_DEBUG ("Next transmission on this sub-band allowed at time: " <<
                (subBand->GetNextTransmissionTime ()).GetSeconds ());
  NS_LOG_DEBUG ("Next aggregated transmission allowed at time " <<
                m_nextAggregatedTransmissionTime.GetSeconds ());
}

double
LogicalLoraChannelHelper::GetTxPowerForChannel (Ptr<LogicalLoraChannel>
                                                logicalChannel)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Get the maxTxPowerDbm from the SubBand this channel is in
  std::list< Ptr< SubBand > >::iterator it;
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
}
}
