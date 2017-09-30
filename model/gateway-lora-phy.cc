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

#include "ns3/gateway-lora-phy.h"
#include "ns3/lora-tag.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GatewayLoraPhy");

NS_OBJECT_ENSURE_REGISTERED (GatewayLoraPhy);

/**************************************
 *    ReceptionPath implementation    *
 **************************************/
GatewayLoraPhy::ReceptionPath::ReceptionPath(double frequencyMHz) :
  m_frequencyMHz (frequencyMHz),
  m_available (1),
  m_event (0)
{
  NS_LOG_FUNCTION_NOARGS ();
}

GatewayLoraPhy::ReceptionPath::~ReceptionPath(void)
{
  NS_LOG_FUNCTION_NOARGS ();
}

double
GatewayLoraPhy::ReceptionPath::GetFrequency (void)
{
  return m_frequencyMHz;
}

bool
GatewayLoraPhy::ReceptionPath::IsAvailable (void)
{
  return m_available;
}

void
GatewayLoraPhy::ReceptionPath::Free (void)
{
  m_available = true;
  m_event = 0;
}

void
GatewayLoraPhy::ReceptionPath::LockOnEvent (Ptr<LoraInterferenceHelper::Event>
                                            event)
{
  m_available = false;
  m_event = event;
}

void
GatewayLoraPhy::ReceptionPath::SetEvent (Ptr<LoraInterferenceHelper::Event>
                                         event)
{
  m_event = event;
}

Ptr<LoraInterferenceHelper::Event>
GatewayLoraPhy::ReceptionPath::GetEvent (void)
{
  return m_event;
}

void
GatewayLoraPhy::ReceptionPath::SetFrequency (double frequencyMHz)
{
  m_frequencyMHz = frequencyMHz;
}

/***********************************************************************
 *                 Implementation of Gateway methods                   *
 ***********************************************************************/

TypeId
GatewayLoraPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GatewayLoraPhy")
    .SetParent<LoraPhy> ()
    .SetGroupName ("lorawan")
    .AddConstructor<GatewayLoraPhy> ()
    .AddTraceSource ("LostPacketBecauseNoMoreReceivers",
                     "Trace source indicating a packet "
                     "could not be correctly received because"
                     "there are no more demodulators available",
                     MakeTraceSourceAccessor
                       (&GatewayLoraPhy::m_noMoreDemodulators),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("OccupiedReceptionPaths",
                     "Number of currently occupied reception paths",
                     MakeTraceSourceAccessor
                       (&GatewayLoraPhy::m_occupiedReceptionPaths),
                     "ns3::TracedValueCallback::Int");
  return tid;
}

GatewayLoraPhy::GatewayLoraPhy () :
  m_isTransmitting (false)
{
  NS_LOG_FUNCTION_NOARGS ();
}

GatewayLoraPhy::~GatewayLoraPhy ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

// Uplink sensitivity (Source: SX1301 datasheet)
// {SF7, SF8, SF9, SF10, SF11, SF12}
const double GatewayLoraPhy::sensitivity[6] =
{-130.0, -132.5, -135.0, -137.5, -140.0, -142.5};

void
GatewayLoraPhy::AddReceptionPath (double frequencyMHz)
{
  NS_LOG_FUNCTION (this << frequencyMHz);

  m_receptionPaths.push_back (Create<GatewayLoraPhy::ReceptionPath>
                                (frequencyMHz));
}

void
GatewayLoraPhy::ResetReceptionPaths (void)
{
  NS_LOG_FUNCTION (this);

  m_receptionPaths.clear ();
}

void
GatewayLoraPhy::Send (Ptr<Packet> packet, LoraTxParameters txParams,
                      double frequencyMHz, double txPowerDbm)
{
  NS_LOG_FUNCTION (this << packet << frequencyMHz << txPowerDbm);

  // Get the time a packet with these parameters will take to be transmitted
  Time duration = GetOnAirTime (packet, txParams);

  /*
   *  Differently from what is done in EndDevices, where packets cannot be
   *  transmitted while in RX state, Gateway sending is assumed to have priority
   *  over reception.
   *
   *  This different behaviour is motivated by the asymmetry in a typical
   *  LoRaWAN network, where Downlink messages are more critical to network
   *  performance than Uplink ones. Even if the gateway is receiving a packet
   *  on the channel when it is asked to transmit by the upper layer, in order
   *  not to miss the receive window of the end device, the gateway will still
   *  need to send the packet. In order to model this fact, the send event is
   *  registered in the gateway's InterferenceHelper as a received event.
   *  While this may not destroy packets incoming on the same frequency, this
   *  is almost always guaranteed to do so due to the fact that this event can
   *  have a power up to 27 dBm.
   */
  m_interference.Add (duration, txPowerDbm, txParams.sf, packet, frequencyMHz);

  // Send the packet in the channel
  m_channel->Send (this, packet, txPowerDbm, txParams, duration, frequencyMHz);

  Simulator::Schedule (duration, &GatewayLoraPhy::TxFinished, this, packet);

  m_isTransmitting = true;

  // Fire the trace source
  if (m_device)
    {
      m_startSending (packet, m_device->GetNode ()->GetId ());
    }
  else
    {
      m_startSending (packet, 0);
    }
}

void
GatewayLoraPhy::TxFinished (Ptr<Packet> packet)
{
  m_isTransmitting = false;
}

bool
GatewayLoraPhy::IsTransmitting (void)
{
  return m_isTransmitting;
}

void
GatewayLoraPhy::StartReceive (Ptr<Packet> packet, double rxPowerDbm,
                              uint8_t sf, Time duration, double frequencyMHz)
{
  NS_LOG_FUNCTION (this << packet << rxPowerDbm << duration << frequencyMHz);

  // Fire the trace source
  m_phyRxBeginTrace (packet);

  // Add the event to the LoraInterferenceHelper
  Ptr<LoraInterferenceHelper::Event> event;
  event = m_interference.Add (duration, rxPowerDbm, sf, packet, frequencyMHz);

  // Cycle over the receive paths to check availability to receive the packet
  std::list<Ptr<GatewayLoraPhy::ReceptionPath> >::iterator it;

  for (it = m_receptionPaths.begin (); it != m_receptionPaths.end (); ++it)
    {
      Ptr<GatewayLoraPhy::ReceptionPath> currentPath = *it;

      NS_LOG_DEBUG ("Current ReceptionPath is centered on frequency = " <<
                    currentPath->GetFrequency ());

      // If the receive path is available and listening on the channel of
      // interest, we have a candidate
      if (currentPath->GetFrequency () == frequencyMHz &&
          currentPath->IsAvailable ())
        {
          // See whether the reception power is above or below the sensitivity
          // for that spreading factor
          double sensitivity = GatewayLoraPhy::sensitivity[unsigned(sf)-7];

          if (rxPowerDbm < sensitivity)   // Packet arrived below sensitivity
            {
              NS_LOG_INFO ("Dropping packet reception of packet with sf = "
                           << unsigned(sf) <<
                           " because under the sensitivity of "
                           << sensitivity << " dBm");

              if (m_device)
                {
                  m_underSensitivity (packet, m_device->GetNode ()->GetId ());
                }
              else
                {
                  m_underSensitivity (packet, 0);
                }

              // Since the packet is below sensitivity, it makes no sense to
              // search for another ReceivePath
              return;
            }
          else    // We have sufficient sensitivity to start receiving
            {
              NS_LOG_INFO ("Scheduling reception of a packet, " <<
                           "occupying one demodulator");

              // Block this resource
              currentPath->LockOnEvent (event);
              m_occupiedReceptionPaths++;

              // Schedule the end of the reception of the packet
              Simulator::Schedule (duration, &LoraPhy::EndReceive, this,
                                   packet, event);

              // Make sure we don't go on searching for other ReceivePaths
              return;
            }
        }
    }
  // If we get to this point, there are no demodulators we can use
  NS_LOG_INFO ("Dropping packet reception of packet with sf = "
               << unsigned(sf) <<
               " because no suitable demodulator was found");

  // Fire the trace source
  if (m_device)
    {
      m_noMoreDemodulators (packet, m_device->GetNode ()->GetId ());
    }
  else
    {
      m_noMoreDemodulators (packet, 0);
    }
}

void
GatewayLoraPhy::EndReceive (Ptr<Packet> packet,
                            Ptr<LoraInterferenceHelper::Event> event)
{
  NS_LOG_FUNCTION (this << packet << *event);

  // Call the trace source
  m_phyRxEndTrace (packet);

  // Call the LoraInterferenceHelper to determine whether there was
  // destructive interference. If the packet is correctly received, this
  // method returns a 0.
  uint8_t packetDestroyed = 0;
  packetDestroyed = m_interference.IsDestroyedByInterference (event);

  // Check whether the packet was destroyed
  if (packetDestroyed != uint8_t (0))
    {
      NS_LOG_DEBUG ("packetDestroyed by " << unsigned(packetDestroyed));

      // Update the packet's LoraTag
      LoraTag tag;
      packet->RemovePacketTag (tag);
      tag.SetDestroyedBy (packetDestroyed);
      packet->AddPacketTag (tag);

      // Fire the trace source
      if (m_device)
        {
          m_interferedPacket (packet, m_device->GetNode ()->GetId ());
        }
      else
        {
          m_interferedPacket (packet, 0);
        }
    }
  else   // Reception was correct
    {
      NS_LOG_INFO ("Packet with SF " <<
                   unsigned(event->GetSpreadingFactor ()) <<
                   " received correctly");

      // Fire the trace source
      if (m_device)
        {
          m_successfullyReceivedPacket (packet, m_device->GetNode ()->GetId ());
        }
      else
        {
          m_successfullyReceivedPacket (packet, 0);
        }

      // Forward the packet to the upper layer
      if (!m_rxOkCallback.IsNull ())
        {
          // Make a copy of the packet
          Ptr<Packet> packetCopy = packet->Copy ();

          // Set the receive power and frequency of this packet in the LoraTag: this
          // information can be useful for upper layers trying to control link
          // quality.
          LoraTag tag;
          packetCopy->RemovePacketTag (tag);
          tag.SetReceivePower (event->GetRxPowerdBm ());
          tag.SetFrequency (event->GetFrequency ());
          packetCopy->AddPacketTag (tag);

          m_rxOkCallback (packetCopy);
        }

    }

  // Search for the demodulator that was locked on this event to free it.

  std::list< Ptr< GatewayLoraPhy::ReceptionPath > >::iterator it;

  for (it = m_receptionPaths.begin (); it != m_receptionPaths.end (); ++it)
    {
      Ptr<GatewayLoraPhy::ReceptionPath> currentPath = *it;

      if (currentPath->GetEvent () == event)
        {
          currentPath->Free ();
          m_occupiedReceptionPaths--;
          return;
        }
    }
}

bool
GatewayLoraPhy::IsOnFrequency (double frequencyMHz)
{
  NS_LOG_FUNCTION (this << frequencyMHz);

  // Search every demodulator to see whether there's one listening on this
  // frequency.
  std::list< Ptr< GatewayLoraPhy::ReceptionPath > >::iterator it;

  for (it = m_receptionPaths.begin (); it != m_receptionPaths.end (); ++it)
    {
      Ptr<GatewayLoraPhy::ReceptionPath> currentPath = *it;

      NS_LOG_DEBUG ("Current reception path is on frequency " <<
                    currentPath->GetFrequency ());

      if (currentPath->GetFrequency () == frequencyMHz)
        {
          return true;
        }
    }
  return false;
}
}
