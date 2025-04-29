/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "simple-end-device-lora-phy.h"

#include "lora-tag.h"

#include "ns3/log.h"
#include "ns3/simulator.h"

#include <algorithm>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("SimpleEndDeviceLoraPhy");

NS_OBJECT_ENSURE_REGISTERED(SimpleEndDeviceLoraPhy);

TypeId
SimpleEndDeviceLoraPhy::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SimpleEndDeviceLoraPhy")
                            .SetParent<EndDeviceLoraPhy>()
                            .SetGroupName("lorawan")
                            .AddConstructor<SimpleEndDeviceLoraPhy>();

    return tid;
}

// Initialize the device with some common settings.
// These will then be changed by helpers.
SimpleEndDeviceLoraPhy::SimpleEndDeviceLoraPhy()
{
}

SimpleEndDeviceLoraPhy::~SimpleEndDeviceLoraPhy()
{
}

void
SimpleEndDeviceLoraPhy::Send(Ptr<Packet> packet,
                             LoraTxParameters txParams,
                             uint32_t frequencyHz,
                             double txPowerDbm)
{
    NS_LOG_FUNCTION(this << packet << txParams << frequencyHz << txPowerDbm);

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
    m_channel->Send(this, packet, txPowerDbm, txParams, duration, frequencyHz);

    // Schedule a call to signal the transmission end.
    Simulator::Schedule(duration, &SimpleEndDeviceLoraPhy::TxFinished, this, packet);

    // Call the trace source
    if (m_device)
    {
        m_startSending(packet, m_device->GetNode()->GetId());
    }
    else
    {
        m_startSending(packet, 0);
    }
}

void
SimpleEndDeviceLoraPhy::StartReceive(Ptr<Packet> packet,
                                     double rxPowerDbm,
                                     uint8_t sf,
                                     Time duration,
                                     uint32_t frequencyHz)
{
    NS_LOG_FUNCTION(this << packet << rxPowerDbm << unsigned(sf) << duration << frequencyHz);

    // Notify the LoraInterferenceHelper of the impinging signal, and remember
    // the event it creates. This will be used then to correctly handle the end
    // of reception event.
    //
    // We need to do this regardless of our state or frequency, since these could
    // change (and making the interference relevant) while the interference is
    // still incoming.

    Ptr<LoraInterferenceHelper::Event> event;
    event = m_interference.Add(duration, rxPowerDbm, sf, packet, frequencyHz);

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
        // - It uses the spreading factor we are configured to look for
        // - Its receive power is above the device sensitivity for that spreading factor

        // Flag to signal whether we can receive the packet or not
        bool canLockOnPacket = true;

        // Save needed sensitivity
        double sensitivity = EndDeviceLoraPhy::sensitivity[unsigned(sf) - 7];

        // Check frequency
        //////////////////
        if (!IsOnFrequency(frequencyHz))
        {
            NS_LOG_INFO("Packet lost because it's on frequency "
                        << frequencyHz << " Hz and we are listening at " << m_frequencyHz << " Hz");

            // Fire the trace source for this event.
            if (m_device)
            {
                m_wrongFrequency(packet, m_device->GetNode()->GetId());
            }
            else
            {
                m_wrongFrequency(packet, 0);
            }

            canLockOnPacket = false;
        }

        // Check Spreading Factor
        /////////////////////////
        if (sf != m_sf)
        {
            NS_LOG_INFO("Packet lost because it's using SF"
                        << unsigned(sf) << ", while we are listening for SF" << unsigned(m_sf));

            // Fire the trace source for this event.
            if (m_device)
            {
                m_wrongSf(packet, m_device->GetNode()->GetId());
            }
            else
            {
                m_wrongSf(packet, 0);
            }

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
            if (m_device)
            {
                m_underSensitivity(packet, m_device->GetNode()->GetId());
            }
            else
            {
                m_underSensitivity(packet, 0);
            }

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
            NS_LOG_INFO("Scheduling reception of a packet. End in " << duration.As(Time::S));

            Simulator::Schedule(duration, &LoraPhy::EndReceive, this, packet, event);

            // Fire the beginning of reception trace source
            m_phyRxBeginTrace(packet);
        }
    }
    }
}

void
SimpleEndDeviceLoraPhy::EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event)
{
    NS_LOG_FUNCTION(this << packet << event);

    // Automatically switch to Standby in either case
    SwitchToStandby();

    // Fire the trace source
    m_phyRxEndTrace(packet);

    // Call the LoraInterferenceHelper to determine whether there was destructive
    // interference on this event.
    bool packetDestroyed = m_interference.IsDestroyedByInterference(event);

    // Fire the trace source if packet was destroyed
    if (packetDestroyed)
    {
        NS_LOG_INFO("Packet destroyed by interference");

        if (m_device)
        {
            m_interferedPacket(packet, m_device->GetNode()->GetId());
        }
        else
        {
            m_interferedPacket(packet, 0);
        }

        // If there is one, perform the callback to inform the upper layer of the
        // lost packet
        if (!m_rxFailedCallback.IsNull())
        {
            m_rxFailedCallback(packet);
        }
    }
    else
    {
        NS_LOG_INFO("Packet received correctly");

        if (m_device)
        {
            m_successfullyReceivedPacket(packet, m_device->GetNode()->GetId());
        }
        else
        {
            m_successfullyReceivedPacket(packet, 0);
        }

        // If there is one, perform the callback to inform the upper layer
        if (!m_rxOkCallback.IsNull())
        {
            LoraTag tag;
            packet->RemovePacketTag(tag);
            tag.SetReceivePower(event->GetRxPowerdBm());
            tag.SetFrequency(event->GetFrequency());
            packet->AddPacketTag(tag);
            m_rxOkCallback(packet);
        }
    }
}
} // namespace lorawan
} // namespace ns3
