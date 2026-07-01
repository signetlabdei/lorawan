/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "lora-phy.h"

#include "lora-channel.h"

#include "ns3/node.h"

namespace ns3
{
namespace lorawan
{

std::ostream&
operator<<(std::ostream& os, const CodingRate& codingRate)
{
    switch (codingRate)
    {
    case CodingRate::CR_4_5:
        return os << "4/5";
    case CodingRate::CR_4_6:
        return os << "4/6";
    case CodingRate::CR_4_7:
        return os << "4/7";
    case CodingRate::CR_4_8:
        return os << "4/8";
    default:
        NS_FATAL_ERROR("Unknown coding rate value");
        return (os << "UNKNOWN");
    }
}

std::istream&
operator>>(std::istream& is, CodingRate& codingRate)
{
    std::string value;
    is >> value;
    if (value == "4/5")
    {
        codingRate = CodingRate::CR_4_5;
    }
    else if (value == "4/6")
    {
        codingRate = CodingRate::CR_4_6;
    }
    else if (value == "4/7")
    {
        codingRate = CodingRate::CR_4_7;
    }
    else if (value == "4/8")
    {
        codingRate = CodingRate::CR_4_8;
    }
    else
    {
        is.setstate(std::ios_base::failbit);
    }
    NS_ABORT_MSG_IF(is.bad(), "Failure to parse " << value);
    return is;
}

std::ostream&
operator<<(std::ostream& os, const LoraTxParameters& params)
{
    os << "LoraTxParameters("
       << "spreadingFactor=" << unsigned(params.spreadingFactor) << ", "
       << "bandwidthHz=" << params.bandwidthHz << ", "
       << "codingRate=" << params.codingRate << ", "
       << "lowDataRateOptimize=" << params.lowDataRateOptimize << ", "
       << "preambleLenSymb: " << params.preambleLenSymb << ", "
       << "implicitHeader=" << params.implicitHeader << ", "
       << "crcEnabled=" << params.crcEnabled << ")";
    return os;
}

NS_LOG_COMPONENT_DEFINE("LoraPhy");

NS_OBJECT_ENSURE_REGISTERED(LoraPhy);

Time
LoraPhy::GetTSym(uint8_t spreadingFactor, uint32_t bandwidthHz)
{
    auto sf = static_cast<double>(spreadingFactor);
    auto bw = static_cast<double>(bandwidthHz);
    return Seconds(pow(2, sf) / bw);
}

Time
LoraPhy::GetTimeOnAir(uint32_t phyPayloadLen, const LoraTxParameters& txParams)
{
    NS_LOG_FUNCTION(phyPayloadLen << txParams);

    // The contents of this function are based on [1].
    // [1] SX1272 LoRa modem designer's guide.

    // Compute the symbol duration in seconds
    double tSym = GetTSym(txParams.spreadingFactor, txParams.bandwidthHz).GetSeconds();

    // Compute the preamble duration
    auto nPreamble = static_cast<double>(txParams.preambleLenSymb);
    double tPreamble = (nPreamble + 4.25) * tSym;

    // Payload size
    NS_LOG_DEBUG("PHY Packet of size " << phyPayloadLen << " bytes");

    // Safety casts since the formula deals with double values.
    auto pl = static_cast<double>(phyPayloadLen);
    auto sf = static_cast<double>(txParams.spreadingFactor);
    auto cr = static_cast<double>(txParams.codingRate);
    // de = 1 when the low data rate optimization is enabled, 0 otherwise
    // h = 1 when header is implicit, 0 otherwise
    // crc = 1 when cyclic redundancy check is present, 0 otherwise
    double de = txParams.lowDataRateOptimize ? 1.0 : 0.0;
    double h = txParams.implicitHeader ? 1.0 : 0.0;
    double crc = txParams.crcEnabled ? 1.0 : 0.0;

    // num and den refer to numerator and denominator of the time on air formula
    double num = 8.0 * pl - 4.0 * sf + 28.0 + 16.0 * crc - 20.0 * h;
    double den = 4.0 * (sf - 2.0 * de);
    double payloadSymbNb = 8.0 + std::max(std::ceil(num / den) * (cr + 4.0), 0.0);

    // Time to transmit the payload
    double tPayload = payloadSymbNb * tSym;

    NS_LOG_DEBUG("Time computation: num = " << num << ", den = " << den << ", payloadSymbNb = "
                                            << payloadSymbNb << ", tSym = " << tSym);
    NS_LOG_DEBUG("tPreamble = " << tPreamble);
    NS_LOG_DEBUG("tPayload = " << tPayload);
    NS_LOG_DEBUG("Total time = " << tPreamble + tPayload);

    // Compute and return the total packet on-air time
    return Seconds(tPreamble + tPayload);
}

TypeId
LoraPhy::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LoraPhy")
            .SetParent<Object>()
            .SetGroupName("lorawan")
            .AddTraceSource("StartSending",
                            "Trace source indicating the PHY layer"
                            "has begun the sending process for a packet",
                            MakeTraceSourceAccessor(&LoraPhy::m_startSending),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("PhyRxBegin",
                            "Trace source indicating a packet "
                            "is now being received from the channel medium "
                            "by the device",
                            MakeTraceSourceAccessor(&LoraPhy::m_phyRxBeginTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("PhyRxEnd",
                            "Trace source indicating the PHY has finished "
                            "the reception process for a packet",
                            MakeTraceSourceAccessor(&LoraPhy::m_phyRxEndTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("ReceivedPacket",
                            "Trace source indicating a packet "
                            "was correctly received",
                            MakeTraceSourceAccessor(&LoraPhy::m_successfullyReceivedPacket),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("LostPacketBecauseInterference",
                            "Trace source indicating a packet "
                            "could not be correctly decoded because of interfering"
                            "signals",
                            MakeTraceSourceAccessor(&LoraPhy::m_interferedPacket),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("LostPacketBecauseUnderSensitivity",
                            "Trace source indicating a packet "
                            "could not be correctly received because"
                            "its received power is below the sensitivity of the receiver",
                            MakeTraceSourceAccessor(&LoraPhy::m_underSensitivity),
                            "ns3::Packet::TracedCallback");
    return tid;
}

LoraPhy::LoraPhy()
{
}

LoraPhy::~LoraPhy()
{
}

void
LoraPhy::SetTxFinishedCallback(TxFinishedCallback callback)
{
    m_txFinishedCallback = callback;
}

void
LoraPhy::SetReceiveOkCallback(RxOkCallback callback)
{
    m_rxOkCallback = callback;
}

void
LoraPhy::SetReceiveFailedCallback(RxFailedCallback callback)
{
    m_rxFailedCallback = callback;
}

Ptr<MobilityModel>
LoraPhy::GetMobility()
{
    NS_LOG_FUNCTION_NOARGS();

    // Return mobility model associated to this PHY, else, take it from the node
    return (m_mobility) ? m_mobility : m_device->GetNode()->GetObject<MobilityModel>();
}

void
LoraPhy::SetMobility(Ptr<MobilityModel> mobility)
{
    NS_LOG_FUNCTION_NOARGS();

    m_mobility = mobility;
}

Ptr<LoraChannel>
LoraPhy::GetChannel() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_channel;
}

void
LoraPhy::SetChannel(Ptr<LoraChannel> channel)
{
    NS_LOG_FUNCTION(this << channel);

    m_channel = channel;
}

Ptr<NetDevice>
LoraPhy::GetDevice() const
{
    return m_device;
}

void
LoraPhy::SetDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);

    m_device = device;
}

} // namespace lorawan
} // namespace ns3
