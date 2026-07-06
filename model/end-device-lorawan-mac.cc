/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *         Martina Capuzzo <capuzzom@dei.unipd.it>
 *
 * Modified by: Peggy Anderson <peggy.anderson@usask.ca>
 */

#include "end-device-lorawan-mac.h"

#include "lora-phy.h"

#include "ns3/energy-source-container.h"
#include "ns3/simulator.h"

#include <bitset>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("EndDeviceLorawanMac");

NS_OBJECT_ENSURE_REGISTERED(EndDeviceLorawanMac);

TypeId
EndDeviceLorawanMac::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::EndDeviceLorawanMac")
            .SetParent<LorawanMac>()
            .SetGroupName("lorawan")
            .AddTraceSource(
                "ConfirmedTransmissionOutcome",
                "Trace number of retransmissions for acknowledgement of confirmed packets",
                MakeTraceSourceAccessor(&EndDeviceLorawanMac::m_confirmedTxOutcomeCallback),
                "ns3::EndDeviceLorawanMac::ConfirmedTxOutcomeCallback")
            .AddAttribute("DataRate",
                          "Data rate currently employed by this end device",
                          UintegerValue(0),
                          MakeUintegerAccessor(&EndDeviceLorawanMac::m_dataRate),
                          MakeUintegerChecker<uint8_t>(0, 5))
            .AddTraceSource("DataRate",
                            "Data rate currently employed by this end device",
                            MakeTraceSourceAccessor(&EndDeviceLorawanMac::m_dataRate),
                            "ns3::TracedValueCallback::uint8_t")
            .AddAttribute(
                "ADR",
                "Ensure to the network server that this device will accept data rate, transmission "
                "power and number of retransmissions configurations received via LinkADRReq. This "
                "also allows the device's local ADR backoff procedure to reset configurations in "
                "case of connectivity loss.",
                BooleanValue(true),
                MakeBooleanAccessor(&EndDeviceLorawanMac::m_adr),
                MakeBooleanChecker())
            .AddTraceSource("TxPower",
                            "Transmission ERP [dBm] currently employed by this end device",
                            MakeTraceSourceAccessor(&EndDeviceLorawanMac::m_txPowerDbm),
                            "ns3::TracedValueCallback::Double")
            .AddTraceSource("LastKnownLinkMargin",
                            "Last known demodulation margin in "
                            "communications between this end device "
                            "and a gateway",
                            MakeTraceSourceAccessor(&EndDeviceLorawanMac::m_lastKnownLinkMarginDb),
                            "ns3::TracedValueCallback::uint8_t")
            .AddTraceSource("LastKnownGatewayCount",
                            "Last known number of gateways able to "
                            "listen to this end device",
                            MakeTraceSourceAccessor(&EndDeviceLorawanMac::m_lastKnownGatewayCount),
                            "ns3::TracedValueCallback::uint8_t")
            .AddTraceSource("AggregatedDutyCycle",
                            "Aggregate duty cycle, in fraction form, "
                            "this end device must respect",
                            MakeTraceSourceAccessor(&EndDeviceLorawanMac::m_aggregatedDutyCycle),
                            "ns3::TracedValueCallback::Double")
            .AddAttribute("MaxTransmissions",
                          "Maximum number of transmissions for a packet (NbTrans)",
                          IntegerValue(1),
                          MakeIntegerAccessor(&EndDeviceLorawanMac::m_nbTrans),
                          MakeIntegerChecker<uint8_t>(1, 15))
            .AddAttribute("FType",
                          "Specify type of message will be sent by this end device.",
                          EnumValue(LorawanMacHeader::FType::UNCONFIRMED_DATA_UP),
                          MakeEnumAccessor<LorawanMacHeader::FType>(&EndDeviceLorawanMac::m_fType),
                          MakeEnumChecker(LorawanMacHeader::FType::UNCONFIRMED_DATA_UP,
                                          "Unconfirmed",
                                          LorawanMacHeader::FType::CONFIRMED_DATA_UP,
                                          "Confirmed"));
    return tid;
}

EndDeviceLorawanMac::EndDeviceLorawanMac()
    : m_address(LoraDeviceAddress(0)), // LoraWAN default
      m_fCnt(0),
      m_adrAckCnt(0),
      m_dataRate(0),
      m_txPowerDbm(14),
      m_nbTrans(1),
      m_codingRate(CodingRate::CR_4_5),
      m_headerDisabled(false),
      m_receiveWindowDurationInSymbols(8),
      m_lastRxSnr(32), // Max initial value
      m_fType(LorawanMacHeader::FType::UNCONFIRMED_DATA_UP),
      m_adr(true),
      m_aggregatedDutyCycle(1),
      m_lastKnownLinkMarginDb(0),
      m_lastKnownGatewayCount(0),
      m_adrAckReq(false)
{
    NS_LOG_FUNCTION(this);
    // Initialize random variable for channel selection
    m_uniformRV = CreateObject<UniformRandomVariable>();
    // Void the next transmission event
    m_nextTx = EventId();
    m_nextTx.Cancel();
}

EndDeviceLorawanMac::~EndDeviceLorawanMac()
{
    NS_LOG_FUNCTION_NOARGS();
}

void
EndDeviceLorawanMac::Send(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    // Check ability to send and compute delay without touching the internal device state
    Time nextTxDelay;
    if (ValidatePacketForSend(packet, nextTxDelay) == false)
    {
        NS_LOG_ERROR("Packet cannot be sent in the current device state, transmission aborted.");
        return;
    }

    // We are sending this packet: overwrite any previously queued transmissions if any
    m_nextTx.Cancel();

    // If it is not possible to transmit now because of the duty cycle or because we are currently
    // in the process of sending/receiving another packet, schedule a tx/retx later
    if (nextTxDelay.IsStrictlyPositive())
    {
        NS_LOG_WARN("Attempting to send, but device is busy or duty cycle won't allow it. "
                    "Rescheduling a tx in "
                    << nextTxDelay.As(Time::S) << ".");
        PostponeTransmission(nextTxDelay, packet);
        m_cannotSendBecauseDutyCycle(packet);
        return;
    }

    /////////////////////////////////////////////////////////////
    // From here on out, immediate pkt transmission is assured //
    /////////////////////////////////////////////////////////////

    DoSend(packet);
}

bool
EndDeviceLorawanMac::ValidatePacketForSend(Ptr<const Packet> packet, Time& nextTxDelay) const
{
    // Initialize output delay to max
    nextTxDelay = Time::Max();

    // Copy current tx parameters and simulate an update on them
    auto tmpDataRate = m_dataRate.Get();
    auto tmpTxPower = m_txPowerDbm.Get();
    auto tmpNbTrans = m_nbTrans;
    auto tmpTxChannels = m_channelHelper->GetRawChannelArray(); // shallow copy to get size
    for (auto& c : tmpTxChannels)
    {
        c = c ? Copy(c) : c; // deep copy
    }

    // Evaluate ADR backoff on copied parameters
    if (m_adr && packet != m_txContext.packet) // Is this a new packet?
    {
        // Is there an ongoing retransmission process that would be interrupted?
        uint16_t tmpAdrAckCnt = m_adrAckCnt + (m_txContext.nbTxLeft > 0);
        // Simulate ADR Backoff on temporary values
        if (tmpAdrAckCnt >= ADR_ACK_LIMIT + ADR_ACK_DELAY)
        {
            DoExecuteADRBackoff(tmpTxPower, tmpDataRate, tmpNbTrans, tmpTxChannels);
        }
    }

    // This check is influenced by ADR backoff. This is OK because (by LoRaWAN design) you
    // either use ADR and constrain your max app payload according to the default initial DR0,
    // or you disable ADR for a fixed data rate, with the possibility of using bigger payloads.
    if (IsPayloadSizeValid(packet->GetSize(), tmpDataRate) == false)
    {
        NS_LOG_WARN("Application payload exceeding maximum size.");
        return false;
    }

    // Check if there is a channel suitable for TX (checks data rate & tx power etc.)
    if (tmpTxChannels = GetCompatibleTxChannels(tmpTxChannels, tmpDataRate, tmpTxPower);
        tmpTxChannels.empty())
    {
        NS_LOG_WARN("No tx channel compatible with current DR/power.");
        return false;
    }

    // Evaluate min send delay and return true
    nextTxDelay = GetNextTransmissionDelay(tmpTxChannels);
    return true;
}

void
EndDeviceLorawanMac::AddMacCommand(Ptr<MacCommand> macCommand)
{
    NS_LOG_FUNCTION(this << macCommand);

    m_macCommandList.push_back(macCommand);
}

void
EndDeviceLorawanMac::SetUplinkAdrBit(bool adr)
{
    NS_LOG_FUNCTION(this << adr);
    m_adr = adr;
}

bool
EndDeviceLorawanMac::GetUplinkAdrBit() const
{
    NS_LOG_FUNCTION(this);
    return m_adr;
}

void
EndDeviceLorawanMac::SetMaxNumberOfTransmissions(uint8_t nbTrans)
{
    NS_LOG_FUNCTION(this << unsigned(nbTrans));
    m_nbTrans = nbTrans;
}

uint8_t
EndDeviceLorawanMac::GetMaxNumberOfTransmissions()
{
    NS_LOG_FUNCTION(this);
    return m_nbTrans;
}

void
EndDeviceLorawanMac::SetDataRate(uint8_t dataRate)
{
    NS_LOG_FUNCTION(this << unsigned(dataRate));

    m_dataRate = dataRate;
}

uint8_t
EndDeviceLorawanMac::GetDataRate()
{
    NS_LOG_FUNCTION(this);

    return m_dataRate;
}

void
EndDeviceLorawanMac::SetTransmissionPowerDbm(double txPowerDbm)
{
    NS_LOG_FUNCTION(this << txPowerDbm);
    m_txPowerDbm = txPowerDbm;
}

double
EndDeviceLorawanMac::GetTransmissionPowerDbm()
{
    NS_LOG_FUNCTION(this);
    return m_txPowerDbm;
}

void
EndDeviceLorawanMac::SetDeviceAddress(LoraDeviceAddress address)
{
    NS_LOG_FUNCTION(this << address);

    m_address = address;
}

LoraDeviceAddress
EndDeviceLorawanMac::GetDeviceAddress()
{
    NS_LOG_FUNCTION(this);

    return m_address;
}

uint8_t
EndDeviceLorawanMac::GetLastKnownLinkMarginDb() const
{
    return m_lastKnownLinkMarginDb;
}

uint8_t
EndDeviceLorawanMac::GetLastKnownGatewayCount() const
{
    return m_lastKnownGatewayCount;
}

double
EndDeviceLorawanMac::GetAggregatedDutyCycle()
{
    NS_LOG_FUNCTION_NOARGS();

    return m_aggregatedDutyCycle;
}

void
EndDeviceLorawanMac::SetFType(LorawanMacHeader::FType fType)
{
    m_fType = fType;
    NS_LOG_DEBUG("Message type is set to " << fType);
}

LorawanMacHeader::FType
EndDeviceLorawanMac::GetFType()
{
    return m_fType;
}

void
EndDeviceLorawanMac::PostponeTransmission(Time nextTxDelay, Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << nextTxDelay << packet);
    m_nextTx = Simulator::Schedule(nextTxDelay, &EndDeviceLorawanMac::Send, this, packet);
}

Ptr<LogicalLoraChannel>
EndDeviceLorawanMac::GetRandomChannelForTx()
{
    NS_LOG_FUNCTION(this);
    /// @todo possibly move to LogicalChannelHelper
    auto channels = m_channelHelper->GetRawChannelArray();
    auto compatible = GetCompatibleTxChannels(channels, m_dataRate, m_txPowerDbm);
    std::vector<Ptr<LogicalLoraChannel>> candidates;
    for (const auto& c : compatible)
    {
        if (m_channelHelper->GetWaitTime(c).IsZero())
        {
            candidates.emplace_back(c);
        }
    }
    if (candidates.empty())
    {
        NS_LOG_DEBUG("No suitable TX channel found");
        return nullptr;
    }
    uint8_t i = m_uniformRV->GetInteger(0, candidates.size() - 1);
    auto channel = candidates.at(i);
    NS_LOG_DEBUG("Selected channel with frequency=" << channel->GetFrequency() << "Hz");
    return channel;
}

void
EndDeviceLorawanMac::ApplyMACCommands(LoraFrameHeader frameHeader)
{
    NS_LOG_FUNCTION(this << frameHeader);
    // Parse and apply downlink MAC commands, queue answers
    for (const auto& c : frameHeader.GetCommands())
    {
        NS_LOG_DEBUG("Iterating over the MAC commands...");
        enum MacCommandType type = (c)->GetCommandType();
        switch (type)
        {
        case (LINK_CHECK_ANS): {
            NS_LOG_DEBUG("Detected a LinkCheckAns command.");
            auto linkCheckAns = DynamicCast<LinkCheckAns>(c);
            OnLinkCheckAns(linkCheckAns->GetMargin(), linkCheckAns->GetGwCnt());
            break;
        }
        case (LINK_ADR_REQ): {
            NS_LOG_DEBUG("Detected a LinkAdrReq command.");
            auto linkAdrReq = DynamicCast<LinkAdrReq>(c);
            OnLinkAdrReq(linkAdrReq->GetDataRate(),
                         linkAdrReq->GetTxPower(),
                         linkAdrReq->GetChMask(),
                         linkAdrReq->GetChMaskCntl(),
                         linkAdrReq->GetNbTrans());
            break;
        }
        case (DUTY_CYCLE_REQ): {
            NS_LOG_DEBUG("Detected a DutyCycleReq command.");
            auto dutyCycleReq = DynamicCast<DutyCycleReq>(c);
            OnDutyCycleReq(dutyCycleReq->GetMaxDutyCycle());
            break;
        }
        case (RX_PARAM_SETUP_REQ): {
            NS_LOG_DEBUG("Detected a RxParamSetupReq command.");
            auto rxParamSetupReq = DynamicCast<RxParamSetupReq>(c);
            OnRxParamSetupReq(rxParamSetupReq->GetRx1DrOffset(),
                              rxParamSetupReq->GetRx2DataRate(),
                              rxParamSetupReq->GetFrequency());
            break;
        }
        case (DEV_STATUS_REQ): {
            NS_LOG_DEBUG("Detected a DevStatusReq command.");
            auto devStatusReq = DynamicCast<DevStatusReq>(c);
            OnDevStatusReq();
            break;
        }
        case (NEW_CHANNEL_REQ): {
            NS_LOG_DEBUG("Detected a NewChannelReq command.");
            auto newChannelReq = DynamicCast<NewChannelReq>(c);
            OnNewChannelReq(newChannelReq->GetChannelIndex(),
                            newChannelReq->GetFrequency(),
                            newChannelReq->GetMinDataRate(),
                            newChannelReq->GetMaxDataRate());
            break;
        }
        case (RX_TIMING_SETUP_REQ):
        case (TX_PARAM_SETUP_REQ):
        case (DL_CHANNEL_REQ):
        default: {
            NS_LOG_ERROR("CID not recognized or supported");
            break;
        }
        }
    }
}

void
EndDeviceLorawanMac::DoExecuteADRBackoff(double& txPowerDbm,
                                         uint8_t& dataRate,
                                         uint8_t& nbTrans,
                                         const std::vector<Ptr<LogicalLoraChannel>>& txChannelArray)
{
    // Adapted from: github.com/Lora-net/SWL2001.git v4.8.0
    // For the time being, this implementation is valid for the EU868 region

    if (txPowerDbm < 14)
    {
        txPowerDbm = 14; // Reset transmission power to default
        return;
    }

    if (dataRate != 0)
    {
        dataRate--;
        return;
    }

    // Set nbTrans to 1 and re-enable default channels
    nbTrans = 1;
    txChannelArray.at(0)->EnableForUplink();
    txChannelArray.at(1)->EnableForUplink();
    txChannelArray.at(2)->EnableForUplink();
}

bool
EndDeviceLorawanMac::IsPayloadSizeValid(uint32_t appPayloadSize, uint8_t dataRate) const
{
    NS_LOG_FUNCTION(this << appPayloadSize << unsigned(dataRate));
    uint32_t fOptsLen = 0;
    for (const auto& c : m_macCommandList)
    {
        fOptsLen += c->GetSerializedSize();
    }
    /// TODO: FPort could be absent
    uint32_t macPayloadSize = 7 + fOptsLen + 1 + appPayloadSize;
    uint32_t maxMacPayloadForDataRate = m_maxMacPayloadForDataRate.at(dataRate);
    NS_LOG_DEBUG("macPayloadSize=" << macPayloadSize << "B, maxMacPayloadForDataRate="
                                   << maxMacPayloadForDataRate << "B");
    return macPayloadSize <= maxMacPayloadForDataRate;
}

std::vector<Ptr<LogicalLoraChannel>>
EndDeviceLorawanMac::GetCompatibleTxChannels(
    const std::vector<Ptr<LogicalLoraChannel>>& txChannelArray,
    uint8_t dataRate,
    double txPowerDbm) const
{
    NS_LOG_FUNCTION(this);
    /// @todo possibly move to LogicalChannelHelper
    std::vector<Ptr<LogicalLoraChannel>> candidates;
    for (const auto& channel : txChannelArray)
    {
        if (channel && channel->IsEnabledForUplink()) // Skip empty frequency channel slots
        {
            uint8_t minDr = channel->GetMinimumDataRate();
            uint8_t maxDr = channel->GetMaximumDataRate();
            double maxTxPower = m_channelHelper->GetTxPowerForChannel(channel);
            NS_LOG_DEBUG("Enabled channel: frequency=" << channel->GetFrequency()
                                                       << "Hz, minDr=" << unsigned(minDr)
                                                       << ", maxDr=" << unsigned(maxDr)
                                                       << ", maxTxPower=" << maxTxPower << "dBm");
            if (dataRate >= minDr && dataRate <= maxDr && txPowerDbm <= maxTxPower)
            {
                candidates.emplace_back(channel);
            }
        }
    }
    return candidates;
}

Time
EndDeviceLorawanMac::GetNextTransmissionDelay(
    const std::vector<Ptr<LogicalLoraChannel>>& txChannelArray) const
{
    NS_LOG_FUNCTION(this);
    // Check duty cycle on provided channels
    auto waitTime = Time::Max();
    for (const auto& c : txChannelArray)
    {
        auto channelWait = m_channelHelper->GetWaitTime(c);
        NS_LOG_LOGIC("frequency=" << c->GetFrequency() << "Hz, "
                                  << "waitTime=" << channelWait.As(Time::S));
        waitTime = Min(waitTime, channelWait);
    }
    NS_LOG_DEBUG("Current minimum duty-cycle wait time is " << waitTime.As(Time::S));

    /// TODO: Check aggregated duty cycle imposed by server

    // Check if we need to postpone more (overridden function!)
    waitTime = Max(waitTime, GetNextClassTransmissionDelay());

    return waitTime;
}

void
EndDeviceLorawanMac::DoSend(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    // Store whether this is a new packet (the context may be overwritten)
    bool packetIsNew = (packet != m_txContext.packet);

    if (packetIsNew) // Transmission of a new packet
    {
        NS_LOG_DEBUG("New FRMPayload from application: " << packet->GetSize() << "B");
        // If re-transmission process of last packet was interrupted, update frame counters
        if (m_txContext.nbTxLeft > 0)
        {
            NS_LOG_DEBUG("Stopping active retransmission process");
            // Update frame counter and ADRACKCnt (normally updated after exhausting all reTxs)
            m_fCnt++;
            m_adrAckCnt++;
            // If needed, trace failed ACKnowledgement of previous packet
            if (m_txContext.needsAck)
            {
                uint8_t txs = m_nbTrans - m_txContext.nbTxLeft;
                NS_LOG_WARN("Previous packet not acknowledged, used "
                            << unsigned(txs) << " transmissions out of " << unsigned(m_nbTrans));
                m_confirmedTxOutcomeCallback(txs,
                                             false,
                                             m_txContext.firstAttempt,
                                             m_txContext.packet);
            }
        }
        // Reset (re)transmission context
        m_txContext = PacketTxContext{
            .packet = packet,
            .firstAttempt = Simulator::Now(),
            .needsAck = (m_fType == LorawanMacHeader::FType::CONFIRMED_DATA_UP),
            .nbTxLeft = int8_t(m_nbTrans),
        };
    }
    else // Retransmission
    {
        // Retransmissions must be scheduled by parent classes only if nbTxLeft > 0
        NS_ASSERT_MSG(m_txContext.nbTxLeft > 0, "No more retransmissions for this packet");
        NS_LOG_DEBUG("Retransmitting an old packet.");
        // Remove obsolete headers
        LorawanMacHeader macHdr;
        packet->RemoveHeader(macHdr);
        LoraFrameHeader frameHdr;
        packet->RemoveHeader(frameHdr);
    }

    // Evaluate ADR backoff as in LoRaWAN specification, V1.0.4 (2020)
    // Adapted from: github.com/Lora-net/SWL2001.git v4.8.0
    m_adrAckReq = (m_adrAckCnt >= ADR_ACK_LIMIT); // Set the ADRACKReq bit in frame header
    if (m_adrAckCnt >= ADR_ACK_LIMIT + ADR_ACK_DELAY)
    {
        // Unreachable by retransmissions: they do not increase ADRACKCnt
        ExecuteADRBackoff();
        m_adrAckCnt = ADR_ACK_LIMIT;
    }
    NS_ASSERT(m_adrAckCnt < 2400);

    // Add the Lora Frame Header to the packet
    LoraFrameHeader frameHdr;
    ApplyNecessaryOptions(frameHdr);
    packet->AddHeader(frameHdr);
    NS_LOG_INFO("Added frame header of size " << frameHdr.GetSerializedSize() << " bytes.");
    // Add the Lora Mac header to the packet
    LorawanMacHeader macHdr;
    ApplyNecessaryOptions(macHdr);
    packet->AddHeader(macHdr);
    NS_LOG_INFO("Added MAC header of size " << macHdr.GetSerializedSize() << " bytes.");

    /// TODO: Add MIC

    // Send packet
    SendToPhy(packet);
    // Decrease the number of transmissions left
    m_txContext.nbTxLeft--;
    // Fire trace source
    if (packetIsNew)
    {
        m_sentNewPacket(packet);
    }
}

void
EndDeviceLorawanMac::ExecuteADRBackoff()
{
    NS_LOG_FUNCTION(this);

    // Adapted from: github.com/Lora-net/SWL2001.git v4.8.0
    // For the time being, this implementation is valid for the EU868 region

    if (!m_adr)
    {
        return;
    }

    // TracedValue are not easily passed by reference
    double txPowerDbm = m_txPowerDbm.Get();
    uint8_t dataRate = m_dataRate.Get();
    DoExecuteADRBackoff(txPowerDbm, dataRate, m_nbTrans, m_channelHelper->GetRawChannelArray());
    m_txPowerDbm = txPowerDbm;
    m_dataRate = dataRate;
}

void
EndDeviceLorawanMac::ApplyNecessaryOptions(LoraFrameHeader& frameHeader)
{
    frameHeader.SetAsUplink();
    frameHeader.SetFPort(1); // TODO Use an appropriate frame port based on the application
    frameHeader.SetAddress(m_address);
    frameHeader.SetAdr(m_adr);
    frameHeader.SetAdrAckReq(m_adrAckReq);

    // FPending does not exist in uplink messages
    frameHeader.SetFCnt(m_fCnt);

    // Add listed MAC commands
    for (const auto& command : m_macCommandList)
    {
        NS_LOG_INFO("Applying a MAC Command of CID "
                    << unsigned(MacCommand::GetCIDFromMacCommand(command->GetCommandType())));

        frameHeader.AddCommand(command);
    }

    NS_LOG_DEBUG(frameHeader);
}

void
EndDeviceLorawanMac::ApplyNecessaryOptions(LorawanMacHeader& macHeader)
{
    macHeader.SetFType(m_fType);
    macHeader.SetMajor(1);

    NS_LOG_DEBUG(macHeader);
}

void
EndDeviceLorawanMac::OnLinkCheckAns(uint8_t margin, uint8_t gwCnt)
{
    NS_LOG_FUNCTION(this << unsigned(margin) << unsigned(gwCnt));

    m_lastKnownLinkMarginDb = margin;
    m_lastKnownGatewayCount = gwCnt;
}

void
EndDeviceLorawanMac::OnLinkAdrReq(uint8_t dataRate,
                                  uint8_t txPower,
                                  uint16_t chMask,
                                  uint8_t chMaskCntl,
                                  uint8_t nbTrans)
{
    NS_LOG_FUNCTION(this << unsigned(dataRate) << unsigned(txPower) << std::bitset<16>(chMask)
                         << unsigned(chMaskCntl) << unsigned(nbTrans));

    // Adapted from: github.com/Lora-net/SWL2001.git v4.3.1
    // For the time being, this implementation is valid for the EU868 region

    NS_ASSERT_MSG(!(dataRate & 0xF0), "dataRate field > 4 bits");
    NS_ASSERT_MSG(!(txPower & 0xF0), "txPower field > 4 bits");
    NS_ASSERT_MSG(!(chMaskCntl & 0xF8), "chMaskCntl field > 3 bits");
    NS_ASSERT_MSG(!(nbTrans & 0xF0), "nbTrans field > 4 bits");

    auto channels = m_channelHelper->GetRawChannelArray();

    bool channelMaskAck = true;
    bool dataRateAck = true;
    bool powerAck = true;

    NS_LOG_DEBUG("Channel mask = " << std::bitset<16>(chMask)
                                   << ", ChMaskCtrl = " << unsigned(chMaskCntl));

    // Check channel mask
    switch (chMaskCntl)
    {
    // Channels 0 to 15
    case 0:
        // Check if all enabled channels have a valid frequency
        for (size_t i = 0; i < channels.size(); ++i)
        {
            if ((chMask & 0b1 << i) && !channels.at(i))
            {
                NS_LOG_WARN("Invalid channel mask");
                channelMaskAck = false;
                break; // break for loop
            }
        }
        break;
    // All channels ON independently of the ChMask field value
    case 6:
        chMask = 0b0;
        for (size_t i = 0; i < channels.size(); ++i)
        {
            if (channels.at(i))
            {
                chMask |= 0b1 << i;
            }
        }
        break;
    default:
        NS_LOG_WARN("Invalid channel mask ctrl field");
        channelMaskAck = false;
        break;
    }

    // check if all channels are disabled
    if (chMask == 0)
    {
        NS_LOG_WARN("Invalid channel mask");
        channelMaskAck = false;
    }

    // Temporary channel mask is built and validated
    if (!m_adr) // ADR disabled, only consider channel mask conf.
    {
        /// @remark Original code considers this to be mobile-mode
        if (channelMaskAck) // valid channel mask
        {
            bool compatible = false;
            // Look for enabled channel that supports current data rate.
            for (size_t i = 0; i < channels.size(); ++i)
            {
                if ((chMask & 0b1 << i) && m_dataRate >= channels.at(i)->GetMinimumDataRate() &&
                    m_dataRate <= channels.at(i)->GetMaximumDataRate())
                { // Found compatible channel, break loop
                    compatible = true;
                    break;
                }
            }
            if (!compatible)
            {
                NS_LOG_WARN("Invalid channel mask for current device data rate (ADR off)");
                channelMaskAck = dataRateAck = powerAck = false; // reject all configurations
            }
            else // apply channel mask configuration
            {
                for (size_t i = 0; i < channels.size(); ++i)
                {
                    if (auto c = channels.at(i); c)
                    {
                        (chMask & 0b1 << i) ? c->EnableForUplink() : c->DisableForUplink();
                    }
                }
                dataRateAck = powerAck = false; // only ack channel mask
            }
        }
        else // reject
        {
            NS_LOG_WARN("Invalid channel mask");
            dataRateAck = powerAck = false; // reject all configurations
        }
    }
    else // Server-side ADR is enabled
    {
        if (dataRate != 0xF) // If value is 0xF, ignore config.
        {
            bool compatible = false;
            // Look for enabled channel that supports config. data rate.
            for (size_t i = 0; i < channels.size(); ++i)
            {
                if (chMask & 0b1 << i) // all enabled by chMask, even if it was invalid
                {
                    if (const auto& c = channels.at(i); c) // exists
                    {
                        if (dataRate >= c->GetMinimumDataRate() &&
                            dataRate <= c->GetMaximumDataRate())
                        { // Found compatible channel, break loop
                            compatible = true;
                            break;
                        }
                    }
                    else // manages invalid case, checks with defaults
                    {
                        if (GetSfFromDataRate(dataRate) && GetBandwidthFromDataRate(dataRate))
                        { // Found compatible (invalid) channel, break loop
                            compatible = true;
                            break;
                        }
                    }
                }
            }
            // Check if it is acceptable
            if (!compatible)
            {
                NS_LOG_WARN("Invalid data rate");
                dataRateAck = false;
            }
        }

        if (txPower != 0xF) // If value is 0xF, ignore config.
        {
            // Check if it is acceptable
            if (GetDbmForTxPower(txPower) < 0)
            {
                NS_LOG_WARN("Invalid tx power");
                powerAck = false;
            }
        }

        // If no error, apply configurations
        if (channelMaskAck && dataRateAck && powerAck)
        {
            for (size_t i = 0; i < channels.size(); ++i)
            {
                if (auto c = channels.at(i); c)
                {
                    (chMask & 0b1 << i) ? c->EnableForUplink() : c->DisableForUplink();
                }
            }
            if (txPower != 0xF) // If value is 0xF, ignore config.
            {
                m_txPowerDbm = GetDbmForTxPower(txPower);
            }
            m_nbTrans = (nbTrans == 0) ? 1 : nbTrans;
            if (dataRate != 0xF) // If value is 0xF, ignore config.
            {
                m_dataRate = dataRate;
            }
            NS_LOG_DEBUG("MacTxDataRateAdr = " << unsigned(m_dataRate));
            NS_LOG_DEBUG("MacTxPower = " << unsigned(m_txPowerDbm) << "dBm");
            NS_LOG_DEBUG("MacNbTrans = " << unsigned(m_nbTrans));
        }
    }

    NS_LOG_INFO("Adding LinkAdrAns reply");
    m_macCommandList.emplace_back(Create<LinkAdrAns>(powerAck, dataRateAck, channelMaskAck));
}

void
EndDeviceLorawanMac::OnDutyCycleReq(uint8_t maxDutyCycle)
{
    NS_LOG_FUNCTION(this << unsigned(maxDutyCycle));
    NS_ASSERT_MSG(!(maxDutyCycle & 0xF0), "maxDutyCycle > 4 bits");
    m_aggregatedDutyCycle = 1 / std::pow(2, maxDutyCycle);
    NS_LOG_INFO("Adding DutyCycleAns reply");
    m_macCommandList.emplace_back(Create<DutyCycleAns>());
}

void
EndDeviceLorawanMac::OnDevStatusReq()
{
    NS_LOG_FUNCTION(this);

    uint8_t battery = 255; // could not measure
    if (m_device && m_device->GetNode())
    {
        if (auto sc = m_device->GetNode()->GetObject<energy::EnergySourceContainer>();
            sc && sc->GetN() == 1)
        {
            battery = sc->Get(0)->GetEnergyFraction() * 253 + 1.5; // range 1-254
        }
    }
    else
    {
        battery = 0; // external power source
    }

    // approximate to nearest integer
    double snr = round(m_lastRxSnr);
    // clamp value to boundaries
    snr = snr < -32 ? -32 : snr > 31 ? 31 : snr;
    // cast to 6-bit signed int and store in uint8_t
    uint8_t margin = std::bitset<6>(snr).to_ulong();

    NS_LOG_INFO("Adding DevStatusAns reply");
    m_macCommandList.emplace_back(Create<DevStatusAns>(battery, margin));
}

void
EndDeviceLorawanMac::OnNewChannelReq(uint8_t chIndex,
                                     uint32_t frequencyHz,
                                     uint8_t minDataRate,
                                     uint8_t maxDataRate)
{
    NS_LOG_FUNCTION(this << unsigned(chIndex) << frequencyHz << unsigned(minDataRate)
                         << unsigned(maxDataRate));

    NS_ASSERT_MSG(!(minDataRate & 0xF0), "minDataRate field > 4 bits");
    NS_ASSERT_MSG(!(maxDataRate & 0xF0), "maxDataRate field > 4 bits");

    // Adapted from: github.com/Lora-net/SWL2001.git v4.3.1
    // For the time being, this implementation is valid for the EU868 region

    bool dataRateRangeOk = true;
    bool channelFrequencyOk = true;

    // Valid Channel Index
    if (chIndex < 3 || chIndex > m_channelHelper->GetRawChannelArray().size() - 1)
    {
        NS_LOG_WARN("[WARNING] Invalid channel index");
        dataRateRangeOk = channelFrequencyOk = false;
    }

    // Valid Frequency
    if (frequencyHz != 0 && !m_channelHelper->IsFrequencyValid(frequencyHz))
    {
        NS_LOG_WARN("[WARNING] Invalid frequency");
        channelFrequencyOk = false;
    }

    // Valid DRMIN/MAX
    if (!GetSfFromDataRate(minDataRate) || !GetBandwidthFromDataRate(minDataRate))
    {
        NS_LOG_WARN("[WARNING] Invalid DR min");
        dataRateRangeOk = false;
    }

    if (!GetSfFromDataRate(maxDataRate) || !GetBandwidthFromDataRate(maxDataRate))
    {
        NS_LOG_WARN("[WARNING] Invalid DR max");
        dataRateRangeOk = false;
    }

    if (maxDataRate < minDataRate)
    {
        NS_LOG_WARN("[WARNING] Invalid DR max < DR min");
        dataRateRangeOk = false;
    }

    if (dataRateRangeOk && channelFrequencyOk)
    {
        auto channel = Create<LogicalLoraChannel>(frequencyHz, minDataRate, maxDataRate);
        (frequencyHz == 0) ? channel->DisableForUplink() : channel->EnableForUplink();
        m_channelHelper->SetChannel(chIndex, channel);
        NS_LOG_DEBUG("MacTxFrequency[" << unsigned(chIndex) << "]=" << frequencyHz
                                       << ", DrMin=" << unsigned(minDataRate)
                                       << ", DrMax=" << unsigned(maxDataRate));
    }

    NS_LOG_INFO("Adding NewChannelAns reply");
    m_macCommandList.emplace_back(Create<NewChannelAns>(dataRateRangeOk, channelFrequencyOk));
}

} // namespace lorawan
} // namespace ns3
