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

#include "ns3/logical-lora-channel.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("LogicalLoraChannel");

NS_OBJECT_ENSURE_REGISTERED (LogicalLoraChannel);

TypeId
LogicalLoraChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LogicalLoraChannel")
    .SetParent<Object> ()
    .SetGroupName ("lorawan");
  return tid;
}

LogicalLoraChannel::LogicalLoraChannel () :
  m_frequency (0),
  m_minDataRate (0),
  m_maxDataRate (5),
  m_enabledForUplink (true)
{
  NS_LOG_FUNCTION (this);
}

LogicalLoraChannel::~LogicalLoraChannel ()
{
  NS_LOG_FUNCTION (this);
}

LogicalLoraChannel::LogicalLoraChannel (double frequency) :
  m_frequency (frequency),
  m_enabledForUplink (true)
{
  NS_LOG_FUNCTION (this);
}

LogicalLoraChannel::LogicalLoraChannel (double frequency, uint8_t minDataRate,
                                        uint8_t maxDataRate) :
  m_frequency (frequency),
  m_minDataRate (minDataRate),
  m_maxDataRate (maxDataRate),
  m_enabledForUplink (true)
{
  NS_LOG_FUNCTION (this);
}

double
LogicalLoraChannel::GetFrequency (void) const
{
  return m_frequency;
}

void
LogicalLoraChannel::SetMinimumDataRate (uint8_t minDataRate)
{
  m_minDataRate = minDataRate;
}

void
LogicalLoraChannel::SetMaximumDataRate (uint8_t maxDataRate)
{
  m_maxDataRate = maxDataRate;
}

uint8_t
LogicalLoraChannel::GetMinimumDataRate (void)
{
  return m_minDataRate;
}

uint8_t
LogicalLoraChannel::GetMaximumDataRate (void)
{
  return m_maxDataRate;
}

void
LogicalLoraChannel::SetEnabledForUplink (void)
{
  m_enabledForUplink = true;
}

void
LogicalLoraChannel::DisableForUplink (void)
{
  m_enabledForUplink = false;
}

bool
LogicalLoraChannel::IsEnabledForUplink (void)
{
  return m_enabledForUplink;
}

bool
operator== (const Ptr<LogicalLoraChannel>& first,
            const Ptr<LogicalLoraChannel>& second)
{
  double thisFreq = first->GetFrequency ();
  double otherFreq = second->GetFrequency ();

  NS_LOG_DEBUG ("Checking equality between logical lora channels: " <<
                thisFreq << " " << otherFreq);

  NS_LOG_DEBUG ("Result:" << (thisFreq == otherFreq));
  return (thisFreq == otherFreq);
}

bool
operator!= (const Ptr<LogicalLoraChannel>& first,
            const Ptr<LogicalLoraChannel>& second)
{
  return !(first == second);
}
}
}
