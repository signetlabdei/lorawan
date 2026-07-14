/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "simple-end-device-lora-phy.h"

#include "lora-channel.h"
#include "lora-tag.h"

#include "ns3/node.h"
#include "ns3/simulator.h"

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

SimpleEndDeviceLoraPhy::SimpleEndDeviceLoraPhy()
{
    NS_LOG_FUNCTION(this);
}

SimpleEndDeviceLoraPhy::~SimpleEndDeviceLoraPhy()
{
    NS_LOG_FUNCTION(this);
}

void
SimpleEndDeviceLoraPhy::Send(Ptr<Packet> packet,
                             uint32_t frequencyHz,
                             IQPolarity iqPolarity,
                             const LoraTxParameters& txParams,
                             double txPowerDbm)
{
    NS_LOG_FUNCTION(this << packet << frequencyHz << iqPolarity << txParams << txPowerDbm);

    // We must be either in SLEEP or STANDBY mode to send a packet
    if (auto state = GetState(); state != State::SLEEP && state != State::STANDBY)
    {
        NS_LOG_ERROR("Cannot send because device is currently not in SLEEP or STANDBY mode");
        return;
    }

    // Write hardware registers of the LoRa chip
    m_regs.spreadingFactor = txParams.spreadingFactor;
    m_regs.bandwidthHz = txParams.bandwidthHz;
    m_regs.codingRate = txParams.codingRate;
    m_regs.lowDataRateOptimize = txParams.lowDataRateOptimize;
    m_regs.preambleLenSymb = txParams.preambleLenSymb;
    m_regs.implicitHeader = txParams.implicitHeader;
    m_regs.payloadLenBytes = packet->GetSize();
    m_regs.crcEnabled = txParams.crcEnabled;
    m_regs.iqPolarity = iqPolarity;
    m_regs.frequencyHz = frequencyHz;
    m_regs.txPowerDbm = txPowerDbm;

    // Tag the packet with information about its Spreading Factor
    LoraTag tag;
    packet->RemovePacketTag(tag);
    tag.SetSpreadingFactor(m_regs.spreadingFactor);
    packet->AddPacketTag(tag);

    // Switch to the TX state
    RequestTxMode();
    // Call the trace source
    m_startSending(packet, (m_device) ? m_device->GetNode()->GetId() : 0);

    // Compute the duration of the modulated transmission
    Time duration = GetTimeOnAir(packet->GetSize(), txParams);
    // Propagate the transmission over the channel (schedule StartReceive for other nodes)
    m_channel->Send(this, packet, frequencyHz, iqPolarity, txParams, txPowerDbm, duration);

    // Schedule a call to self to signal the transmission modulation end
    Simulator::Schedule(duration, &SimpleEndDeviceLoraPhy::TxFinished, this, packet);
}

void
SimpleEndDeviceLoraPhy::StartReceive(Ptr<Packet> packet,
                                     uint32_t frequencyHz,
                                     IQPolarity iqPolarity,
                                     uint8_t spreadingFactor,
                                     double rxPowerDbm,
                                     Time duration)
{
    NS_LOG_FUNCTION(this << packet << frequencyHz << iqPolarity << unsigned(spreadingFactor)
                         << rxPowerDbm << duration);

    // Notify the LoraInterferenceHelper of the impinging signal, and remember
    // the event it creates. This will be used then to correctly handle the end
    // of reception event.
    //
    // We need to do this regardless of our state or frequency, since these could
    // change (and making the interference relevant) while the interference is
    // still incoming.
    auto event = m_interference.Add(duration, rxPowerDbm, spreadingFactor, packet, frequencyHz);

    // Check that the current PHY state is RX_ENABLED
    if (auto state = GetState(); state != State::RX_ENABLED)
    {
        if (state == State::RX_ACTIVE)
        {
            NS_LOG_INFO("Device already locked onto another transmission");
        }
        else
        {
            NS_LOG_INFO("Dropping packet because device is in " << state << " state");
        }
        return;
    }

    // If we are in RX_ENABLED mode, we can potentially lock on the currently incoming
    // transmission There are a series of properties the packet needs to respect in order
    // for us to be able to lock on it:
    // - It's on frequency we are listening on
    // - It's using the right modulation polarity (uplink or downlink)
    // - It uses the spreading factor we are configured to look for
    // - Its receive power is above the device sensitivity for that spreading factor

    // Check frequency
    if (frequencyHz != m_regs.frequencyHz)
    {
        NS_LOG_INFO("Packet ignored because it's on frequency "
                    << frequencyHz << " Hz and we are listening to " << m_regs.frequencyHz
                    << " Hz");
        // Fire the trace source for this event.
        m_wrongFrequency(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
        return;
    }
    // Check modulation I/Q polarity
    else if (iqPolarity != m_regs.iqPolarity)
    {
        NS_LOG_INFO("Packet ignored because it's " << iqPolarity << "LINK and we are listening for "
                                                   << m_regs.iqPolarity << "LINK transmissions");
        // Fire the trace source for this event.
        m_wrongPolarity(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
        return;
    }
    // Check Spreading Factor
    else if (spreadingFactor != m_regs.spreadingFactor)
    {
        NS_LOG_INFO("Packet lost because it's using SF" << unsigned(spreadingFactor)
                                                        << ", while we are listening for SF"
                                                        << unsigned(m_regs.spreadingFactor));
        // Fire the trace source for this event.
        m_wrongSf(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
        return;
    }
    // Check Sensitivity
    else if (double sens = EndDeviceLoraPhy::SENSITIVITY[spreadingFactor - 7]; rxPowerDbm < sens)
    {
        NS_LOG_INFO("Dropping packet reception of packet with SF"
                    << unsigned(spreadingFactor) << " because under the sensitivity of " << sens
                    << " dBm");
        // Fire the trace source for this event.
        m_underSensitivity(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
        return;
    }

    // Preamble detected, we lock onto the transmission and start receiving
    DoStartReceive();
    // Fire the beginning of reception trace source
    m_phyRxBeginTrace(packet);

    // Schedule the end of the reception of the packet
    NS_LOG_INFO("Scheduling reception of a packet. End in " << duration.As(Time::S));
    Simulator::Schedule(duration, &SimpleEndDeviceLoraPhy::EndReceive, this, packet, event);
}

void
SimpleEndDeviceLoraPhy::EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event)
{
    NS_LOG_FUNCTION(this << packet << event);

    // The demodulation terminated, automatically switch to Standby
    DoEndReceive();
    // Fire the trace source
    m_phyRxEndTrace(packet);

    // Call the LoraInterferenceHelper to determine whether there was destructive
    // interference on this event.
    if (m_interference.IsDestroyedByInterference(event))
    {
        NS_LOG_INFO("Packet destroyed by interference");
        // Fire the trace source if packet was destroyed
        m_interferedPacket(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
        // If there is one, perform the callback to inform the upper layer of the
        // lost packet
        if (!m_rxFailedCallback.IsNull())
        {
            m_rxFailedCallback(packet);
        }
        return;
    }

    NS_LOG_INFO("Packet received correctly");
    m_successfullyReceivedPacket(packet, (m_device) ? m_device->GetNode()->GetId() : 0);

    LoraTag tag;
    packet->RemovePacketTag(tag);
    tag.SetReceivePower(event->GetRxPowerDbm());
    tag.SetFrequency(event->GetFrequency());
    packet->AddPacketTag(tag);

    // If there is one, perform the callback to inform the upper layer
    if (!m_rxOkCallback.IsNull())
    {
        m_rxOkCallback(packet);
    }
}

} // namespace lorawan
} // namespace ns3
