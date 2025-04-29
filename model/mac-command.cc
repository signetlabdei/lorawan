/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "mac-command.h"

#include "ns3/log.h"

#include <bitset>
#include <cmath>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("MacCommand");

NS_OBJECT_ENSURE_REGISTERED(MacCommand);

TypeId
MacCommand::GetTypeId()
{
    static TypeId tid = TypeId("ns3::MacCommand").SetParent<Object>().SetGroupName("lorawan");
    return tid;
}

MacCommand::MacCommand()
{
    NS_LOG_FUNCTION(this);
}

MacCommand::~MacCommand()
{
    NS_LOG_FUNCTION(this);
}

enum MacCommandType
MacCommand::GetCommandType() const
{
    NS_LOG_FUNCTION(this);
    return m_commandType;
}

uint8_t
MacCommand::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    return m_serializedSize;
}

uint8_t
MacCommand::GetCIDFromMacCommand(enum MacCommandType commandType)
{
    NS_LOG_FUNCTION_NOARGS();
    switch (commandType)
    {
    case (INVALID): {
        return 0x0;
    }
    case (LINK_CHECK_REQ):
    case (LINK_CHECK_ANS): {
        return 0x02;
    }
    case (LINK_ADR_REQ):
    case (LINK_ADR_ANS): {
        return 0x03;
    }
    case (DUTY_CYCLE_REQ):
    case (DUTY_CYCLE_ANS): {
        return 0x04;
    }
    case (RX_PARAM_SETUP_REQ):
    case (RX_PARAM_SETUP_ANS): {
        return 0x05;
    }
    case (DEV_STATUS_REQ):
    case (DEV_STATUS_ANS): {
        return 0x06;
    }
    case (NEW_CHANNEL_REQ):
    case (NEW_CHANNEL_ANS): {
        return 0x07;
    }
    case (RX_TIMING_SETUP_REQ):
    case (RX_TIMING_SETUP_ANS): {
        return 0x08;
    }
    case (TX_PARAM_SETUP_REQ):
    case (TX_PARAM_SETUP_ANS): {
        return 0x09;
    }
    case (DL_CHANNEL_REQ):
    case (DL_CHANNEL_ANS): {
        return 0x0A;
    }
    }
    return 0;
}

//////////////////
// LinkCheckReq //
//////////////////

LinkCheckReq::LinkCheckReq()
{
    NS_LOG_FUNCTION(this);
    m_commandType = LINK_CHECK_REQ;
    m_serializedSize = 1;
}

void
LinkCheckReq::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
}

uint8_t
LinkCheckReq::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    return m_serializedSize;
}

void
LinkCheckReq::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "LinkCheckReq()";
}

//////////////////
// LinkCheckAns //
//////////////////

LinkCheckAns::LinkCheckAns()
{
    NS_LOG_FUNCTION(this);
    m_commandType = LINK_CHECK_ANS;
    m_serializedSize = 3;
}

LinkCheckAns::LinkCheckAns(uint8_t margin, uint8_t gwCnt)
    : m_margin(margin),
      m_gwCnt(gwCnt)
{
    NS_LOG_FUNCTION(this << unsigned(margin) << unsigned(gwCnt));
    m_commandType = LINK_CHECK_ANS;
    m_serializedSize = 3;
}

void
LinkCheckAns::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
    start.WriteU8(m_margin);                            // Write the margin
    start.WriteU8(m_gwCnt);                             // Write the gwCnt
}

uint8_t
LinkCheckAns::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    m_margin = start.ReadU8();
    m_gwCnt = start.ReadU8();
    return m_serializedSize;
}

void
LinkCheckAns::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "LinkCheckAns(";
    os << "Margin=" << unsigned(m_margin);
    os << ", GwCnt=" << unsigned(m_gwCnt);
    os << ")";
}

uint8_t
LinkCheckAns::GetMargin() const
{
    NS_LOG_FUNCTION(this);
    return m_margin;
}

uint8_t
LinkCheckAns::GetGwCnt() const
{
    NS_LOG_FUNCTION(this);

    return m_gwCnt;
}

////////////////
// LinkAdrReq //
////////////////

LinkAdrReq::LinkAdrReq()
{
    NS_LOG_FUNCTION(this);
    m_commandType = LINK_ADR_REQ;
    m_serializedSize = 5;
}

LinkAdrReq::LinkAdrReq(uint8_t dataRate,
                       uint8_t txPower,
                       uint16_t chMask,
                       uint8_t chMaskCntl,
                       uint8_t nbTrans)
    : m_dataRate(dataRate),
      m_txPower(txPower),
      m_chMask(chMask),
      m_chMaskCntl(chMaskCntl),
      m_nbTrans(nbTrans)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(!(dataRate & 0xF0), "dataRate field > 4 bits");
    NS_ASSERT_MSG(!(txPower & 0xF0), "txPower field > 4 bits");
    NS_ASSERT_MSG(!(chMaskCntl & 0xF8), "chMaskCntl field > 3 bits");
    NS_ASSERT_MSG(!(nbTrans & 0xF0), "nbTrans field > 4 bits");
    m_commandType = LINK_ADR_REQ;
    m_serializedSize = 5;
}

void
LinkAdrReq::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
    start.WriteU8(m_dataRate << 4 | (m_txPower & 0b1111));
    start.WriteU16(m_chMask);
    start.WriteU8(m_chMaskCntl << 4 | (m_nbTrans & 0b1111));
}

uint8_t
LinkAdrReq::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    uint8_t firstByte = start.ReadU8();
    m_dataRate = firstByte >> 4;
    m_txPower = firstByte & 0b1111;
    m_chMask = start.ReadU16();
    uint8_t fourthByte = start.ReadU8();
    m_chMaskCntl = fourthByte >> 4;
    m_nbTrans = fourthByte & 0b1111;
    return m_serializedSize;
}

void
LinkAdrReq::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "LinkAdrReq(";
    os << "DataRate=" << unsigned(m_dataRate);
    os << ", TXPower=" << unsigned(m_txPower);
    os << ", ChMask=" << std::bitset<16>(m_chMask);
    os << ", ChMaskCntl=" << unsigned(m_chMaskCntl);
    os << ", NbTrans=" << unsigned(m_nbTrans);
    os << ")";
}

uint8_t
LinkAdrReq::GetDataRate() const
{
    NS_LOG_FUNCTION(this);
    return m_dataRate;
}

uint8_t
LinkAdrReq::GetTxPower() const
{
    NS_LOG_FUNCTION(this);
    return m_txPower;
}

uint16_t
LinkAdrReq::GetChMask() const
{
    NS_LOG_FUNCTION(this);
    return m_chMask;
}

uint8_t
LinkAdrReq::GetChMaskCntl() const
{
    NS_LOG_FUNCTION(this);
    return m_chMaskCntl;
}

uint8_t
LinkAdrReq::GetNbTrans() const
{
    NS_LOG_FUNCTION(this);
    return m_nbTrans;
}

////////////////
// LinkAdrAns //
////////////////

LinkAdrAns::LinkAdrAns()
{
    NS_LOG_FUNCTION(this);
    m_commandType = LINK_ADR_ANS;
    m_serializedSize = 2;
}

LinkAdrAns::LinkAdrAns(bool powerAck, bool dataRateAck, bool channelMaskAck)
    : m_powerAck(powerAck),
      m_dataRateAck(dataRateAck),
      m_channelMaskAck(channelMaskAck)
{
    NS_LOG_FUNCTION(this);
    m_commandType = LINK_ADR_ANS;
    m_serializedSize = 2;
}

void
LinkAdrAns::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
    start.WriteU8((uint8_t(m_powerAck) << 2) | (uint8_t(m_dataRateAck) << 1) |
                  uint8_t(m_channelMaskAck));
}

uint8_t
LinkAdrAns::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    uint8_t byte = start.ReadU8();
    m_powerAck = byte & 0b100;
    m_dataRateAck = byte & 0b10;
    m_channelMaskAck = byte & 0b1;
    return m_serializedSize;
}

void
LinkAdrAns::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "LinkAdrAns(";
    os << "PowerACK=" << m_powerAck;
    os << ", DataRateACK=" << m_dataRateAck;
    os << ", ChannelMaskACK=" << m_channelMaskAck;
    os << ")";
}

bool
LinkAdrAns::GetPowerAck() const
{
    NS_LOG_FUNCTION(this);
    return m_powerAck;
}

bool
LinkAdrAns::GetDataRateAck() const
{
    NS_LOG_FUNCTION(this);
    return m_dataRateAck;
}

bool
LinkAdrAns::GetChannelMaskAck() const
{
    NS_LOG_FUNCTION(this);
    return m_channelMaskAck;
}

//////////////////
// DutyCycleReq //
//////////////////

DutyCycleReq::DutyCycleReq()
{
    NS_LOG_FUNCTION(this);
    m_commandType = DUTY_CYCLE_REQ;
    m_serializedSize = 2;
}

DutyCycleReq::DutyCycleReq(uint8_t maxDutyCycle)
{
    NS_LOG_FUNCTION(this << unsigned(maxDutyCycle));
    NS_ASSERT_MSG(!(maxDutyCycle & 0xF0), "maxDutyCycle > 4 bits");
    m_maxDutyCycle = maxDutyCycle;
    m_commandType = DUTY_CYCLE_REQ;
    m_serializedSize = 2;
}

void
DutyCycleReq::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
    start.WriteU8(m_maxDutyCycle);
}

uint8_t
DutyCycleReq::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    m_maxDutyCycle = start.ReadU8();
    return m_serializedSize;
}

void
DutyCycleReq::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "DutyCycleReq(";
    os << "MaxDutyCycle=" << unsigned(m_maxDutyCycle);
    os << ")";
}

uint8_t
DutyCycleReq::GetMaxDutyCycle() const
{
    NS_LOG_FUNCTION(this);
    return m_maxDutyCycle;
}

//////////////////
// DutyCycleAns //
//////////////////

DutyCycleAns::DutyCycleAns()
{
    NS_LOG_FUNCTION(this);
    m_commandType = DUTY_CYCLE_ANS;
    m_serializedSize = 1;
}

void
DutyCycleAns::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
}

uint8_t
DutyCycleAns::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    return m_serializedSize;
}

void
DutyCycleAns::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "DutyCycleAns()";
}

/////////////////////
// RxParamSetupReq //
/////////////////////

RxParamSetupReq::RxParamSetupReq()
{
    NS_LOG_FUNCTION(this);
    m_commandType = RX_PARAM_SETUP_REQ;
    m_serializedSize = 5;
}

RxParamSetupReq::RxParamSetupReq(uint8_t rx1DrOffset, uint8_t rx2DataRate, uint32_t frequencyHz)
    : m_rx1DrOffset(rx1DrOffset),
      m_rx2DataRate(rx2DataRate),
      m_frequencyHz(frequencyHz)
{
    NS_LOG_FUNCTION(this << unsigned(rx1DrOffset) << unsigned(rx2DataRate) << frequencyHz);
    NS_ASSERT_MSG(!(rx1DrOffset & 0xF8), "rx1DrOffset > 3 bits");
    NS_ASSERT_MSG(!(rx2DataRate & 0xF0), "rx2DataRate > 4 bits");
    m_commandType = RX_PARAM_SETUP_REQ;
    m_serializedSize = 5;
}

void
RxParamSetupReq::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
    start.WriteU8((m_rx1DrOffset & 0b111) << 4 | (m_rx2DataRate & 0b1111));
    uint32_t encodedFrequency = m_frequencyHz / 100;
    // Frequency is in little endian (lsb -> msb)
    start.WriteU8(encodedFrequency);       // Least significant byte
    start.WriteU8(encodedFrequency >> 8);  // Middle byte
    start.WriteU8(encodedFrequency >> 16); // Most significant byte
}

uint8_t
RxParamSetupReq::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    uint8_t firstByte = start.ReadU8();
    m_rx1DrOffset = (firstByte & 0b1110000) >> 4;
    m_rx2DataRate = firstByte & 0b1111;
    uint32_t encodedFrequency = 0;
    // Frequency is in little endian (lsb -> msb)
    encodedFrequency += start.ReadU8();       // Least significant byte
    encodedFrequency += start.ReadU8() << 8;  // Middle byte
    encodedFrequency += start.ReadU8() << 16; // Most significant byte
    m_frequencyHz = encodedFrequency * 100;
    return m_serializedSize;
}

void
RxParamSetupReq::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "RxParamSetupReq(";
    os << "RX1DROffset=" << unsigned(m_rx1DrOffset);
    os << ", RX2DataRate=" << unsigned(m_rx2DataRate);
    os << ", Frequency=" << m_frequencyHz;
    os << ")";
}

uint8_t
RxParamSetupReq::GetRx1DrOffset()
{
    NS_LOG_FUNCTION(this);
    return m_rx1DrOffset;
}

uint8_t
RxParamSetupReq::GetRx2DataRate()
{
    NS_LOG_FUNCTION(this);
    return m_rx2DataRate;
}

uint32_t
RxParamSetupReq::GetFrequency()
{
    NS_LOG_FUNCTION(this);
    return m_frequencyHz;
}

/////////////////////
// RxParamSetupAns //
/////////////////////

RxParamSetupAns::RxParamSetupAns()
{
    NS_LOG_FUNCTION(this);
    m_commandType = RX_PARAM_SETUP_ANS;
    m_serializedSize = 2;
}

RxParamSetupAns::RxParamSetupAns(bool rx1DrOffsetAck, bool rx2DataRateAck, bool channelAck)
    : m_rx1DrOffsetAck(rx1DrOffsetAck),
      m_rx2DataRateAck(rx2DataRateAck),
      m_channelAck(channelAck)
{
    NS_LOG_FUNCTION(this << rx1DrOffsetAck << rx2DataRateAck << channelAck);
    m_commandType = RX_PARAM_SETUP_ANS;
    m_serializedSize = 2;
}

void
RxParamSetupAns::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
    start.WriteU8(uint8_t(m_rx1DrOffsetAck) << 2 | uint8_t(m_rx2DataRateAck) << 1 |
                  uint8_t(m_channelAck));
}

uint8_t
RxParamSetupAns::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    uint8_t byte = start.ReadU8();
    m_rx1DrOffsetAck = (byte & 0b100) >> 2;
    m_rx2DataRateAck = (byte & 0b10) >> 1;
    m_channelAck = byte & 0b1;
    return m_serializedSize;
}

void
RxParamSetupAns::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "RxParamSetupAns(";
    os << "RX1DROffsetACK=" << m_rx1DrOffsetAck;
    os << ", RX2DataRateACK=" << m_rx2DataRateAck;
    os << ", ChannelACK=" << m_channelAck;
    os << ")";
}

bool
RxParamSetupAns::GetRx1DrOffsetAck() const
{
    NS_LOG_FUNCTION(this);
    return m_rx1DrOffsetAck;
}

bool
RxParamSetupAns::GetRx2DataRateAck() const
{
    NS_LOG_FUNCTION(this);
    return m_rx2DataRateAck;
}

bool
RxParamSetupAns::GetChannelAck() const
{
    NS_LOG_FUNCTION(this);
    return m_channelAck;
}

//////////////////
// DevStatusReq //
//////////////////

DevStatusReq::DevStatusReq()
{
    NS_LOG_FUNCTION(this);
    m_commandType = DEV_STATUS_REQ;
    m_serializedSize = 1;
}

void
DevStatusReq::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
}

uint8_t
DevStatusReq::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    return m_serializedSize;
}

void
DevStatusReq::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "DevStatusReq()";
}

//////////////////
// DevStatusAns //
//////////////////

DevStatusAns::DevStatusAns()
{
    NS_LOG_FUNCTION(this);
    m_commandType = DEV_STATUS_ANS;
    m_serializedSize = 3;
}

DevStatusAns::DevStatusAns(uint8_t battery, uint8_t margin)
    : m_battery(battery),
      m_margin(margin)
{
    NS_LOG_FUNCTION(this << unsigned(battery) << unsigned(margin));
    NS_ASSERT_MSG(!(margin & 0xC0), "margin > 6 bits");
    m_commandType = DEV_STATUS_ANS;
    m_serializedSize = 3;
}

void
DevStatusAns::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
    start.WriteU8(m_battery);
    start.WriteU8(m_margin);
}

uint8_t
DevStatusAns::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    m_battery = start.ReadU8();
    m_margin = start.ReadU8();
    return m_serializedSize;
}

void
DevStatusAns::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "DevStatusAns(";
    os << "Battery=" << unsigned(m_battery);
    os << ", Margin=" << unsigned(m_margin);
    os << ")";
}

uint8_t
DevStatusAns::GetBattery() const
{
    NS_LOG_FUNCTION(this);
    return m_battery;
}

uint8_t
DevStatusAns::GetMargin() const
{
    NS_LOG_FUNCTION(this);
    return m_margin;
}

//////////////////
// NewChannelReq //
//////////////////

NewChannelReq::NewChannelReq()
{
    NS_LOG_FUNCTION(this);
    m_commandType = NEW_CHANNEL_REQ;
    m_serializedSize = 6;
}

NewChannelReq::NewChannelReq(uint8_t chIndex,
                             uint32_t frequencyHz,
                             uint8_t minDataRate,
                             uint8_t maxDataRate)
    : m_chIndex(chIndex),
      m_frequencyHz(frequencyHz),
      m_minDataRate(minDataRate),
      m_maxDataRate(maxDataRate)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(!(minDataRate & 0xF0), "minDataRate > 4 bits");
    NS_ASSERT_MSG(!(maxDataRate & 0xF0), "maxDataRate > 4 bits");
    m_commandType = NEW_CHANNEL_REQ;
    m_serializedSize = 6;
}

void
NewChannelReq::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
    start.WriteU8(m_chIndex);
    uint32_t encodedFrequency = m_frequencyHz / 100;
    // Frequency is in little endian (lsb -> msb)
    start.WriteU8(encodedFrequency);       // Least significant byte
    start.WriteU8(encodedFrequency >> 8);  // Middle byte
    start.WriteU8(encodedFrequency >> 16); // Most significant byte
    start.WriteU8((m_maxDataRate << 4) | (m_minDataRate & 0xf));
}

uint8_t
NewChannelReq::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    m_chIndex = start.ReadU8();
    uint32_t encodedFrequency = 0;
    // Frequency is in little endian (lsb -> msb)
    encodedFrequency += start.ReadU8();       // Least significant byte
    encodedFrequency += start.ReadU8() << 8;  // Middle byte
    encodedFrequency += start.ReadU8() << 16; // Most significant byte
    m_frequencyHz = encodedFrequency * 100;
    uint8_t dataRateByte = start.ReadU8();
    m_maxDataRate = dataRateByte >> 4;
    m_minDataRate = dataRateByte & 0xf;
    return m_serializedSize;
}

void
NewChannelReq::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "NewChannelReq(";
    os << "ChIndex=" << unsigned(m_chIndex);
    os << ", Frequency=" << uint32_t(m_frequencyHz);
    os << ", MaxDR=" << unsigned(m_maxDataRate);
    os << ", MinDR=" << unsigned(m_minDataRate);
    os << ")";
}

uint8_t
NewChannelReq::GetChannelIndex() const
{
    NS_LOG_FUNCTION(this);
    return m_chIndex;
}

uint32_t
NewChannelReq::GetFrequency() const
{
    NS_LOG_FUNCTION(this);
    return m_frequencyHz;
}

uint8_t
NewChannelReq::GetMinDataRate() const
{
    NS_LOG_FUNCTION(this);
    return m_minDataRate;
}

uint8_t
NewChannelReq::GetMaxDataRate() const
{
    NS_LOG_FUNCTION(this);
    return m_maxDataRate;
}

///////////////////
// NewChannelAns //
///////////////////

NewChannelAns::NewChannelAns()
{
    NS_LOG_FUNCTION(this);
    m_commandType = NEW_CHANNEL_ANS;
    m_serializedSize = 2;
}

NewChannelAns::NewChannelAns(bool dataRateRangeOk, bool channelFrequencyOk)
    : m_dataRateRangeOk(dataRateRangeOk),
      m_channelFrequencyOk(channelFrequencyOk)
{
    NS_LOG_FUNCTION(this);
    m_commandType = NEW_CHANNEL_ANS;
    m_serializedSize = 2;
}

void
NewChannelAns::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
    start.WriteU8((uint8_t(m_dataRateRangeOk) << 1) | uint8_t(m_channelFrequencyOk));
}

uint8_t
NewChannelAns::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8();                // Consume the CID
    uint8_t byte = start.ReadU8(); // Read the data
    m_dataRateRangeOk = (byte & 0b10) >> 1;
    m_channelFrequencyOk = (byte & 0b1);
    return m_serializedSize;
}

void
NewChannelAns::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "NewChannelAns(";
    os << "DataRateRangeOk=" << m_dataRateRangeOk;
    os << ", ChannelFrequencyOk=" << m_channelFrequencyOk;
    os << ")";
}

bool
NewChannelAns::GetDataRateRangeOk() const
{
    NS_LOG_FUNCTION(this);
    return m_dataRateRangeOk;
}

bool
NewChannelAns::GetChannelFrequencyOk() const
{
    NS_LOG_FUNCTION(this);
    return m_channelFrequencyOk;
}

//////////////////////
// RxTimingSetupReq //
//////////////////////

RxTimingSetupReq::RxTimingSetupReq()
{
    NS_LOG_FUNCTION(this);
    m_commandType = RX_TIMING_SETUP_REQ;
    m_serializedSize = 2;
}

RxTimingSetupReq::RxTimingSetupReq(uint8_t delay)
    : m_delay(delay)
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(!(delay & 0xF0), "delay field > 4 bits");
    m_commandType = RX_TIMING_SETUP_REQ;
    m_serializedSize = 2;
}

void
RxTimingSetupReq::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
    start.WriteU8(m_delay & 0xF);                       // Write the data
}

uint8_t
RxTimingSetupReq::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8();                 // Consume the CID
    m_delay = start.ReadU8() & 0xF; // Read the data
    return m_serializedSize;
}

void
RxTimingSetupReq::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "RxTimingSetupReq()";
}

Time
RxTimingSetupReq::GetDelay()
{
    NS_LOG_FUNCTION(this);
    return Seconds((m_delay) ? m_delay : 0);
}

//////////////////
// RxTimingSetupAns //
//////////////////

RxTimingSetupAns::RxTimingSetupAns()
{
    NS_LOG_FUNCTION(this);
    m_commandType = DEV_STATUS_REQ;
    m_serializedSize = 1;
}

void
RxTimingSetupAns::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
}

uint8_t
RxTimingSetupAns::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    return m_serializedSize;
}

void
RxTimingSetupAns::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "RxTimingSetupAns()";
}

//////////////////
// DlChannelAns //
//////////////////

DlChannelAns::DlChannelAns()
{
    NS_LOG_FUNCTION(this);
    m_commandType = DEV_STATUS_REQ;
    m_serializedSize = 1;
}

void
DlChannelAns::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
}

uint8_t
DlChannelAns::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    return m_serializedSize;
}

void
DlChannelAns::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "DlChannelAns()";
}

//////////////////
// TxParamSetupReq //
//////////////////

TxParamSetupReq::TxParamSetupReq()
{
    NS_LOG_FUNCTION(this);
    m_commandType = DEV_STATUS_REQ;
    m_serializedSize = 1;
}

void
TxParamSetupReq::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
}

uint8_t
TxParamSetupReq::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    return m_serializedSize;
}

void
TxParamSetupReq::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "TxParamSetupReq()";
}

//////////////////
// TxParamSetupAns //
//////////////////

TxParamSetupAns::TxParamSetupAns()
{
    NS_LOG_FUNCTION(this);
    m_commandType = DEV_STATUS_REQ;
    m_serializedSize = 1;
}

void
TxParamSetupAns::Serialize(Buffer::Iterator& start) const
{
    NS_LOG_FUNCTION(this);
    start.WriteU8(GetCIDFromMacCommand(m_commandType)); // Write the CID
}

uint8_t
TxParamSetupAns::Deserialize(Buffer::Iterator& start)
{
    NS_LOG_FUNCTION(this);
    start.ReadU8(); // Consume the CID
    return m_serializedSize;
}

void
TxParamSetupAns::Print(std::ostream& os) const
{
    NS_LOG_FUNCTION(this);
    os << "TxParamSetupAns()";
}

} // namespace lorawan
} // namespace ns3
