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

#include "end-device-lora-phy.h"

#include "ns3/log.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lora-tag.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/simulator.h"

#include <algorithm>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("EndDeviceLoraPhy");

NS_OBJECT_ENSURE_REGISTERED(EndDeviceLoraPhy);

/**************************
 *  Listener destructor  *
 *************************/

EndDeviceLoraPhyListener::~EndDeviceLoraPhyListener()
{
}

TypeId
EndDeviceLoraPhy::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::EndDeviceLoraPhy")
            .SetParent<LoraPhy>()
            .SetGroupName("lorawan")
            .AddConstructor<EndDeviceLoraPhy>()
            .AddTraceSource("LostPacketBecauseWrongFrequency",
                            "Trace source indicating a packet "
                            "could not be correctly decoded because"
                            "the ED was listening on a different frequency",
                            MakeTraceSourceAccessor(&EndDeviceLoraPhy::m_wrongFrequency),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("LostPacketBecauseWrongSpreadingFactor",
                            "Trace source indicating a packet "
                            "could not be correctly decoded because"
                            "the ED was listening for a different Spreading Factor",
                            MakeTraceSourceAccessor(&EndDeviceLoraPhy::m_wrongSf),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("EndDeviceState",
                            "The current state of the device",
                            MakeTraceSourceAccessor(&EndDeviceLoraPhy::m_state),
                            "ns3::TracedValueCallback::EndDeviceLoraPhy::State");
    return tid;
}

// Initialize the device with some common settings.
// These will then be changed by helpers.
EndDeviceLoraPhy::EndDeviceLoraPhy()
    : m_state(SLEEP),
      m_rxSf(12),
      m_rxFrequency(0)
{
    NS_LOG_FUNCTION(this);
}

EndDeviceLoraPhy::~EndDeviceLoraPhy()
{
    NS_LOG_FUNCTION(this);
}

void
EndDeviceLoraPhy::Send(Ptr<Packet> packet,
                       LoraPhyTxParameters txParams,
                       double frequency,
                       double txPowerDbm)
{
    NS_LOG_FUNCTION(this << packet << txParams << frequency << txPowerDbm);

    NS_LOG_INFO("Current state: " << m_state);
    // We must be in STANDBY mode to send a packet
    if (m_state != STANDBY)
    {
        NS_LOG_ERROR("Cannot send because device is currently not in STANDBY mode");
        return;
    }

    // Tag the packet with information about its Spreading Factor
    LoraTag tag;
    packet->RemovePacketTag(tag);
    tag.SetTxParameters(txParams);
    packet->AddPacketTag(tag);

    // Get the time a packet with these parameters will take to be transmitted
    Time duration = GetTimeOnAir(packet, txParams);
    NS_LOG_DEBUG("Duration of packet: " << duration.As(Time::MS) << ", SF"
                                        << unsigned(txParams.sf));

    // We can send the packet: switch to the TX state
    SwitchToTx(txPowerDbm);
    // Send the packet over the channel
    NS_LOG_INFO("Sending the packet in the channel");
    m_channel->Send(this, packet, txPowerDbm, txParams.sf, duration, frequency);
    // Call the trace source
    m_startSending(packet, m_nodeId);

    // Schedule the txFinished call
    Simulator::Schedule(duration, &EndDeviceLoraPhy::TxFinished, this, packet);
}

void
EndDeviceLoraPhy::TxFinished(Ptr<Packet> packet)
{
    NS_LOG_FUNCTION(this << packet);
    // Switch back to STANDBY mode.
    // For reference see SX1272 datasheet, section 4.1.6
    SwitchToStandby();
    // Forward packet to the upper layer
    if (!m_txFinishedCallback.IsNull())
    {
        m_txFinishedCallback(packet);
    }
    // Schedule the sniffer trace source
    if (!m_phySniffTxTrace.IsEmpty())
    {
        m_phySniffTxTrace(packet);
    }
}

void
EndDeviceLoraPhy::StartReceive(Ptr<Packet> packet,
                               double rxPowerDbm,
                               uint8_t sf,
                               Time duration,
                               double frequency)
{
    NS_LOG_FUNCTION(this << packet << rxPowerDbm << unsigned(sf) << duration << frequency);
    // Notify the LoraInterferenceHelper of the impinging signal, and remember
    // the event it creates. This will be used then to correctly handle the end
    // of reception event.
    //
    // We need to do this regardless of our state or frequency, since these could
    // change (and making the interference relevant) while the interference is
    // still incoming.
    auto event = m_interference->Add(duration, rxPowerDbm, sf, packet, frequency);
    // Switch on the current PHY state
    switch (m_state)
    {
    // In the SLEEP, TX and RX cases we cannot receive the packet: we only add
    // it to the list of interferers and do not schedule an EndReceive event for
    // it.
    case SLEEP: {
        NS_LOG_INFO("Dropping packet because device is in SLEEP state");
        break;
    }
    case TX: {
        NS_LOG_INFO("Dropping packet because device is in TX state");
        break;
    }
    case RX: {
        NS_LOG_INFO("Dropping packet because device is already in RX state");
        break;
    }
    // If we are in STANDBY mode, we can potentially lock on the currently
    // incoming transmission
    case STANDBY: {
        // There are a series of properties the packet needs to respect in order
        // for us to be able to lock on it:
        // - It's on frequency we are listening on
        // - It uses the SF we are configured to look for
        // - Its receive power is above the device sensitivity for that SF

        // Flag to signal whether we can receive the packet or not
        bool canLockOnPacket = true;
        // Save needed sensitivity
        double sensitivity = EndDeviceLoraPhy::sensitivity[unsigned(sf) - 7];
        // Check frequency (manage double comparison)
        //////////////////
        if (frequency != m_rxFrequency)
        {
            NS_LOG_INFO("Packet lost because it's on frequency "
                        << frequency << " Hz and we are listening at " << m_rxFrequency << " Hz");
            // Fire the trace source for this event.
            m_wrongFrequency(packet, m_nodeId);
            canLockOnPacket = false;
        }
        // Check Spreading Factor
        /////////////////////////
        if (sf != m_rxSf)
        {
            NS_LOG_INFO("Packet lost because it's using SF"
                        << unsigned(sf) << ", while we are listening for SF" << unsigned(m_rxSf));
            // Fire the trace source for this event.
            m_wrongSf(packet, m_nodeId);
            canLockOnPacket = false;
        }
        // Check Sensitivity
        ////////////////////
        if (rxPowerDbm < sensitivity)
        {
            NS_LOG_INFO("Dropping packet reception of packet with sf = "
                        << unsigned(sf) << " because under the sensitivity of " << sensitivity
                        << " dBm");
            // Fire the trace source for this event.
            m_underSensitivity(packet, m_nodeId);
            canLockOnPacket = false;
        }

        // Check if one of the above failed
        ///////////////////////////////////
        if (canLockOnPacket)
        {
            // Packet Filtering based on Preamble Start (SX1272 Datasheet)
            duration = GetFilteredDuration(packet, duration);
            // Switch to RX state
            // EndReceive will handle the switch back to STANDBY state
            SwitchToRx();
            // Schedule the end of the reception of the packet
            NS_LOG_INFO("Scheduling reception of a packet. End in " << duration.GetSeconds()
                                                                    << " seconds");
            Simulator::Schedule(duration, &EndDeviceLoraPhy::EndReceive, this, packet, event);
            // Fire the beginning of reception trace source
            m_phyRxBeginTrace(packet);
        }
    }
    }
}

Time
EndDeviceLoraPhy::GetFilteredDuration(Ptr<const Packet> packet, Time duration) const
{
    // Work on a packet copy
    auto copy = packet->Copy();
    LorawanMacHeader mHdr;
    copy->RemoveHeader(mHdr);
    NS_ASSERT_MSG(!mHdr.IsUplink(), "We should not be able to lock onto uplink preambles");
    // Check address
    LoraFrameHeader fHdr;
    fHdr.SetAsDownlink();
    copy->RemoveHeader(fHdr);
    if (m_address != fHdr.GetAddress())
    {
        // Get transmission parameters
        LoraTag tag;
        copy->RemovePacketTag(tag);
        // MHDR (1B) + 4B of Addr in FHdr
        return GetTimeOnAir(Create<Packet>(5), tag.GetTxParameters());
    }
    return duration;
}

void
EndDeviceLoraPhy::EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event)
{
    NS_LOG_FUNCTION(this << packet << event);
    // Automatically switch to Standby
    SwitchToStandby();
    // Fire the trace source
    m_phyRxEndTrace(packet);

    // Check early returns from filtered packets
    auto copy = packet->Copy();
    LorawanMacHeader mHdr;
    copy->RemoveHeader(mHdr);
    LoraFrameHeader fHdr;
    fHdr.SetAsDownlink();
    copy->RemoveHeader(fHdr);
    // Check address
    if (m_address != fHdr.GetAddress())
    {
        NS_LOG_INFO("Packet filtered early due to wrong destination address");
        // If there is one, perform the callback to inform the upper layer of the
        // failed reception attempt
        if (!m_rxFailedCallback.IsNull())
        {
            m_rxFailedCallback(packet);
        }
        // Possibly a trace source callback could be added here
        return;
    }

    // Call the LoraInterferenceHelper to determine whether there was destructive
    // interference on this event.
    uint8_t packetDestroyed = m_interference->IsDestroyedByInterference(event);
    if (packetDestroyed)
    {
        NS_LOG_INFO("Packet destroyed by interference");
        // Update the packet's LoraTag
        LoraTag tag;
        packet->RemovePacketTag(tag);
        tag.SetDestroyedBy(packetDestroyed);
        tag.SetReceptionTime(Simulator::Now());
        packet->AddPacketTag(tag);
        // If there is one, perform the callback to inform the upper layer of the
        // lost packet
        if (!m_rxFailedCallback.IsNull())
        {
            m_rxFailedCallback(packet);
        }
        // Fire the trace source
        m_interferedPacket(packet, m_nodeId);
        return;
    }

    NS_LOG_INFO("Packet received correctly");
    // Set the receive power, frequency and SNR of this packet in the LoraTag:
    // here this information is useful for filling the packet sniffing header.
    LoraTag tag;
    packet->RemovePacketTag(tag);
    tag.SetReceptionTime(Simulator::Now());
    tag.SetReceivePower(event->GetRxPowerdBm());
    tag.SetSnr(RxPowerToSNR(event->GetRxPowerdBm()));
    packet->AddPacketTag(tag);
    // If there is one, perform the callback to inform the upper layer
    if (!m_rxOkCallback.IsNull())
    {
        m_rxOkCallback(packet);
    }
    // Fire the trace source
    m_successfullyReceivedPacket(packet, m_nodeId);
    // Fire the sniffer trace source
    if (!m_phySniffRxTrace.IsEmpty())
    {
        m_phySniffRxTrace(packet);
    }
}

void
EndDeviceLoraPhy::SwitchToStandby()
{
    NS_LOG_FUNCTION_NOARGS();
    m_state = STANDBY;
    // Notify listeners of the state change
    for (const auto& l : m_listeners)
        l->NotifyStandby();
}

void
EndDeviceLoraPhy::SwitchToSleep()
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(m_state == STANDBY);
    m_state = SLEEP;
    // Notify listeners of the state change
    for (const auto& l : m_listeners)
        l->NotifySleep();
}

void
EndDeviceLoraPhy::SwitchToRx()
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(m_state == STANDBY);
    m_state = RX;
    // Notify listeners of the state change
    for (const auto& l : m_listeners)
        l->NotifyRxStart();
}

void
EndDeviceLoraPhy::SwitchToTx(double txPowerDbm)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(m_state == STANDBY);
    m_state = TX;
    // Notify listeners of the state change
    for (const auto& l : m_listeners)
        l->NotifyTxStart(txPowerDbm);
}

EndDeviceLoraPhy::State
EndDeviceLoraPhy::GetState()
{
    NS_LOG_FUNCTION_NOARGS();
    return m_state;
}

bool
EndDeviceLoraPhy::IsTransmitting()
{
    NS_LOG_FUNCTION_NOARGS();
    return m_state == TX;
}

void
EndDeviceLoraPhy::SetRxSpreadingFactor(uint8_t sf)
{
    NS_LOG_FUNCTION_NOARGS();
    m_rxSf = sf;
}

void
EndDeviceLoraPhy::SetRxFrequency(double frequency)
{
    NS_LOG_FUNCTION(this << frequency);
    m_rxFrequency = frequency;
}

void
EndDeviceLoraPhy::SetDeviceAddress(LoraDeviceAddress address)
{
    m_address = address;
}

void
EndDeviceLoraPhy::RegisterListener(EndDeviceLoraPhyListener* listener)
{
    NS_LOG_FUNCTION(this << listener);
    m_listeners.push_back(listener);
}

void
EndDeviceLoraPhy::UnregisterListener(EndDeviceLoraPhyListener* listener)
{
    NS_LOG_FUNCTION(this << listener);
    auto i = find(m_listeners.begin(), m_listeners.end(), listener);
    if (i != m_listeners.end())
    {
        m_listeners.erase(i);
    }
}

void
EndDeviceLoraPhy::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_listeners.clear();
    LoraPhy::DoDispose();
}

// Downlink sensitivity (from SX1272 datasheet)
// {SF7, SF8, SF9, SF10, SF11, SF12}
// These sensitivites are for a bandwidth of 125000 Hz
const double EndDeviceLoraPhy::sensitivity[6] = {-124, -127, -130, -133, -135, -137};

} // namespace lorawan
} // namespace ns3
