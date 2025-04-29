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

#include "class-a-end-device-lorawan-mac.h"
#include "end-device-lora-phy.h"

#include "ns3/energy-source-container.h"
#include "ns3/log.h"
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
            .AddTraceSource("RequiredTransmissions",
                            "Total number of transmissions required to deliver this packet",
                            MakeTraceSourceAccessor(&EndDeviceLorawanMac::m_requiredTxCallback),
                            "ns3::TracedValueCallback::uint8_t")
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
                "power and number of retransmissions configurations received via LinkADRReq.",
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
                          MakeIntegerChecker<uint8_t>())
            .AddAttribute("EnableEDDataRateAdaptation",
                          "Whether the end device should up its data rate "
                          "in case it doesn't get a reply from the network server.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&EndDeviceLorawanMac::m_enableDRAdapt),
                          MakeBooleanChecker())
            .AddAttribute("MType",
                          "Specify type of message will be sent by this end device.",
                          EnumValue(LorawanMacHeader::UNCONFIRMED_DATA_UP),
                          MakeEnumAccessor<LorawanMacHeader::MType>(&EndDeviceLorawanMac::m_mType),
                          MakeEnumChecker(LorawanMacHeader::UNCONFIRMED_DATA_UP,
                                          "Unconfirmed",
                                          LorawanMacHeader::CONFIRMED_DATA_UP,
                                          "Confirmed"));
    return tid;
}

EndDeviceLorawanMac::EndDeviceLorawanMac()
    : m_enableDRAdapt(false),
      m_nbTrans(1),
      m_dataRate(0),
      m_txPowerDbm(14),
      m_codingRate(1),
      // LoraWAN default
      m_headerDisabled(false),
      // LoraWAN default
      m_address(LoraDeviceAddress(0)),
      // LoraWAN default
      m_receiveWindowDurationInSymbols(8),
      // Max initial value
      m_lastRxSnr(32),
      m_adr(true),
      m_lastKnownLinkMarginDb(0),
      m_lastKnownGatewayCount(0),
      m_aggregatedDutyCycle(1),
      m_mType(LorawanMacHeader::CONFIRMED_DATA_UP),
      m_currentFCnt(0)
{
    NS_LOG_FUNCTION(this);

    // Initialize the random variable we'll use to decide which channel to
    // transmit on.
    m_uniformRV = CreateObject<UniformRandomVariable>();

    // Void the transmission event
    m_nextTx = EventId();
    m_nextTx.Cancel();

    // Initialize structure for retransmission parameters
    m_retxParams = EndDeviceLorawanMac::LoraRetxParameters();
    m_retxParams.retxLeft = m_nbTrans;
}

EndDeviceLorawanMac::~EndDeviceLorawanMac()
{
    NS_LOG_FUNCTION_NOARGS();
}

////////////////////////
//  Sending methods   //
////////////////////////

void
EndDeviceLorawanMac::Send(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    // If it is not possible to transmit now because of the duty cycle,
    // or because we are receiving, schedule a tx/retx later

    Time netxTxDelay = GetNextTransmissionDelay();
    if (netxTxDelay.IsStrictlyPositive())
    {
        postponeTransmission(netxTxDelay, packet);
        return;
    }

    // Pick a channel on which to transmit the packet
    Ptr<LogicalLoraChannel> txChannel = GetChannelForTx();

    if (!(txChannel && m_retxParams.retxLeft > 0))
    {
        if (!txChannel)
        {
            m_cannotSendBecauseDutyCycle(packet);
        }
        else
        {
            NS_LOG_INFO("Max number of transmission achieved: packet not transmitted.");
        }
    }
    else
    // the transmitting channel is available and we have not run out the maximum number of
    // retransmissions
    {
        // Make sure we can transmit at the current power on this channel
        NS_ASSERT_MSG(m_txPowerDbm <= m_channelHelper->GetTxPowerForChannel(txChannel),
                      " The selected power is too high to be supported by this channel.");
        DoSend(packet);
    }
}

void
EndDeviceLorawanMac::postponeTransmission(Time netxTxDelay, Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);
    // Delete previously scheduled transmissions if any.
    Simulator::Cancel(m_nextTx);
    m_nextTx = Simulator::Schedule(netxTxDelay, &EndDeviceLorawanMac::DoSend, this, packet);
    NS_LOG_WARN("Attempting to send, but the aggregate duty cycle won't allow it. Scheduling a tx "
                "at a delay "
                << netxTxDelay.As(Time::S) << ".");
}

void
EndDeviceLorawanMac::DoSend(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this);
    // Checking if this is the transmission of a new packet
    if (packet != m_retxParams.packet)
    {
        NS_LOG_DEBUG(
            "Received a new packet from application. Resetting retransmission parameters.");
        m_currentFCnt++;
        NS_LOG_DEBUG("APP packet: " << packet << ".");

        // Add the Lora Frame Header to the packet
        LoraFrameHeader frameHdr;
        ApplyNecessaryOptions(frameHdr);
        packet->AddHeader(frameHdr);

        auto fhdrSize = frameHdr.GetSerializedSize();
        NS_LOG_INFO("Added frame header of size " << fhdrSize << " bytes.");

        // Check that MACPayload length is below the allowed maximum
        if (packet->GetSize() > m_maxAppPayloadForDataRate.at(m_dataRate))
        {
            NS_LOG_WARN("Attempting to send a packet larger than the maximum allowed"
                        << " size at this Data Rate (DR" << unsigned(m_dataRate)
                        << "). Transmission canceled.");
            return;
        }

        // Add the Lora Mac header to the packet
        LorawanMacHeader macHdr;
        ApplyNecessaryOptions(macHdr);
        packet->AddHeader(macHdr);

        // Reset MAC command list
        m_macCommandList.clear();

        if (m_retxParams.waitingAck)
        {
            // Call the callback to notify about the failure
            uint8_t txs = m_nbTrans - (m_retxParams.retxLeft);
            m_requiredTxCallback(txs, false, m_retxParams.firstAttempt, m_retxParams.packet);
            NS_LOG_DEBUG(" Received new packet from the application layer: stopping retransmission "
                         "procedure. Used "
                         << unsigned(txs) << " transmissions out of a maximum of "
                         << unsigned(m_nbTrans) << ".");
        }

        // Reset retransmission parameters
        resetRetransmissionParameters();

        // If this is the first transmission of a confirmed packet, save parameters for the
        // (possible) next retransmissions.
        if (m_mType == LorawanMacHeader::CONFIRMED_DATA_UP)
        {
            m_retxParams.packet = packet->Copy();
            m_retxParams.retxLeft = m_nbTrans;
            m_retxParams.waitingAck = true;
            m_retxParams.firstAttempt = Now();
            m_retxParams.retxLeft =
                m_retxParams.retxLeft - 1; // decreasing the number of retransmissions

            NS_LOG_DEBUG("Message type is " << m_mType);
            NS_LOG_DEBUG("It is a confirmed packet. Setting retransmission parameters and "
                         "decreasing the number of transmissions left.");

            auto mhdrSize = macHdr.GetSerializedSize();
            NS_LOG_INFO("Added MAC header of size " << mhdrSize << " bytes.");

            // Sent a new packet
            NS_LOG_DEBUG("Copied packet: " << m_retxParams.packet);
            m_sentNewPacket(m_retxParams.packet);

            // static_cast<ClassAEndDeviceLorawanMac*>(this)->SendToPhy (m_retxParams.packet);
            SendToPhy(m_retxParams.packet);
        }
        else
        {
            m_sentNewPacket(packet);
            // static_cast<ClassAEndDeviceLorawanMac*>(this)->SendToPhy (packet);
            SendToPhy(packet);
        }
    }
    // this is a retransmission
    else
    {
        if (m_retxParams.waitingAck)
        {
            // Remove the headers
            LorawanMacHeader macHdr;
            LoraFrameHeader frameHdr;
            packet->RemoveHeader(macHdr);
            packet->RemoveHeader(frameHdr);

            // Add the Lora Frame Header to the packet
            frameHdr = LoraFrameHeader();
            ApplyNecessaryOptions(frameHdr);
            packet->AddHeader(frameHdr);

            auto fhdrSize = frameHdr.GetSerializedSize();
            NS_LOG_INFO("Added frame header of size " << fhdrSize << " bytes.");

            // Add the Lorawan Mac header to the packet
            macHdr = LorawanMacHeader();
            ApplyNecessaryOptions(macHdr);
            packet->AddHeader(macHdr);
            m_retxParams.retxLeft =
                m_retxParams.retxLeft - 1; // decreasing the number of retransmissions
            NS_LOG_DEBUG("Retransmitting an old packet.");

            // static_cast<ClassAEndDeviceLorawanMac*>(this)->SendToPhy (m_retxParams.packet);
            SendToPhy(m_retxParams.packet);
        }
    }
}

void
EndDeviceLorawanMac::SendToPhy(Ptr<Packet> packet)
{
}

//////////////////////////
//  Receiving methods   //
//////////////////////////

void
EndDeviceLorawanMac::Receive(Ptr<const Packet> packet)
{
}

void
EndDeviceLorawanMac::FailedReception(Ptr<const Packet> packet)
{
}

void
EndDeviceLorawanMac::ParseCommands(LoraFrameHeader frameHeader)
{
    NS_LOG_FUNCTION(this << frameHeader);

    if (m_retxParams.waitingAck)
    {
        if (frameHeader.GetAck())
        {
            NS_LOG_INFO("The message is an ACK, not waiting for it anymore.");

            NS_LOG_DEBUG("Reset retransmission variables to default values and cancel "
                         "retransmission if already scheduled.");

            uint8_t txs = m_nbTrans - (m_retxParams.retxLeft);
            m_requiredTxCallback(txs, true, m_retxParams.firstAttempt, m_retxParams.packet);
            NS_LOG_DEBUG("Received ACK packet after "
                         << unsigned(txs) << " transmissions: stopping retransmission procedure. ");

            // Reset retransmission parameters
            resetRetransmissionParameters();
        }
        else
        {
            NS_LOG_ERROR(
                "Received downlink message not containing an ACK while we were waiting for it!");
        }
    }

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
EndDeviceLorawanMac::ApplyNecessaryOptions(LoraFrameHeader& frameHeader)
{
    NS_LOG_FUNCTION_NOARGS();

    frameHeader.SetAsUplink();
    frameHeader.SetFPort(1); // TODO Use an appropriate frame port based on the application
    frameHeader.SetAddress(m_address);
    frameHeader.SetAdr(m_adr);
    frameHeader.SetAdrAckReq(false); // TODO Set ADRACKREQ if a member variable is true

    // FPending does not exist in uplink messages
    frameHeader.SetFCnt(m_currentFCnt);

    // Add listed MAC commands
    for (const auto& command : m_macCommandList)
    {
        NS_LOG_INFO("Applying a MAC Command of CID "
                    << unsigned(MacCommand::GetCIDFromMacCommand(command->GetCommandType())));

        frameHeader.AddCommand(command);
    }
}

void
EndDeviceLorawanMac::ApplyNecessaryOptions(LorawanMacHeader& macHeader)
{
    NS_LOG_FUNCTION_NOARGS();

    macHeader.SetMType(m_mType);
    macHeader.SetMajor(1);
}

void
EndDeviceLorawanMac::SetMType(LorawanMacHeader::MType mType)
{
    m_mType = mType;
    NS_LOG_DEBUG("Message type is set to " << mType);
}

LorawanMacHeader::MType
EndDeviceLorawanMac::GetMType()
{
    return m_mType;
}

void
EndDeviceLorawanMac::TxFinished(Ptr<const Packet> packet)
{
}

Time
EndDeviceLorawanMac::GetNextClassTransmissionDelay(Time waitTime)
{
    NS_LOG_FUNCTION_NOARGS();
    return waitTime;
}

Time
EndDeviceLorawanMac::GetNextTransmissionDelay()
{
    NS_LOG_FUNCTION(this);
    // Check duty cycle
    /// @todo possibly move to LogicalChannelHelper
    auto waitTime = Time::Max();
    for (const auto& channel : m_channelHelper->GetRawChannelArray())
    {
        if (channel && channel->IsEnabledForUplink()) // Skip empty frequency channel slots
        {
            auto curr = m_channelHelper->GetWaitTime(channel);
            if (curr < waitTime)
            {
                waitTime = curr;
            }
            NS_LOG_DEBUG("frequency=" << channel->GetFrequency() << "Hz,"
                                      << " waitTime=" << waitTime.As(Time::S));
        }
    }
    return GetNextClassTransmissionDelay(waitTime);
}

Ptr<LogicalLoraChannel>
EndDeviceLorawanMac::GetChannelForTx()
{
    NS_LOG_FUNCTION(this);
    /// @todo possibly move to LogicalChannelHelper
    std::vector<Ptr<LogicalLoraChannel>> candidates;
    for (const auto& channel : m_channelHelper->GetRawChannelArray())
    {
        if (channel && channel->IsEnabledForUplink()) // Skip empty frequency channel slots
        {
            uint8_t minDr = channel->GetMinimumDataRate();
            uint8_t maxDr = channel->GetMaximumDataRate();
            Time waitTime = m_channelHelper->GetWaitTime(channel);
            NS_LOG_DEBUG("Enabled channel: frequency="
                         << channel->GetFrequency() << " Hz, minDr=" << unsigned(minDr)
                         << ", maxDr=" << unsigned(maxDr) << ", waitTime=" << waitTime.As(Time::S));
            if (m_dataRate >= minDr && m_dataRate <= maxDr && waitTime.IsZero())
            {
                candidates.emplace_back(channel);
            }
        }
    }
    if (candidates.empty())
    {
        NS_LOG_DEBUG("No suitable TX channel found");
        return nullptr;
    }
    uint8_t i = m_uniformRV->GetInteger(0, candidates.size() - 1);
    auto channel = candidates.at(i);
    NS_LOG_DEBUG("Selected channel with frequency=" << channel->GetFrequency() << " Hz");
    return channel;
}

/////////////////////////
// Setters and Getters //
/////////////////////////

void
EndDeviceLorawanMac::resetRetransmissionParameters()
{
    m_retxParams.waitingAck = false;
    m_retxParams.retxLeft = m_nbTrans;
    m_retxParams.packet = nullptr;
    m_retxParams.firstAttempt = Time(0);

    // Cancel next retransmissions, if any
    Simulator::Cancel(m_nextTx);
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
    m_retxParams.retxLeft = nbTrans;
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
EndDeviceLorawanMac::AddMacCommand(Ptr<MacCommand> macCommand)
{
    NS_LOG_FUNCTION(this << macCommand);

    m_macCommandList.push_back(macCommand);
}

} // namespace lorawan
} // namespace ns3
