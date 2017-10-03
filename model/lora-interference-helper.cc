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

#include "ns3/lora-interference-helper.h"
#include "ns3/log.h"
#include <limits>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraInterferenceHelper");

/***************************************
 *    LoraInterferenceHelper::Event    *
 ***************************************/

// Event Constructor
LoraInterferenceHelper::Event::Event (Time duration, double rxPowerdBm,
                                      uint8_t spreadingFactor,
                                      Ptr<Packet> packet, double frequencyMHz) :
  m_startTime (Simulator::Now ()),
  m_endTime (m_startTime + duration),
  m_sf (spreadingFactor),
  m_rxPowerdBm (rxPowerdBm),
  m_packet (packet),
  m_frequencyMHz (frequencyMHz)
{
  // NS_LOG_FUNCTION_NOARGS ();
}

// Event Destructor
LoraInterferenceHelper::Event::~Event ()
{
  // NS_LOG_FUNCTION_NOARGS ();
}

// Getters
Time
LoraInterferenceHelper::Event::GetStartTime (void) const
{
  return m_startTime;
}

Time
LoraInterferenceHelper::Event::GetEndTime (void) const
{
  return m_endTime;
}

Time
LoraInterferenceHelper::Event::GetDuration (void) const
{
  return m_endTime - m_startTime;
}

double
LoraInterferenceHelper::Event::GetRxPowerdBm (void) const
{
  return m_rxPowerdBm;
}

uint8_t
LoraInterferenceHelper::Event::GetSpreadingFactor (void) const
{
  return m_sf;
}

Ptr<Packet>
LoraInterferenceHelper::Event::GetPacket (void) const
{
  return m_packet;
}

double
LoraInterferenceHelper::Event::GetFrequency (void) const
{
  return m_frequencyMHz;
}

void
LoraInterferenceHelper::Event::Print (std::ostream &stream) const
{
  stream << "(" << m_startTime.GetSeconds () << " s - " <<
  m_endTime.GetSeconds () << " s), SF" << unsigned(m_sf) << ", " <<
  m_rxPowerdBm << " dBm, "<< m_frequencyMHz << " MHz";
}

std::ostream &operator << (std::ostream &os, const LoraInterferenceHelper::Event &event)
{
  event.Print (os);

  return os;
}

/****************************
 *  LoraInterferenceHelper  *
 ****************************/

NS_OBJECT_ENSURE_REGISTERED (LoraInterferenceHelper);

TypeId
LoraInterferenceHelper::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraInterferenceHelper")
    .SetParent<Object> ()
    .SetGroupName ("lorawan");

  return tid;
}

LoraInterferenceHelper::LoraInterferenceHelper ()
{
  NS_LOG_FUNCTION (this);
}

LoraInterferenceHelper::~LoraInterferenceHelper ()
{
  NS_LOG_FUNCTION (this);
}

// This collision matrix can be used for comparisons with the performance of Aloha
// systems, where collisions imply the loss of both packets.
// double inf = std::numeric_limits<double>::max();
// const double LoraInterferenceHelper::collisionSnir[6][6] =
// {
// //   7   8   9  10  11  12
// {inf, inf, inf, inf, inf, inf},  // SF7
// {inf, inf, inf, inf, inf, inf},  // SF8
// {inf, inf, inf, inf, inf, inf},  // SF9
// {inf, inf, inf, inf, inf, inf},  // SF10
// {inf, inf, inf, inf, inf, inf},  // SF11
// {inf, inf, inf, inf, inf, inf}   // SF12
// };

// LoRa Collision Matrix (Goursaud)
// Values are inverted w.r.t. the paper since here we interpret this as an
// _isolation_ matrix instead of a cochannel _rejection_ matrix like in
// Goursaud's paper.
const double LoraInterferenceHelper::collisionSnir[6][6] =
{
// SF7  SF8  SF9  SF10 SF11 SF12
  {  6, -16, -18, -19, -19, -20},  // SF7
  {-24,   6, -20, -22, -22, -22},  // SF8
  {-27, -27,   6, -23, -25, -25},  // SF9
  {-30, -30, -30,   6, -26, -28},  // SF10
  {-33, -33, -33, -33,   6, -29},  // SF11
  {-36, -36, -36, -36, -36,   6}   // SF12
};

Time LoraInterferenceHelper::oldEventThreshold = Seconds (2);

Ptr<LoraInterferenceHelper::Event>
LoraInterferenceHelper::Add (Time duration, double rxPower,
                             uint8_t spreadingFactor, Ptr<Packet> packet,
                             double frequencyMHz)
{

  NS_LOG_FUNCTION (this << duration.GetSeconds () << rxPower << unsigned
                   (spreadingFactor) << packet << frequencyMHz);

  // Create an event based on the parameters
  Ptr<LoraInterferenceHelper::Event> event =
    Create<LoraInterferenceHelper::Event> (duration, rxPower, spreadingFactor,
                                           packet, frequencyMHz);

  // Add the event to the list
  m_events.push_back (event);

  // Clean the event list
  if (m_events.size () > 100)
    {
      CleanOldEvents ();
    }

  return event;
}

void
LoraInterferenceHelper::CleanOldEvents (void)
{
  NS_LOG_FUNCTION (this);

  // Cycle the events, and clean up if an event is old.
  for (auto it = m_events.begin (); it != m_events.end ();)
    {
      if ((*it)->GetEndTime () + oldEventThreshold < Simulator::Now ())
        {
          it = m_events.erase (it);
        }
      it++;
    }
}

std::list<Ptr<LoraInterferenceHelper::Event> >
LoraInterferenceHelper::GetInterferers ()
{
  return m_events;
}

void
LoraInterferenceHelper::PrintEvents (std::ostream &stream)
{
  NS_LOG_FUNCTION_NOARGS ();

  stream << "Currently registered events:" << std::endl;

  for (auto it = m_events.begin (); it != m_events.end (); it++)
    {
      (*it)->Print (stream);
      stream << std::endl;
    }
}

uint8_t
LoraInterferenceHelper::IsDestroyedByInterference
  (Ptr<LoraInterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (this << event);

  NS_LOG_INFO ("Current number of events in LoraInterferenceHelper: " << m_events.size ());

  // We want to see the interference affecting this event: cycle through events
  // that overlap with this one and see whether it survives the interference or
  // not.

  // Gather information about the event
  double rxPowerDbm = event->GetRxPowerdBm ();
  uint8_t sf = event->GetSpreadingFactor ();
  double frequency = event->GetFrequency ();

  // Handy information about the time frame when the packet was received
  Time now = Simulator::Now ();
  Time duration = event->GetDuration ();
  Time packetStartTime = now - duration;
  Time packetEndTime = now;

  // Get the list of interfering events
  std::list<Ptr<LoraInterferenceHelper::Event> >::iterator it;

  // Energy for interferers of various SFs
  std::vector<double> cumulativeInterferenceEnergy (6,0);

  // Cycle over the events
  for (it = m_events.begin (); it != m_events.end ();)
    {
      // Pointer to the current interferer
      Ptr< LoraInterferenceHelper::Event > interferer = *it;

      // Only consider the current event if the channel is the same: we
      // assume there's no interchannel interference. Also skip the current
      // event if it's the same that we want to analyze.
      if (!(interferer->GetFrequency () == frequency) || interferer == event)
        {
          NS_LOG_DEBUG ("Different channel");
          it++;
          continue;   // Continues from the first line inside the for cycle
        }

      NS_LOG_DEBUG ("Interferer on same channel");

      // Gather information about this interferer
      uint8_t interfererSf = interferer->GetSpreadingFactor ();
      double interfererPower = interferer->GetRxPowerdBm ();
      Time interfererStartTime = interferer->GetStartTime ();
      Time interfererEndTime = interferer->GetEndTime ();

      NS_LOG_INFO ("Found an interferer: sf = " << unsigned(interfererSf)
                                                << ", power = " << interfererPower
                                                << ", start time = " << interfererStartTime
                                                << ", end time = " << interfererEndTime);

      // Compute the fraction of time the two events are overlapping
      Time overlap = GetOverlapTime (event, interferer);

      NS_LOG_DEBUG ("The two events overlap for " << overlap.GetSeconds () << " s.");

      // Compute the equivalent energy of the interference
      // Power [mW] = 10^(Power[dBm]/10)
      // Power [W] = Power [mW] / 1000
      double interfererPowerW = pow (10, interfererPower/10) / 1000;
      // Energy [J] = Time [s] * Power [W]
      double interferenceEnergy = overlap.GetSeconds () * interfererPowerW;
      cumulativeInterferenceEnergy.at (unsigned(interfererSf)-7) += interferenceEnergy;
      NS_LOG_DEBUG ("Interferer power in W: " << interfererPowerW);
      NS_LOG_DEBUG ("Interference energy: " << interferenceEnergy);
      it++;
    }

  // For each SF, check if there was destructive interference
  for (uint8_t currentSf = uint8_t (7); currentSf <= uint8_t (12); currentSf++)
    {
      NS_LOG_DEBUG ("Cumulative Interference Energy: " <<
                    cumulativeInterferenceEnergy.at (unsigned(currentSf)-7));

      // Use the computed cumulativeInterferenceEnergy to determine whether the
      // interference with this SF destroys the packet
      double signalPowerW = pow (10, rxPowerDbm/10) / 1000;
      double signalEnergy = duration.GetSeconds () * signalPowerW;
      NS_LOG_DEBUG ("Signal power in W: " << signalPowerW);
      NS_LOG_DEBUG ("Signal energy: " << signalEnergy);

      // Check whether the packet survives the interference of this SF
      double snirIsolation = collisionSnir [unsigned(sf)-7][unsigned(currentSf)-7];
      NS_LOG_DEBUG ("The needed isolation to survive is "
                    << snirIsolation << " dB");
      double snir = 10*log10 (signalEnergy/cumulativeInterferenceEnergy.at (unsigned(currentSf)-7));
      NS_LOG_DEBUG ("The current SNIR is " << snir << " dB");

      if (snir >= snirIsolation)
        {
          // Move on and check the rest of the interferers
          NS_LOG_DEBUG ("Packet survived interference");
        }
      else
        {
          NS_LOG_DEBUG ("Packet destroyed by interference with SF" <<
                        unsigned(currentSf));

          return currentSf;
        }
    }
  // If we get to here, it means that the packet survived all interference
  NS_LOG_DEBUG ("Packet survived all interference");

  // Since the packet was not destroyed, we return 0.
  return uint8_t (0);
}

void
LoraInterferenceHelper::ClearAllEvents (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_events.clear ();
}

Time
LoraInterferenceHelper::GetOverlapTime (Ptr<LoraInterferenceHelper::Event> event1,
                                        Ptr<LoraInterferenceHelper::Event> event2)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Create the value we will return later
  Time overlap;

  // Get handy values
  Time s1 = event1->GetStartTime (); // Start times
  Time s2 = event2->GetStartTime ();
  Time e1 = event1->GetEndTime ();   // End times
  Time e2 = event2->GetEndTime ();

  // Event1 starts before Event2
  if (s1 < s2)
    {
      // Non-overlapping events
      if (e1 < s2)
        {
          overlap = Seconds (0);
        }
      // event1 contains event2
      else if (e1 >= e2)
        {
          overlap = e2 - s2;
        }
      // Partially overlapping events
      else
        {
          overlap = e1 - s2;
        }
    }
  // Event2 starts before Event1
  else
    {
      // Non-overlapping events
      if (e2 < s1)
        {
          overlap = Seconds (0);
        }
      // event2 contains event1
      else if (e2 >= e1)
        {
          overlap = e1 - s1;
        }
      // Partially overlapping events
      else
        {
          overlap = e2 - s1;
        }
    }
  return overlap;
}
}
