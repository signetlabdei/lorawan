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

LoraHelper::LoraHelper ()
{
}

LoraHelper::~LoraHelper ()
{
}

NetDeviceContainer
LoraHelper::Install ( const LoraPhyHelper &phyHelper,
                      const LoraMacHelper &macHelper,
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
      Ptr<LoraMac> mac = macHelper.Create (node, device);
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
                      const LoraMacHelper &mac,
                      Ptr<Node> node) const
{
  return Install (phy, mac, NodeContainer (node));
}

void
LoraHelper::EnablePacketTracking (std::string filename)
{
  NS_LOG_FUNCTION (this << filename);

  // Create the packet tracker
  m_packetTracker = new LoraPacketTracker (filename);
}

void
LoraHelper::EnableSimulationTimePrinting (void)
{
  m_oldtime = std::time (0);
  Simulator::Schedule (Minutes (30), &LoraHelper::PrintSimulationTime, this);
}

void
LoraHelper::PrintSimulationTime (void)
{
  // NS_LOG_INFO ("Time: " << Simulator::Now().GetHours());
  std::cout << "Simulated time: " << Simulator::Now ().GetHours () << " hours" << std::endl;
  std::cout << "Real time from last call: " << std::time (0) - m_oldtime << " seconds" << std::endl;
  m_oldtime = std::time (0);
  Simulator::Schedule (Minutes (30), &LoraHelper::PrintSimulationTime, this);
}

void
LoraHelper::PrintPerformance (Time start, Time stop)
{
  m_packetTracker->PrintPerformance (start, stop);
}

void
LoraHelper::CountPhyPackets (Time start, Time stop)
{
  // Statistics in considered time frame
  m_packetTracker->CountPhyPackets (start, stop);
}

void
LoraHelper::PrintEndDevices (NodeContainer endDevices, NodeContainer gateways,
                             std::string filename)
{
  const char * c = filename.c_str ();
  std::ofstream spreadingFactorFile;
  spreadingFactorFile.open (c);
  for (NodeContainer::Iterator j = endDevices.Begin (); j != endDevices.End (); ++j)
    {
      Ptr<Node> object = *j;
      Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
      NS_ASSERT (position != 0);
      Ptr<NetDevice> netDevice = object->GetDevice (0);
      Ptr<LoraNetDevice> loraNetDevice = netDevice->GetObject<LoraNetDevice> ();
      NS_ASSERT (loraNetDevice != 0);
      Ptr<EndDeviceLoraMac> mac = loraNetDevice->GetMac ()->GetObject<EndDeviceLoraMac> ();
      int sf = int(mac->GetDataRate ());
      Vector pos = position->GetPosition ();
      spreadingFactorFile << pos.x << " " << pos.y << " " << sf << std::endl;
    }
  // Also print the gateways
  for (NodeContainer::Iterator j = gateways.Begin (); j != gateways.End (); ++j)
    {
      Ptr<Node> object = *j;
      Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
      Vector pos = position->GetPosition ();
      spreadingFactorFile << pos.x << " " << pos.y << " GW" << std::endl;
    }
  spreadingFactorFile.close ();
}
}
}
