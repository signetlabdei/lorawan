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
 *              qiuyukang <b612n@qq.com>
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "class-a-end-device-lorawan-mac.h"

#include "ns3/lora-tag.h"

#define RETRANSMIT_TIMEOUT 5

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("ClassAEndDeviceLorawanMac");

NS_OBJECT_ENSURE_REGISTERED(ClassAEndDeviceLorawanMac);

TypeId
ClassAEndDeviceLorawanMac::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ClassAEndDeviceLorawanMac")
            .SetParent<BaseEndDeviceLorawanMac>()
            .SetGroupName("lorawan")
            .AddConstructor<ClassAEndDeviceLorawanMac>()
            .AddAttribute("RecvWinSymb",
                          "The duration of a receive window in number of symbols.",
                          UintegerValue(8),
                          MakeUintegerAccessor(&ClassAEndDeviceLorawanMac::m_recvWinSymb),
                          MakeUintegerChecker<uint16_t>(4, 1023));
    return tid;
}

ClassAEndDeviceLorawanMac::ClassAEndDeviceLorawanMac()
    : m_recvWinSymb(8),
      // LoRaWAN default
      m_rx1DrOffset(0),
      m_lastTxCh(nullptr)
{
    NS_LOG_FUNCTION(this);
    m_rwm = CreateObject<RecvWindowManager>();
    m_rwm->SetNoRecvCallback(MakeCallback(&ClassAEndDeviceLorawanMac::NoReception, this));
}

ClassAEndDeviceLorawanMac::~ClassAEndDeviceLorawanMac()
{
    NS_LOG_FUNCTION(this);
}

/////////////////////
// Sending methods //
/////////////////////

void
ClassAEndDeviceLorawanMac::SendToPhy(Ptr<Packet> packet)
{
    NS_LOG_DEBUG("Packet: " << packet);

    // Configure PHY tx params
    m_txParams.sf = GetSfFromDataRate(m_dataRate);
    m_txParams.bandwidthHz = GetBandwidthFromDataRate(m_dataRate);
    m_txParams.lowDataRateOptimizationEnabled = LoraPhy::GetTSym(m_txParams) > MilliSeconds(16);
    NS_LOG_DEBUG("DR: " << unsigned(m_dataRate));
    NS_LOG_DEBUG("SF: " << unsigned(m_txParams.sf));
    NS_LOG_DEBUG("BW: " << m_txParams.bandwidthHz << " Hz");

    m_lastTxCh = GetChannelForTx();
    double frequency = m_lastTxCh->GetFrequency();
    // Make sure we can transmit at the current power on this channel
    NS_ASSERT_MSG(m_txPower <= m_channelManager->GetTxPowerForChannel(m_lastTxCh),
                  " The selected power is too hight to be supported by this channel.");
    NS_LOG_DEBUG("Freq: " << frequency << " Hz");

    // Tag packet with datarate and frequency
    LoraTag tag;
    packet->RemovePacketTag(tag); // Needed in case of retx!
    tag.SetDataRate(m_dataRate);
    tag.SetFrequency(frequency);
    packet->AddPacketTag(tag);

    // Get the duration
    Time duration = m_phy->GetTimeOnAir(packet, m_txParams);
    NS_LOG_DEBUG("Duration: " << duration.GetSeconds());
    // Add the event to the channelHelper to keep track of duty cycle
    m_channelManager->AddEvent(duration, m_lastTxCh);

    // Send the packet to the PHY layer to send it on the channel
    DynamicCast<EndDeviceLoraPhy>(m_phy)->SwitchToStandby();
    m_phy->Send(packet, m_txParams, frequency, m_txPower);
    // Fire trace source
    m_sentNewPacket(packet);
}

void
ClassAEndDeviceLorawanMac::TxFinished(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION_NOARGS();
    // Switch the PHY to sleep
    DynamicCast<EndDeviceLoraPhy>(m_phy)->SwitchToSleep();

    // Set dynamic reception windows parameters
    uint8_t dr = m_replyDataRateMatrix.at(m_dataRate).at(m_rx1DrOffset);
    m_rwm->SetSf(RecvWindowManager::FIRST, GetSfFromDataRate(dr));
    m_rwm->SetDuration(RecvWindowManager::FIRST, GetReceptionWindowDuration(dr));
    m_rwm->SetFrequency(RecvWindowManager::FIRST, m_lastTxCh->GetReplyFrequency());

    // Schedule the opening of the receive windows
    m_rwm->Start();
}

Time
ClassAEndDeviceLorawanMac::GetBusyTransmissionDelay()
{
    NS_LOG_FUNCTION_NOARGS();
    // If we are in the process of sending or receiving, postpone transmission
    // (we try to be as accurate as possible)
    if (m_txContext.busy)
    {
        NS_LOG_WARN("Attempting to send when device is already busy, postponed.");
        return Seconds(m_uniformRV->GetValue(4, 5));
    }
    return Seconds(0);
}

Time
ClassAEndDeviceLorawanMac::GetReceptionWindowDuration(uint8_t datarate)
{
    return m_recvWinSymb * GetTSym(datarate);
}

//////////////////////////
//  Reception windows   //
//////////////////////////

void
ClassAEndDeviceLorawanMac::Receive(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    NS_LOG_INFO("Downlink packet for us arrived at MAC layer.");
    // Stop all reception windows and ensure the device is sleeping
    m_rwm->Stop();
    // Open the context to new transmissions
    m_txContext.busy = false;
    // Reset ADR backoff counter
    m_ADRACKCnt = 0;
    // Clear commands that are re-sent until downlink (DlChannelAns and RxTimingSetupAns)
    m_fOpts.clear();

    // Work on a copy of the packet
    Ptr<Packet> packetCopy = packet->Copy();
    // Remove MIC (currently we do not check it)
    packetCopy->RemoveAtEnd(4);
    // Remove the Mac Header to get some information
    LorawanMacHeader mHdr;
    packetCopy->RemoveHeader(mHdr);
    NS_LOG_DEBUG("Mac Header: " << mHdr);
    // Remove the Frame Header
    LoraFrameHeader fHdr;
    fHdr.SetAsDownlink();
    int deserialized = packetCopy->RemoveHeader(fHdr);
    NS_LOG_DEBUG("Deserialized bytes: " << deserialized << ", Frame Header:\n" << fHdr);
    // Parse and apply all MAC commands received
    ApplyMACCommands(fHdr, packetCopy);

    if (!m_receiveCallback.IsNull())
    {
        m_receiveCallback(this, packetCopy);
    }
    // Call the trace source
    m_receivedPacket(packet);

    ManageRetransmissions(fHdr.GetAck() ? ACK : RECV);
}

void
ClassAEndDeviceLorawanMac::FailedReception(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    // Ensure device is sleeping without canceling future reception windows
    m_rwm->ForceSleep();
    // Check if we have exahusted the reception windows
    if (m_rwm->NoMoreWindows())
    {
        ManageRetransmissions(FAIL);
        // Open the context to new transmissions
        m_txContext.busy = false;
    }
}

void
ClassAEndDeviceLorawanMac::NoReception()
{
    NS_LOG_FUNCTION_NOARGS();
    // We are here if no reception happened
    ManageRetransmissions(NONE);
    m_txContext.busy = false;
}

void
ClassAEndDeviceLorawanMac::ManageRetransmissions(RxOutcome outcome)
{
    bool recv = (outcome == RECV || outcome == ACK); // We received something
    bool needAck = m_txContext.waitingAck;           // We were waiting for acknowledgement
    bool gotAck = (outcome == ACK);                  // We got acknowledgement
    bool canReTx = (m_txContext.nbTxLeft > 0 && m_nextTx.IsExpired()); // We can retransmit

    // Condition to schedule retransmission:
    // either we did not receive or we weren't acknowledged + we can retransmit
    if ((!recv || (needAck && !gotAck)) && canReTx)
    {
        if (outcome == RECV)
        {
            NS_LOG_DEBUG("Received packet without ACK: rescheduling transmission.");
        }
        else if (outcome == FAIL)
        {
            NS_LOG_DEBUG("Reception failed: rescheduling transmission.");
        }
        else if (outcome == NONE)
        {
            NS_LOG_DEBUG("No reception initiated by PHY: rescheduling transmission.");
        }
        NS_LOG_INFO("We have " << unsigned(m_txContext.nbTxLeft) << " retransmissions left.");
        postponeTransmission(Seconds(RETRANSMIT_TIMEOUT), m_txContext.packet);
        return;
    }

    uint8_t txs = m_nbTrans - m_txContext.nbTxLeft;
    // Acknowledgement success of confirmed txs
    if (recv && needAck && gotAck)
    {
        m_requiredTxCallback(txs, true, m_txContext.firstAttempt, m_txContext.packet);
        NS_LOG_DEBUG("Received ACK packet after "
                     << unsigned(txs) << " transmissions: stopping retransmission procedure. ");
        return;
    }
    // Acknowledgement failure of confirmed txs
    else if (needAck && !gotAck && !canReTx)
    {
        m_requiredTxCallback(txs, false, m_txContext.firstAttempt, m_txContext.packet);
        NS_LOG_DEBUG("Ack failure: no more retransmissions left. Used " << unsigned(txs)
                                                                        << " transmissions.");
        return;
    }
}

/////////////////////////
// MAC command methods //
/////////////////////////

void
ClassAEndDeviceLorawanMac::OnRxParamSetupReq(Ptr<RxParamSetupReq> rxParamSetupReq)
{
    NS_LOG_FUNCTION(this << rxParamSetupReq);

    uint8_t rx1DrOffset = rxParamSetupReq->GetRx1DrOffset();
    uint8_t rx2DataRate = rxParamSetupReq->GetRx2DataRate();
    double frequency = rxParamSetupReq->GetFrequency();

    NS_LOG_INFO(unsigned(rx1DrOffset) << unsigned(rx2DataRate) << frequency);

    // Check that the desired offset is valid
    bool offsetOk = (0 <= rx1DrOffset && rx1DrOffset <= 5);
    // Check that the desired data rate is valid
    bool dataRateOk =
        (GetSfFromDataRate(rx2DataRate) != 0 && GetBandwidthFromDataRate(rx2DataRate) != 0);
    // Check that channel is in known bands
    bool channelOk = bool(m_channelManager->GetSubBandFromFrequency(frequency));

    if (offsetOk && dataRateOk && channelOk)
    {
        // RxWin1
        m_rx1DrOffset = rx1DrOffset;

        // RxWin2
        m_rwm->SetSf(RecvWindowManager::SECOND, GetSfFromDataRate(rx2DataRate));
        m_rwm->SetDuration(RecvWindowManager::SECOND, GetReceptionWindowDuration(rx2DataRate));
        m_rwm->SetFrequency(RecvWindowManager::SECOND, frequency);
    }

    // Craft a RxParamSetupAns as response
    NS_LOG_INFO("Adding RxParamSetupAns reply");
    m_fOpts.push_back(Create<RxParamSetupAns>(offsetOk, dataRateOk, channelOk));
}

void
ClassAEndDeviceLorawanMac::OnRxTimingSetupReq(Time delay)
{
    NS_LOG_FUNCTION(this << delay);

    m_rwm->SetRx1Delay(delay);

    NS_LOG_INFO("Adding RxTimingSetupAns reply");
    m_fOpts.push_back(Create<RxTimingSetupAns>());
}

/////////////////////////
// Getters and Setters //
/////////////////////////

void
ClassAEndDeviceLorawanMac::SetSecondReceiveWindowDataRate(uint8_t dataRate)
{
    m_rwm->SetSf(RecvWindowManager::SECOND, GetSfFromDataRate(dataRate));
    m_rwm->SetDuration(RecvWindowManager::SECOND, GetReceptionWindowDuration(dataRate));
}

void
ClassAEndDeviceLorawanMac::SetSecondReceiveWindowFrequency(double frequency)
{
    m_rwm->SetFrequency(RecvWindowManager::SECOND, frequency);
}

void
ClassAEndDeviceLorawanMac::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    auto phy = DynamicCast<EndDeviceLoraPhy>(m_phy);
    NS_ABORT_MSG_UNLESS(bool(phy) != 0,
                        "This object requires an EndDeviceLoraPhy installed to work");
    m_rwm->SetPhy(phy);
    BaseEndDeviceLorawanMac::DoInitialize();
}

void
ClassAEndDeviceLorawanMac::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_lastTxCh = nullptr;
    m_rwm->Dispose();
    m_rwm = nullptr;
    BaseEndDeviceLorawanMac::DoDispose();
}

} /* namespace lorawan */
} /* namespace ns3 */
