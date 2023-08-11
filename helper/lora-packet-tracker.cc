/*
 * Copyright (c) 2018 University of Padova
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
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "lora-packet-tracker.h"

#include "ns3/log.h"
#include "ns3/lora-phy.h"
#include "ns3/lora-tag.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/simulator.h"

#include <fstream>
#include <iostream>

namespace ns3
{
namespace lorawan
{
NS_LOG_COMPONENT_DEFINE("LoraPacketTracker");

LoraPacketTracker::LoraPacketTracker()
    : m_oldPacketThreshold(Seconds(0)),
      m_lastPacketCleanup(Seconds(0))
{
    NS_LOG_FUNCTION(this);
}

LoraPacketTracker::~LoraPacketTracker()
{
    NS_LOG_FUNCTION(this);
    m_packetTracker.clear();
    m_macPacketTracker.clear();
    m_reTransmissionTracker.clear();
}

/////////////////
// MAC metrics //
/////////////////

void
LoraPacketTracker::MacTransmissionCallback(Ptr<const Packet> packet)
{
    if (IsUplink(packet))
    {
        NS_LOG_INFO("A new packet was sent by the MAC layer");

        MacPacketStatus status;
        status.packet = packet;
        status.sendTime = Simulator::Now();
        status.senderId = Simulator::GetContext();
        status.receivedTime = Time::Max();

        m_macPacketTracker.insert(std::pair<Ptr<const Packet>, MacPacketStatus>(packet, status));
        CleanupOldPackets();
    }
}

void
LoraPacketTracker::RequiredTransmissionsCallback(uint8_t reqTx,
                                                 bool success,
                                                 Time firstAttempt,
                                                 Ptr<Packet> packet)
{
    NS_LOG_INFO("Finished retransmission attempts for a packet");
    NS_LOG_DEBUG("Packet: " << packet << "ReqTx " << unsigned(reqTx) << ", succ: " << success
                            << ", firstAttempt: " << firstAttempt.GetSeconds());

    RetransmissionStatus entry;
    entry.firstAttempt = firstAttempt;
    entry.finishTime = Simulator::Now();
    entry.reTxAttempts = reqTx;
    entry.successful = success;

    m_reTransmissionTracker.insert(std::pair<Ptr<Packet>, RetransmissionStatus>(packet, entry));
    CleanupOldPackets();
}

void
LoraPacketTracker::MacGwReceptionCallback(Ptr<const Packet> packet)
{
    if (IsUplink(packet))
    {
        NS_LOG_INFO("A packet was successfully received"
                    << " at the MAC layer of gateway " << Simulator::GetContext());

        // Find the received packet in the m_macPacketTracker
        auto it = m_macPacketTracker.find(packet);
        if (it != m_macPacketTracker.end())
        {
            (*it).second.receptionTimes.insert(
                std::pair<int, Time>(Simulator::GetContext(), Simulator::Now()));
            if (Simulator::Now() < (*it).second.receivedTime)
            {
                (*it).second.receivedTime = Simulator::Now();
            }
        }
        else
        {
            NS_ABORT_MSG("Packet not found in tracker");
        }
    }
}

/////////////////
// PHY metrics //
/////////////////

void
LoraPacketTracker::TransmissionCallback(Ptr<const Packet> packet, uint32_t edId)
{
    if (IsUplink(packet))
    {
        NS_LOG_INFO("PHY packet " << packet << " was transmitted by device " << edId);
        // Create a packetStatus
        PacketStatus status;
        status.packet = packet;
        status.sendTime = Simulator::Now();
        status.senderId = edId;

        m_packetTracker.insert(std::pair<Ptr<const Packet>, PacketStatus>(packet, status));
        CleanupOldPackets();
    }
}

void
LoraPacketTracker::PacketReceptionCallback(Ptr<const Packet> packet, uint32_t gwId)
{
    if (IsUplink(packet))
    {
        // Remove the successfully received packet from the list of sent ones
        NS_LOG_INFO("PHY packet " << packet << " was successfully received at gateway " << gwId);

        std::map<Ptr<const Packet>, PacketStatus>::iterator it = m_packetTracker.find(packet);
        (*it).second.outcomes.insert(std::pair<int, enum PhyPacketOutcome>(gwId, RECEIVED));
    }
}

void
LoraPacketTracker::InterferenceCallback(Ptr<const Packet> packet, uint32_t gwId)
{
    if (IsUplink(packet))
    {
        NS_LOG_INFO("PHY packet " << packet << " was interfered at gateway " << gwId);

        std::map<Ptr<const Packet>, PacketStatus>::iterator it = m_packetTracker.find(packet);
        (*it).second.outcomes.insert(std::pair<int, enum PhyPacketOutcome>(gwId, INTERFERED));
    }
}

void
LoraPacketTracker::NoMoreReceiversCallback(Ptr<const Packet> packet, uint32_t gwId)
{
    if (IsUplink(packet))
    {
        NS_LOG_INFO("PHY packet " << packet << " was lost because no more receivers at gateway "
                                  << gwId);
        std::map<Ptr<const Packet>, PacketStatus>::iterator it = m_packetTracker.find(packet);
        (*it).second.outcomes.insert(
            std::pair<int, enum PhyPacketOutcome>(gwId, NO_MORE_RECEIVERS));
    }
}

void
LoraPacketTracker::UnderSensitivityCallback(Ptr<const Packet> packet, uint32_t gwId)
{
    if (IsUplink(packet))
    {
        NS_LOG_INFO("PHY packet " << packet << " was lost because under sensitivity at gateway "
                                  << gwId);

        std::map<Ptr<const Packet>, PacketStatus>::iterator it = m_packetTracker.find(packet);
        (*it).second.outcomes.insert(
            std::pair<int, enum PhyPacketOutcome>(gwId, UNDER_SENSITIVITY));
    }
}

void
LoraPacketTracker::LostBecauseTxCallback(Ptr<const Packet> packet, uint32_t gwId)
{
    if (IsUplink(packet))
    {
        NS_LOG_INFO("PHY packet " << packet << " was lost because of GW transmission at gateway "
                                  << gwId);

        std::map<Ptr<const Packet>, PacketStatus>::iterator it = m_packetTracker.find(packet);
        (*it).second.outcomes.insert(std::pair<int, enum PhyPacketOutcome>(gwId, LOST_BECAUSE_TX));
    }
}

bool
LoraPacketTracker::IsUplink(Ptr<const Packet> packet)
{
    NS_LOG_FUNCTION(this);

    LorawanMacHeader mHdr;
    Ptr<Packet> copy = packet->Copy();
    copy->RemoveHeader(mHdr);
    return mHdr.IsUplink();
}

////////////////////////
// Counting Functions //
////////////////////////

std::vector<int>
LoraPacketTracker::CountPhyPacketsPerGw(Time startTime, Time stopTime, int gwId)
{
    // Vector packetCounts will contain - for the interval given in the input of
    // the function, the following fields: totPacketsSent receivedPackets
    // interferedPackets noMoreGwPackets underSensitivityPackets lostBecauseTxPackets

    std::vector<int> packetCounts(6, 0);

    for (auto itPhy = m_packetTracker.begin(); itPhy != m_packetTracker.end(); ++itPhy)
    {
        if ((*itPhy).second.sendTime >= startTime && (*itPhy).second.sendTime <= stopTime)
        {
            packetCounts.at(0)++;

            NS_LOG_DEBUG("Dealing with packet " << (*itPhy).second.packet);
            NS_LOG_DEBUG("This packet was received by " << (*itPhy).second.outcomes.size()
                                                        << " gateways");

            if ((*itPhy).second.outcomes.count(gwId) > 0)
            {
                switch ((*itPhy).second.outcomes.at(gwId))
                {
                case RECEIVED: {
                    packetCounts.at(1)++;
                    break;
                }
                case INTERFERED: {
                    packetCounts.at(2)++;
                    break;
                }
                case NO_MORE_RECEIVERS: {
                    packetCounts.at(3)++;
                    break;
                }
                case UNDER_SENSITIVITY: {
                    packetCounts.at(4)++;
                    break;
                }
                case LOST_BECAUSE_TX: {
                    packetCounts.at(5)++;
                    break;
                }
                case UNSET: {
                    break;
                }
                }
            }
        }
    }

    return packetCounts;
}

std::string
LoraPacketTracker::PrintPhyPacketsPerGw(Time startTime, Time stopTime, int gwId)
{
    // Vector packetCounts will contain - for the interval given in the input of
    // the function, the following fields: totPacketsSent receivedPackets
    // interferedPackets noMoreGwPackets underSensitivityPackets lostBecauseTxPackets

    std::vector<int> packetCounts(CountPhyPacketsPerGw(startTime, stopTime, gwId));

    std::string output("");
    for (int i = 0; i < 6; ++i)
    {
        output += std::to_string(packetCounts.at(i)) + " ";
    }

    return output;
}

void
LoraPacketTracker::CountPhyPacketsAllGws(Time startTime, Time stopTime, GwsPhyPktCount& output)
{
    output.clear();
    for (const auto& ppd : m_packetTracker)
    {
        if (ppd.second.sendTime >= startTime && ppd.second.sendTime <= stopTime)
        {
            NS_LOG_DEBUG("Dealing with packet " << ppd.second.packet);
            NS_LOG_DEBUG("This packet was received by " << ppd.second.outcomes.size()
                                                        << " gateways");
            for (const auto& out : ppd.second.outcomes)
            {
                output[out.first].v[0]++;
                switch (out.second)
                {
                case RECEIVED: {
                    output[out.first].v[1]++;
                    break;
                }
                case INTERFERED: {
                    output[out.first].v[2]++;
                    break;
                }
                case NO_MORE_RECEIVERS: {
                    output[out.first].v[3]++;
                    break;
                }
                case LOST_BECAUSE_TX: {
                    output[out.first].v[4]++;
                    break;
                }
                case UNDER_SENSITIVITY: {
                    output[out.first].v[5]++;
                    break;
                }
                case UNSET: {
                    break;
                }
                }
            }
        }
    }
}

void
LoraPacketTracker::PrintPhyPacketsAllGws(Time startTime, Time stopTime, GwsPhyPktPrint& output)
{
    output.clear();
    GwsPhyPktCount count;
    CountPhyPacketsAllGws(startTime, stopTime, count);
    for (const auto& gw : count)
    {
        std::string out("");
        for (int i = 0; i < 5; ++i)
        {
            out += std::to_string(gw.second.v[i]) + " ";
        }
        out += std::to_string(gw.second.v[5]);
        output[gw.first].s = out;
    }
}

std::string
LoraPacketTracker::PrintPhyPacketsGlobally(Time startTime, Time stopTime)
{
    NS_LOG_FUNCTION(this << startTime << stopTime);

    std::vector<int> count(6, 0);

    for (const auto& ppd : m_packetTracker)
    {
        if (ppd.second.sendTime >= startTime && ppd.second.sendTime <= stopTime)
        {
            count[0]++;
            bool received = false;
            bool interfered = false;
            bool noPaths = false;
            bool busyGw = false;
            for (const auto& out : ppd.second.outcomes)
            {
                if (out.second == RECEIVED)
                {
                    received = true;
                    break;
                }
                else if (!interfered and out.second == INTERFERED)
                {
                    interfered = true;
                }
                else if (!noPaths and out.second == NO_MORE_RECEIVERS)
                {
                    noPaths = true;
                }
                else if (!busyGw and out.second == LOST_BECAUSE_TX)
                {
                    busyGw = true;
                }
            }
            if (received)
            {
                count[1]++;
            }
            else if (interfered)
            {
                count[2]++;
            }
            else if (noPaths)
            {
                count[3]++;
            }
            else if (busyGw)
            {
                count[4]++;
            }
            else
            {
                count[5]++;
            }
        }
    }

    std::string output("");
    for (int i = 0; i < 5; ++i)
    {
        output += std::to_string(count[i]) + " ";
    }
    output += std::to_string(count[5]);
    return output;
}

std::string
LoraPacketTracker::CountMacPacketsGlobally(Time startTime, Time stopTime)
{
    NS_LOG_FUNCTION(this << startTime << stopTime);

    int sent = 0;
    int received = 0;
    for (auto it = m_macPacketTracker.begin(); it != m_macPacketTracker.end(); ++it)
    {
        if ((*it).second.sendTime >= startTime && (*it).second.sendTime <= stopTime)
        {
            sent++;
            if (!(*it).second.receptionTimes.empty())
            {
                received++;
            }
        }
    }

    return std::to_string(sent) + " " + std::to_string(received);
}

std::string
LoraPacketTracker::CountMacPacketsGloballyCpsr(Time startTime, Time stopTime)
{
    NS_LOG_FUNCTION(this << startTime << stopTime);

    int sent = 0;
    int received = 0;
    for (auto it = m_reTransmissionTracker.begin(); it != m_reTransmissionTracker.end(); ++it)
    {
        if ((*it).second.firstAttempt >= startTime && (*it).second.firstAttempt <= stopTime)
        {
            sent++;
            NS_LOG_DEBUG("Found a packet");
            NS_LOG_DEBUG("Number of attempts: " << unsigned(it->second.reTxAttempts)
                                                << ", successful: " << it->second.successful);
            if (it->second.successful)
            {
                received++;
            }
        }
    }

    return std::to_string(sent) + " " + std::to_string(received);
}

std::string
LoraPacketTracker::PrintDevicePackets(Time startTime, Time stopTime, uint32_t devId)
{
    NS_LOG_FUNCTION(this << startTime << stopTime << devId);

    int sent = 0;
    int received = 0;
    for (auto it = m_macPacketTracker.begin(); it != m_macPacketTracker.end(); ++it)
    {
        if ((*it).second.sendTime >= startTime && (*it).second.sendTime <= stopTime &&
            (*it).second.senderId == devId)
        {
            sent++;
            if (!(*it).second.receptionTimes.empty())
            {
                received++;
            }
        }
    }

    return std::to_string(sent) + " " + std::to_string(received);
}

void
LoraPacketTracker::CountAllDevicesPackets(Time startTime, Time stopTime, DevPktCount& out)
{
    NS_LOG_FUNCTION(this << startTime << stopTime);

    out.clear();
    for (const auto& mpd : m_macPacketTracker)
    {
        if (mpd.second.sendTime >= startTime && mpd.second.sendTime <= stopTime)
        {
            out[mpd.second.senderId].sent++;
            if (mpd.second.receptionTimes.size())
                out[mpd.second.senderId].received++;
        }
    }
}

std::string
LoraPacketTracker::PrintSimulationStatistics(Time startTime)
{
    NS_ASSERT(startTime < Simulator::Now());

    uint32_t total = 0;
    double totReceived = 0;
    double totInterfered = 0;
    double totNoMorePaths = 0;
    double totBusyGw = 0;
    double totUnderSens = 0;

    std::vector<double> sentSF(6, 0);
    std::vector<double> receivedSF(6, 0);

    double totBytesReceived = 0;
    double totBytesSent = 0;

    double totOffTraff = 0.0;

    for (const auto& pd : m_packetTracker)
    {
        if (pd.second.sendTime < startTime - Seconds(5))
        {
            continue;
        }

        bool received = false;
        bool interfered = false;
        bool noPaths = false;
        bool busyGw = false;

        LoraPhyTxParameters params;
        LoraTag tag;
        pd.first->Copy()->RemovePacketTag(tag);
        params.sf = tag.GetTxParameters().sf;
        params.lowDataRateOptimizationEnabled = LoraPhy::GetTSym(params) > MilliSeconds(16);
        totOffTraff += LoraPhy::GetTimeOnAir(pd.first->Copy(), params).GetSeconds();

        total++;
        totBytesSent += pd.first->GetSize();
        sentSF[tag.GetDataRate()]++;
        for (const auto& out : pd.second.outcomes)
        {
            if (out.second == RECEIVED)
            {
                received = true;
                receivedSF[tag.GetDataRate()]++;
                totBytesReceived += pd.first->GetSize();
                break;
            }
            else if (!interfered and out.second == INTERFERED)
            {
                interfered = true;
            }
            else if (!noPaths and out.second == NO_MORE_RECEIVERS)
            {
                noPaths = true;
            }
            else if (!busyGw and out.second == LOST_BECAUSE_TX)
            {
                busyGw = true;
            }
        }
        if (received)
        {
            totReceived++;
        }
        else if (interfered)
        {
            totInterfered++;
        }
        else if (noPaths)
        {
            totNoMorePaths++;
        }
        else if (busyGw)
        {
            totBusyGw++;
        }
        else
        {
            totUnderSens++;
        }
    }

    std::stringstream ss;
    ss << "\nPackets outcomes distribution (" << total << " sent, " << totReceived << " received):"
       << "\n  RECEIVED: " << totReceived / total * 100
       << "%\n  INTERFERED: " << totInterfered / total * 100
       << "%\n  NO_MORE_RECEIVERS: " << totNoMorePaths / total * 100
       << "%\n  BUSY_GATEWAY: " << totBusyGw / total * 100
       << "%\n  UNDER_SENSITIVITY: " << totUnderSens / total * 100 << "%\n";

    ss << "\nPDR: ";
    for (int dr = 5; dr >= 0; --dr)
    {
        ss << "SF" << 12 - dr << " " << receivedSF[dr] / sentSF[dr] * 100 << "%, ";
    }
    ss << "\n";

    double totTime = (Simulator::Now() - startTime).GetSeconds();
    ss << "\nInput Traffic: " << totBytesSent * 8 / totTime
       << " b/s\nNetwork Throughput: " << totBytesReceived * 8 / totTime << " b/s\n";

    totOffTraff /= totTime;
    ss << "\nTotal (empirical) offered traffic: " << totOffTraff << " E\n";

    return ss.str();
}

void
LoraPacketTracker::EnableOldPacketsCleanup(Time oldPacketThreshold)
{
    NS_ASSERT_MSG(
        oldPacketThreshold > Minutes(30),
        "Threshold to consider packets old should be > 30 min to avoid risk of partial entries");
    m_oldPacketThreshold = oldPacketThreshold;
}

void
LoraPacketTracker::CleanupOldPackets()
{
    if (m_oldPacketThreshold.IsZero())
    {
        return;
    }
    if (Simulator::Now() < m_lastPacketCleanup + m_oldPacketThreshold)
    {
        return;
    }

    for (auto it = m_packetTracker.cbegin(); it != m_packetTracker.cend();)
    {
        if ((*it).second.sendTime < Simulator::Now() - m_oldPacketThreshold)
        {
            it = m_packetTracker.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (auto it = m_macPacketTracker.cbegin(); it != m_macPacketTracker.cend();)
    {
        if ((*it).second.sendTime < Simulator::Now() - m_oldPacketThreshold)
        {
            it = m_macPacketTracker.erase(it);
        }
        else
        {
            ++it;
        }
    }

    for (auto it = m_reTransmissionTracker.cbegin(); it != m_reTransmissionTracker.cend();)
    {
        if ((*it).second.firstAttempt < Simulator::Now() - m_oldPacketThreshold)
        {
            it = m_reTransmissionTracker.erase(it);
        }
        else
        {
            ++it;
        }
    }

    m_lastPacketCleanup = Simulator::Now();
}

} // namespace lorawan
} // namespace ns3
