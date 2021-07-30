/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 */

#include "ns3/lora-helper.h"
#include "ns3/log.h"

#include <fstream>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("LoraHelper");

  LoraHelper::LoraHelper () :
    m_lastPhyPerformanceUpdate (Seconds (0)),
    m_lastGlobalPerformanceUpdate (Seconds (0))
  {
  }

  LoraHelper::~LoraHelper ()
  {
  }

  NetDeviceContainer
  LoraHelper::Install ( const LoraPhyHelper &phyHelper,
                        const LorawanMacHelper &macHelper,
                        NodeContainer c) const
  {
    NS_LOG_FUNCTION_NOARGS ();

    NetDeviceContainer devices;

    // Go over the various nodes in which to install the NetDevice
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
      {
        Ptr<Node> node = *i;

        // Create the LoraNetDevice
        Ptr<LoraNetDevice> device = CreateObject<LoraNetDevice> ();

        // Create the PHY
        Ptr<LoraPhy> phy = phyHelper.Create (node, device);
        NS_ASSERT (phy != 0);
        device->SetPhy (phy);
        NS_LOG_DEBUG ("Done creating the PHY");

        // Connect Trace Sources if necessary
        if (m_packetTracker)
          {
            if (phyHelper.GetDeviceType () ==
                TypeId::LookupByName ("ns3::SimpleEndDeviceLoraPhy"))
              {
                phy->TraceConnectWithoutContext ("StartSending",
                                                 MakeCallback
                                                 (&LoraPacketTracker::TransmissionCallback,
                                                  m_packetTracker));
              }
            else if (phyHelper.GetDeviceType () ==
                     TypeId::LookupByName ("ns3::SimpleGatewayLoraPhy"))
            {
              phy->TraceConnectWithoutContext ("StartSending",
                                               MakeCallback
                                               (&LoraPacketTracker::TransmissionCallback,
                                                m_packetTracker));
              phy->TraceConnectWithoutContext ("ReceivedPacket",
                                               MakeCallback
                                                 (&LoraPacketTracker::PacketReceptionCallback,
                                                 m_packetTracker));
              phy->TraceConnectWithoutContext ("LostPacketBecauseInterference",
                                               MakeCallback
                                                 (&LoraPacketTracker::InterferenceCallback,
                                                 m_packetTracker));
              phy->TraceConnectWithoutContext ("LostPacketBecauseNoMoreReceivers",
                                               MakeCallback
                                                 (&LoraPacketTracker::NoMoreReceiversCallback,
                                                 m_packetTracker));
              phy->TraceConnectWithoutContext ("LostPacketBecauseUnderSensitivity",
                                               MakeCallback
                                                 (&LoraPacketTracker::UnderSensitivityCallback,
                                                 m_packetTracker));
              phy->TraceConnectWithoutContext ("NoReceptionBecauseTransmitting",
                                               MakeCallback
                                                 (&LoraPacketTracker::LostBecauseTxCallback,
                                                 m_packetTracker));
            }
        }

      // Create the MAC
      Ptr<LorawanMac> mac = macHelper.Create (node, device);
      NS_ASSERT (mac != 0);
      mac->SetPhy (phy);
      NS_LOG_DEBUG ("Done creating the MAC");
      device->SetMac (mac);

      if (m_packetTracker)
        {
          if (phyHelper.GetDeviceType () ==
              TypeId::LookupByName ("ns3::SimpleEndDeviceLoraPhy"))
            {
              mac->TraceConnectWithoutContext ("SentNewPacket",
                                               MakeCallback
                                                 (&LoraPacketTracker::MacTransmissionCallback,
                                                 m_packetTracker));

              mac->TraceConnectWithoutContext ("RequiredTransmissions",
                                               MakeCallback
                                                 (&LoraPacketTracker::RequiredTransmissionsCallback,
                                                 m_packetTracker));
            }
          else if (phyHelper.GetDeviceType () ==
                   TypeId::LookupByName ("ns3::SimpleGatewayLoraPhy"))
            {
              mac->TraceConnectWithoutContext ("SentNewPacket",
                                               MakeCallback
                                               (&LoraPacketTracker::MacTransmissionCallback,
                                                m_packetTracker));

              mac->TraceConnectWithoutContext ("ReceivedPacket",
                                               MakeCallback
                                               (&LoraPacketTracker::MacGwReceptionCallback,
                                                m_packetTracker));
            }
        }

      node->AddDevice (device);
      devices.Add (device);
      NS_LOG_DEBUG ("node=" << node << ", mob=" << node->GetObject<MobilityModel> ()->GetPosition ());
    }
  return devices;
}

NetDeviceContainer
LoraHelper::Install ( const LoraPhyHelper &phy,
                      const LorawanMacHelper &mac,
                      Ptr<Node> node) const
{
  return Install (phy, mac, NodeContainer (node));
}

void
LoraHelper::EnablePacketTracking ()
{
  NS_LOG_FUNCTION (this);

  // Create the packet tracker
  m_packetTracker = new LoraPacketTracker ();
}

LoraPacketTracker&
LoraHelper::GetPacketTracker (void)
{
  NS_LOG_FUNCTION (this);

  return *m_packetTracker;
}

void
LoraHelper::EnableSimulationTimePrinting (Time interval)
{
  m_oldtime = std::time (0);
  Simulator::Schedule (Seconds (0), &LoraHelper::DoPrintSimulationTime, this,
                       interval);
}

void
LoraHelper::EnablePeriodicDeviceStatusPrinting (NodeContainer endDevices,
                                                NodeContainer gateways,
                                                std::string filename,
                                                Time interval)
{
  NS_LOG_FUNCTION (this);

  DoPrintDeviceStatus (endDevices, gateways, filename);

  // Schedule periodic printing
  Simulator::Schedule (interval,
                       &LoraHelper::EnablePeriodicDeviceStatusPrinting, this,
                       endDevices, gateways, filename, interval);
}

void
LoraHelper::DoPrintDeviceStatus (NodeContainer endDevices, NodeContainer gateways,
                                 std::string filename)
{
  const char * c = filename.c_str ();
  std::ofstream outputFile;
  if (Simulator::Now () == Seconds (0))
    {
      // Delete contents of the file as it is opened
      outputFile.open (c, std::ofstream::out | std::ofstream::trunc);
    }
  else
    {
      // Only append to the file
      outputFile.open (c, std::ofstream::out | std::ofstream::app);
    }

  Time currentTime = Simulator::Now();
  for (NodeContainer::Iterator j = endDevices.Begin (); j != endDevices.End (); ++j)
    {
      Ptr<Node> object = *j;
      Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
      NS_ASSERT (position != 0);
      Ptr<NetDevice> netDevice = object->GetDevice (0);
      Ptr<LoraNetDevice> loraNetDevice = netDevice->GetObject<LoraNetDevice> ();
      NS_ASSERT (loraNetDevice != 0);
      Ptr<ClassAEndDeviceLorawanMac> mac = loraNetDevice->GetMac ()->GetObject<ClassAEndDeviceLorawanMac> ();
      int dr = int(mac->GetDataRate ());
      double txPower = mac->GetTransmissionPower ();
      Vector pos = position->GetPosition ();
      outputFile << currentTime.GetSeconds () << " "
                 << object->GetId () <<  " "
                 << pos.x << " " << pos.y << " " << dr << " "
                 << unsigned(txPower) << std::endl;
    }
  // for (NodeContainer::Iterator j = gateways.Begin (); j != gateways.End (); ++j)
  //   {
  //     Ptr<Node> object = *j;
  //     Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
  //     Vector pos = position->GetPosition ();
  //     outputFile << currentTime.GetSeconds () << " "
  //                << object->GetId () <<  " "
  //                << pos.x << " " << pos.y << " " << "-1 -1" << std::endl;
  //   }
  outputFile.close ();
}


void
LoraHelper::EnablePeriodicPhyPerformancePrinting (NodeContainer gateways,
                                                  std::string filename,
                                                  Time interval)
{
  NS_LOG_FUNCTION (this);

  DoPrintPhyPerformance (gateways, filename);

  Simulator::Schedule (interval,
                       &LoraHelper::EnablePeriodicPhyPerformancePrinting,
                       this,
                       gateways, filename, interval);
}

void
LoraHelper::DoPrintPhyPerformance (NodeContainer gateways,
                                   std::string filename)
{
  NS_LOG_FUNCTION (this);

  const char * c = filename.c_str ();
  std::ofstream outputFile;
  if (Simulator::Now () == Seconds (0))
    {
      // Delete contents of the file as it is opened
      outputFile.open (c, std::ofstream::out | std::ofstream::trunc);
    }
  else
    {
      // Only append to the file
      outputFile.open (c, std::ofstream::out | std::ofstream::app);
    }

  for (auto it = gateways.Begin (); it != gateways.End (); ++it)
    {
      int systemId = (*it)->GetId ();
      outputFile << Simulator::Now ().GetSeconds () << " " <<
        std::to_string(systemId) << " " <<
        m_packetTracker->PrintPhyPacketsPerGw(m_lastPhyPerformanceUpdate,
                                              Simulator::Now (),
                                              systemId) << std::endl;
    }

  m_lastPhyPerformanceUpdate = Simulator::Now ();

  outputFile.close();
}

void
LoraHelper::EnablePeriodicGlobalPerformancePrinting (std::string filename,
                                                     Time interval)
{
  NS_LOG_FUNCTION (this << filename << interval);

  DoPrintGlobalPerformance (filename);

  Simulator::Schedule (interval,
                       &LoraHelper::EnablePeriodicGlobalPerformancePrinting,
                       this,
                       filename, interval);
}

void
LoraHelper::DoPrintGlobalPerformance (std::string filename)
{
  NS_LOG_FUNCTION (this);

  const char * c = filename.c_str ();
  std::ofstream outputFile;
  if (Simulator::Now () == Seconds (0))
    {
      // Delete contents of the file as it is opened
      outputFile.open (c, std::ofstream::out | std::ofstream::trunc);
    }
  else
    {
      // Only append to the file
      outputFile.open (c, std::ofstream::out | std::ofstream::app);
    }

  outputFile << Simulator::Now ().GetSeconds () << " " <<
    m_packetTracker->CountMacPacketsGlobally (m_lastGlobalPerformanceUpdate,
                                              Simulator::Now ()) <<
    std::endl;

  m_lastGlobalPerformanceUpdate = Simulator::Now ();

  outputFile.close();
}

void
LoraHelper::DoPrintSimulationTime (Time interval)
{
  // NS_LOG_INFO ("Time: " << Simulator::Now().GetHours());
  std::cout << "Simulated time: " << Simulator::Now ().GetHours () << " hours" << std::endl;
  std::cout << "Real time from last call: " << std::time (0) - m_oldtime << " seconds" << std::endl;
  m_oldtime = std::time (0);
  Simulator::Schedule (interval, &LoraHelper::DoPrintSimulationTime, this, interval);
}

}
}
