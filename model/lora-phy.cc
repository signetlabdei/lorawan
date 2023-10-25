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
 */

#include "lora-phy.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

#include <algorithm>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraPhy");

NS_OBJECT_ENSURE_REGISTERED(LoraPhy);

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

Ptr<LoraChannel>
LoraPhy::GetChannel() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_channel;
}

Ptr<MobilityModel>
LoraPhy::GetMobility()
{
    NS_LOG_FUNCTION_NOARGS();

    // If there is a mobility model associated to this PHY, take the mobility from
    // there
    if (m_mobility)
    {
        return m_mobility;
    }
    else // Else, take it from the node
    {
        return m_device->GetNode()->GetObject<MobilityModel>();
    }
}

void
LoraPhy::SetMobility(Ptr<MobilityModel> mobility)
{
    NS_LOG_FUNCTION_NOARGS();

    m_mobility = mobility;
}

void
LoraPhy::SetChannel(Ptr<LoraChannel> channel)
{
    NS_LOG_FUNCTION(this << channel);

    m_channel = channel;
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

void
LoraPhy::SetTxFinishedCallback(TxFinishedCallback callback)
{
    m_txFinishedCallback = callback;
}

Time
LoraPhy::GetTSym(LoraTxParameters txParams)
{
    return Seconds(pow(2, int(txParams.sf)) / (txParams.bandwidthHz));
}

Time
LoraPhy::GetOnAirTime(Ptr<Packet> packet, LoraTxParameters txParams)
{
    NS_LOG_FUNCTION(packet << txParams);

    // The contents of this function are based on [1].
    // [1] SX1272 LoRa modem designer's guide.

    // Compute the symbol duration
    // Bandwidth is in Hz
    double tSym = GetTSym(txParams).GetSeconds();

    // Compute the preamble duration
    double tPreamble = (double(txParams.nPreamble) + 4.25) * tSym;

    // Payload size
    uint32_t pl = packet->GetSize(); // Size in bytes
    NS_LOG_DEBUG("Packet of size " << pl << " bytes");

    // This step is needed since the formula deals with double values.
    // de = 1 when the low data rate optimization is enabled, 0 otherwise
    // h = 1 when header is implicit, 0 otherwise
    double de = txParams.lowDataRateOptimizationEnabled ? 1 : 0;
    double h = txParams.headerDisabled ? 1 : 0;
    double crc = txParams.crcEnabled ? 1 : 0;

    // num and den refer to numerator and denominator of the time on air formula
    double num = 8 * pl - 4 * txParams.sf + 28 + 16 * crc - 20 * h;
    double den = 4 * (txParams.sf - 2 * de);
    double payloadSymbNb =
        8 + std::max(std::ceil(num / den) * (txParams.codingRate + 4), double(0));

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

std::ostream&
operator<<(std::ostream& os, const LoraTxParameters& params)
{
    os << "SF: " << unsigned(params.sf) << ", headerDisabled: " << params.headerDisabled
       << ", codingRate: " << unsigned(params.codingRate) << ", bandwidthHz: " << params.bandwidthHz
       << ", nPreamble: " << params.nPreamble << ", crcEnabled: " << params.crcEnabled
       << ", lowDataRateOptimizationEnabled: " << params.lowDataRateOptimizationEnabled << ")";

    return os;
}
} // namespace lorawan
} // namespace ns3
