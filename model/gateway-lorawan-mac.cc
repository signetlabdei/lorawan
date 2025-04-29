/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "gateway-lorawan-mac.h"

#include "lora-frame-header.h"
#include "lora-net-device.h"
#include "lorawan-mac-header.h"

#include "ns3/log.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("GatewayLorawanMac");

NS_OBJECT_ENSURE_REGISTERED(GatewayLorawanMac);

TypeId
GatewayLorawanMac::GetTypeId()
{
    static TypeId tid = TypeId("ns3::GatewayLorawanMac")
                            .SetParent<LorawanMac>()
                            .AddConstructor<GatewayLorawanMac>()
                            .SetGroupName("lorawan");
    return tid;
}

GatewayLorawanMac::GatewayLorawanMac()
{
    NS_LOG_FUNCTION(this);
}

GatewayLorawanMac::~GatewayLorawanMac()
{
    NS_LOG_FUNCTION(this);
}

void
GatewayLorawanMac::Send(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    // Get data rate to send this packet with
    LoraTag tag;
    packet->RemovePacketTag(tag);
    uint8_t dataRate = tag.GetDataRate();
    uint32_t frequencyHz = tag.GetFrequency();
    NS_LOG_DEBUG("DR: " << unsigned(dataRate));
    NS_LOG_DEBUG("SF: " << unsigned(GetSfFromDataRate(dataRate)));
    NS_LOG_DEBUG("BW: " << GetBandwidthFromDataRate(dataRate));
    NS_LOG_DEBUG("Freq: " << frequencyHz << " Hz");
    packet->AddPacketTag(tag);

    // Make sure we can transmit this packet
    if (GetWaitTime(frequencyHz).IsStrictlyPositive())
    {
        // We cannot send now!
        NS_LOG_WARN("Trying to send a packet but Duty Cycle won't allow it. Aborting.");
        return;
    }

    LoraTxParameters params;
    params.sf = GetSfFromDataRate(dataRate);
    params.headerDisabled = false;
    params.codingRate = 1;
    params.bandwidthHz = GetBandwidthFromDataRate(dataRate);
    params.nPreamble = 8;
    params.crcEnabled = true;
    params.lowDataRateOptimizationEnabled = LoraPhy::GetTSym(params) > MilliSeconds(16);

    // Get the duration
    Time duration = LoraPhy::GetOnAirTime(packet, params);

    NS_LOG_DEBUG("Duration: " << duration.As(Time::S));

    // Find the channel with the desired frequency
    double sendingPower = m_channelHelper->GetTxPowerForChannel(frequencyHz);

    // Add the event to the channelHelper to keep track of duty cycle
    m_channelHelper->AddEvent(duration, frequencyHz);

    // Send the packet to the PHY layer to send it on the channel
    m_phy->Send(packet, params, frequencyHz, sendingPower);

    m_sentNewPacket(packet);
}

bool
GatewayLorawanMac::IsTransmitting()
{
    return m_phy->IsTransmitting();
}

void
GatewayLorawanMac::Receive(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);

    // Make a copy of the packet to work on
    Ptr<Packet> packetCopy = packet->Copy();

    // Only forward the packet if it's uplink
    LorawanMacHeader macHdr;
    packetCopy->PeekHeader(macHdr);

    if (macHdr.IsUplink())
    {
        DynamicCast<LoraNetDevice>(m_device)->Receive(packetCopy);

        NS_LOG_DEBUG("Received packet: " << packet);

        m_receivedPacket(packet);
    }
    else
    {
        NS_LOG_DEBUG("Not forwarding downlink message to NetDevice");
    }
}

void
GatewayLorawanMac::FailedReception(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
}

void
GatewayLorawanMac::TxFinished(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION_NOARGS();
}

Time
GatewayLorawanMac::GetWaitTime(uint32_t frequencyHz)
{
    NS_LOG_FUNCTION_NOARGS();
    return m_channelHelper->GetWaitTime(frequencyHz);
}
} // namespace lorawan
} // namespace ns3
