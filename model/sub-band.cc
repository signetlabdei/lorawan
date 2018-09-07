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

#include "ns3/sub-band.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("SubBand");

NS_OBJECT_ENSURE_REGISTERED (SubBand);

TypeId
SubBand::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SubBand")
    .SetParent<Object> ()
    .SetGroupName ("lorawan");
  return tid;
}

SubBand::SubBand ()
{
  NS_LOG_FUNCTION (this);
}

  SubBand::SubBand (double firstFrequency, double lastFrequency, double dutyCycle,
                    double maxTxPowerDbm) :
    m_firstFrequency (firstFrequency),
    m_lastFrequency (lastFrequency),
    m_dutyCycle (dutyCycle),
    m_nextTransmissionTime (Seconds (0)),
    m_maxTxPowerDbm (maxTxPowerDbm)
  {
    NS_LOG_FUNCTION (this << firstFrequency << lastFrequency << dutyCycle <<
                     maxTxPowerDbm);
  }

  SubBand::~SubBand ()
  {
    NS_LOG_FUNCTION (this);
  }

  double
  SubBand::GetFirstFrequency (void)
  {
    return m_firstFrequency;
  }

  double
  SubBand::GetDutyCycle (void)
  {
    return m_dutyCycle;
  }

  bool
  SubBand::BelongsToSubBand (double frequency)
  {
    return (frequency > m_firstFrequency) && (frequency < m_lastFrequency);
  }

  bool
  SubBand::BelongsToSubBand (Ptr<LogicalLoraChannel> logicalChannel)
  {
    double frequency = logicalChannel->GetFrequency ();
    return BelongsToSubBand (frequency);
  }

  void
  SubBand::SetNextTransmissionTime (Time nextTime)
  {
    m_nextTransmissionTime = nextTime;
  }

  Time
  SubBand::GetNextTransmissionTime (void)
  {
    return m_nextTransmissionTime;
  }

  void
  SubBand::SetMaxTxPowerDbm (double maxTxPowerDbm)
  {
    m_maxTxPowerDbm = maxTxPowerDbm;
  }

  double
  SubBand::GetMaxTxPowerDbm (void)
  {
    return m_maxTxPowerDbm;
  }
}
}
