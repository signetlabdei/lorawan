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

#include "ns3/mac-command.h"
#include "ns3/log.h"
#include <bitset>
#include <cmath>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("MacCommand");

NS_OBJECT_ENSURE_REGISTERED (MacCommand);

TypeId
MacCommand::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MacCommand")
    .SetParent<Object> ()
    .SetGroupName ("lorawan")
  ;
  return tid;
}

MacCommand::MacCommand ()
{
  NS_LOG_FUNCTION (this);
}

MacCommand::~MacCommand ()
{
  NS_LOG_FUNCTION (this);
}

enum MacCommandType
MacCommand::GetCommandType (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_commandType;
}

uint8_t
MacCommand::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_serializedSize;
}

uint8_t
MacCommand::GetCIDFromMacCommand (enum MacCommandType commandType)
{
  NS_LOG_FUNCTION_NOARGS ();

  switch (commandType)
    {
    case (INVALID):
      {
        return 0x0;
      }
    case (LINK_CHECK_REQ):
    case (LINK_CHECK_ANS):
      {
        return 0x02;
      }
    case (LINK_ADR_REQ):
    case (LINK_ADR_ANS):
      {
        return 0x03;
      }
    case (DUTY_CYCLE_REQ):
    case (DUTY_CYCLE_ANS):
      {
        return 0x04;
      }
    case (RX_PARAM_SETUP_REQ):
    case (RX_PARAM_SETUP_ANS):
      {
        return 0x05;
      }
    case (DEV_STATUS_REQ):
    case (DEV_STATUS_ANS):
      {
        return 0x06;
      }
    case (NEW_CHANNEL_REQ):
    case (NEW_CHANNEL_ANS):
      {
        return 0x07;
      }
    case (RX_TIMING_SETUP_REQ):
    case (RX_TIMING_SETUP_ANS):
      {
        return 0x08;
      }
    case (TX_PARAM_SETUP_REQ):
    case (TX_PARAM_SETUP_ANS):
      {
        return 0x09;
      }
    case (DL_CHANNEL_REQ):
    case (DL_CHANNEL_ANS):
      {
        return 0x0A;
      }
    }
  return 0;
}

//////////////////
// LinkCheckReq //
//////////////////

LinkCheckReq::LinkCheckReq ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_commandType = LINK_CHECK_REQ;
  m_serializedSize = 1;
}

LinkCheckReq::~LinkCheckReq ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
LinkCheckReq::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID and we're done
  uint8_t cid = GetCIDFromMacCommand (m_commandType);
  start.WriteU8 (cid);
  NS_LOG_DEBUG ("Serialized LinkCheckReq: " << unsigned (cid));
}

uint8_t
LinkCheckReq::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Read the CID
  start.ReadU8 ();

  return m_serializedSize;
}

void
LinkCheckReq::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "LinkCheckReq" << std::endl;
}

//////////////////
// LinkCheckAns //
//////////////////

LinkCheckAns::LinkCheckAns () :
  m_margin (0),
  m_gwCnt (0)
{
  NS_LOG_FUNCTION (this);

  m_commandType = LINK_CHECK_ANS;
  m_serializedSize = 3;
}

LinkCheckAns::LinkCheckAns (uint8_t margin, uint8_t gwCnt) :
  m_margin (margin),
  m_gwCnt (gwCnt)
{
  NS_LOG_FUNCTION (this << unsigned(margin) << unsigned(gwCnt));

  m_commandType = LINK_CHECK_ANS;
  m_serializedSize = 3;
}

void
LinkCheckAns::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
  // Write the margin
  start.WriteU8 (m_margin);
  // Write the gwCnt
  start.WriteU8 (m_gwCnt);
}

uint8_t
LinkCheckAns::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();
  m_margin = start.ReadU8 ();
  m_gwCnt = start.ReadU8 ();
  return m_serializedSize;
}

void
LinkCheckAns::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "LinkCheckAns" << std::endl;
  os << "margin: " << unsigned (m_margin) << std::endl;
  os << "gwCnt: " << unsigned (m_gwCnt) << std::endl;
}

void
LinkCheckAns::SetMargin (uint8_t margin)
{
  NS_LOG_FUNCTION (this << unsigned (margin));

  m_margin = margin;
}

uint8_t
LinkCheckAns::GetMargin (void) const
{
  NS_LOG_FUNCTION (this);

  return m_margin;
}

void
LinkCheckAns::SetGwCnt (uint8_t gwCnt)
{
  NS_LOG_FUNCTION (this << unsigned (gwCnt));

  m_gwCnt = gwCnt;
}

uint8_t
LinkCheckAns::GetGwCnt (void) const
{
  NS_LOG_FUNCTION (this);

  return m_gwCnt;
}

void
LinkCheckAns::IncrementGwCnt (void)
{
  NS_LOG_FUNCTION (this);

  m_gwCnt++;
}

////////////////
// LinkAdrReq //
////////////////

LinkAdrReq::LinkAdrReq ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = LINK_ADR_REQ;
  m_serializedSize = 5;
}

LinkAdrReq::LinkAdrReq (uint8_t dataRate, uint8_t txPower, uint16_t channelMask,
                        uint8_t chMaskCntl, uint8_t nbRep) :
  m_dataRate (dataRate),
  m_txPower (txPower),
  m_channelMask (channelMask),
  m_chMaskCntl (chMaskCntl),
  m_nbRep (nbRep)
{
  NS_LOG_FUNCTION (this);

  m_commandType = LINK_ADR_REQ;
  m_serializedSize = 5;
}

void
LinkAdrReq::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
  start.WriteU8 (m_dataRate << 4 | (m_txPower & 0b1111));
  start.WriteU16 (m_channelMask);
  start.WriteU8 (m_chMaskCntl << 4 | (m_nbRep & 0b1111));
}

uint8_t
LinkAdrReq::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();
  uint8_t firstByte = start.ReadU8 ();
  m_dataRate = firstByte >> 4;
  m_txPower = firstByte & 0b1111;
  m_channelMask = start.ReadU16 ();
  uint8_t fourthByte = start.ReadU8 ();
  m_chMaskCntl = fourthByte >> 4;
  m_nbRep = fourthByte & 0b1111;

  return m_serializedSize;
}

void
LinkAdrReq::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "LinkAdrReq" << std::endl;
  os << "dataRate: " << unsigned (m_dataRate) << std::endl;
  os << "txPower: " << unsigned (m_txPower) << std::endl;
  os << "channelMask: " << std::bitset<16> (m_channelMask) << std::endl;
  os << "chMaskCntl: " << unsigned (m_chMaskCntl) << std::endl;
  os << "nbRep: " << unsigned (m_nbRep) << std::endl;
}

uint8_t
LinkAdrReq::GetDataRate (void)
{
  NS_LOG_FUNCTION (this);

  return m_dataRate;
}

uint8_t
LinkAdrReq::GetTxPower (void)
{
  NS_LOG_FUNCTION (this);

  return m_txPower;
}

std::list<int>
LinkAdrReq::GetEnabledChannelsList (void)
{
  NS_LOG_FUNCTION (this);

  std::list<int> channelIndices;
  for (int i = 0; i < 16; i++)
    {
      if (m_channelMask & (0b1 << i))     // Take channel mask's i-th bit
        {
          NS_LOG_DEBUG ("Adding channel index " << i);
          channelIndices.push_back (i);
        }
    }

  return channelIndices;
}

int
LinkAdrReq::GetRepetitions (void)
{
  NS_LOG_FUNCTION (this);

  return m_nbRep;
}
////////////////
// LinkAdrAns //
////////////////

LinkAdrAns::LinkAdrAns ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = LINK_ADR_ANS;
  m_serializedSize = 2;
}

LinkAdrAns::LinkAdrAns (bool powerAck, bool dataRateAck, bool channelMaskAck) :
  m_powerAck (powerAck),
  m_dataRateAck (dataRateAck),
  m_channelMaskAck (channelMaskAck)
{
  NS_LOG_FUNCTION (this);

  m_commandType = LINK_ADR_ANS;
  m_serializedSize = 2;
}

void
LinkAdrAns::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
  // We can assume that true will be converted to 1 and that false will be
  // converted to 0 on any C++ compiler
  start.WriteU8 ((uint8_t (m_powerAck) << 2) | (uint8_t (m_dataRateAck) << 1) |
                 uint8_t (m_channelMaskAck));
}

uint8_t
LinkAdrAns::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();

  uint8_t byte = start.ReadU8 ();

  m_powerAck = byte & 0b100;
  m_dataRateAck = byte & 0b10;
  m_channelMaskAck = byte & 0b1;

  return m_serializedSize;
}

void
LinkAdrAns::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "LinkAdrAns" << std::endl;
}

//////////////////
// DutyCycleReq //
//////////////////

DutyCycleReq::DutyCycleReq ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = DUTY_CYCLE_REQ;
  m_serializedSize = 2;
}

DutyCycleReq::DutyCycleReq (uint8_t dutyCycle) :
  m_maxDCycle (dutyCycle)
{
  NS_LOG_FUNCTION (this);

  m_commandType = DUTY_CYCLE_REQ;
  m_serializedSize = 2;
}

void
DutyCycleReq::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
  start.WriteU8 (m_maxDCycle);
}

uint8_t
DutyCycleReq::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();
  m_maxDCycle = start.ReadU8 ();

  return m_serializedSize;
}

void
DutyCycleReq::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "DutyCycleReq" << std::endl;
  os << "maxDCycle: " << unsigned (m_maxDCycle) << std::endl;
  os << "maxDCycle (fraction): " << GetMaximumAllowedDutyCycle () << std::endl;
}

double
DutyCycleReq::GetMaximumAllowedDutyCycle (void) const
{
  NS_LOG_FUNCTION (this);

  // Check if we need to turn off completely
  if (m_maxDCycle == 255)
    {
      return 0;
    }

  if (m_maxDCycle == 0)
    {
      return 1;
    }

  return 1 / std::pow (2,double(m_maxDCycle));
}

//////////////////
// DutyCycleAns //
//////////////////

DutyCycleAns::DutyCycleAns ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = DUTY_CYCLE_ANS;
  m_serializedSize = 1;
}

void
DutyCycleAns::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
}

uint8_t
DutyCycleAns::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();
  return m_serializedSize;
}

void
DutyCycleAns::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "DutyCycleAns" << std::endl;
}

//////////////////
// RxParamSetupReq //
//////////////////

RxParamSetupReq::RxParamSetupReq ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = RX_PARAM_SETUP_REQ;
  m_serializedSize = 5;
}

RxParamSetupReq::RxParamSetupReq (uint8_t rx1DrOffset, uint8_t rx2DataRate,
                                  double frequency) :
  m_rx1DrOffset (rx1DrOffset),
  m_rx2DataRate (rx2DataRate),
  m_frequency (frequency)
{
  NS_LOG_FUNCTION (this << unsigned (rx1DrOffset) << unsigned (rx2DataRate) <<
                   frequency);

  if ((rx1DrOffset & 0b11111000) != 0)
    {
      NS_LOG_WARN ("Warning: received an rx1DrOffset greater than 7. Actual value will be different.");
    }
  if ((rx2DataRate & 0b11110000) != 0)
    {
      NS_LOG_WARN ("Warning: received a rx2DataRate greater than 15. Actual value will be different.");
    }

  m_commandType = RX_PARAM_SETUP_REQ;
  m_serializedSize = 5;
}

void
RxParamSetupReq::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
  // Data serialization
  start.WriteU8 ((m_rx1DrOffset & 0b111) << 4 | (m_rx2DataRate & 0b1111));
  uint32_t encodedFrequency = uint32_t (m_frequency / 100);
  NS_LOG_DEBUG (unsigned (encodedFrequency));
  NS_LOG_DEBUG (std::bitset<32> (encodedFrequency));
  start.WriteU8 ((encodedFrequency & 0xff0000) >> 16);     // Most significant byte
  start.WriteU8 ((encodedFrequency & 0xff00) >> 8);     // Middle byte
  start.WriteU8 (encodedFrequency & 0xff);     // Least significant byte
}

uint8_t
RxParamSetupReq::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();
  // Data serialization
  uint8_t firstByte = start.ReadU8 ();
  m_rx1DrOffset = (firstByte & 0b1110000) >> 4;
  m_rx2DataRate = firstByte & 0b1111;
  uint32_t secondByte = start.ReadU8 ();
  uint32_t thirdByte = start.ReadU8 ();
  uint32_t fourthByte = start.ReadU8 ();
  uint32_t encodedFrequency = (secondByte << 16) | (thirdByte << 8) | fourthByte;
  NS_LOG_DEBUG (std::bitset<32> (encodedFrequency));
  m_frequency = double(encodedFrequency) * 100;

  return m_serializedSize;
}

void
RxParamSetupReq::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "RxParamSetupReq" << std::endl;
  os << "rx1DrOffset: " << unsigned( m_rx1DrOffset) << std::endl;
  os << "rx2DataRate: " << unsigned( m_rx2DataRate) << std::endl;
  os << "frequency: " << m_frequency << std::endl;
}

uint8_t
RxParamSetupReq::GetRx1DrOffset (void)
{
  NS_LOG_FUNCTION (this);

  return m_rx1DrOffset;
}
uint8_t
RxParamSetupReq::GetRx2DataRate (void)
{
  NS_LOG_FUNCTION (this);

  return m_rx2DataRate;
}
double
RxParamSetupReq::GetFrequency (void)
{
  NS_LOG_FUNCTION (this);

  return m_frequency;
}

/////////////////////
// RxParamSetupAns //
/////////////////////

RxParamSetupAns::RxParamSetupAns ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = RX_PARAM_SETUP_ANS;
  m_serializedSize = 2;
}

RxParamSetupAns::RxParamSetupAns (bool rx1DrOffsetAck, bool rx2DataRateAck,
                                  bool channelAck) :
  m_rx1DrOffsetAck (rx1DrOffsetAck),
  m_rx2DataRateAck (rx2DataRateAck),
  m_channelAck (channelAck)
{
  NS_LOG_FUNCTION (this << rx1DrOffsetAck << rx2DataRateAck << channelAck);

  m_commandType = RX_PARAM_SETUP_ANS;
  m_serializedSize = 2;
}

void
RxParamSetupAns::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
  // Data serialization
  start.WriteU8 (uint8_t (m_rx1DrOffsetAck) << 2 |
                 uint8_t (m_rx2DataRateAck) << 1 |
                 uint8_t (m_channelAck));

}

uint8_t
RxParamSetupAns::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();

  uint8_t byte = start.ReadU8 ();

  m_rx1DrOffsetAck = (byte & 0b100) >> 2;
  m_rx2DataRateAck = (byte & 0b10) >> 1;
  m_channelAck = byte & 0b1;

  return m_serializedSize;
}

void
RxParamSetupAns::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "RxParamSetupAns" << std::endl;
  os << "m_rx1DrOffsetAck: " << m_rx1DrOffsetAck << std::endl;
  os << "m_rx2DataRateAck: " << m_rx2DataRateAck << std::endl;
  os << "m_channelAck: " << m_channelAck << std::endl;
}

//////////////////
// DevStatusReq //
//////////////////

DevStatusReq::DevStatusReq ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = DEV_STATUS_REQ;
  m_serializedSize = 1;
}

void
DevStatusReq::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
}

uint8_t
DevStatusReq::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();

  return m_serializedSize;
}

void
DevStatusReq::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "DevStatusReq" << std::endl;
}

//////////////////
// DevStatusAns //
//////////////////

DevStatusAns::DevStatusAns ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = DEV_STATUS_ANS;
  m_serializedSize = 3;
}

DevStatusAns::DevStatusAns (uint8_t battery, uint8_t margin) :
  m_battery (battery),
  m_margin (margin)
{
  NS_LOG_FUNCTION (this << unsigned (battery) << unsigned (margin));

  m_commandType = DEV_STATUS_ANS;
  m_serializedSize = 3;
}

void
DevStatusAns::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
  start.WriteU8 (m_battery);
  start.WriteU8 (m_margin);
}

uint8_t
DevStatusAns::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();
  m_battery = start.ReadU8 ();
  m_margin = start.ReadU8 () & 0b111111;

  return m_serializedSize;
}

void
DevStatusAns::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "DevStatusAns" << std::endl;
  os << "Battery: " << unsigned (m_battery) << std::endl;
  os << "Margin: " << unsigned (m_margin) << std::endl;
}

uint8_t
DevStatusAns::GetBattery (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_battery;
}

uint8_t
DevStatusAns::GetMargin (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_margin;
}

//////////////////
// NewChannelReq //
//////////////////

NewChannelReq::NewChannelReq ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = NEW_CHANNEL_REQ;
  m_serializedSize = 6;
}

NewChannelReq::NewChannelReq (uint8_t chIndex, double frequency,
                              uint8_t minDataRate, uint8_t maxDataRate) :
  m_chIndex (chIndex),
  m_frequency (frequency),
  m_minDataRate (minDataRate),
  m_maxDataRate (maxDataRate)
{
  NS_LOG_FUNCTION (this);

  m_commandType = NEW_CHANNEL_REQ;
  m_serializedSize = 6;
}

void
NewChannelReq::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));

  start.WriteU8 (m_chIndex);
  uint32_t encodedFrequency = uint32_t (m_frequency / 100);
  start.WriteU8 ((encodedFrequency & 0xff0000) >> 16);
  start.WriteU8 ((encodedFrequency & 0xff00) >> 8);
  start.WriteU8 (encodedFrequency & 0xff);
  start.WriteU8 ((m_maxDataRate << 4) | (m_minDataRate & 0xf));
}

uint8_t
NewChannelReq::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();
  // Read the data
  m_chIndex = start.ReadU8 ();
  uint32_t encodedFrequency = 0;
  encodedFrequency |= uint32_t (start.ReadU16 ()) << 8;
  encodedFrequency |= uint32_t (start.ReadU8 ());
  m_frequency = double (encodedFrequency) * 100;
  uint8_t dataRateByte = start.ReadU8 ();
  m_maxDataRate = dataRateByte >> 4;
  m_minDataRate = dataRateByte & 0xf;

  return m_serializedSize;
}

void
NewChannelReq::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "NewChannelReq" << std::endl;

}

uint8_t
NewChannelReq::GetChannelIndex (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_chIndex;
}

double
NewChannelReq::GetFrequency (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_frequency;
}

uint8_t
NewChannelReq::GetMinDataRate (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_minDataRate;
}

uint8_t
NewChannelReq::GetMaxDataRate (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_maxDataRate;
}

///////////////////
// NewChannelAns //
///////////////////

NewChannelAns::NewChannelAns ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = NEW_CHANNEL_ANS;
  m_serializedSize = 2;
}

NewChannelAns::NewChannelAns (bool dataRateRangeOk, bool channelFrequencyOk) :
  m_dataRateRangeOk (dataRateRangeOk),
  m_channelFrequencyOk (channelFrequencyOk)
{
  NS_LOG_FUNCTION (this);

  m_commandType = NEW_CHANNEL_ANS;
  m_serializedSize = 2;
}

void
NewChannelAns::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));

  start.WriteU8 ((uint8_t (m_dataRateRangeOk) << 1) |
                 uint8_t (m_channelFrequencyOk));
}

uint8_t
NewChannelAns::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();
  // Read the data
  uint8_t byte = start.ReadU8 ();
  m_dataRateRangeOk = (byte & 0b10) >> 1;
  m_channelFrequencyOk = (byte & 0b1);

  return m_serializedSize;
}

void
NewChannelAns::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "NewChannelAns" << std::endl;
  os << "DataRateRangeOk: " << m_dataRateRangeOk << std::endl;
  os << "ChannelFrequencyOk: " << m_channelFrequencyOk << std::endl;
}

//////////////////////
// RxTimingSetupReq //
//////////////////////

RxTimingSetupReq::RxTimingSetupReq ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = RX_TIMING_SETUP_REQ;
  m_serializedSize = 2;
}

RxTimingSetupReq::RxTimingSetupReq (uint8_t delay) :
  m_delay (delay)
{
  NS_LOG_FUNCTION (this);

  m_commandType = RX_TIMING_SETUP_REQ;
  m_serializedSize = 2;
}

void
RxTimingSetupReq::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
  // Write the data
  start.WriteU8 (m_delay & 0xf);
}

uint8_t
RxTimingSetupReq::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();
  // Read the data
  m_delay = start.ReadU8 () & 0xf;

  return m_serializedSize;
}

void
RxTimingSetupReq::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "RxTimingSetupReq" << std::endl;
}

Time
RxTimingSetupReq::GetDelay (void)
{
  NS_LOG_FUNCTION (this);

  if (m_delay == 0)
    {
      return Seconds (1);
    }
  return Seconds (m_delay);
}
//////////////////
// RxTimingSetupAns //
//////////////////

RxTimingSetupAns::RxTimingSetupAns ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = DEV_STATUS_REQ;
  m_serializedSize = 1;
}

void
RxTimingSetupAns::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
}

uint8_t
RxTimingSetupAns::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();

  return m_serializedSize;
}

void
RxTimingSetupAns::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "RxTimingSetupAns" << std::endl;
}

//////////////////
// DlChannelAns //
//////////////////

DlChannelAns::DlChannelAns ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = DEV_STATUS_REQ;
  m_serializedSize = 1;
}

void
DlChannelAns::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
}

uint8_t
DlChannelAns::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();

  return m_serializedSize;
}

void
DlChannelAns::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "DlChannelAns" << std::endl;
}

//////////////////
// TxParamSetupReq //
//////////////////

TxParamSetupReq::TxParamSetupReq ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = DEV_STATUS_REQ;
  m_serializedSize = 1;
}

void
TxParamSetupReq::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
}

uint8_t
TxParamSetupReq::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();

  return m_serializedSize;
}

void
TxParamSetupReq::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "TxParamSetupReq" << std::endl;
}

//////////////////
// TxParamSetupAns //
//////////////////

TxParamSetupAns::TxParamSetupAns ()
{
  NS_LOG_FUNCTION (this);

  m_commandType = DEV_STATUS_REQ;
  m_serializedSize = 1;
}

void
TxParamSetupAns::Serialize (Buffer::Iterator &start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Write the CID
  start.WriteU8 (GetCIDFromMacCommand (m_commandType));
}

uint8_t
TxParamSetupAns::Deserialize (Buffer::Iterator &start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Consume the CID
  start.ReadU8 ();

  return m_serializedSize;
}

void
TxParamSetupAns::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "TxParamSetupAns" << std::endl;
}

}
}
