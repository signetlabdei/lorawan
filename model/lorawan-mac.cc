/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "lorawan-mac.h"

#include "ns3/log.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LorawanMac");

NS_OBJECT_ENSURE_REGISTERED(LorawanMac);

TypeId
LorawanMac::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LorawanMac")
            .SetParent<Object>()
            .SetGroupName("lorawan")
            .AddTraceSource("SentNewPacket",
                            "Trace source indicating a new packet "
                            "arrived at the MAC layer",
                            MakeTraceSourceAccessor(&LorawanMac::m_sentNewPacket),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("ReceivedPacket",
                            "Trace source indicating a packet "
                            "was correctly received at the MAC layer",
                            MakeTraceSourceAccessor(&LorawanMac::m_receivedPacket),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("CannotSendBecauseDutyCycle",
                            "Trace source indicating a packet "
                            "could not be sent immediately because of duty cycle limitations",
                            MakeTraceSourceAccessor(&LorawanMac::m_cannotSendBecauseDutyCycle),
                            "ns3::Packet::TracedCallback");
    return tid;
}

LorawanMac::LorawanMac()
{
    NS_LOG_FUNCTION(this);
}

LorawanMac::~LorawanMac()
{
    NS_LOG_FUNCTION(this);
}

void
LorawanMac::SetDevice(Ptr<NetDevice> device)
{
    m_device = device;
}

Ptr<NetDevice>
LorawanMac::GetDevice()
{
    return m_device;
}

Ptr<LoraPhy>
LorawanMac::GetPhy()
{
    return m_phy;
}

void
LorawanMac::SetPhy(Ptr<LoraPhy> phy)
{
    // Set the phy
    m_phy = phy;

    // Connect the receive callbacks
    m_phy->SetReceiveOkCallback(MakeCallback(&LorawanMac::Receive, this));
    m_phy->SetReceiveFailedCallback(MakeCallback(&LorawanMac::FailedReception, this));
    m_phy->SetTxFinishedCallback(MakeCallback(&LorawanMac::TxFinished, this));
}

Ptr<LogicalLoraChannelHelper>
LorawanMac::GetLogicalLoraChannelHelper()
{
    return m_channelHelper;
}

void
LorawanMac::SetLogicalLoraChannelHelper(Ptr<LogicalLoraChannelHelper> helper)
{
    m_channelHelper = helper;
}

uint8_t
LorawanMac::GetSfFromDataRate(uint8_t dataRate)
{
    NS_LOG_FUNCTION(this << unsigned(dataRate));

    // Check we are in range
    if (dataRate >= m_sfForDataRate.size())
    {
        return 0;
    }

    return m_sfForDataRate.at(dataRate);
}

double
LorawanMac::GetBandwidthFromDataRate(uint8_t dataRate)
{
    NS_LOG_FUNCTION(this << unsigned(dataRate));

    // Check we are in range
    if (dataRate > m_bandwidthForDataRate.size())
    {
        return 0;
    }

    return m_bandwidthForDataRate.at(dataRate);
}

double
LorawanMac::GetDbmForTxPower(uint8_t txPower)
{
    NS_LOG_FUNCTION(this << unsigned(txPower));

    if (txPower > m_txDbmForTxPower.size() - 1)
    {
        return -1;
    }

    return m_txDbmForTxPower.at(txPower);
}

void
LorawanMac::SetSfForDataRate(std::vector<uint8_t> sfForDataRate)
{
    m_sfForDataRate = sfForDataRate;
}

void
LorawanMac::SetBandwidthForDataRate(std::vector<double> bandwidthForDataRate)
{
    m_bandwidthForDataRate = bandwidthForDataRate;
}

void
LorawanMac::SetMaxAppPayloadForDataRate(std::vector<uint32_t> maxAppPayloadForDataRate)
{
    m_maxAppPayloadForDataRate = maxAppPayloadForDataRate;
}

void
LorawanMac::SetTxDbmForTxPower(std::vector<double> txDbmForTxPower)
{
    m_txDbmForTxPower = txDbmForTxPower;
}

void
LorawanMac::SetNPreambleSymbols(int nPreambleSymbols)
{
    m_nPreambleSymbols = nPreambleSymbols;
}

int
LorawanMac::GetNPreambleSymbols() const
{
    return m_nPreambleSymbols;
}

void
LorawanMac::SetReplyDataRateMatrix(ReplyDataRateMatrix replyDataRateMatrix)
{
    m_replyDataRateMatrix = replyDataRateMatrix;
}
} // namespace lorawan
} // namespace ns3
