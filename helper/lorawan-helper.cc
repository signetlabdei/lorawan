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
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "lorawan-helper.h"

#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/energy-source-container.h"
#include "ns3/log.h"
#include "ns3/lora-application.h"
#include "ns3/loratap-header.h"

#include <fstream>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LorawanHelper");

LorawanHelper::LorawanHelper()
    : m_lastPhyPerformanceUpdate(Seconds(0)),
      m_lastGlobalPerformanceUpdate(Seconds(0)),
      m_lastDeviceStatusUpdate(Seconds(0)),
      m_lastSFStatusUpdate(Seconds(0))
{
}

LorawanHelper::~LorawanHelper()
{
    delete m_packetTracker;
}

NetDeviceContainer
LorawanHelper::Install(const LoraPhyHelper& phyHelper,
                       const LorawanMacHelper& macHelper,
                       NodeContainer c) const
{
    NetDeviceContainer devices;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        Ptr<Node> node = *i;
        Ptr<LoraNetDevice> device = CreateObject<LoraNetDevice>();
        Ptr<LoraPhy> phy = phyHelper.Install(device);
        Ptr<LorawanMac> mac = macHelper.Install(device);
        if (m_packetTracker)
        {
            if (DynamicCast<EndDeviceLoraPhy>(phy) != nullptr)
            {
                phy->TraceConnectWithoutContext(
                    "StartSending",
                    MakeCallback(&LoraPacketTracker::TransmissionCallback, m_packetTracker));
                mac->TraceConnectWithoutContext(
                    "SentNewPacket",
                    MakeCallback(&LoraPacketTracker::MacTransmissionCallback, m_packetTracker));
                mac->TraceConnectWithoutContext(
                    "RequiredTransmissions",
                    MakeCallback(&LoraPacketTracker::RequiredTransmissionsCallback,
                                 m_packetTracker));
            }
            else if (DynamicCast<GatewayLoraPhy>(phy) != nullptr)
            {
                phy->TraceConnectWithoutContext(
                    "ReceivedPacket",
                    MakeCallback(&LoraPacketTracker::PacketReceptionCallback, m_packetTracker));
                phy->TraceConnectWithoutContext(
                    "LostPacketBecauseInterference",
                    MakeCallback(&LoraPacketTracker::InterferenceCallback, m_packetTracker));
                phy->TraceConnectWithoutContext(
                    "LostPacketBecauseNoMoreReceivers",
                    MakeCallback(&LoraPacketTracker::NoMoreReceiversCallback, m_packetTracker));
                phy->TraceConnectWithoutContext(
                    "LostPacketBecauseUnderSensitivity",
                    MakeCallback(&LoraPacketTracker::UnderSensitivityCallback, m_packetTracker));
                phy->TraceConnectWithoutContext(
                    "NoReceptionBecauseTransmitting",
                    MakeCallback(&LoraPacketTracker::LostBecauseTxCallback, m_packetTracker));
                mac->TraceConnectWithoutContext(
                    "ReceivedPacket",
                    MakeCallback(&LoraPacketTracker::MacGwReceptionCallback, m_packetTracker));
            }
        }
        node->AddDevice(device);
        devices.Add(device);
        NS_LOG_DEBUG("node=" << node << ", mob=" << node->GetObject<MobilityModel>());
    }
    return devices;
}

NetDeviceContainer
LorawanHelper::Install(const LoraPhyHelper& phy, const LorawanMacHelper& mac, Ptr<Node> node) const
{
    return Install(phy, mac, NodeContainer(node));
}

void
LorawanHelper::EnablePacketTracking()
{
    NS_LOG_FUNCTION(this);

    // Create the packet tracker
    m_packetTracker = new LoraPacketTracker();
}

LoraPacketTracker&
LorawanHelper::GetPacketTracker()
{
    NS_LOG_FUNCTION(this);

    return *m_packetTracker;
}

void
LorawanHelper::EnableSimulationTimePrinting(Time interval)
{
    m_oldtime = std::time(nullptr);
    Simulator::Schedule(Seconds(0), &LorawanHelper::DoPrintSimulationTime, this, interval);
}

void
LorawanHelper::DoPrintSimulationTime(Time interval)
{
    // NS_LOG_INFO ("Time: " << Simulator::Now().GetHours());
    std::cout << "Simulated time: " << Simulator::Now().GetHours() << " hours, ";
    std::cout << "Real time from last call: " << std::time(nullptr) - m_oldtime << " seconds"
              << std::endl;
    m_oldtime = std::time(nullptr);
    Simulator::Schedule(interval, &LorawanHelper::DoPrintSimulationTime, this, interval);
}

void
LorawanHelper::EnablePeriodicDeviceStatusPrinting(NodeContainer endDevices,
                                                  NodeContainer gateways,
                                                  std::string filename,
                                                  Time interval)
{
    NS_LOG_FUNCTION(this);

    DoPrintDeviceStatus(endDevices, gateways, filename);

    // Schedule periodic printing
    Simulator::Schedule(interval,
                        &LorawanHelper::EnablePeriodicDeviceStatusPrinting,
                        this,
                        endDevices,
                        gateways,
                        filename,
                        interval);
}

void
LorawanHelper::DoPrintDeviceStatus(NodeContainer endDevices,
                                   NodeContainer gateways,
                                   std::string filename)
{
    const char* c = filename.c_str();
    std::ofstream outputFile;
    if (Simulator::Now() == Seconds(0))
    {
        // Delete contents of the file as it is opened
        outputFile.open(c, std::ofstream::out | std::ofstream::trunc);
    }
    else
    {
        // Only append to the file
        outputFile.open(c, std::ofstream::out | std::ofstream::app);
    }

    Time currentTime = Simulator::Now();
    DevPktCount devPktCount;
    m_packetTracker->CountAllDevicesPackets(m_lastDeviceStatusUpdate, currentTime, devPktCount);

    for (NodeContainer::Iterator j = endDevices.Begin(); j != endDevices.End(); ++j)
    {
        auto node = *j;
        auto position = node->GetObject<MobilityModel>();
        auto loraNetDevice = DynamicCast<LoraNetDevice>(node->GetDevice(0));
        auto mac = DynamicCast<BaseEndDeviceLorawanMac>(loraNetDevice->GetMac());
        auto app = DynamicCast<LoraApplication>(node->GetApplication(0));

        Vector pos = position->GetPosition();

        double gwdist = std::numeric_limits<double>::max();
        for (auto gw = gateways.Begin(); gw != gateways.End(); ++gw)
        {
            gwdist = std::min(gwdist, (*gw)->GetObject<MobilityModel>()->GetDistanceFrom(position));
        }

        int dr = int(mac->GetDataRate());

        double txPower = mac->GetTransmissionPower();

        devCount_t& count = devPktCount[node->GetId()];

        // Add: #sent, #received, max-offered-traffic, duty-cycle
        uint8_t size = app->GetPacketSize();
        double interval = app->GetInterval().GetSeconds();
        LoraPhyTxParameters params;
        params.sf = 12 - dr;
        params.lowDataRateOptimizationEnabled = LoraPhy::GetTSym(params) > MilliSeconds(16);
        double maxot =
            LoraPhy::GetTimeOnAir(Create<Packet>(size + 13), params).GetSeconds() / interval;
        maxot = std::min(maxot, 0.01);

        double ot = mac->GetAggregatedDutyCycle();
        ot = std::min(ot, maxot);

        outputFile << currentTime.GetSeconds() << " " << node->GetId() << " " << pos.x << " "
                   << pos.y << " " << pos.z << " " << gwdist << " " << dr << " "
                   << unsigned(txPower) << " " << count.sent << " " << count.received << " "
                   << maxot << " " << ot << std::endl;
    }
    m_lastDeviceStatusUpdate = Simulator::Now();
    outputFile.close();
}

void
LorawanHelper::EnablePeriodicGwsPerformancePrinting(NodeContainer gateways,
                                                    std::string filename,
                                                    Time interval)
{
    NS_LOG_FUNCTION(this);

    DoPrintGwsPerformance(gateways, filename);

    Simulator::Schedule(interval,
                        &LorawanHelper::EnablePeriodicGwsPerformancePrinting,
                        this,
                        gateways,
                        filename,
                        interval);
}

void
LorawanHelper::DoPrintGwsPerformance(NodeContainer gateways, std::string filename)
{
    NS_LOG_FUNCTION(this);

    const char* c = filename.c_str();
    std::ofstream outputFile;
    if (Simulator::Now() == Seconds(0))
    {
        // Delete contents of the file as it is opened
        outputFile.open(c, std::ofstream::out | std::ofstream::trunc);
    }
    else
    {
        // Only append to the file
        outputFile.open(c, std::ofstream::out | std::ofstream::app);
    }

    GwsPhyPktPrint strings;
    m_packetTracker->PrintPhyPacketsAllGws(m_lastPhyPerformanceUpdate, Simulator::Now(), strings);
    for (auto it = gateways.Begin(); it != gateways.End(); ++it)
    {
        int systemId = (*it)->GetId();
        outputFile << Simulator::Now().GetSeconds() << " " << std::to_string(systemId) << " "
                   << strings[systemId].s << std::endl;
    }

    m_lastPhyPerformanceUpdate = Simulator::Now();

    outputFile.close();
}

void
LorawanHelper::EnablePeriodicGlobalPerformancePrinting(std::string filename, Time interval)
{
    NS_LOG_FUNCTION(this << filename << interval);

    DoPrintGlobalPerformance(filename);

    Simulator::Schedule(interval,
                        &LorawanHelper::EnablePeriodicGlobalPerformancePrinting,
                        this,
                        filename,
                        interval);
}

void
LorawanHelper::DoPrintGlobalPerformance(std::string filename)
{
    NS_LOG_FUNCTION(this);

    const char* c = filename.c_str();
    std::ofstream outputFile;
    if (Simulator::Now() == Seconds(0))
    {
        // Delete contents of the file as it is opened
        outputFile.open(c, std::ofstream::out | std::ofstream::trunc);
    }
    else
    {
        // Only append to the file
        outputFile.open(c, std::ofstream::out | std::ofstream::app);
    }

    outputFile << Simulator::Now().GetSeconds() << " "
               << m_packetTracker->PrintPhyPacketsGlobally(m_lastGlobalPerformanceUpdate,
                                                           Simulator::Now())
               << std::endl;

    m_lastGlobalPerformanceUpdate = Simulator::Now();

    outputFile.close();
}

void
LorawanHelper::EnablePeriodicSFStatusPrinting(NodeContainer endDevices,
                                              NodeContainer gateways,
                                              std::string filename,
                                              Time interval)
{
    NS_LOG_FUNCTION(this);

    DoPrintSFStatus(endDevices, gateways, filename);

    // Schedule periodic printing
    Simulator::Schedule(interval,
                        &LorawanHelper::EnablePeriodicSFStatusPrinting,
                        this,
                        endDevices,
                        gateways,
                        filename,
                        interval);
}

void
LorawanHelper::DoPrintSFStatus(NodeContainer endDevices,
                               NodeContainer gateways,
                               std::string filename)
{
    const char* c = filename.c_str();
    std::ofstream outputFile;
    if (Simulator::Now() == Seconds(0))
    {
        // Delete contents of the file as it is opened
        outputFile.open(c, std::ofstream::out | std::ofstream::trunc);
    }
    else
    {
        // Only append to the file
        outputFile.open(c, std::ofstream::out | std::ofstream::app);
    }

    Time currentTime = Simulator::Now();
    DevPktCount devPktCount;
    m_packetTracker->CountAllDevicesPackets(m_lastSFStatusUpdate, currentTime, devPktCount);

    struct sfStatus_t
    {
        int sent = 0;
        int received = 0;
        double totMaxOT = 0.0;
        double totAggDC = 0.0;
        double totEnergy = 0.0;
    };

    using sfMap_t = std::map<int, sfStatus_t>;
    sfMap_t sfmap;

    for (NodeContainer::Iterator j = endDevices.Begin(); j != endDevices.End(); ++j)
    {
        // Obtain device information
        auto node = *j;
        auto loraNetDevice = DynamicCast<LoraNetDevice>(node->GetDevice(0));
        auto mac = DynamicCast<BaseEndDeviceLorawanMac>(loraNetDevice->GetMac());
        auto app = DynamicCast<LoraApplication>(node->GetApplication(0));

        int dr = int(mac->GetDataRate());
        sfStatus_t& sfstat = sfmap[dr];

        // Sent, received
        devCount_t& count = devPktCount[node->GetId()];
        sfstat.sent += count.sent;
        sfstat.received += count.received;

        // Max-offered-traffic, duty-cycle
        uint8_t size = app->GetPacketSize();
        double interval = app->GetInterval().GetSeconds();
        LoraPhyTxParameters params;
        params.sf = 12 - dr;
        params.lowDataRateOptimizationEnabled = LoraPhy::GetTSym(params) > MilliSeconds(16);
        double maxot =
            LoraPhy::GetTimeOnAir(Create<Packet>(size + 13), params).GetSeconds() / interval;
        maxot = std::min(maxot, 0.01);
        double ot = mac->GetAggregatedDutyCycle();
        ot = std::min(ot, maxot);
        sfstat.totMaxOT += maxot;
        sfstat.totAggDC += ot;

        // Total energy consumed
        if (auto esc = node->GetObject<EnergySourceContainer>())
        {
            auto demc = esc->Get(0)->FindDeviceEnergyModels("ns3::LoraRadioEnergyModel");
            if (demc.GetN())
            {
                sfstat.totEnergy += demc.Get(0)->GetTotalEnergyConsumption();
            }
        }
    }

    for (const auto& sf : sfmap)
        outputFile << currentTime.GetSeconds() << " " << sf.first << " " << sf.second.sent << " "
                   << sf.second.received << " " << sf.second.totMaxOT << " " << sf.second.totAggDC
                   << " " << sf.second.totEnergy << std::endl;

    m_lastSFStatusUpdate = Simulator::Now();
    outputFile.close();
}

void
LorawanHelper::EnablePrinting(NodeContainer endDevices,
                              NodeContainer gateways,
                              std::vector<enum TraceLevel> levels,
                              Time samplePeriod)
{
    std::vector<bool> active(5, false);
    for (auto l : levels)
    {
        if (active[l])
            continue;
        switch (l)
        {
        case NET:
            EnablePeriodicGlobalPerformancePrinting("globalPerformance.txt", samplePeriod);
            break;
        case GW:
            EnablePeriodicGwsPerformancePrinting(gateways, "gwData.txt", samplePeriod);
            break;
        case SF:
            EnablePeriodicSFStatusPrinting(endDevices, gateways, "sfData.txt", samplePeriod);
            break;
        case DEV:
            EnablePeriodicDeviceStatusPrinting(endDevices,
                                               gateways,
                                               "deviceStatus.txt",
                                               samplePeriod);
            break;
        case PKT:
        default:
            break;
        }
        active[l] = true;
    }
}

void
LorawanHelper::EnablePcapInternal(std::string prefix,
                                  Ptr<NetDevice> nd,
                                  bool promiscuous,
                                  bool explicitFilename)
{
    NS_LOG_FUNCTION(this << prefix << nd << promiscuous << explicitFilename);

    //
    // All of the Pcap enable functions vector through here including the ones
    // that are wandering through all of devices on perhaps all of the nodes in
    // the system.  We can only deal with devices of type LoraNetDevice.
    //
    Ptr<LoraNetDevice> device = DynamicCast<LoraNetDevice>(nd);
    if (bool(device) == 0)
    {
        NS_LOG_INFO("LorawanHelper::EnablePcapInternal(): Device "
                    << device << " not of type ns3::LoraNetDevice");
        return;
    }

    auto phy = device->GetPhy();
    NS_ABORT_MSG_IF(bool(phy) == 0,
                    "LoRaHelper::EnablePcapInternal(): Phy layer in LoraNetDevice must be set");

    PcapHelper pcapHelper;

    std::string filename;
    if (explicitFilename)
    {
        filename = prefix;
    }
    else
    {
        filename = pcapHelper.GetFilenameFromDevice(prefix, device);
    }

    auto file = pcapHelper.CreateFile(filename, std::ios::out, PcapHelper::DLT_LORATAP);
    phy->TraceConnectWithoutContext("SnifferRx",
                                    MakeBoundCallback(&LorawanHelper::PcapSniffRxEvent, file));
    phy->TraceConnectWithoutContext("SnifferTx",
                                    MakeBoundCallback(&LorawanHelper::PcapSniffTxEvent, file));
}

void
LorawanHelper::PcapSniffRxEvent(Ptr<PcapFileWrapper> file, Ptr<const Packet> packet)
{
    Ptr<Packet> p = packet->Copy();
    LoraTag tag;
    p->RemovePacketTag(tag);
    LoratapHeader header;
    header.Fill(tag);
    p->AddHeader(header);
    file->Write(Simulator::Now(), p);
}

void
LorawanHelper::PcapSniffTxEvent(Ptr<PcapFileWrapper> file, Ptr<const Packet> packet)
{
    Ptr<Packet> p = packet->Copy();
    LoraTag tag;
    p->RemovePacketTag(tag);
    LoratapHeader header;
    header.Fill(tag);
    p->AddHeader(header);
    file->Write(Simulator::Now(), p);
}

} // namespace lorawan
} // namespace ns3
