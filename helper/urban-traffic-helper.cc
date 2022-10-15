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
 * Author: Alessandro Aimi <alessandro.aimi@cnam.fr>
 *                         <alessandro.aimi@orange.com>
 */

#include "ns3/urban-traffic-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/periodic-sender.h"
#include "ns3/poisson-sender.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("UrbanTrafficHelper");

UrbanTrafficHelper::UrbanTrafficHelper ()
{
  m_intervalProb = CreateObject<UniformRandomVariable> ();
  m_intervalProb->SetAttribute ("Min", DoubleValue (0));
  m_intervalProb->SetAttribute ("Max", DoubleValue (1));

  const std::vector<double> pdf = {20.947, 2200.0, 316.47, 15.03,   15.03,  69.823, 3845.0,
                                   384.5,  3845.0, 3845.0, 26915.0, 7690.0, 11535.0};
  m_cdf = std::vector<double> (pdf.size (), 0.0);
  double sum = 0.0;
  for (size_t i = 0; i < pdf.size (); ++i)
    {
      sum += pdf[i];
      m_cdf[i] += sum;
    }
  for (size_t i = 0; i < pdf.size (); ++i)
    m_cdf[i] /= sum;
}

UrbanTrafficHelper::~UrbanTrafficHelper ()
{
}

ApplicationContainer
UrbanTrafficHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UrbanTrafficHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
UrbanTrafficHelper::InstallPriv (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);

  double intervalProb = m_intervalProb->GetValue ();

  Time interval = Minutes (10);
  uint8_t pktSize = 18;
  bool poisson = false;
  std::string type = "generic";

  Ptr<LoraApplication> app;

  /**
   * From [IEEE C802.16p-11/0102r2]
   * 
   * ------------------------------------------------------------------------------------
   * |        Application       |   Density   | Interval | PacketSize |     Traffic     | Discussion:
   * |  (Commercial, In-House)  | [nodes/km2] |   [s]    |    [B]     |                 |
   * ------------------------------------------------------------------------------------    
   * | Credit machine (grocery) | 20.947      | 120      | 24         | Poisson         | reliability critical, duty-cycle limited on SF12
   * | Credit machine (shop)    | 2200.0      | 1800     | 24         | Poisson         | reliability critical
   * | Roadway sign             | 316.47      | 30       | 1          | Uniform         | duty-cycle limited on SF11 & SF12
   * | Traffic light            | 15.03       | 60       | 1          | Uniform         | duty-cycle limited on SF12
   * | Traffic sensor           | 15.03       | 60       | 1          | Poisson         | duty-cycle limited on SF12
   * | Movie rental machine     | 69.823      | 28800    | 51         | Poisson         | do they actually still exist?
   * ------------------------------------------------------------------------------------
   * | Home security system     | 3845.0      | 600      | 20         | Poisson/uniform | reliability critical
   * | Elderly sensor device    | 384.5       | 20       | 43         | Poisson/uniform | reliability critical, duty-cycle limited from SF8
   * | Refrigerator             | 3845.0      | 3600     | 30         | Poisson/uniform |
   * | Freezer                  | 3845.0      | 86400    | 30         | Poisson/uniform |
   * | Other house appliance    | 26915.0     | 86400    | 8          | Poisson/uniform |
   * | PHEV charging station    | 7690.0      | 2100     | 49         | Poisson/uniform |
   * | Smart meter              | 11535.0     | 300      | 48         | Poisson/uniform |
   * ------------------------------------------------------------------------------------
   * 
   * We could implement packet fragmentation...
   */

  if (intervalProb < m_cdf[0]) // Credit machine (grocery)
    {
      interval = Minutes (2);
      pktSize = 24;
      poisson = true;
      type = "Credit machine (grocery)";
    }
  else if (intervalProb < m_cdf[1]) // Credit machine (shop)
    {
      interval = Minutes (30);
      pktSize = 24;
      poisson = true;
      type = "Credit machine (shop)";
    }
  else if (intervalProb < m_cdf[2]) // Roadway sign
    {
      interval = Seconds (30);
      pktSize = 1;
      poisson = false;
      type = "Roadway sign";
    }
  else if (intervalProb < m_cdf[3]) // Traffic light
    {
      interval = Minutes (1);
      pktSize = 1;
      poisson = false;
      type = "Traffic light";
    }
  else if (intervalProb < m_cdf[4]) // Traffic sensor
    {
      interval = Minutes (1);
      pktSize = 1;
      poisson = true;
      type = "Traffic sensor";
    }
  else if (intervalProb < m_cdf[5]) // Movie rental machine
    {
      interval = Hours (8);
      pktSize = 51;
      poisson = true;
      type = "Movie rental machine";
    }
  else if (intervalProb < m_cdf[6]) // Home security system
    {
      interval = Minutes (10);
      pktSize = 20;
      poisson = (bool) m_intervalProb->GetInteger (0, 1);
      type = "Home security system";
    }
  else if (intervalProb < m_cdf[7]) // Elderly sensor device
    {
      interval = Seconds (20);
      pktSize = 43;
      poisson = (bool) m_intervalProb->GetInteger (0, 1);
      type = "Elderly sensor device";
    }
  else if (intervalProb < m_cdf[8]) // Refrigerator
    {
      interval = Hours (1);
      pktSize = 30;
      poisson = (bool) m_intervalProb->GetInteger (0, 1);
      type = "Refrigerator";
    }
  else if (intervalProb < m_cdf[9]) // Freezer
    {
      interval = Days (1);
      pktSize = 30;
      poisson = (bool) m_intervalProb->GetInteger (0, 1);
      type = "Freezer";
    }
  else if (intervalProb < m_cdf[10]) // Other house appliance
    {
      interval = Days (1);
      pktSize = 8;
      poisson = (bool) m_intervalProb->GetInteger (0, 1);
      type = "Other house appliance";
    }
  else if (intervalProb < m_cdf[11]) // PHEV charging station
    {
      interval = Minutes (35);
      pktSize = 49;
      poisson = (bool) m_intervalProb->GetInteger (0, 1);
      type = "PHEV charging station";
    }
  else // m_cdf[12], Smart meter
    {
      interval = Minutes (10);
      pktSize = 48;
      poisson = (bool) m_intervalProb->GetInteger (0, 1);
      type = "Smart meter";
    }

  if (poisson)
    app = CreateObjectWithAttributes<PoissonSender> ("Interval", TimeValue (interval), "PacketSize",
                                                     UintegerValue (pktSize));
  else
    app = CreateObjectWithAttributes<PeriodicSender> ("Interval", TimeValue (interval),
                                                      "PacketSize", UintegerValue (pktSize));

  NS_LOG_DEBUG ("Created: " << type << " (" << interval.GetSeconds () << "s, "
                             << (unsigned) pktSize << "B, " << ((poisson) ? "poisson)" : "uniform)"));
  app->SetInitialDelay (Seconds (m_intervalProb->GetValue (0, interval.GetSeconds ())));

  app->SetNode (node);
  node->AddApplication (app);

  return app;
}

} // namespace lorawan
} // namespace ns3
