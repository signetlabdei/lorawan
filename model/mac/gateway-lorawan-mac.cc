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
 * 11/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "gateway-lorawan-mac.h"

#include "ns3/log.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lorawan-mac-header.h"

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

    // Get DataRate to send this packet with
    LoraTag tag;
    packet->PeekPacketTag(tag);
    uint8_t dataRate = tag.GetDataRate();
    double frequency = tag.GetFrequency();

    // Configure PHY tx params
    m_txParams.sf = GetSfFromDataRate(dataRate);
    m_txParams.bandwidthHz = GetBandwidthFromDataRate(dataRate);
    m_txParams.lowDataRateOptimizationEnabled = LoraPhy::GetTSym(m_txParams) > MilliSeconds(16);
    NS_LOG_DEBUG("DR: " << unsigned(dataRate));
    NS_LOG_DEBUG("SF: " << unsigned(m_txParams.sf));
    NS_LOG_DEBUG("BW: " << m_txParams.bandwidthHz << " Hz");

    // Find the transmission power for the desired frequency (always max possible)
    double txPower = m_channelManager->GetTxPowerForChannel(Create<LogicalChannel>(frequency));
    NS_LOG_DEBUG("Freq: " << frequency << " Hz");

    // Get the duration
    Time duration = m_phy->GetTimeOnAir(packet, m_txParams);
    NS_LOG_DEBUG("Duration: " << duration.GetSeconds());
    // Add the event to the channelHelper to keep track of duty cycle
    m_channelManager->AddEvent(duration, Create<LogicalChannel>(frequency));

    // Send the packet to the PHY layer to send it on the channel
    m_phy->Send(packet, m_txParams, frequency, txPower);
    // Fire trace source
    m_sentNewPacket(packet);
}

void
GatewayLorawanMac::TxFinished(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION_NOARGS();
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
    LorawanMacHeader mHdr;
    packetCopy->PeekHeader(mHdr);

    if (mHdr.IsUplink())
    {
        if (!m_receiveCallback.IsNull())
        {
            m_receiveCallback(this, packetCopy);
        }

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

Time
GatewayLorawanMac::GetWaitingTime(double frequency)
{
    NS_LOG_FUNCTION_NOARGS();

    return m_channelManager->GetWaitingTime(Create<LogicalChannel>(frequency));
}
} // namespace lorawan
} // namespace ns3
