/*
 * Copyright (c) 2018 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "network-controller-components.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("NetworkControllerComponent");

NS_OBJECT_ENSURE_REGISTERED(NetworkControllerComponent);

TypeId
NetworkControllerComponent::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::NetworkControllerComponent").SetParent<Object>().SetGroupName("lorawan");
    return tid;
}

NetworkControllerComponent::NetworkControllerComponent()
{
}

NetworkControllerComponent::~NetworkControllerComponent()
{
}

////////////////////////////////
// ConfirmedMessagesComponent //
////////////////////////////////
TypeId
ConfirmedMessagesComponent::GetTypeId()
{
    static TypeId tid = TypeId("ns3::ConfirmedMessagesComponent")
                            .SetParent<NetworkControllerComponent>()
                            .AddConstructor<ConfirmedMessagesComponent>()
                            .SetGroupName("lorawan");
    return tid;
}

ConfirmedMessagesComponent::ConfirmedMessagesComponent()
{
}

ConfirmedMessagesComponent::~ConfirmedMessagesComponent()
{
}

void
ConfirmedMessagesComponent::OnReceivedPacket(Ptr<const Packet> packet,
                                             Ptr<EndDeviceStatus> status,
                                             Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this->GetTypeId() << packet << networkStatus);

    // Check whether the received packet requires an acknowledgment.
    LorawanMacHeader mHdr;
    LoraFrameHeader fHdr;
    fHdr.SetAsUplink();
    Ptr<Packet> myPacket = packet->Copy();
    myPacket->RemoveHeader(mHdr);
    myPacket->RemoveHeader(fHdr);

    NS_LOG_INFO("Received packet Mac Header: " << mHdr);
    NS_LOG_INFO("Received packet Frame Header: " << fHdr);

    if (mHdr.GetMType() == LorawanMacHeader::CONFIRMED_DATA_UP)
    {
        NS_LOG_INFO("Packet requires confirmation");

        // Set up the ACK bit on the reply
        status->m_reply.frameHeader.SetAsDownlink();
        status->m_reply.frameHeader.SetAck(true);
        status->m_reply.frameHeader.SetAddress(fHdr.GetAddress());
        status->m_reply.macHeader.SetMType(LorawanMacHeader::UNCONFIRMED_DATA_DOWN);
        status->m_reply.needsReply = true;

        // Note that the acknowledgment procedure dies here: "Acknowledgments
        // are only snt in response to the latest message received and are never
        // retransmitted". We interpret this to mean that only the current
        // reception window can be used, and that the Ack field should be
        // emptied in case transmission cannot be performed in the current
        // window. Because of this, in this component's OnFailedReply method we
        // void the ack bits.
    }
}

void
ConfirmedMessagesComponent::BeforeSendingReply(Ptr<EndDeviceStatus> status,
                                               Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this << status << networkStatus);
    // Nothing to do in this case
}

void
ConfirmedMessagesComponent::OnFailedReply(Ptr<EndDeviceStatus> status,
                                          Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this << networkStatus);

    // Empty the Ack bit.
    status->m_reply.frameHeader.SetAck(false);
}

////////////////////////
// LinkCheckComponent //
////////////////////////
TypeId
LinkCheckComponent::GetTypeId()
{
    static TypeId tid = TypeId("ns3::LinkCheckComponent")
                            .SetParent<NetworkControllerComponent>()
                            .AddConstructor<LinkCheckComponent>()
                            .SetGroupName("lorawan");
    return tid;
}

LinkCheckComponent::LinkCheckComponent()
{
}

LinkCheckComponent::~LinkCheckComponent()
{
}

void
LinkCheckComponent::OnReceivedPacket(Ptr<const Packet> packet,
                                     Ptr<EndDeviceStatus> status,
                                     Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this->GetTypeId() << packet << networkStatus);

    // We will only act just before reply, when all Gateways will have received
    // the packet.
}

void
LinkCheckComponent::BeforeSendingReply(Ptr<EndDeviceStatus> status,
                                       Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this << status << networkStatus);

    Ptr<Packet> myPacket = status->GetLastPacketReceivedFromDevice()->Copy();
    LorawanMacHeader mHdr;
    LoraFrameHeader fHdr;
    fHdr.SetAsUplink();
    myPacket->RemoveHeader(mHdr);
    myPacket->RemoveHeader(fHdr);

    Ptr<LinkCheckReq> command = fHdr.GetMacCommand<LinkCheckReq>();

    // GetMacCommand returns 0 if no command is found
    if (command)
    {
        status->m_reply.needsReply = true;

        auto info = status->GetLastReceivedPacketInfo();

        // Adapted from: github.com/chirpstack/chirpstack v4.9.0

        // Get the best demodulation margin of the most recent LinkCheckReq command
        double maxRssi = -1e3;
        for (const auto& [_, gwRxData] : info.gwList)
        {
            if (gwRxData.rxPower > maxRssi)
            {
                maxRssi = gwRxData.rxPower;
            }
        }
        /// @see ns3::lorawan::AdrComponent::RxPowerToSNR
        double maxSnr = maxRssi + 174 - 10 * log10(125000) - 6;
        /// @todo make this a global PHY constant, manage unknown sf values
        double requiredSnr[] = {-20.0, -17.5, -15.0, -12.5, -10.0, -7.5, -5};
        double diff = maxSnr - requiredSnr[12 - info.sf];
        uint8_t margin = (diff < 0) ? 0 : (diff > 254) ? 254 : diff;

        // Get the number of gateways that received the most recent LinkCheckReq command
        uint8_t gwCount = info.gwList.size();

        auto replyCommand = Create<LinkCheckAns>(margin, gwCount);
        status->m_reply.frameHeader.SetAsDownlink();
        status->m_reply.frameHeader.AddCommand(replyCommand);
        status->m_reply.macHeader.SetMType(LorawanMacHeader::UNCONFIRMED_DATA_DOWN);
    }
    else
    {
        // Do nothing
    }
}

void
LinkCheckComponent::OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this->GetTypeId() << networkStatus);
}
} // namespace lorawan
} // namespace ns3
