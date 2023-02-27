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

#include "ns3/end-device-lora-phy.h"

#include "ns3/log.h"
#include "ns3/lora-tag.h"
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
EndDeviceLoraPhy::GetTypeId(void)
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
    : m_frequency(868100000),
      m_sf(7),
      m_state(SLEEP)
{
}

EndDeviceLoraPhy::~EndDeviceLoraPhy()
{
}

void
EndDeviceLoraPhy::Send(Ptr<Packet> packet,
                       LoraTxParameters txParams,
                       double frequency,
                       double txPowerDbm)
{
    NS_LOG_FUNCTION(this << packet << txParams << frequency << txPowerDbm);
    NS_LOG_INFO("Current state: " << m_state);
    // We must be either in STANDBY or SLEEP mode to send a packet
    if (m_state != STANDBY && m_state != SLEEP)
    {
        NS_LOG_INFO("Cannot send because device is currently not in STANDBY or SLEEP mode");
        return;
    }
    // Compute the duration of the transmission
    Time duration = GetOnAirTime(packet, txParams);
    // We can send the packet: switch to the TX state
    SwitchToTx(txPowerDbm);
    // Tag the packet with information about its Spreading Factor
    LoraTag tag;
    packet->RemovePacketTag(tag);
    tag.SetSpreadingFactor(txParams.sf);
    packet->AddPacketTag(tag);
    // Send the packet over the channel
    NS_LOG_INFO("Sending the packet in the channel");
    m_channel->Send(this, packet, txPowerDbm, txParams.sf, duration, frequency);
    // Schedule the txFinished call
    Simulator::Schedule(duration, &EndDeviceLoraPhy::TxFinished, this, packet);
    // Call the trace source
    m_startSending(packet, m_context);
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
        if (frequency != m_frequency)
        {
            NS_LOG_INFO("Packet lost because it's on frequency "
                        << frequency << " Hz and we are listening at " << m_frequency << " Hz");
            // Fire the trace source for this event.
            m_wrongFrequency(packet, m_context);
            canLockOnPacket = false;
        }
        // Check Spreading Factor
        /////////////////////////
        if (sf != m_sf)
        {
            NS_LOG_INFO("Packet lost because it's using SF"
                        << unsigned(sf) << ", while we are listening for SF" << unsigned(m_sf));
            // Fire the trace source for this event.
            m_wrongSf(packet, m_context);
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
            m_underSensitivity(packet, m_context);
            canLockOnPacket = false;
        }
        // Check if one of the above failed
        ///////////////////////////////////
        if (canLockOnPacket)
        {
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

bool
EndDeviceLoraPhy::IsTransmitting(void)
{
    NS_LOG_FUNCTION_NOARGS();
    return m_state == TX;
}

void
EndDeviceLoraPhy::SwitchToStandby(void)
{
    NS_LOG_FUNCTION_NOARGS();
    m_state = STANDBY;
    // Notify listeners of the state change
    for (const auto& l : m_listeners)
        l->NotifyStandby();
}

void
EndDeviceLoraPhy::SwitchToSleep(void)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(m_state == STANDBY);
    m_state = SLEEP;
    // Notify listeners of the state change
    for (const auto& l : m_listeners)
        l->NotifySleep();
}

void
EndDeviceLoraPhy::SetSpreadingFactor(uint8_t sf)
{
    NS_LOG_FUNCTION_NOARGS();
    m_sf = sf;
}

void
EndDeviceLoraPhy::SetFrequency(double frequency)
{
    NS_LOG_FUNCTION(this << frequency);
    m_frequency = frequency;
}

EndDeviceLoraPhy::State
EndDeviceLoraPhy::GetState(void)
{
    NS_LOG_FUNCTION_NOARGS();
    return m_state;
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
        m_listeners.erase(i);
}

void
EndDeviceLoraPhy::DoDispose()
{
    NS_LOG_FUNCTION(this);
    for (auto l : m_listeners)
    {
        delete l;
    }
    m_listeners.clear();
    LoraPhy::DoDispose();
}

void
EndDeviceLoraPhy::EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event)
{
    NS_LOG_FUNCTION(this << packet << event);
    // Automatically switch to Standby in either case
    SwitchToStandby();
    // Fire the trace source
    m_phyRxEndTrace(packet);
    // Call the LoraInterferenceHelper to determine whether there was destructive
    // interference on this event.
    if (m_interference->IsDestroyedByInterference(event))
    {
        NS_LOG_INFO("Packet destroyed by interference");
        // Fire the trace source
        m_interferedPacket(packet, m_context);
        // If there is one, perform the callback to inform the upper layer of the
        // lost packet
        if (!m_rxFailedCallback.IsNull())
            m_rxFailedCallback(packet);
    }
    else
    {
        NS_LOG_INFO("Packet received correctly");
        // Set the receive power, frequency and SNR of this packet in the LoraTag:
        // here this information is useful for filling the packet sniffing header.
        LoraTag tag;
        packet->RemovePacketTag(tag);
        tag.SetReceivePower(event->GetRxPowerdBm());
        tag.SetFrequency(event->GetFrequency());
        tag.SetSnr(RxPowerToSNR(event->GetRxPowerdBm()));
        tag.SetReceptionTime(Simulator::Now());
        packet->AddPacketTag(tag);
        // If there is one, perform the callback to inform the upper layer
        if (!m_rxOkCallback.IsNull())
            m_rxOkCallback(packet);
        // Fire the trace source
        m_successfullyReceivedPacket(packet, m_context);
        // Fire the sniffer trace source
        if (!m_phySniffRxTrace.IsEmpty())
            m_phySniffRxTrace(packet);
    }
}

void
EndDeviceLoraPhy::TxFinished(Ptr<Packet> packet)
{
    // Switch back to STANDBY mode.
    // For reference see SX1272 datasheet, section 4.1.6
    SwitchToStandby();
    // Forward packet to the upper layer
    if (!m_txFinishedCallback.IsNull())
        m_txFinishedCallback(packet);
    // Schedule the sniffer trace source
    if (!m_phySniffTxTrace.IsEmpty())
        m_phySniffTxTrace(packet);
}

void
EndDeviceLoraPhy::SwitchToRx(void)
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
    NS_ASSERT(m_state != RX);
    m_state = TX;
    // Notify listeners of the state change
    for (const auto& l : m_listeners)
        l->NotifyTxStart(txPowerDbm);
}

// Downlink sensitivity (from SX1272 datasheet)
// {SF7, SF8, SF9, SF10, SF11, SF12}
// These sensitivites are for a bandwidth of 125000 Hz
const double EndDeviceLoraPhy::sensitivity[6] = {-124, -127, -130, -133, -135, -137};

} // namespace lorawan
} // namespace ns3
