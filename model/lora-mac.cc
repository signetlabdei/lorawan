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

#include "ns3/lora-mac.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LoraMac");

NS_OBJECT_ENSURE_REGISTERED (LoraMac);

TypeId
LoraMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::LoraMac")
    .SetParent<Object> ()
    .SetGroupName ("lorawan")
    .AddTraceSource ("SentNewPacket",
                     "Trace source indicating a new packet "
                     "arrived at the MAC layer",
                     MakeTraceSourceAccessor (&LoraMac::m_sentNewPacket),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("ReceivedPacket",
                     "Trace source indicating a packet "
                     "was correctly received at the MAC layer",
                     MakeTraceSourceAccessor (&LoraMac::m_receivedPacket),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("CannotSendBecauseDutyCycle",
                     "Trace source indicating a packet "
                     "could not be sent immediately because of duty cycle limitations",
                     MakeTraceSourceAccessor (&LoraMac::m_cannotSendBecauseDutyCycle),
                     "ns3::Packet::TracedCallback");
  return tid;
}

LoraMac::LoraMac ()
{
  NS_LOG_FUNCTION (this);
}

LoraMac::~LoraMac () {
  NS_LOG_FUNCTION (this);
}

void
LoraMac::SetDevice (Ptr<NetDevice> device)
{
  m_device = device;
}

Ptr<NetDevice>
LoraMac::GetDevice (void)
{
  return m_device;
}

Ptr<LoraPhy>
LoraMac::GetPhy (void)
{
  return m_phy;
}

void
LoraMac::SetPhy (Ptr<LoraPhy> phy)
{
  // Set the phy
  m_phy = phy;

  // Connect the receive callbacks
  m_phy->SetReceiveOkCallback (MakeCallback (&LoraMac::Receive, this));
  m_phy->SetReceiveFailedCallback (MakeCallback (&LoraMac::FailedReception, this));
  m_phy->SetTxFinishedCallback (MakeCallback (&LoraMac::TxFinished, this));
}

LogicalLoraChannelHelper
LoraMac::GetLogicalLoraChannelHelper (void)
{
  return m_channelHelper;
}

void
LoraMac::SetLogicalLoraChannelHelper (LogicalLoraChannelHelper helper)
{
  m_channelHelper = helper;
}

uint8_t
LoraMac::GetSfFromDataRate (uint8_t dataRate)
{
  NS_LOG_FUNCTION (this << unsigned(dataRate));

  // Check we are in range
  if (dataRate >= m_sfForDataRate.size ())
    {
      return 0;
    }

  return m_sfForDataRate.at (dataRate);
}

double
LoraMac::GetBandwidthFromDataRate (uint8_t dataRate)
{
  NS_LOG_FUNCTION (this << unsigned(dataRate));

  // Check we are in range
  if (dataRate > m_bandwidthForDataRate.size ())
    {
      return 0;
    }

  return m_bandwidthForDataRate.at (dataRate);
}

double
LoraMac::GetDbmForTxPower (uint8_t txPower)
{
  NS_LOG_FUNCTION (this << unsigned (txPower));

  if (txPower > m_txDbmForTxPower.size ())
    {
      return 0;
    }

  return m_txDbmForTxPower.at (txPower);
}

void
LoraMac::SetSfForDataRate (std::vector<uint8_t> sfForDataRate)
{
  m_sfForDataRate = sfForDataRate;
}

void
LoraMac::SetBandwidthForDataRate (std::vector<double> bandwidthForDataRate)
{
  m_bandwidthForDataRate = bandwidthForDataRate;
}

void
LoraMac::SetMaxAppPayloadForDataRate (std::vector<uint32_t> maxAppPayloadForDataRate)
{
  m_maxAppPayloadForDataRate = maxAppPayloadForDataRate;
}

void
LoraMac::SetTxDbmForTxPower (std::vector<double> txDbmForTxPower)
{
  m_txDbmForTxPower = txDbmForTxPower;
}

void
LoraMac::SetNPreambleSymbols (int nPreambleSymbols)
{
  m_nPreambleSymbols = nPreambleSymbols;
}

int
LoraMac::GetNPreambleSymbols (void)
{
  return m_nPreambleSymbols;
}

void
LoraMac::SetReplyDataRateMatrix (ReplyDataRateMatrix replyDataRateMatrix)
{
  m_replyDataRateMatrix = replyDataRateMatrix;
}
}
