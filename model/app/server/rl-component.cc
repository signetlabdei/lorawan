/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2022 Orange SA
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
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#include "rl-component.h"

#include "ns3/lora-phy.h"

#include <bitset>

#define MIN_ESP -150

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("RlComponent");

NS_OBJECT_ENSURE_REGISTERED(RlComponent);

std::string
RlComponent::state_t::serialize()
{
    std::string s = "[";
    for (auto esp : espVec)
        s += IpcHandler::FullPrecision(esp) + ", ";
    s += std::to_string(cluster) + "]";
    return s;
}

void
RlComponent::reward_t::update(uint32_t dev, double mpe)
{
    if (mpeMap.count(dev))            /* exists already */
        value += (mpeMap[dev] - mpe); /* update */
    else
        value += (1.0 - mpe); /* initialize */
    mpeMap[dev] = mpe;
}

std::string
RlComponent::reward_t::serialize()
{
    return IpcHandler::FullPrecision(value);
}

TypeId
RlComponent::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::RlComponent")
                            .SetGroupName("lorawan")
                            .AddConstructor<RlComponent>()
                            .SetParent<NetworkControllerComponent>()
                            .AddAttribute("StartTime",
                                          "Time at which we start inter process comms",
                                          TimeValue(Hours(24)),
                                          MakeTimeAccessor(&RlComponent::m_start),
                                          MakeTimeChecker(Hours(0)))
                            .AddAttribute("EndTime",
                                          "Time after which we stop inter process comms",
                                          TimeValue(Hours(48)),
                                          MakeTimeAccessor(&RlComponent::m_end),
                                          MakeTimeChecker(Hours(0)));
    return tid;
}

RlComponent::RlComponent()
    : m_terminal(false),
      m_targets({0.95})
{
}

RlComponent::~RlComponent()
{
}

void
RlComponent::OnReceivedPacket(Ptr<const Packet> packet,
                              Ptr<EndDeviceStatus> status,
                              Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(packet << status << networkStatus);
    if (m_terminal)
        return;

    /* get device address */
    uint32_t devaddr = GetFHeader(packet).GetAddress().Get();
    if (m_clusterMap.count(devaddr))
        return;
    /* dev not yet in cluster membership map, add it */
    m_clusterMap[devaddr] = status->GetMac()->GetCluster();

    /* if m_gwIdMap is empty get it from status */
    if (!m_gwIdMap.empty())
        return;
    int i = 0;
    for (auto& gw : networkStatus->m_gatewayStatuses)
    {
        m_gwIdMap[gw.first] = i;
        i++;
    }
}

void
RlComponent::BeforeSendingReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(status << networkStatus);
    if (m_terminal)
        return;

    /* check requirements to start */
    if (Now() < m_start)
        return;
    const auto& history = status->GetReceivedPacketList();
    uint16_t k = history.size() - 1; // remove 1 because current is already in
    if (k < 1)                       // can we start computing the reward?
        return;

    /* retrieve useful data */
    auto packetInfo = history.back();
    auto fhead = GetFHeader(packetInfo.first);
    uint32_t devaddr = fhead.GetAddress().Get();
    NS_LOG_INFO("Device address: " << (unsigned)devaddr << ", history size k: " << (unsigned)k);

    /* update reward */
    uint16_t currFCnt = fhead.GetFCnt();
    int oldFCnt = GetFHeader(history.front().first).GetFCnt();
    if (oldFCnt > currFCnt + 10000) // uint16_t has overflowed
        oldFCnt -= 65536;
    NS_ASSERT_MSG(
        oldFCnt <= currFCnt,
        "Frame counter can't decrease, as re-connections to the network are not implemented.");
    double pdr = (double)k / (currFCnt - oldFCnt); // mean Packet Delivery Ratio (PDR) k-estimator
    NS_LOG_INFO("Current fCnt: " << (unsigned)currFCnt << ", k-old fCnt: " << oldFCnt
                                 << ", k-PDR estimator: " << pdr);
    double target = m_targets[m_clusterMap[devaddr]]; // device target PDR
    double mpe = std::max(target - pdr, 0.0);         // Mean PDR Error (MPE)
    NS_LOG_INFO("Target: " << target << ", mean PDR error: " << mpe
                           << ", old reward: " << m_r.serialize());
    m_r.update(devaddr, mpe);

    /* create state based on all messages received up to now */
    state_t s;
    s.cluster = m_clusterMap[devaddr];
    s.espVec = std::vector<double>(m_gwIdMap.size(), MIN_ESP);
    for (auto& gwInfo : packetInfo.second.gwList)
    {
        double rssi = gwInfo.second.rxPower;
        double snr = LoraPhy::RxPowerToSNR(rssi);
        /* rssi to 1dB precision, snr to 0.1dB precision, as per Semtech's UDP protocol */
        // rssi = double(int(rssi));
        // snr = double(int(snr * 10)) / 10;
        /* compute esp */
        double esp = rssi + snr - 10 * log10(1 + exp10(snr / 10));
        NS_LOG_INFO("Gateway: " << gwInfo.first << ", RSSI: " << rssi << ", SNR: " << snr
                                << ", ESP: " << esp);
        s.espVec[m_gwIdMap[gwInfo.first]] = esp;
    }

    /* update the model and get next action */
    if (Now() >= m_end)
        m_terminal = true;
    std::string a_str = ipc.GetAction(s.serialize(), m_r.serialize(), m_terminal);
    action_t a = uint8_t(std::stoi(a_str));

    /* if 'do nothing' action, return */
    if (a == 0)
        return;

    std::list<int> enabledChannels;
    for (int i = 0; i < 16; i++)
        if (a & (0b1 << i)) // Take channel mask's i-th bit
            enabledChannels.push_back(i);
    NS_LOG_INFO("New channel mask: " << std::bitset<16>(a));

    /* only change enabled channels */
    auto mac = status->GetMac();
    uint8_t dr = mac->GetDataRate();
    uint8_t txPow = (14 - mac->GetTransmissionPower()) / 2;
    uint8_t reTxs = mac->GetNumberOfTransmissions();
    status->m_reply.frameHeader.AddLinkAdrReq(dr, txPow, enabledChannels, reTxs);
    status->m_reply.frameHeader.SetAsDownlink();
    status->m_reply.macHeader.SetFType(LorawanMacHeader::UNCONFIRMED_DATA_DOWN);
    status->m_reply.needsReply = true;
}

void
RlComponent::SetTargets(targets_t targets)
{
    NS_LOG_FUNCTION(targets);
    m_targets = targets;
}

LoraFrameHeader
RlComponent::GetFHeader(Ptr<const Packet> packet)
{
    Ptr<Packet> packetCopy = packet->Copy();
    LorawanMacHeader mhead;
    packetCopy->RemoveHeader(mhead);
    LoraFrameHeader fhead;
    fhead.SetAsUplink(); //<! Needed by Deserialize ()
    packetCopy->RemoveHeader(fhead);
    return fhead;
}

// unused
void
RlComponent::OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus)
{
    NS_LOG_FUNCTION(this->GetTypeId() << networkStatus);
}

} // namespace lorawan
} // namespace ns3