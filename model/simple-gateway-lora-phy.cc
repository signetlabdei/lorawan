/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "simple-gateway-lora-phy.h"

#include "lora-channel.h"
#include "lora-tag.h"

#include "ns3/node.h"
#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("SimpleGatewayLoraPhy");

NS_OBJECT_ENSURE_REGISTERED(SimpleGatewayLoraPhy);

/***********************************************************************
 *                 Implementation of gateway methods                   *
 ***********************************************************************/

TypeId
SimpleGatewayLoraPhy::GetTypeId()
{
    static TypeId tid = TypeId("ns3::SimpleGatewayLoraPhy")
                            .SetParent<GatewayLoraPhy>()
                            .SetGroupName("lorawan")
                            .AddConstructor<SimpleGatewayLoraPhy>();

    return tid;
}

SimpleGatewayLoraPhy::SimpleGatewayLoraPhy()
{
    NS_LOG_FUNCTION(this);
}

SimpleGatewayLoraPhy::~SimpleGatewayLoraPhy()
{
    NS_LOG_FUNCTION(this);
}

void
SimpleGatewayLoraPhy::Send(Ptr<Packet> packet,
                           uint32_t frequencyHz,
                           IQPolarity iqPolarity,
                           const LoraTxParameters& txParams,
                           double txPowerDbm)
{
    NS_LOG_FUNCTION(this << packet << frequencyHz << iqPolarity << txParams << txPowerDbm);

    // Interrupt all receive operations
    for (auto& rxPath : m_receptionPaths)
    {
        if (!rxPath->IsAvailable()) // Reception path is occupied
        {
            // Fire the trace source
            m_noReceptionBecauseTransmitting(rxPath->GetEvent()->GetPacket(),
                                             (m_device) ? m_device->GetNode()->GetId() : 0);
            // Free the reception path (reset state and cancels reception end)
            rxPath->Free();
        }
    }

    // Switch to TX state
    m_isTransmitting = true;
    // Fire the trace source
    m_startSending(packet, (m_device) ? m_device->GetNode()->GetId() : 0);

    // Compute the duration of the modulated transmission
    Time duration = GetTimeOnAir(packet->GetSize(), txParams);
    // Propagate the transmission over the channel (schedule StartReceive for other nodes)
    m_channel->Send(this, packet, frequencyHz, iqPolarity, txParams, txPowerDbm, duration);

    // Schedule a call to self to signal the transmission modulation end
    Simulator::Schedule(duration, &SimpleGatewayLoraPhy::TxFinished, this, packet);
}

void
SimpleGatewayLoraPhy::StartReceive(Ptr<Packet> packet,
                                   uint32_t frequencyHz,
                                   IQPolarity iqPolarity,
                                   uint8_t spreadingFactor,
                                   double rxPowerDbm,
                                   Time duration)
{
    NS_LOG_FUNCTION(this << packet << frequencyHz << iqPolarity << unsigned(spreadingFactor)
                         << rxPowerDbm << duration);

    // Add the event to the LoraInterferenceHelper
    auto event = m_interference.Add(duration, rxPowerDbm, spreadingFactor, packet, frequencyHz);

    // Check whether the gateway is currently transmitting downlink
    if (m_isTransmitting)
    {
        NS_LOG_INFO("Dropping packet reception of packet with SF" << unsigned(spreadingFactor)
                                                                  << " because we are in TX mode");
        // Fire the trace source
        m_noReceptionBecauseTransmitting(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
        return;
    }
    // Check whether the gateway is configured to listen for the channel of the transmission
    else if (!IsOnFrequency(frequencyHz))
    {
        // Unknown frequency
        NS_LOG_INFO("Dropping packet reception of packet with SF"
                    << unsigned(spreadingFactor) << " because we are not listening to frequency "
                    << frequencyHz << " Hz");
        /// TODO: implement trace source
        return;
    }
    // Check modulation I/Q polarity (gateway PHYs listen for uplinks by default)
    else if (iqPolarity != IQPolarity::UP)
    {
        NS_LOG_INFO("Dropping packet reception of packet with SF"
                    << unsigned(spreadingFactor)
                    << " because we are not listening to uplink transmissions");
        /// TODO: implement trace source
        return;
    }
    // See whether the reception power is above or below the sensitivity for that spreading factor
    else if (double sens = SimpleGatewayLoraPhy::SENSITIVITY[spreadingFactor - 7];
             rxPowerDbm < sens)
    {
        // Packet arrived below sensitivity
        NS_LOG_INFO("Dropping packet reception of packet with SF"
                    << unsigned(spreadingFactor) << " because under the sensitivity of " << sens
                    << " dBm");
        // Fire the trace source for this event.
        m_underSensitivity(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
        return;
    }

    // Cycle over the receive paths to check availability to receive the packet
    for (auto& rxPath : m_receptionPaths)
    {
        // If the receive path is available and listening on the channel of
        // interest, we have a candidate
        if (rxPath->IsAvailable())
        {
            NS_LOG_INFO("Scheduling reception of a packet, occupying one demodulator");
            // Block this resource
            rxPath->LockOnEvent(event);
            m_occupiedReceptionPaths++;
            // Schedule the end of the reception of the packet
            EventId endReceiveEventId = Simulator::Schedule(duration,
                                                            &SimpleGatewayLoraPhy::EndReceive,
                                                            this,
                                                            packet,
                                                            event);
            rxPath->SetEndReceive(endReceiveEventId);
            // Fire the trace source
            m_phyRxBeginTrace(packet);
            return;
        }
    }

    // If we get to this point, there are no demodulators we can use
    NS_LOG_INFO("Dropping packet reception of packet with SF"
                << unsigned(spreadingFactor) << " and frequency " << frequencyHz
                << "Hz because no suitable demodulator was found");
    // Fire the trace source
    m_noMoreDemodulators(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
}

void
SimpleGatewayLoraPhy::EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event)
{
    NS_LOG_FUNCTION(this << packet << *event);

    // Search for the demodulator that was locked on this event to free it.
    for (auto& rxPath : m_receptionPaths)
    {
        if (rxPath->GetEvent() == event)
        {
            rxPath->Free();
            m_occupiedReceptionPaths--;
            continue;
        }
    }
    // Call the trace source
    m_phyRxEndTrace(packet);

    // Call the LoraInterferenceHelper to determine whether there was destructive interference.
    if (auto sf = m_interference.IsDestroyedByInterference(event); sf != 0)
    {
        NS_LOG_DEBUG("packetDestroyed by " << unsigned(sf));
        // Update the packet's LoraTag
        LoraTag tag;
        packet->RemovePacketTag(tag);
        tag.SetDestroyedBy(sf);
        packet->AddPacketTag(tag);
        // Fire the trace source
        m_interferedPacket(packet, (m_device) ? m_device->GetNode()->GetId() : 0);
        return;
    }

    NS_LOG_INFO("Packet with SF " << unsigned(event->GetSpreadingFactor())
                                  << " received correctly");
    // Fire the trace source
    m_successfullyReceivedPacket(packet, (m_device) ? m_device->GetNode()->GetId() : 0);

    // Forward the packet to the upper layer
    if (!m_rxOkCallback.IsNull())
    {
        // Set the receive power and frequency of this packet in the LoraTag: this information can
        // be useful for upper layers trying to control link quality.
        LoraTag tag;
        packet->RemovePacketTag(tag);
        tag.SetReceivePower(event->GetRxPowerDbm());
        tag.SetFrequency(event->GetFrequency());
        packet->AddPacketTag(tag);
        m_rxOkCallback(packet);
    }
}

} // namespace lorawan
} // namespace ns3
