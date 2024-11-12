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

#include "ns3/log.h"
#include "ns3/simulator.h"

#include <algorithm>
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
                            "Transmission power currently employed by this end device",
                            MakeTraceSourceAccessor(&EndDeviceLorawanMac::m_txPower),
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
      m_txPower(14),
      m_codingRate(1),
      // LoraWAN default
      m_headerDisabled(false),
      // LoraWAN default
      m_address(LoraDeviceAddress(0)),
      // LoraWAN default
      m_receiveWindowDurationInSymbols(8),
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
    if (netxTxDelay != Seconds(0))
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
        NS_ASSERT_MSG(m_txPower <= m_channelHelper->GetTxPowerForChannel(txChannel),
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
                << netxTxDelay.GetSeconds() << ".");
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
            m_retxParams.firstAttempt = Simulator::Now();
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
                         linkAdrReq->GetChMaskCtrl(),
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
EndDeviceLorawanMac::GetNextClassTransmissionDelay(Time waitingTime)
{
    NS_LOG_FUNCTION_NOARGS();
    return waitingTime;
}

Time
EndDeviceLorawanMac::GetNextTransmissionDelay()
{
    NS_LOG_FUNCTION_NOARGS();

    //    Check duty cycle    //

    // Pick a random channel to transmit on
    std::vector<Ptr<LogicalLoraChannel>> logicalChannels;
    logicalChannels =
        m_channelHelper->GetEnabledChannelList(); // Use a separate list to do the shuffle
    // logicalChannels = Shuffle (logicalChannels);

    Time waitingTime = Time::Max();

    // Try every channel
    std::vector<Ptr<LogicalLoraChannel>>::iterator it;
    for (it = logicalChannels.begin(); it != logicalChannels.end(); ++it)
    {
        // Pointer to the current channel
        Ptr<LogicalLoraChannel> logicalChannel = *it;
        double frequency = logicalChannel->GetFrequency();

        waitingTime = std::min(waitingTime, m_channelHelper->GetWaitingTime(logicalChannel));

        NS_LOG_DEBUG("Waiting time before the next transmission in channel with frequency "
                     << frequency << " is = " << waitingTime.GetSeconds() << ".");
    }

    waitingTime = GetNextClassTransmissionDelay(waitingTime);

    return waitingTime;
}

Ptr<LogicalLoraChannel>
EndDeviceLorawanMac::GetChannelForTx()
{
    NS_LOG_FUNCTION_NOARGS();

    // Pick a random channel to transmit on
    std::vector<Ptr<LogicalLoraChannel>> logicalChannels;
    logicalChannels =
        m_channelHelper->GetEnabledChannelList(); // Use a separate list to do the shuffle
    logicalChannels = Shuffle(logicalChannels);

    // Try every channel
    std::vector<Ptr<LogicalLoraChannel>>::iterator it;
    for (it = logicalChannels.begin(); it != logicalChannels.end(); ++it)
    {
        // Pointer to the current channel
        Ptr<LogicalLoraChannel> logicalChannel = *it;
        double frequency = logicalChannel->GetFrequency();

        NS_LOG_DEBUG("Frequency of the current channel: " << frequency);

        // Verify that we can send the packet
        Time waitingTime = m_channelHelper->GetWaitingTime(logicalChannel);

        NS_LOG_DEBUG("Waiting time for current channel = " << waitingTime.GetSeconds());

        // Send immediately if we can
        if (waitingTime == Seconds(0))
        {
            return *it;
        }
        else
        {
            NS_LOG_DEBUG("Packet cannot be immediately transmitted on "
                         << "the current channel because of duty cycle limitations.");
        }
    }
    return nullptr; // In this case, no suitable channel was found
}

std::vector<Ptr<LogicalLoraChannel>>
EndDeviceLorawanMac::Shuffle(std::vector<Ptr<LogicalLoraChannel>> vector)
{
    NS_LOG_FUNCTION_NOARGS();

    int size = vector.size();

    for (int i = 0; i < size; ++i)
    {
        uint16_t random = std::floor(m_uniformRV->GetValue(0, size));
        Ptr<LogicalLoraChannel> temp = vector.at(random);
        vector.at(random) = vector.at(i);
        vector.at(i) = temp;
    }

    return vector;
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
    m_retxParams.firstAttempt = Seconds(0);

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
                                  uint8_t chMaskCtrl,
                                  uint8_t nbTrans)
{
    NS_LOG_FUNCTION(this << unsigned(dataRate) << unsigned(txPower) << std::bitset<16>(chMask)
                         << unsigned(chMaskCtrl) << unsigned(nbTrans));

    // Adapted from: github.com/Lora-net/SWL2001.git v4.3.1
    // For the time being, this implementation is valid for the EU868 region

    NS_ASSERT_MSG(!(dataRate & 0xF0), "dataRate field > 4 bits");
    NS_ASSERT_MSG(!(txPower & 0xF0), "txPower field > 4 bits");
    NS_ASSERT_MSG(!(chMaskCtrl & 0xF8), "chMaskCtrl field > 3 bits");
    NS_ASSERT_MSG(!(nbTrans & 0xF0), "nbTrans field > 4 bits");

    auto channelList = m_channelHelper->GetChannelList();

    bool channelMaskAck = true;
    bool dataRateAck = true;
    bool powerAck = true;

    NS_LOG_DEBUG("Channel mask = " << std::bitset<16>(chMask)
                                   << ", ChMaskCtrl = " << unsigned(chMaskCtrl));

    // Check channel mask
    switch (chMaskCtrl)
    {
    // Channels 0 to 15
    case 0:
        // Check if all enabled channels have a valid frequency
        for (uint8_t i = 0; i < 16; ++i)
        {
            if ((chMask & 0b1 << i) && i >= channelList.size())
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
        for (uint8_t i = 0; i < 16; ++i)
        {
            if (i < channelList.size())
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
        if (channelMaskAck) // valid channel mask
        {
            bool compatible = false;
            // Look for enabled channel that supports current data rate.
            // Note: Original code checks for DR0 because this is considered mobile-mode
            for (uint8_t i = 0; i < 16; ++i)
            {
                if ((chMask & 0b1 << i) && m_dataRate >= channelList[i]->GetMinimumDataRate() &&
                    m_dataRate <= channelList[i]->GetMaximumDataRate())
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
                for (size_t i = 0; i < channelList.size(); ++i)
                {
                    (chMask & 0b1 << i) ? channelList[i]->SetEnabledForUplink()
                                        : channelList[i]->DisableForUplink();
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
            for (uint8_t i = 0; i < channelList.size() && i < 16; ++i)
            {
                if ((chMask & 0b1 << i) && dataRate >= channelList[i]->GetMinimumDataRate() &&
                    dataRate <= channelList[i]->GetMaximumDataRate())
                { // Found compatible channel, break loop
                    compatible = true;
                    break;
                }
            }
            // Check if it is acceptable
            if (!compatible || !GetSfFromDataRate(dataRate) || !GetBandwidthFromDataRate(dataRate))
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
            for (size_t i = 0; i < channelList.size(); ++i)
            {
                (chMask & 0b1 << i) ? channelList[i]->SetEnabledForUplink()
                                    : channelList[i]->DisableForUplink();
            }
            if (txPower != 0xF) // If value is 0xF, ignore config.
            {
                m_txPower = GetDbmForTxPower(txPower);
            }
            m_nbTrans = (nbTrans == 0) ? 1 : nbTrans;
            if (dataRate != 0xF) // If value is 0xF, ignore config.
            {
                m_dataRate = dataRate;
            }
            NS_LOG_DEBUG("MacTxDataRateAdr = " << unsigned(m_dataRate));
            NS_LOG_DEBUG("MacTxPower = " << unsigned(m_txPower) << "dBm");
            NS_LOG_DEBUG("MacNbTrans = " << unsigned(m_nbTrans));
        }
    }

    // Craft a LinkAdrAns MAC command as a response
    ///////////////////////////////////////////////
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

    uint8_t battery = 10; // XXX Fake battery level
    uint8_t margin = 10;  // XXX Fake margin

    // Craft a RxParamSetupAns as response
    NS_LOG_INFO("Adding DevStatusAns reply");
    m_macCommandList.emplace_back(Create<DevStatusAns>(battery, margin));
}

void
EndDeviceLorawanMac::OnNewChannelReq(uint8_t chIndex,
                                     double frequency,
                                     uint8_t minDataRate,
                                     uint8_t maxDataRate)
{
    NS_LOG_FUNCTION(this);

    bool dataRateRangeOk = true;    // XXX Check whether the new data rate range is ok
    bool channelFrequencyOk = true; // XXX Check whether the frequency is ok

    // TODO Return false if one of the checks above failed
    // TODO Create new channel in the LogicalLoraChannelHelper

    SetLogicalChannel(chIndex, frequency, minDataRate, maxDataRate);

    NS_LOG_INFO("Adding NewChannelAns reply");
    m_macCommandList.emplace_back(Create<NewChannelAns>(dataRateRangeOk, channelFrequencyOk));
}

void
EndDeviceLorawanMac::AddLogicalChannel(double frequency)
{
    NS_LOG_FUNCTION(this << frequency);

    m_channelHelper->AddChannel(frequency);
}

void
EndDeviceLorawanMac::AddLogicalChannel(Ptr<LogicalLoraChannel> logicalChannel)
{
    NS_LOG_FUNCTION(this << logicalChannel);

    m_channelHelper->AddChannel(logicalChannel);
}

void
EndDeviceLorawanMac::SetLogicalChannel(uint8_t chIndex,
                                       double frequency,
                                       uint8_t minDataRate,
                                       uint8_t maxDataRate)
{
    NS_LOG_FUNCTION(this << unsigned(chIndex) << frequency << unsigned(minDataRate)
                         << unsigned(maxDataRate));

    m_channelHelper->SetChannel(
        chIndex,
        CreateObject<LogicalLoraChannel>(frequency, minDataRate, maxDataRate));
}

void
EndDeviceLorawanMac::AddSubBand(double startFrequency,
                                double endFrequency,
                                double dutyCycle,
                                double maxTxPowerDbm)
{
    NS_LOG_FUNCTION_NOARGS();

    m_channelHelper->AddSubBand(startFrequency, endFrequency, dutyCycle, maxTxPowerDbm);
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

uint8_t
EndDeviceLorawanMac::GetTransmissionPower()
{
    return m_txPower;
}
} // namespace lorawan
} // namespace ns3
