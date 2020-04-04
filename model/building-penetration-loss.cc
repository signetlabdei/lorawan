/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "ns3/building-penetration-loss.h"
#include "ns3/mobility-building-info.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include <cmath>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("BuildingPenetrationLoss");

NS_OBJECT_ENSURE_REGISTERED (BuildingPenetrationLoss);

TypeId
BuildingPenetrationLoss::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BuildingPenetrationLoss")
    .SetParent<PropagationLossModel> ()
    .SetGroupName ("Lora")
    .AddConstructor<BuildingPenetrationLoss> ()
  ;
  return tid;
}

BuildingPenetrationLoss::BuildingPenetrationLoss ()
{
  NS_LOG_FUNCTION_NOARGS ();

  // Initialize the random variable
  m_uniformRV = CreateObject<UniformRandomVariable> ();
}

BuildingPenetrationLoss::~BuildingPenetrationLoss ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

double
BuildingPenetrationLoss::DoCalcRxPower (double txPowerDbm,
                                        Ptr<MobilityModel> a,
                                        Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << txPowerDbm << a << b);

  Ptr<MobilityBuildingInfo> a1 = a->GetObject<MobilityBuildingInfo> ();
  Ptr<MobilityBuildingInfo> b1 = b->GetObject<MobilityBuildingInfo> ();

  // These are the components of the loss due to building penetration
  double externalWallLoss = 0;
  double tor1 = 0;
  double tor3 = 0;
  double gfh = 0;

  // Go through various cases in which a and b are indoors or outdoors
  if ((b1->IsIndoor () && !a1->IsIndoor ()))
    {
      NS_LOG_INFO ("Tx is outdoors and Rx is indoors");

      externalWallLoss = GetWallLoss (b);     // External wall loss due to b
      tor1 = GetTor1 (b);     // Internal wall loss due to b
      tor3 = 0.6 * m_uniformRV->GetValue (0, 15);
      gfh = 0;

    }
  else if ((!b1->IsIndoor () && a1->IsIndoor ()))
    {
      NS_LOG_INFO ("Rx is outdoors and Tx is indoors");

      // These are the components of the loss due to building penetration
      externalWallLoss = GetWallLoss (a);
      tor1 = GetTor1 (a);
      tor3 = 0.6 * m_uniformRV->GetValue (0, 15);
      gfh = 0;

    }
  else if (!a1->IsIndoor ()&& !b1->IsIndoor ())
    {
      NS_LOG_DEBUG ("No penetration loss since both devices are outside");
    }
  else if (a1->IsIndoor ()&& b1->IsIndoor ())
    {
      // They are in the same building
      if (a1->GetBuilding () == b1->GetBuilding ())
        {
          NS_LOG_INFO ("Devices are in the same building");
          // Only internal wall loss
          tor1 = GetTor1 (b);
          tor3 = 0.6 * m_uniformRV->GetValue (0, 15);
        }
      // They are in different buildings
      else
        {
          // These are the components of the loss due to building penetration
          externalWallLoss = GetWallLoss (b) + GetWallLoss (a);
          tor1 = GetTor1 (b) + GetTor1 (a);
          tor3 = 0.6 * m_uniformRV->GetValue (0, 15);
          gfh = 0;
        }
    }

  NS_LOG_DEBUG ("Building penetration loss:" <<
                " externalWallLoss = " << externalWallLoss <<
                ", tor1 = " << tor1 <<
                ", tor3 = " << tor3 <<
                ", GFH = " << gfh);

  // Put together all the pieces
  double loss = externalWallLoss + std::max (tor1, tor3) - gfh;

  NS_LOG_DEBUG ("Total loss due to building penetration: " << loss);

  return txPowerDbm - loss;
}

int64_t
BuildingPenetrationLoss::DoAssignStreams (int64_t stream)
{
  m_uniformRV->SetStream (stream);
  return 1;
}

int
BuildingPenetrationLoss::GetPValue (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // We need to decide on the p value to return
  double random = m_uniformRV->GetValue (0.0, 1.0);

  // Distribution is specified in TR 45.820, page 482, first scenario
  if (random < 0.2833)
    {
      return 0;
    }
  else if (random < 0.566)
    {
      return 1;
    }
  else if (random < 0.85)
    {
      return 2;
    }
  else
    {
      return 3;
    }
}

int
BuildingPenetrationLoss::GetWallLossValue (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // We need to decide on the random value to return
  double random = m_uniformRV->GetValue (0.0, 1.0);

  // Distribution is specified in TR 45.820, page 482, first scenario
  if (random < 0.25)
    {
      return 0;
    }
  else if (random < 0.9)
    {
      return 1;
    }
  else
    {
      return 2;
    }
}

double
BuildingPenetrationLoss::GetWallLoss (Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << b);

  std::map<Ptr<MobilityModel>, int>::const_iterator it;

  // Check whether the b device already has a wall loss value
  it = m_wallLossMap.find (b);
  if (it == m_wallLossMap.end ())
    {
      // Create a random value and insert it on the map
      m_wallLossMap[b] = GetWallLossValue ();
      NS_LOG_DEBUG ("Inserted a new wall loss value: " <<
                    m_wallLossMap.find (b)->second);
    }

  switch (m_wallLossMap.find (b)->second)
    {
    case 0:
      return m_uniformRV->GetValue (4, 11);
    case 1:
      return m_uniformRV->GetValue (11, 19);
    case 2:
      return m_uniformRV->GetValue (19, 23);
    }

  // Case in which something goes wrong
  return 0;
}

double
BuildingPenetrationLoss::GetTor1 (Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << b);

  std::map<Ptr<MobilityModel>, int>::const_iterator it;

  // Check whether the b device already has a p value
  it = m_pMap.find (b);
  if (it == m_pMap.end ())
    {
      // Create a random p value and insert it on the map
      m_pMap[b] = GetPValue ();
      NS_LOG_DEBUG ("Inserted a new p value: " << m_pMap.find (b)->second);
    }
  return m_uniformRV->GetValue (4, 10) * m_pMap.find (b)->second;
}
}
}
