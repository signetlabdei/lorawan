/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *         Martina Capuzzo <capuzzom@dei.unipd.it>
 *
 * Modified by: Peggy Anderson <peggy.anderson@usask.ca>
 *              qiuyukang <b612n@qq.com>
 */

#include "class-a-end-device-lorawan-mac.h"

#include "end-device-lora-phy.h"
#include "lora-tag.h"

#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("ClassAEndDeviceLorawanMac");

NS_OBJECT_ENSURE_REGISTERED(ClassAEndDeviceLorawanMac);

TypeId
ClassAEndDeviceLorawanMac::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ClassAEndDeviceLorawanMac")
                            .SetParent<EndDeviceLorawanMac>()
                            .SetGroupName("lorawan")
                            .AddConstructor<ClassAEndDeviceLorawanMac>();
    return tid;
}

ClassAEndDeviceLorawanMac::ClassAEndDeviceLorawanMac()
    : // LoraWAN default
      m_receiveDelay1(Seconds(1)),
      m_rx1DrOffset(0),
      m_receiveDelay2(Seconds(2)),
      m_isSecondWindowOpen(false)
{
    NS_LOG_FUNCTION(this);

    // Void the RX2 event
    m_secondReceiveWindow = EventId();
    m_secondReceiveWindow.Cancel();
}

ClassAEndDeviceLorawanMac::~ClassAEndDeviceLorawanMac()
{
    NS_LOG_FUNCTION_NOARGS();
}

/////////////////////
// Sending methods //
/////////////////////

void
ClassAEndDeviceLorawanMac::SendToPhy(Ptr<Packet> packetToSend)
{
    NS_LOG_DEBUG("PacketToSend: " << packetToSend);

    /////////////////////////
    // Prepare TX parameters
    /////////////////////////

    auto sf = GetSfFromDataRate(m_dataRate);
    auto bw = GetBandwidthFromDataRate(m_dataRate);
    // see SX1272/73 Datasheet, Section 4.1.1.6, Rev. 4, Jan. 2019
    auto ldro = bool((sf == 11 || sf == 12) && bw == 125'000);

    // Craft LoraTxParameters object
    LoraTxParameters params;
    params.spreadingFactor = sf;
    params.bandwidthHz = bw;
    params.codingRate = m_codingRate;
    params.lowDataRateOptimize = ldro;
    params.preambleLenSymb = m_nPreambleSymbols;
    params.implicitHeader = m_headerDisabled;
    params.crcEnabled = true;

    // Select random frequency channel
    Ptr<LogicalLoraChannel> txChannel = GetRandomChannelForTx();

    ///////////////////////////////////////////////
    // Register packet transmission for duty cycle
    ///////////////////////////////////////////////

    // Compute tx duration for duty-cycle management
    Time duration = LoraPhy::GetTimeOnAir(packetToSend->GetSize(), params);
    // Register the tx duration into the LogicalLoraChannelHelper
    m_channelHelper->AddEvent(duration, txChannel);

    /////////////////////////////////////////
    // Store dynamic RX1 window parameters //
    /////////////////////////////////////////

    // Switch the PHY to the channel so that it will listen here for downlink
    m_firstReceiveWindowFrequencyHz = txChannel->GetFrequency();

    /////////////////////
    // Init transmission
    /////////////////////

    // Check that the PHY layer is in an expected state
    auto phyState = DynamicCast<EndDeviceLoraPhy>(m_phy)->GetState();
    NS_ASSERT_MSG(phyState == EndDeviceLoraPhy::State::SLEEP ||
                      phyState == EndDeviceLoraPhy::State::STANDBY,
                  "Busy PHY device (not in SLEEP or STANDBY state): phyState=" << phyState);
    // Wake up PHY layer and directly send the packet
    m_phy->Send(packetToSend, txChannel->GetFrequency(), IQPolarity::UP, params, m_txPowerDbm);
}

//////////////////////////
//  Receiving methods   //
//////////////////////////
void
ClassAEndDeviceLorawanMac::Receive(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    // We should always be in STANDBY mode at this point, as this function is meant to be invoked as
    // a callback by the PHY on reception end
    auto phyState = DynamicCast<EndDeviceLoraPhy>(m_phy)->GetState();
    NS_ASSERT_MSG(phyState == EndDeviceLoraPhy::State::STANDBY,
                  "Unexpected PHY state on RX end: phyState=" << phyState);

    // Here is for sure closed, can be improved
    m_isSecondWindowOpen = false;

    // Work on a copy of the packet
    Ptr<Packet> packetCopy = packet->Copy();

    // Remove the Mac Header to get some information
    LorawanMacHeader mHdr;
    packetCopy->RemoveHeader(mHdr);
    NS_ASSERT_MSG(mHdr.IsUplink() == false, "Received uplink package, check PHY polarity");
    NS_LOG_DEBUG("Downlink Mac Header: " << mHdr);
    // Remove the Frame Header
    LoraFrameHeader fHdr;
    fHdr.SetAsDownlink();
    packetCopy->RemoveHeader(fHdr);
    NS_LOG_DEBUG("Downlink Frame Header: " << fHdr);

    // Determine whether this packet is for us
    bool messageForUs = (m_address == fHdr.GetAddress());

    if (messageForUs)
    {
        NS_LOG_INFO("The message is for us!");

        // If it exists, cancel the second receive window event
        // THIS WILL BE GetReceiveWindow()
        Simulator::Cancel(m_secondReceiveWindow);

        // Reset ADR backoff counter
        m_adrAckCnt = 0;

        LoraTag tag;
        packet->PeekPacketTag(tag);
        /// @see ns3::lorawan::AdrComponent::RxPowerToSNR
        m_lastRxSnr = tag.GetReceivePower() + 174 - 10 * log10(125000) - 6;

        // Parse the MAC commands
        ParseCommands(fHdr);

        // Pass the packet up to the NetDevice
        if (!m_receiveCallback.IsNull())
        {
            m_receiveCallback(packetCopy);
        }
        // Call the trace source
        m_receivedPacket(packet);
    }
    else
    {
        NS_LOG_DEBUG("The message is intended for another recipient.");

        // In this case, we are either receiving in the first receive window
        // and finishing reception inside the second one, or receiving a
        // packet in the second receive window and finding out, after the
        // fact, that the packet is not for us. In either case, if we no
        // longer have any retransmissions left, we declare failure.
        if (m_txContext.needsAck && m_secondReceiveWindow.IsExpired())
        {
            /// TODO: UNCONFIRMED packets CAN be retransmitted, but behave slightly differently.
            /// The current implementation only considers re-txs for CONFIRMED, change this
            if (m_txContext.nbTxLeft == 0)
            {
                uint8_t txs = m_nbTrans - (m_txContext.nbTxLeft);
                m_confirmedTxOutcomeCallback(txs,
                                             false,
                                             m_txContext.firstAttempt,
                                             m_txContext.packet);
                NS_LOG_DEBUG("Failure: no more retransmissions left. Used " << unsigned(txs)
                                                                            << " transmissions.");

                // Reset retransmission parameters
                ResetRetransmissionParameters();
            }
            else // Reschedule
            {
                this->Send(m_txContext.packet);
                NS_LOG_INFO("We have " << unsigned(m_txContext.nbTxLeft)
                                       << " retransmissions left: rescheduling transmission.");
            }
        }
    }

    DynamicCast<EndDeviceLoraPhy>(m_phy)->Sleep();
}

void
ClassAEndDeviceLorawanMac::FailedReception(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    // We should always be in STANDBY mode at this point, as this function is meant to be
    // invoked as a callback by the PHY on reception end
    auto phyState = DynamicCast<EndDeviceLoraPhy>(m_phy)->GetState();
    NS_ASSERT_MSG(phyState == EndDeviceLoraPhy::State::STANDBY,
                  "Unexpected PHY state on RX end: phyState=" << phyState);
    // Switch to sleep after a failed reception
    DynamicCast<EndDeviceLoraPhy>(m_phy)->Sleep();

    // Here is for sure closed, can be improved
    m_isSecondWindowOpen = false;

    if (m_secondReceiveWindow.IsExpired() && m_retxParams.needsAck)
    {
        if (m_txContext.nbTxLeft > 0)
        {
            this->Send(m_txContext.packet);
            NS_LOG_INFO("We have " << unsigned(m_txContext.nbTxLeft)
                                   << " retransmissions left: rescheduling transmission.");
        }
        else
        {
            uint8_t txs = m_nbTrans - (m_txContext.nbTxLeft);
            m_confirmedTxOutcomeCallback(txs, false, m_txContext.firstAttempt, m_txContext.packet);
            NS_LOG_DEBUG("Failure: no more retransmissions left. Used " << unsigned(txs)
                                                                        << " transmissions.");

            // Reset retransmission parameters
            ResetRetransmissionParameters();
        }
    }
}

void
ClassAEndDeviceLorawanMac::TxFinished(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    // We should always be in STANDBY mode at this point, as this function is meant to be
    // invoked as a callback by the PHY on transmission end
    auto phyState = DynamicCast<EndDeviceLoraPhy>(m_phy)->GetState();
    NS_ASSERT_MSG(phyState == EndDeviceLoraPhy::State::STANDBY,
                  "Unexpected PHY state on TX conclusion: phyState=" << phyState);
    // Switch the PHY to sleep
    DynamicCast<EndDeviceLoraPhy>(m_phy)->Sleep();

    // Schedule the opening of the first receive window
    Simulator::Schedule(m_receiveDelay1, &ClassAEndDeviceLorawanMac::OpenFirstReceiveWindow, this);

    // Schedule the opening of the second receive window
    m_secondReceiveWindow = Simulator::Schedule(m_receiveDelay2,
                                                &ClassAEndDeviceLorawanMac::OpenSecondReceiveWindow,
                                                this);
}

void
ClassAEndDeviceLorawanMac::OpenFirstReceiveWindow()
{
    NS_LOG_FUNCTION(this);
    // We should always be in SLEEP mode at this point
    auto phyState = DynamicCast<EndDeviceLoraPhy>(m_phy)->GetState();
    NS_ASSERT_MSG(phyState == EndDeviceLoraPhy::State::SLEEP,
                  "Unexpected PHY state on RX1 opening: phyState=" << phyState);

    // Gather the parameters required for the first reception window
    auto rx1DataRate = GetFirstReceiveWindowDataRate();
    NS_LOG_DEBUG("m_dataRate=" << unsigned(m_dataRate)
                               << ", m_rx1DrOffset=" << unsigned(m_rx1DrOffset)
                               << ", rx1DataRate=" << unsigned(rx1DataRate));
    // Request a timed reception from the PHY
    DynamicCast<EndDeviceLoraPhy>(m_phy)->ReceiveSingle(
        m_firstReceiveWindowFrequencyHz,
        IQPolarity::DOWN,
        GetSfFromDataRate(rx1DataRate),
        GetBandwidthFromDataRate(rx1DataRate),
        m_receiveWindowDurationInSymbols,
        MakeCallback(&ClassAEndDeviceLorawanMac::CloseFirstReceiveWindow, this));
}

void
ClassAEndDeviceLorawanMac::CloseFirstReceiveWindow()
{
    NS_LOG_FUNCTION(this);
    // We should always be in STANDBY mode at this point, as this function is meant to be
    // invoked as a callback by the PHY on reception timeout
    auto phyState = DynamicCast<EndDeviceLoraPhy>(m_phy)->GetState();
    NS_ASSERT_MSG(phyState == EndDeviceLoraPhy::State::STANDBY,
                  "Unexpected PHY state on RX1 closure: phyState=" << phyState);
    DynamicCast<EndDeviceLoraPhy>(m_phy)->Sleep();
}

void
ClassAEndDeviceLorawanMac::OpenSecondReceiveWindow()
{
    NS_LOG_FUNCTION(this);
    // We might be in SLEEP or in RX_ACTIVE mode at this point
    auto phyState = DynamicCast<EndDeviceLoraPhy>(m_phy)->GetState();
    NS_ASSERT_MSG(phyState == EndDeviceLoraPhy::State::SLEEP ||
                      phyState == EndDeviceLoraPhy::State::RX_ACTIVE,
                  "Unexpected PHY state on RX2 opening: phyState=" << phyState);

    // Return immediately if a reception started on RX1 has not concluded yet
    if (phyState == EndDeviceLoraPhy::State::RX_ACTIVE)
    {
        return;
    }

    // Gather the parameters required for the second reception window
    NS_LOG_DEBUG("m_secondReceiveWindowFrequencyHz=" << m_secondReceiveWindowFrequencyHz
                                                     << ", m_secondReceiveWindowDataRate="
                                                     << unsigned(m_secondReceiveWindowDataRate));
    // Request a timed reception from the PHY
    DynamicCast<EndDeviceLoraPhy>(m_phy)->ReceiveSingle(
        m_secondReceiveWindowFrequencyHz,
        IQPolarity::DOWN,
        GetSfFromDataRate(m_secondReceiveWindowDataRate),
        GetBandwidthFromDataRate(m_secondReceiveWindowDataRate),
        m_receiveWindowDurationInSymbols,
        MakeCallback(&ClassAEndDeviceLorawanMac::CloseSecondReceiveWindow, this));
    m_isSecondWindowOpen = true;
}

void
ClassAEndDeviceLorawanMac::CloseSecondReceiveWindow()
{
    NS_LOG_FUNCTION(this);
    m_isSecondWindowOpen = false;
    // We should always be in STANDBY mode at this point, as this function is meant to be
    // invoked as a callback by the PHY on reception timeout
    auto phyState = DynamicCast<EndDeviceLoraPhy>(m_phy)->GetState();
    NS_ASSERT_MSG(phyState == EndDeviceLoraPhy::State::STANDBY,
                  "Unexpected PHY state on RX2 closure: phyState=" << phyState);
    DynamicCast<EndDeviceLoraPhy>(m_phy)->Sleep();

    if (m_txContext.needsAck)
    {
        NS_LOG_DEBUG("No reception initiated by PHY: rescheduling transmission.");
        if (m_txContext.nbTxLeft > 0)
        {
            NS_LOG_INFO("We have " << unsigned(m_txContext.nbTxLeft)
                                   << " retransmissions left: rescheduling transmission.");
            this->Send(m_txContext.packet);
        }

        else if (m_txContext.nbTxLeft == 0 && DynamicCast<EndDeviceLoraPhy>(m_phy)->GetState() !=
                                                  EndDeviceLoraPhy::State::RX_ACTIVE)
        {
            uint8_t txs = m_nbTrans - (m_txContext.nbTxLeft);
            m_confirmedTxOutcomeCallback(txs, false, m_txContext.firstAttempt, m_txContext.packet);
            NS_LOG_DEBUG("Failure: no more retransmissions left. Used " << unsigned(txs)
                                                                        << " transmissions.");

            // Reset retransmission parameters
            ResetRetransmissionParameters();
        }

        else
        {
            NS_ABORT_MSG("The number of retransmissions left is negative ! ");
        }
    }
    else
    {
        uint8_t txs = m_nbTrans - (m_txContext.nbTxLeft);
        m_confirmedTxOutcomeCallback(txs, true, m_txContext.firstAttempt, m_txContext.packet);
        NS_LOG_INFO(
            "We have " << unsigned(m_txContext.nbTxLeft)
                       << " transmissions left. We were not transmitting confirmed messages.");

        // Reset retransmission parameters
        ResetRetransmissionParameters();
    }
}

/////////////////////////
// Getters and Setters //
/////////////////////////

Time
ClassAEndDeviceLorawanMac::GetNextClassTransmissionDelay(Time waitTime)
{
    NS_LOG_FUNCTION_NOARGS();

    // This is a new packet from APP; it can not be sent until the end of the
    // second receive window (if the second receive window has not closed yet)
    if (!m_txContext.needsAck)
    {
        if (!m_secondReceiveWindow.IsExpired() || m_isSecondWindowOpen)
        {
            NS_LOG_WARN(
                "Attempting to send when there are receive windows: Transmission postponed.");
            // Compute the worst case (SF12, 64B) reception end in the second receive window
            Time worstCaseEndReceive = Time(m_secondReceiveWindow.GetTs()) + Seconds(2.79);
            NS_LOG_DEBUG("Duration until worstCaseEndReceive for new transmission:"
                         << (worstCaseEndReceive - Now()).As(Time::S));
            waitTime = Max(waitTime, worstCaseEndReceive - Now());
        }
    }
    // This is a retransmitted packet, it can not be sent until the end of
    // ACK_TIMEOUT (this timer starts when the second receive window was open)
    else
    {
        double ack_timeout = m_uniformRV->GetValue(1, 3);
        // Compute the duration until ACK_TIMEOUT (It may be a negative number, but it doesn't
        // matter.)
        Time retransmitWaitTime =
            Time(m_secondReceiveWindow.GetTs()) - Now() + Seconds(ack_timeout);

        NS_LOG_DEBUG("ack_timeout:" << ack_timeout
                                    << " retransmitWaitTime:" << retransmitWaitTime.As(Time::S));
        waitTime = Max(waitTime, retransmitWaitTime);
    }

    return waitTime;
}

uint8_t
ClassAEndDeviceLorawanMac::GetFirstReceiveWindowDataRate()
{
    return m_replyDataRateMatrix.at(m_dataRate).at(m_rx1DrOffset);
}

void
ClassAEndDeviceLorawanMac::SetSecondReceiveWindowDataRate(uint8_t dataRate)
{
    m_secondReceiveWindowDataRate = dataRate;
}

uint8_t
ClassAEndDeviceLorawanMac::GetSecondReceiveWindowDataRate() const
{
    return m_secondReceiveWindowDataRate;
}

void
ClassAEndDeviceLorawanMac::SetSecondReceiveWindowFrequency(uint32_t frequencyHz)
{
    m_secondReceiveWindowFrequencyHz = frequencyHz;
}

uint32_t
ClassAEndDeviceLorawanMac::GetSecondReceiveWindowFrequency() const
{
    return m_secondReceiveWindowFrequencyHz;
}

/////////////////////////
// MAC command methods //
/////////////////////////

void
ClassAEndDeviceLorawanMac::OnRxParamSetupReq(uint8_t rx1DrOffset,
                                             uint8_t rx2DataRate,
                                             double frequencyHz)
{
    NS_LOG_FUNCTION(this << unsigned(rx1DrOffset) << unsigned(rx2DataRate)
                         << uint32_t(frequencyHz));

    // Adapted from: github.com/Lora-net/SWL2001.git v4.3.1
    // For the time being, this implementation is valid for the EU868 region

    bool rx1DrOffsetAck = true;
    bool rx2DataRateAck = true;
    bool channelAck = true;

    if (rx1DrOffset >= m_replyDataRateMatrix.at(m_dataRate).size())
    {
        NS_LOG_WARN("Invalid rx1DrOffset");
        rx1DrOffsetAck = false;
    }

    if (!GetSfFromDataRate(rx2DataRate) || !GetBandwidthFromDataRate(rx2DataRate))
    {
        NS_LOG_WARN("Invalid rx2DataRate");
        rx2DataRateAck = false;
    }

    if (!m_channelHelper->IsFrequencyValid(frequencyHz))
    {
        NS_LOG_WARN("Invalid rx2 frequency");
        channelAck = false;
    }

    if (rx1DrOffsetAck && rx2DataRateAck && channelAck)
    {
        m_rx1DrOffset = rx1DrOffset;
        m_secondReceiveWindowDataRate = rx2DataRate;
        m_secondReceiveWindowFrequencyHz = frequencyHz;
    }

    NS_LOG_INFO("Adding RxParamSetupAns reply");
    m_macCommandList.emplace_back(
        Create<RxParamSetupAns>(rx1DrOffsetAck, rx2DataRateAck, channelAck));
}

} /* namespace lorawan */
} /* namespace ns3 */
