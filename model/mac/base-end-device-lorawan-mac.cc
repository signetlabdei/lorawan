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
 *         Martina Capuzzo <capuzzom@dei.unipd.it>
 *
 * Modified by: Peggy Anderson <peggy.anderson@usask.ca>
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "base-end-device-lorawan-mac.h"

#include "ns3/end-device-lora-phy.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("BaseEndDeviceLorawanMac");

NS_OBJECT_ENSURE_REGISTERED(BaseEndDeviceLorawanMac);

TypeId
BaseEndDeviceLorawanMac::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::BaseEndDeviceLorawanMac")
            .SetParent<LorawanMac>()
            .SetGroupName("lorawan")
            .AddAttribute("DataRate",
                          "Data Rate currently employed by this end device",
                          UintegerValue(0),
                          MakeUintegerAccessor(&BaseEndDeviceLorawanMac::m_dataRate),
                          MakeUintegerChecker<uint8_t>(0, 5))
            .AddAttribute("ADRBit",
                          "Whether to request the NS to control this device's Data Rate",
                          BooleanValue(),
                          MakeBooleanAccessor(&BaseEndDeviceLorawanMac::m_ADRBit),
                          MakeBooleanChecker())
            .AddAttribute("NbTrans",
                          "Default number of transmissions for each packet",
                          IntegerValue(1),
                          MakeIntegerAccessor(&BaseEndDeviceLorawanMac::m_nbTrans),
                          MakeIntegerChecker<uint8_t>())
            .AddAttribute("ADRBackoff",
                          "Whether the End Device should up its Data Rate "
                          "in case it doesn't get a reply from the NS.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&BaseEndDeviceLorawanMac::m_enableADRBackoff),
                          MakeBooleanChecker())
            .AddAttribute("FType",
                          "Specify type of message will be sent by this ED.",
                          EnumValue(LorawanMacHeader::UNCONFIRMED_DATA_UP),
                          MakeEnumAccessor(&BaseEndDeviceLorawanMac::m_fType),
                          MakeEnumChecker(LorawanMacHeader::UNCONFIRMED_DATA_UP,
                                          "Unconfirmed",
                                          LorawanMacHeader::CONFIRMED_DATA_UP,
                                          "Confirmed"))
            .AddAttribute(
                "EnableCryptography",
                "Whether the End Device should compute the uplink Message Integrity Code, "
                "and decode the downlink payload according to specifications, i.e. using "
                "real cryptographic libraries (slower).",
                BooleanValue(false),
                MakeBooleanAccessor(&BaseEndDeviceLorawanMac::m_enableCrypto),
                MakeBooleanChecker())
            .AddTraceSource("RequiredTransmissions",
                            "Total number of transmissions required to deliver this packet",
                            MakeTraceSourceAccessor(&BaseEndDeviceLorawanMac::m_requiredTxCallback),
                            "ns3::TracedValueCallback::uint8_t")
            .AddTraceSource("DataRate",
                            "Data Rate currently employed by this end device",
                            MakeTraceSourceAccessor(&BaseEndDeviceLorawanMac::m_dataRate),
                            "ns3::TracedValueCallback::uint8_t")
            .AddTraceSource("TxPower",
                            "Transmission power currently employed by this end device",
                            MakeTraceSourceAccessor(&BaseEndDeviceLorawanMac::m_txPower),
                            "ns3::TracedValueCallback::Double")
            .AddTraceSource(
                "LastKnownLinkMargin",
                "Last known demodulation margin in "
                "communications between this end device "
                "and a gateway",
                MakeTraceSourceAccessor(&BaseEndDeviceLorawanMac::m_lastKnownLinkMargin),
                "ns3::TracedValueCallback::Double")
            .AddTraceSource(
                "LastKnownGatewayCount",
                "Last known number of gateways able to "
                "listen to this end device",
                MakeTraceSourceAccessor(&BaseEndDeviceLorawanMac::m_lastKnownGatewayCount),
                "ns3::TracedValueCallback::Int")
            .AddTraceSource(
                "AggregatedDutyCycle",
                "Aggregate duty cycle, in fraction form, "
                "this end device must respect",
                MakeTraceSourceAccessor(&BaseEndDeviceLorawanMac::m_aggregatedDutyCycle),
                "ns3::TracedValueCallback::Double");
    return tid;
}

BaseEndDeviceLorawanMac::BaseEndDeviceLorawanMac()
    // Protected MAC layer settings
    : m_dataRate(0),
      m_txPower(14),
      m_nbTrans(1),
      // Protected MAC layer context
      m_ADRACKCnt(0),
      // Private Header fields
      m_fType(LorawanMacHeader::UNCONFIRMED_DATA_UP),
      m_address(LoraDeviceAddress(0)),
      m_ADRBit(0),
      m_ADRACKReq(0),
      m_fCnt(0),
      // Private MAC layer settings
      m_enableADRBackoff(false),
      m_enableCrypto(false),
      m_aggregatedDutyCycle(1),
      // Private MAC layer context
      m_lastKnownLinkMargin(0),
      m_lastKnownGatewayCount(0)
{
    NS_LOG_FUNCTION(this);
    m_crypto = new LoRaMacCrypto();
    m_uniformRV = CreateObject<UniformRandomVariable>();
}

BaseEndDeviceLorawanMac::~BaseEndDeviceLorawanMac()
{
    NS_LOG_FUNCTION(this);
}

////////////////////////
//  Sending methods   //
////////////////////////

void
BaseEndDeviceLorawanMac::Send(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    // Delete previously scheduled transmissions if any.
    Simulator::Cancel(m_nextTx);

    // If it is not possible to transmit now, schedule a tx later
    if (Time nextTxDelay = GetNextTransmissionDelay(); nextTxDelay > Seconds(0))
    {
        m_cannotSendBecauseDutyCycle(packet);
        postponeTransmission(nextTxDelay, packet);
        NS_LOG_DEBUG("Attempting to send, but device is busy or duty cycle won't allow it. "
                     "Scheduling a tx in "
                     << nextTxDelay.As(Time::S) << ".");
        return;
    }

    DoSend(packet);
}

Time
BaseEndDeviceLorawanMac::GetNextTransmissionDelay()
{
    NS_LOG_FUNCTION(this);

    // Check legal duty cycle
    Time waitingTime = Time::Max();
    for (const auto& llc : m_channelManager->GetEnabledChannelList())
    {
        waitingTime = std::min(waitingTime, m_channelManager->GetWaitingTime(llc));
        NS_LOG_DEBUG("Waiting time before the next transmission in channel with frequecy "
                     << llc->GetFrequency() << " is = " << waitingTime.GetSeconds() << ".");
    }

    // Check if we are busy and if we need to postpone more (overridden function!)
    waitingTime = Max(waitingTime, GetBusyTransmissionDelay());

    // Check aggregated duty cycle imposed by server
    Time aggregatedDelay = m_channelManager->GetAggregatedWaitingTime(m_aggregatedDutyCycle);
    waitingTime = Max(waitingTime, aggregatedDelay);

    return waitingTime;
}

void
BaseEndDeviceLorawanMac::postponeTransmission(Time nextTxDelay, Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << nextTxDelay << packet);

    m_nextTx = Simulator::Schedule(nextTxDelay + NanoSeconds(10), &LorawanMac::Send, this, packet);
}

void
BaseEndDeviceLorawanMac::DoSend(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    bool packetIsNew = packet != m_txContext.packet;

    // If this is the transmission of a new packet, overwrite context
    if (packetIsNew)
    {
        // Tracing: previous packet was not acknowledged, reTxs procedure interrupted
        if (m_txContext.nbTxLeft && m_txContext.waitingAck)
        {
            // Call the callback to notify about the failure
            uint8_t txs = m_nbTrans - m_txContext.nbTxLeft;
            m_requiredTxCallback(txs, false, m_txContext.firstAttempt, m_txContext.packet);
            NS_LOG_DEBUG(" Received new packet from the application layer: stopping retransmission "
                         "procedure. Previous packet not acknowledged. Used "
                         << unsigned(txs) << " transmissions out of a maximum of "
                         << unsigned(m_nbTrans) << ".");
        }
        m_txContext = {Simulator::Now(),
                       packet,
                       m_nbTrans,
                       m_fType == LorawanMacHeader::CONFIRMED_DATA_UP,
                       false};
        NS_LOG_DEBUG("New APP packet: " << packet << ".");
    }
    else // Retransmission
    {
        // Remove MIC and headers
        packet->RemoveAtEnd(4);
        LorawanMacHeader mHdr;
        packet->RemoveHeader(mHdr);
        LoraFrameHeader fHdr;
        fHdr.SetAsUplink();
        packet->RemoveHeader(fHdr);
        NS_LOG_DEBUG("Retransmitting an old packet.");
    }

    if (m_enableADRBackoff)
    {
        // ADR backoff as in LoRaWAN specification, V1.0.4 (2020)
        ExecuteADRBackoff();
    }

    // Add the Lora Frame Header to the packet
    LoraFrameHeader fHdr;
    FillHeader(fHdr);
    packet->AddHeader(fHdr);
    NS_LOG_INFO("Added frame header of size " << (unsigned)fHdr.GetSerializedSize() << " bytes.");
    // Check that MACPayload length is below the allowed maximum
    if (packet->GetSize() > m_maxMacPayloadForDataRate.at(m_dataRate))
    {
        NS_LOG_ERROR("Attempting to send a packet ("
                     << (unsigned)packet->GetSize() << "B) larger than the maximum allowed"
                     << " size (" << (unsigned)m_maxMacPayloadForDataRate.at(m_dataRate)
                     << "B) at this DataRate (DR" << unsigned(m_dataRate)
                     << "). Transmission canceled.");
        return;
    }

    // Add the Lorawan Mac header to the packet
    NS_LOG_DEBUG("Message type is " << m_fType);
    LorawanMacHeader mHdr;
    FillHeader(mHdr);
    packet->AddHeader(mHdr);
    NS_LOG_INFO("Added MAC header of size " << mHdr.GetSerializedSize() << " bytes.");

    // Add (eventually encrypted) MIC to the end of the packet
    AddMIC(packet);

    // Set context to busy
    m_txContext.busy = true;
    SendToPhy(packet);
    // Decrease transmissions counter
    m_txContext.nbTxLeft--;
    if (packetIsNew)
    {
        // Increase frame counter
        m_fCnt++;
        if (m_ADRACKCnt < MAX_ADR_ACK_CNT) // overflow prevention
        {
            m_ADRACKCnt++;
        }
        // Fire trace source
        m_sentNewPacket(packet);
    }
}

void
BaseEndDeviceLorawanMac::ExecuteADRBackoff()
{
    NS_LOG_FUNCTION(this);

    // ADR backoff as in LoRaWAN specification, V1.0.4 (2020)
    if (m_ADRACKCnt == ADR_ACK_LIMIT)
    {
        m_ADRACKReq = 1; // Set the ADRACKReq bit in frame header
    }
    else if (m_ADRACKCnt == ADR_ACK_LIMIT + ADR_ACK_DELAY)
    {
        m_txPower = 14; // Reset transmission power to default
    }
    else if (m_ADRACKCnt > ADR_ACK_LIMIT && !((m_ADRACKCnt - ADR_ACK_LIMIT) % ADR_ACK_DELAY))
    {
        if (m_dataRate)
        {
            m_dataRate--; // Decrease data rate
        }
        else
        {
            // Enable default channels and set nbTrans to 1
            m_channelManager->GetChannel(0)->EnableForUplink();
            m_channelManager->GetChannel(1)->EnableForUplink();
            m_channelManager->GetChannel(2)->EnableForUplink();
            m_nbTrans = 1;
        }
    }
}

Ptr<LogicalChannel>
BaseEndDeviceLorawanMac::GetChannelForTx()
{
    NS_LOG_FUNCTION(this);

    auto channels = Shuffle(m_channelManager->GetEnabledChannelList());
    for (auto& llc : channels)
    {
        NS_LOG_DEBUG("Frequency of the current channel: " << llc->GetFrequency());

        // Verify that we can send the packet
        Time waitingTime = m_channelManager->GetWaitingTime(llc);
        NS_LOG_DEBUG("Waiting time for current channel = " << waitingTime.GetSeconds());

        // Send immediately if we can
        if (waitingTime == Seconds(0))
            return llc;
        else
            NS_LOG_DEBUG("Packet cannot be immediately transmitted on "
                         << "the current channel because of duty cycle limitations.");
    }
    return nullptr; // In this case, no suitable channel was found
}

std::vector<Ptr<LogicalChannel>>
BaseEndDeviceLorawanMac::Shuffle(std::vector<Ptr<LogicalChannel>> vector)
{
    NS_LOG_FUNCTION(this << vector);

    int size = vector.size();
    for (int i = 0; i < size; ++i)
    {
        uint8_t random = m_uniformRV->GetInteger(0, size - 1);
        auto tmp = vector.at(random);
        vector.at(random) = vector.at(i);
        vector.at(i) = tmp;
    }

    return vector;
}

////////////////////////
// MAC layer actions  //
////////////////////////

void
BaseEndDeviceLorawanMac::AddMacCommand(Ptr<MacCommand> macCommand)
{
    NS_LOG_FUNCTION(this << macCommand);

    m_fOpts.push_back(macCommand);
}

void
BaseEndDeviceLorawanMac::FillHeader(LoraFrameHeader& fHdr)
{
    NS_LOG_FUNCTION(this << fHdr);

    fHdr.SetAsUplink();
    fHdr.SetFPort(1); // TODO Use an appropriate frame port based on the application
    fHdr.SetAddress(m_address);
    fHdr.SetAdr(m_ADRBit);
    fHdr.SetAdrAckReq(m_ADRACKReq);

    // FPending does not exist in uplink messages
    fHdr.SetFCnt(m_fCnt);

    // Tmp list to save commands that need to be kept sent until downlink
    std::list<Ptr<MacCommand>> tmpCmdList;

    // Add listed MAC commands to header
    for (const auto& command : m_fOpts)
    {
        auto type = command->GetCommandType();
        NS_LOG_INFO("Applying a MAC Command of CID "
                    << unsigned(MacCommand::GetCIDFromMacCommand(type)));
        fHdr.AddCommand(command);
        // Keep sending them or not on next uplink (by specifications)
        if (type == MacCommandType::DL_CHANNEL_ANS || type == MacCommandType::RX_TIMING_SETUP_ANS)
        {
            tmpCmdList.push_back(command);
        }
    }

    // Reset MAC command list
    // (but leave DlChannelAns and RxTimingSetupAns)
    m_fOpts = tmpCmdList;
}

void
BaseEndDeviceLorawanMac::FillHeader(LorawanMacHeader& mHdr)
{
    NS_LOG_FUNCTION(this << mHdr);

    mHdr.SetFType(m_fType);
    mHdr.SetMajor(0);
}

void
BaseEndDeviceLorawanMac::AddMIC(Ptr<Packet> packet)
{
    // 4 Bytes of MIC
    uint32_t mic = 0;
    if (m_enableCrypto)
    {
        uint8_t buff[256];
        packet->CopyData(buff, 256);
        m_crypto->ComputeCmacB0(buff,
                                packet->GetSize(),
                                F_NWK_S_INT_KEY,
                                false,
                                UPLINK,
                                m_address.Get(),
                                m_fCnt,
                                &mic);
    }
    // Re-serialize message to add the MIC
    uint8_t micser[4];
    mempcpy(micser, &mic, 4);
    packet->AddAtEnd(Create<Packet>(micser, 4));
}

void
BaseEndDeviceLorawanMac::ApplyMACCommands(LoraFrameHeader fHdr, Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << fHdr << packet);

    // Parse the MAC commands
    NS_ASSERT_MSG(!(fHdr.GetFOptsLen() > 0 && fHdr.GetFPort() == 0),
                  "Error: FOptsLen > 0 and FPort == 0 (forbidden by specifications)");
    if (fHdr.GetFPort() == 0 && packet->GetSize() > 0) // Commands are in the FRMPayload
    {
        AppendCmdsFromFRMPayload(fHdr, packet);
    }

    // Parse and apply downlink MAC commands, queue answers
    for (auto& cmd : fHdr.GetCommands())
    {
        NS_LOG_DEBUG("Iterating over the MAC commands...");
        switch (cmd->GetCommandType())
        {
        case (LINK_CHECK_ANS): {
            NS_LOG_DEBUG("Detected a LinkCheckAns command.");
            // Cast the command
            auto linkCheckAns = DynamicCast<LinkCheckAns>(cmd);
            // Call the appropriate function to take action
            OnLinkCheckAns(linkCheckAns->GetMargin(), linkCheckAns->GetGwCnt());
            break;
        }
        case (LINK_ADR_REQ): {
            NS_LOG_DEBUG("Detected a LinkAdrReq command.");
            // Cast the command
            auto linkAdrReq = DynamicCast<LinkAdrReq>(cmd);
            // Call the appropriate function to take action
            OnLinkAdrReq(linkAdrReq->GetDataRate(),
                         linkAdrReq->GetTxPower(),
                         linkAdrReq->GetEnabledChannelsList(),
                         linkAdrReq->GetRepetitions());
            break;
        }
        case (DUTY_CYCLE_REQ): {
            NS_LOG_DEBUG("Detected a DutyCycleReq command.");
            // Cast the command
            auto dutyCycleReq = DynamicCast<DutyCycleReq>(cmd);
            // Call the appropriate function to take action
            OnDutyCycleReq(dutyCycleReq->GetMaximumAllowedDutyCycle());
            break;
        }
        case (RX_PARAM_SETUP_REQ): {
            NS_LOG_DEBUG("Detected a RxParamSetupReq command.");
            // Cast the command
            auto rxParamSetupReq = DynamicCast<RxParamSetupReq>(cmd);
            // Call the appropriate function to take action
            OnRxParamSetupReq(rxParamSetupReq);
            break;
        }
        case (DEV_STATUS_REQ): {
            NS_LOG_DEBUG("Detected a DevStatusReq command.");
            // Cast the command
            auto devStatusReq = DynamicCast<DevStatusReq>(cmd);
            // Call the appropriate function to take action
            OnDevStatusReq();
            break;
        }
        case (NEW_CHANNEL_REQ): {
            NS_LOG_DEBUG("Detected a NewChannelReq command.");
            // Cast the command
            auto newChannelReq = DynamicCast<NewChannelReq>(cmd);
            // Call the appropriate function to take action
            OnNewChannelReq(newChannelReq->GetChannelIndex(),
                            newChannelReq->GetFrequency(),
                            newChannelReq->GetMinDataRate(),
                            newChannelReq->GetMaxDataRate());
            break;
        }
        case (RX_TIMING_SETUP_REQ): {
            NS_LOG_DEBUG("Detected a RxTimingSetupReq command.");
            // Cast the command
            auto rxTimingSetupReq = DynamicCast<RxTimingSetupReq>(cmd);
            // Call the appropriate function to take action
            OnRxTimingSetupReq(rxTimingSetupReq->GetDelay());
            break;
        }
        case (TX_PARAM_SETUP_REQ): {
            /* Not mandatory in the EU868 region */
            break;
        }
        case (DL_CHANNEL_REQ): {
            NS_LOG_DEBUG("Detected a DlChannelReq command.");
            // Cast the command
            auto dlChannelReq = DynamicCast<DlChannelReq>(cmd);
            // Call the appropriate function to take action
            OnDlChannelReq(dlChannelReq->GetChannelIndex(), dlChannelReq->GetFrequency());
            break;
        }
        default: {
            NS_LOG_ERROR("CID not recognized");
            break;
        }
        }
    }
}

void
BaseEndDeviceLorawanMac::AppendCmdsFromFRMPayload(LoraFrameHeader& fHdr, Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << fHdr << packet);

    uint32_t size = packet->GetSize();
    NS_LOG_DEBUG("Commands in the FRMPayload. Size = " << (unsigned)size);
    uint8_t cmds[256];
    packet->CopyData(cmds, 256);

    /* Decrypt payload if enabled */
    if (m_enableCrypto)
    {
        char str[341];
        str[size - 1] = 0;
        for (uint32_t j = 0; j < size; j++)
        {
            sprintf(&str[2 * j], "%02X", cmds[j]);
        }
        NS_LOG_INFO("Encrypted payload: " << std::hex << str << std::dec);

        int result = m_crypto->PayloadEncrypt(cmds,
                                              size,
                                              F_NWK_S_INT_KEY,
                                              m_address.Get(),
                                              DOWNLINK,
                                              fHdr.GetFCnt());

        for (uint32_t j = 0; j < size; j++)
        {
            sprintf(&str[2 * j], "%02X", cmds[j]);
        }
        NS_LOG_INFO("Decryption result: " << result << ", payload: " << std::hex << str
                                          << std::dec);
    }

    //! Trigger alternative de/serialization
    fHdr.SetFRMPaylodCmdsLen(size);

    /* Append commands to the frame header and deserialize it again */
    auto buffer = Buffer();                      //! Create buffer
    buffer.AddAtStart(size);                     //! Allocate space for commands
    buffer.Begin().Write(cmds, size);            //! Add serialized payload with commands
    buffer.AddAtStart(fHdr.GetSerializedSize()); //! Allocate space for header
    fHdr.Serialize(buffer.Begin());              //! Add frame header (but not FPort)
    fHdr.Deserialize(buffer.Begin());
}

void
BaseEndDeviceLorawanMac::OnLinkCheckAns(uint8_t margin, uint8_t gwCnt)
{
    NS_LOG_FUNCTION(this << unsigned(margin) << unsigned(gwCnt));

    m_lastKnownLinkMargin = margin;
    m_lastKnownGatewayCount = gwCnt;
}

void
BaseEndDeviceLorawanMac::OnLinkAdrReq(uint8_t dataRate,
                                      uint8_t txPower,
                                      std::list<int> enabledChannels,
                                      int repetitions)
{
    NS_LOG_FUNCTION(this << unsigned(dataRate) << unsigned(txPower) << repetitions);

    // Three bools for three requirements before setting things up
    bool channelMaskOk = !enabledChannels.empty();
    bool dataRateOk = true;
    bool txPowerOk = true;

    // Check the channel mask
    /////////////////////////
    // Check whether all specified channels exist on this device
    for (auto& chIndex : enabledChannels)
        if (!m_channelManager->GetChannel(chIndex))
        {
            channelMaskOk = false;
            break;
        }

    // Check the dataRate
    /////////////////////
    // We need to know we can use it at all
    // To assess this, we try and convert it to a SF/BW combination and check if
    // those values are valid. Since GetSfFromDataRate and
    // GetBandwidthFromDataRate return 0 if the dataRate is not recognized, we
    // can check against this.
    uint8_t sf = GetSfFromDataRate(dataRate);
    double bw = GetBandwidthFromDataRate(dataRate);
    NS_LOG_DEBUG("SF: " << unsigned(sf) << ", BW: " << bw);
    if (sf == 0 || bw == 0)
    {
        dataRateOk = false;
        NS_LOG_DEBUG("Data rate non valid");
    }

    // We need to know we can use it in at least one of the enabled channels
    // Cycle through available channels, stop when at least one is enabled for the
    // specified dataRate.
    if (dataRateOk && channelMaskOk) // If false, skip the check
    {
        bool foundAvailableChannel = false;
        for (auto& chIndex : enabledChannels)
        {
            auto ch = m_channelManager->GetChannel(chIndex);
            NS_LOG_DEBUG("MinDR: " << unsigned(ch->GetMinimumDataRate()));
            NS_LOG_DEBUG("MaxDR: " << unsigned(ch->GetMaximumDataRate()));
            if (ch->GetMinimumDataRate() <= dataRate && ch->GetMaximumDataRate() >= dataRate)
            {
                foundAvailableChannel = true;
                break;
            }
        }

        if (!foundAvailableChannel)
        {
            dataRateOk = false;
            NS_LOG_DEBUG("Available channel not found");
        }
    }

    // Check the txPower
    ////////////////////
    // Check whether we can use this transmission power
    if (GetDbmForTxPower(txPower) == -1)
    {
        txPowerOk = false;
    }

    NS_LOG_DEBUG("Finished checking. "
                 << "ChannelMaskOk: " << channelMaskOk << ", "
                 << "DataRateOk: " << dataRateOk << ", "
                 << "txPowerOk: " << txPowerOk);

    // If all checks are successful, set parameters up
    //////////////////////////////////////////////////
    if (channelMaskOk && dataRateOk && txPowerOk)
    {
        // Cycle over all channels in the list
        for (uint32_t i = 0; i < m_channelManager->GetChannelList().size(); i++)
        {
            if (std::find(enabledChannels.begin(), enabledChannels.end(), i) !=
                enabledChannels.end())
            {
                m_channelManager->GetChannelList().at(i)->EnableForUplink();
                NS_LOG_DEBUG("Channel " << i << " enabled");
            }
            else
            {
                m_channelManager->GetChannelList().at(i)->DisableForUplink();
                NS_LOG_DEBUG("Channel " << i << " disabled");
            }
        }

        // Set the data rate
        m_dataRate = dataRate;

        // Set the transmission power
        m_txPower = GetDbmForTxPower(txPower);

        // Set the number of redundant transmissions
        m_nbTrans = repetitions;
    }

    // Craft a LinkAdrAns MAC command as a response
    ///////////////////////////////////////////////
    m_fOpts.push_back(Create<LinkAdrAns>(txPowerOk, dataRateOk, channelMaskOk));
}

void
BaseEndDeviceLorawanMac::OnDutyCycleReq(double dutyCycle)
{
    NS_LOG_FUNCTION(this << dutyCycle);

    // Make sure we get a value that makes sense
    NS_ASSERT(0 <= dutyCycle && dutyCycle <= 1);

    // Set the new duty cycle value
    m_aggregatedDutyCycle = dutyCycle;

    // Craft a DutyCycleAns as response
    NS_LOG_INFO("Adding DutyCycleAns reply");
    m_fOpts.push_back(Create<DutyCycleAns>());
}

void
BaseEndDeviceLorawanMac::OnDevStatusReq()
{
    NS_LOG_FUNCTION(this);

    uint8_t battery = 10; // XXX Fake battery level
    uint8_t margin = 10;  // XXX Fake margin

    // Craft a RxParamSetupAns as response
    NS_LOG_INFO("Adding DevStatusAns reply");
    m_fOpts.push_back(Create<DevStatusAns>(battery, margin));
}

void
BaseEndDeviceLorawanMac::OnNewChannelReq(uint8_t chIndex,
                                         double frequency,
                                         uint8_t minDataRate,
                                         uint8_t maxDataRate)
{
    NS_LOG_FUNCTION(this << unsigned(chIndex) << frequency << unsigned(minDataRate)
                         << unsigned(maxDataRate));

    // Check whether the new data rate range is ok
    bool dataRateRangeOk = (minDataRate >= 0 && maxDataRate <= 5);
    // Check whether the frequency is ok
    bool channelFrequencyOk = bool(m_channelManager->GetSubBandFromFrequency(frequency));
    if (dataRateRangeOk && channelFrequencyOk)
    {
        auto logicalChannel = Create<LogicalChannel>(frequency, minDataRate, maxDataRate);
        m_channelManager->AddChannel(chIndex, logicalChannel);
    }

    NS_LOG_INFO("Adding NewChannelAns reply");
    m_fOpts.push_back(Create<NewChannelAns>(dataRateRangeOk, channelFrequencyOk));
}

void
BaseEndDeviceLorawanMac::OnDlChannelReq(uint8_t chIndex, double frequency)
{
    NS_LOG_FUNCTION(this << unsigned(chIndex) << frequency);

    // Check whether the uplink frequency exists in this channel
    bool uplinkFrequencyExists = bool(m_channelManager->GetChannel(chIndex));

    // Check whether the downlink frequency can be used by this device
    bool channelFrequencyOk = bool(m_channelManager->GetSubBandFromFrequency(frequency));

    if (uplinkFrequencyExists && channelFrequencyOk)
    {
        m_channelManager->SetReplyFrequency(chIndex, frequency);
    }

    NS_LOG_INFO("Adding DlChannelAns reply");
    m_fOpts.push_back(Create<DlChannelAns>(uplinkFrequencyExists, channelFrequencyOk));
}

/////////////////////////
// Setters and Getters //
/////////////////////////

void
BaseEndDeviceLorawanMac::SetDeviceAddress(LoraDeviceAddress address)
{
    m_address = address;
}

LoraDeviceAddress
BaseEndDeviceLorawanMac::GetDeviceAddress()
{
    return m_address;
}

void
BaseEndDeviceLorawanMac::SetFType(LorawanMacHeader::FType fType)
{
    m_fType = fType;
    NS_LOG_DEBUG("Message type is set to " << fType);
}

LorawanMacHeader::FType
BaseEndDeviceLorawanMac::GetFType()
{
    return m_fType;
}

void
BaseEndDeviceLorawanMac::SetDataRate(uint8_t dataRate)
{
    m_dataRate = dataRate;
}

uint8_t
BaseEndDeviceLorawanMac::GetDataRate()
{
    return m_dataRate;
}

uint8_t
BaseEndDeviceLorawanMac::GetTransmissionPower()
{
    return m_txPower;
}

void
BaseEndDeviceLorawanMac::SetTransmissionPower(uint8_t txPower)
{
    m_txPower = txPower;
}

double
BaseEndDeviceLorawanMac::GetAggregatedDutyCycle()
{
    return m_aggregatedDutyCycle;
}

void
BaseEndDeviceLorawanMac::SetAggregatedDutyCycle(double aggregatedDutyCycle)
{
    m_aggregatedDutyCycle = aggregatedDutyCycle;
}

void
BaseEndDeviceLorawanMac::SetNumberOfTransmissions(uint8_t nbTrans)
{
    m_nbTrans = nbTrans;
}

uint8_t
BaseEndDeviceLorawanMac::GetNumberOfTransmissions() const
{
    return m_nbTrans;
}

void
BaseEndDeviceLorawanMac::SetADRBackoff(bool backoff)
{
    m_enableADRBackoff = backoff;
}

bool
BaseEndDeviceLorawanMac::GetADRBackoff() const
{
    return m_enableADRBackoff;
}

void
BaseEndDeviceLorawanMac::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    auto phy = DynamicCast<EndDeviceLoraPhy>(m_phy);
    NS_ABORT_MSG_UNLESS(bool(phy) != 0,
                        "This object requires an EndDeviceLoraPhy installed to work");
    phy->SetDeviceAddress(m_address);
    LorawanMac::DoInitialize();
}

void
BaseEndDeviceLorawanMac::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_fOpts.clear();
    m_txContext.packet = nullptr;
    m_uniformRV = nullptr;
    m_nextTx.Cancel();
    delete m_crypto;
    LorawanMac::DoDispose();
}

} // namespace lorawan
} // namespace ns3
