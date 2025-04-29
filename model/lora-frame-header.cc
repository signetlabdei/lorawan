/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "lora-frame-header.h"

#include "ns3/log.h"

#include <bitset>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraFrameHeader");

// Initialization list
LoraFrameHeader::LoraFrameHeader()
    : m_fPort(0),
      m_address(LoraDeviceAddress(0, 0)),
      m_adr(false),
      m_adrAckReq(false),
      m_ack(false),
      m_fPending(false),
      m_fOptsLen(0),
      m_fCnt(0)
{
}

LoraFrameHeader::~LoraFrameHeader()
{
}

TypeId
LoraFrameHeader::GetTypeId()
{
    static TypeId tid =
        TypeId("LoraFrameHeader").SetParent<Header>().AddConstructor<LoraFrameHeader>();
    return tid;
}

TypeId
LoraFrameHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
LoraFrameHeader::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    // 4 for DevAddr + 1 for FCtrl + 2 for FCnt + 1 for FPort + 0-15 for FOpts
    uint32_t size = 8 + m_fOptsLen;
    NS_LOG_INFO("LoraFrameHeader serialized size: " << size);
    return size;
}

void
LoraFrameHeader::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION(this);

    // Device Address field
    start.WriteU32(m_address.Get());

    // fCtrl field
    //
    // TODO FCtrl has different meanings for UL and DL packets. Handle this
    // correctly here.
    uint8_t fCtrl = 0;
    fCtrl |= uint8_t(m_adr << 7 & 0b10000000);
    fCtrl |= uint8_t(m_adrAckReq << 6 & 0b1000000);
    fCtrl |= uint8_t(m_ack << 5 & 0b100000);
    fCtrl |= uint8_t(m_fPending << 4 & 0b10000);
    fCtrl |= m_fOptsLen & 0b1111;
    start.WriteU8(fCtrl);

    // FCnt field
    start.WriteU16(m_fCnt);

    // FOpts field
    for (const auto& c : m_macCommands)
    {
        NS_LOG_DEBUG("Serializing a MAC command");
        c->Serialize(start);
    }

    // FPort
    start.WriteU8(m_fPort);
}

uint32_t
LoraFrameHeader::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this);

    // Empty the list of MAC commands
    m_macCommands.clear();

    // Read from buffer and save into local variables
    m_address.Set(start.ReadU32());
    // TODO FCtrl has different meanings for UL and DL packets. Handle this
    // correctly here.
    uint8_t fCtrl = start.ReadU8();
    m_adr = (fCtrl >> 7) & 0b1;
    m_adrAckReq = (fCtrl >> 6) & 0b1;
    m_ack = (fCtrl >> 5) & 0b1;
    m_fPending = (fCtrl >> 4) & 0b1;
    m_fOptsLen = fCtrl & 0b1111;
    m_fCnt = start.ReadU16();

    // Deserialize MAC commands
    NS_LOG_DEBUG("Starting deserialization of MAC commands");
    for (uint8_t byteNumber = 0; byteNumber < m_fOptsLen;)
    {
        uint8_t cid = start.PeekU8();
        NS_LOG_DEBUG("CID: " << unsigned(cid));

        // Divide Uplink and Downlink messages
        // This needs to be done because they have the same CID, and the context
        // about where this message will be Serialized/Deserialized (i.e., at the
        // end device or at the network server) is umportant.
        Ptr<MacCommand> command;
        if (m_isUplink)
        {
            switch (cid)
            {
            // In the case of Uplink messages, the network server will deserialize the
            // request for a link check
            case (0x02): {
                NS_LOG_DEBUG("Creating a LinkCheckReq command");
                command = Create<LinkCheckReq>();
                break;
            }
            case (0x03): {
                NS_LOG_DEBUG("Creating a LinkAdrAns command");
                command = Create<LinkAdrAns>();
                break;
            }
            case (0x04): {
                NS_LOG_DEBUG("Creating a DutyCycleAns command");
                command = Create<DutyCycleAns>();
                break;
            }
            case (0x05): {
                NS_LOG_DEBUG("Creating a RxParamSetupAns command");
                command = Create<RxParamSetupAns>();
                break;
            }
            case (0x06): {
                NS_LOG_DEBUG("Creating a DevStatusAns command");
                command = Create<DevStatusAns>();
                break;
            }
            case (0x07): {
                NS_LOG_DEBUG("Creating a NewChannelAns command");
                command = Create<NewChannelAns>();
                break;
            }
            case (0x08): {
                NS_LOG_DEBUG("Creating a RxTimingSetupAns command");
                command = Create<RxTimingSetupAns>();
                break;
            }
            case (0x09): {
                NS_LOG_DEBUG("Creating a TxParamSetupAns command");
                command = Create<TxParamSetupAns>();
                break;
            }
            case (0x0A): {
                NS_LOG_DEBUG("Creating a DlChannelAns command");
                command = Create<DlChannelAns>();
                break;
            }
            default: {
                NS_LOG_ERROR("CID not recognized during deserialization");
            }
            }
        }
        else
        {
            switch (cid)
            {
            // In the case of Downlink messages, the end device will deserialize the
            // answer to a link check
            case (0x02): {
                NS_LOG_DEBUG("Creating a LinkCheckAns command");
                command = Create<LinkCheckAns>();
                break;
            }
            case (0x03): {
                NS_LOG_DEBUG("Creating a LinkAdrReq command");
                command = Create<LinkAdrReq>();
                break;
            }
            case (0x04): {
                NS_LOG_DEBUG("Creating a DutyCycleReq command");
                command = Create<DutyCycleReq>();
                break;
            }
            case (0x05): {
                NS_LOG_DEBUG("Creating a RxParamSetupReq command");
                command = Create<RxParamSetupReq>();
                break;
            }
            case (0x06): {
                NS_LOG_DEBUG("Creating a DevStatusReq command");
                command = Create<DevStatusReq>();
                break;
            }
            case (0x07): {
                NS_LOG_DEBUG("Creating a NewChannelReq command");
                command = Create<NewChannelReq>();
                break;
            }
            case (0x08): {
                NS_LOG_DEBUG("Creating a RxTimingSetupReq command");
                command = Create<RxTimingSetupReq>();
                break;
            }
            case (0x09): {
                NS_LOG_DEBUG("Creating a TxParamSetupReq command");
                command = Create<TxParamSetupReq>();
                break;
            }
            default: {
                NS_LOG_ERROR("CID not recognized during deserialization");
            }
            }
        }
        byteNumber += command->Deserialize(start);
        m_macCommands.emplace_back(command);
    }
    m_fPort = start.ReadU8();
    return 8 + m_fOptsLen; // the number of bytes consumed.
}

void
LoraFrameHeader::Print(std::ostream& os) const
{
    os << "Address=" << m_address.Print();
    os << ", ADR=" << m_adr;
    os << ", ADRAckReq=" << m_adrAckReq;
    os << ", ACK=" << m_ack;
    os << ", FPending=" << m_fPending;
    os << ", FOptsLen=" << unsigned(m_fOptsLen);
    os << ", FCnt=" << unsigned(m_fCnt);
    for (const auto& c : m_macCommands)
    {
        os << ", ";
        c->Print(os);
    }
    os << ", FPort=" << unsigned(m_fPort);
}

void
LoraFrameHeader::SetAsUplink()
{
    NS_LOG_FUNCTION(this);
    m_isUplink = true;
}

void
LoraFrameHeader::SetAsDownlink()
{
    NS_LOG_FUNCTION(this);
    m_isUplink = false;
}

void
LoraFrameHeader::SetFPort(uint8_t fPort)
{
    m_fPort = fPort;
}

uint8_t
LoraFrameHeader::GetFPort() const
{
    return m_fPort;
}

void
LoraFrameHeader::SetAddress(LoraDeviceAddress address)
{
    m_address = address;
}

LoraDeviceAddress
LoraFrameHeader::GetAddress() const
{
    return m_address;
}

void
LoraFrameHeader::SetAdr(bool adr)
{
    NS_LOG_FUNCTION(this << adr);
    m_adr = adr;
}

bool
LoraFrameHeader::GetAdr() const
{
    return m_adr;
}

void
LoraFrameHeader::SetAdrAckReq(bool adrAckReq)
{
    m_adrAckReq = adrAckReq;
}

bool
LoraFrameHeader::GetAdrAckReq() const
{
    return m_adrAckReq;
}

void
LoraFrameHeader::SetAck(bool ack)
{
    NS_LOG_FUNCTION(this << ack);
    m_ack = ack;
}

bool
LoraFrameHeader::GetAck() const
{
    return m_ack;
}

void
LoraFrameHeader::SetFPending(bool fPending)
{
    m_fPending = fPending;
}

bool
LoraFrameHeader::GetFPending() const
{
    return m_fPending;
}

uint8_t
LoraFrameHeader::GetFOptsLen() const
{
    uint8_t fOptsLen = 0;
    for (const auto& c : m_macCommands)
    {
        fOptsLen += c->GetSerializedSize();
    }
    return fOptsLen;
}

void
LoraFrameHeader::SetFCnt(uint16_t fCnt)
{
    m_fCnt = fCnt;
}

uint16_t
LoraFrameHeader::GetFCnt() const
{
    return m_fCnt;
}

void
LoraFrameHeader::AddLinkCheckReq()
{
    NS_LOG_FUNCTION(this);
    auto command = Create<LinkCheckReq>();
    m_macCommands.emplace_back(command);
    m_fOptsLen += command->GetSerializedSize();
}

void
LoraFrameHeader::AddLinkCheckAns(uint8_t margin, uint8_t gwCnt)
{
    NS_LOG_FUNCTION(this << unsigned(margin) << unsigned(gwCnt));
    auto command = Create<LinkCheckAns>(margin, gwCnt);
    m_macCommands.emplace_back(command);
    m_fOptsLen += command->GetSerializedSize();
}

void
LoraFrameHeader::AddLinkAdrReq(uint8_t dataRate,
                               uint8_t txPower,
                               std::list<int> enabledChannels,
                               int repetitions)
{
    NS_LOG_FUNCTION(this << unsigned(dataRate) << txPower << repetitions);
    uint16_t channelMask = 0;
    for (const auto chId : enabledChannels)
    {
        NS_ASSERT(chId < 16 && chId > -1);
        channelMask |= 0b1 << chId;
    }
    /// @todo Implement chMaskCntl field
    auto command = Create<LinkAdrReq>(dataRate, txPower, channelMask, 0, repetitions);
    m_macCommands.emplace_back(command);
    m_fOptsLen += command->GetSerializedSize();
}

void
LoraFrameHeader::AddLinkAdrAns(bool powerAck, bool dataRateAck, bool channelMaskAck)
{
    NS_LOG_FUNCTION(this << powerAck << dataRateAck << channelMaskAck);
    auto command = Create<LinkAdrAns>(powerAck, dataRateAck, channelMaskAck);
    m_macCommands.emplace_back(command);
    m_fOptsLen += command->GetSerializedSize();
}

void
LoraFrameHeader::AddDutyCycleReq(uint8_t dutyCycle)
{
    NS_LOG_FUNCTION(this << unsigned(dutyCycle));
    auto command = Create<DutyCycleReq>(dutyCycle);
    m_macCommands.emplace_back(command);
    m_fOptsLen += command->GetSerializedSize();
}

void
LoraFrameHeader::AddDutyCycleAns()
{
    NS_LOG_FUNCTION(this);
    auto command = Create<DutyCycleAns>();
    m_macCommands.emplace_back(command);
    m_fOptsLen += command->GetSerializedSize();
}

void
LoraFrameHeader::AddRxParamSetupReq(uint8_t rx1DrOffset, uint8_t rx2DataRate, uint32_t frequencyHz)
{
    NS_LOG_FUNCTION(this << unsigned(rx1DrOffset) << unsigned(rx2DataRate) << frequencyHz);
    // Evaluate whether to eliminate this assert in case new offsets can be defined.
    NS_ASSERT(0 <= rx1DrOffset && rx1DrOffset <= 5);
    auto command = Create<RxParamSetupReq>(rx1DrOffset, rx2DataRate, frequencyHz);
    m_macCommands.emplace_back(command);
    m_fOptsLen += command->GetSerializedSize();
}

void
LoraFrameHeader::AddRxParamSetupAns()
{
    NS_LOG_FUNCTION(this);
    auto command = Create<RxParamSetupAns>();
    m_macCommands.emplace_back(command);
    m_fOptsLen += command->GetSerializedSize();
}

void
LoraFrameHeader::AddDevStatusReq()
{
    NS_LOG_FUNCTION(this);
    auto command = Create<DevStatusReq>();
    m_macCommands.emplace_back(command);
    m_fOptsLen += command->GetSerializedSize();
}

void
LoraFrameHeader::AddNewChannelReq(uint8_t chIndex,
                                  uint32_t frequencyHz,
                                  uint8_t minDataRate,
                                  uint8_t maxDataRate)
{
    NS_LOG_FUNCTION(this << unsigned(chIndex) << frequencyHz << unsigned(minDataRate)
                         << unsigned(maxDataRate));
    auto command = Create<NewChannelReq>(chIndex, frequencyHz, minDataRate, maxDataRate);
    m_macCommands.emplace_back(command);
    m_fOptsLen += command->GetSerializedSize();
}

std::vector<Ptr<MacCommand>>
LoraFrameHeader::GetCommands()
{
    NS_LOG_FUNCTION(this);
    return m_macCommands;
}

void
LoraFrameHeader::AddCommand(Ptr<MacCommand> macCommand)
{
    NS_LOG_FUNCTION(this << macCommand);
    m_macCommands.emplace_back(macCommand);
    m_fOptsLen += macCommand->GetSerializedSize();
}

} // namespace lorawan
} // namespace ns3
