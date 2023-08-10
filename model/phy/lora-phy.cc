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
 *
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "lora-phy.h"

#include "ns3/node.h"

#define NOISE_FIGURE 6 //! Noise Figure (dB)

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
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("SnifferRx",
                            "Trace source simulating a device "
                            "sniffing all received frames",
                            MakeTraceSourceAccessor(&LoraPhy::m_phySniffRxTrace),
                            "ns3::LoraPhy::SnifferRxTracedCallback")
            .AddTraceSource("SnifferTx",
                            "Trace source simulating a device "
                            "sniffing all frames being transmitted",
                            MakeTraceSourceAccessor(&LoraPhy::m_phySniffTxTrace),
                            "ns3::LoraPhy::SnifferRxTracedCallback");
    return tid;
}

LoraPhy::LoraPhy()
    : m_nodeId(0)
{
    NS_LOG_FUNCTION(this);
    m_interference = CreateObject<LoraInterferenceHelper>();
}

LoraPhy::~LoraPhy()
{
    NS_LOG_FUNCTION(this);
}

void
LoraPhy::DoInitialize()
{
    NS_LOG_FUNCTION(this);
    // This method ensures that the local mobility model pointer holds
    // a pointer to the Node's aggregated mobility model (if one exists)
    // in the case that the user has not directly called SetMobility()
    // on this LoraPhy during simulation setup.  If the mobility model
    // needs to be added or changed during simulation runtime, users must
    // call SetMobility() on this object.
    if (!m_mobility)
    {
        NS_ABORT_MSG_UNLESS(bool(m_device) && bool(m_device->GetNode()),
                            "Either install a MobilityModel on this object or ensure that this "
                            "object is part of a Node and NetDevice");
        m_mobility = m_device->GetNode()->GetObject<MobilityModel>();
        if (!m_mobility)
        {
            NS_LOG_WARN("Mobility not found, propagation models might not work properly");
        }
    }

    // Get node id (if possible) to format context in tracing callbacks
    if (m_device && m_device->GetNode())
    {
        m_nodeId = m_device->GetNode()->GetId();
    }
    Object::DoInitialize();
}

void
LoraPhy::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_device = nullptr;
    m_mobility = nullptr;
    if (m_interference)
    {
        m_interference->Dispose();
    }
    m_interference = nullptr;
    m_channel = nullptr;
    Object::DoDispose();
}

void
LoraPhy::SetInterferenceHelper(const Ptr<LoraInterferenceHelper> helper)
{
    m_interference = helper;
}

void
LoraPhy::SetChannel(Ptr<LoraChannel> channel)
{
    NS_LOG_FUNCTION(this << channel);
    m_channel = channel;
    m_channel->Add(this);
}

Ptr<LoraChannel>
LoraPhy::GetChannel() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_channel;
}

Ptr<MobilityModel>
LoraPhy::GetMobility() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_mobility;
}

void
LoraPhy::SetMobility(Ptr<MobilityModel> mobility)
{
    NS_LOG_FUNCTION(this << mobility);
    m_mobility = mobility;
}

Ptr<NetDevice>
LoraPhy::GetDevice() const
{
    NS_LOG_FUNCTION_NOARGS();
    return m_device;
}

void
LoraPhy::SetDevice(Ptr<NetDevice> device)
{
    NS_LOG_FUNCTION(this << device);
    m_device = device;
}

void
LoraPhy::SetReceiveOkCallback(RxOkCallback callback)
{
    NS_LOG_FUNCTION_NOARGS();
    m_rxOkCallback = callback;
}

void
LoraPhy::SetReceiveFailedCallback(RxFailedCallback callback)
{
    NS_LOG_FUNCTION_NOARGS();
    m_rxFailedCallback = callback;
}

void
LoraPhy::SetTxFinishedCallback(TxFinishedCallback callback)
{
    NS_LOG_FUNCTION_NOARGS();
    m_txFinishedCallback = callback;
}

Time
LoraPhy::GetTSym(const LoraPhyTxParameters& txParams)
{
    NS_LOG_FUNCTION(txParams);
    return Seconds(pow(2, int(txParams.sf)) / (txParams.bandwidthHz));
}

Time
LoraPhy::GetTimeOnAir(Ptr<const Packet> packet, const LoraPhyTxParameters& txParams)
{
    NS_LOG_FUNCTION(packet << txParams);

    // The contents of this function are based on [1].
    // [1] SX1272 LoRa modem designer's guide.

    // Compute the symbol duration
    Time tSym = GetTSym(txParams);

    // Compute the preamble duration
    Time tPreamble = (double(txParams.nPreamble) + 4.25) * tSym;

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
    Time tPayload = payloadSymbNb * tSym;

    NS_LOG_DEBUG("Time computation: num = " << num << ", den = " << den << ", payloadSymbNb = "
                                            << payloadSymbNb << ", tSym = " << tSym);
    NS_LOG_DEBUG("tPreamble = " << tPreamble);
    NS_LOG_DEBUG("tPayload = " << tPayload);
    NS_LOG_DEBUG("Total time = " << tPreamble + tPayload);

    // Compute and return the total packet on-air time
    return tPreamble + tPayload;
}

double
LoraPhy::RxPowerToSNR(double transmissionPower, double bandwidth)
{
    NS_LOG_FUNCTION(transmissionPower);
    // The following conversion ignores interfering packets
    return transmissionPower + 174 - 10 * log10(bandwidth) - NOISE_FIGURE;
}

std::ostream&
operator<<(std::ostream& os, const LoraPhyTxParameters& params)
{
    os << "SF: " << unsigned(params.sf) << ", headerDisabled: " << params.headerDisabled
       << ", codingRate: " << unsigned(params.codingRate) << ", bandwidthHz: " << params.bandwidthHz
       << ", nPreamble: " << params.nPreamble << ", crcEnabled: " << params.crcEnabled
       << ", lowDataRateOptimizationEnabled: " << params.lowDataRateOptimizationEnabled << ")";

    return os;
}
} // namespace lorawan
} // namespace ns3
