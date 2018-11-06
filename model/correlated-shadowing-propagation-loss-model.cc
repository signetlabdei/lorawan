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

#include "ns3/correlated-shadowing-propagation-loss-model.h"
#include "ns3/double.h"
#include "ns3/log.h"
#include <cmath>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("CorrelatedShadowingPropagationLossModel");

NS_OBJECT_ENSURE_REGISTERED (CorrelatedShadowingPropagationLossModel);

TypeId
CorrelatedShadowingPropagationLossModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CorrelatedShwodingPropagationLossModel")
    .SetParent<PropagationLossModel> ()
    .SetGroupName ("Lora")
    .AddConstructor<CorrelatedShadowingPropagationLossModel> ()
    .AddAttribute ("CorrelationDistance",
                   "The distance at which the computed shadowing becomes"
                   "uncorrelated",
                   DoubleValue (110.0),
                   MakeDoubleAccessor
                     (&CorrelatedShadowingPropagationLossModel::m_correlationDistance),
                   MakeDoubleChecker<double> ());
  return tid;
}

CorrelatedShadowingPropagationLossModel::CorrelatedShadowingPropagationLossModel ()
{
}

double
CorrelatedShadowingPropagationLossModel::DoCalcRxPower (double txPowerDbm,
                                                        Ptr<MobilityModel> a,
                                                        Ptr<MobilityModel> b) const
{
  NS_LOG_FUNCTION (this << txPowerDbm << a << b);

  /*
   * Check whether the a MobilityModel is in a grid square that already has
   * its shadowing map.
   */
  Vector position = a->GetPosition ();

  double x = position.x;
  double y = position.y;

  // Compute the coordinates of the grid square (i.e., round the raw position)
  // (x > 0) - (x < 0) is the sign function
  int xcoord =
    ((x > 0) - (x < 0)) * ((std::fabs (x) + m_correlationDistance / 2) / m_correlationDistance);
  int ycoord =
    ((y > 0) - (y < 0)) * ((std::fabs (y) + m_correlationDistance / 2) / m_correlationDistance);

  // Wrap coordinates up in a pair
  std::pair<int, int> coordinates (xcoord, ycoord);

  NS_LOG_DEBUG ("x " << x << ", y " << y);
  NS_LOG_DEBUG ("xcoord " << xcoord << ", ycoord " << ycoord);

  // Look for the computed coordinates in the shadowingGrid
  std::map<std::pair<int,int>, Ptr<ShadowingMap> >::const_iterator it;

  it = m_shadowingGrid.find (coordinates);

  if (it == m_shadowingGrid.end ())     // Did not find the coordinates
    {
      // If this shadowing grid was not found, create it
      NS_LOG_DEBUG ("Creating a new shadowing map to be used at coordinates "
                    << coordinates.first << " " << coordinates.second);

      Ptr<ShadowingMap> shadowingMap =
        Create<CorrelatedShadowingPropagationLossModel::ShadowingMap> ();

      m_shadowingGrid[coordinates] = shadowingMap;
    }
  else
    {
      NS_LOG_DEBUG ("This square already has its shadowingMap!");
    }

  // Place the iterator on the coordinates
  it = m_shadowingGrid.find (coordinates);

  // Get b's position in a's ShadowingMap
  CorrelatedShadowingPropagationLossModel::Position bPosition
    (b->GetPosition ().x, b->GetPosition ().y);

  // Use the map of the a MobilityModel to determine the value of shadowing
  // that corresponds to the position of the MobilityModel b.
  double loss = it->second->GetLoss (bPosition);

  NS_LOG_INFO ("Shadowing loss: " << loss);

  return txPowerDbm - loss;
}

int64_t
CorrelatedShadowingPropagationLossModel::DoAssignStreams (int64_t stream)
{
  return 0;
}

/*********************************
 *  ShadowingMap implementation  *
 *********************************/

// k^{-1} was computed offline
const double CorrelatedShadowingPropagationLossModel::ShadowingMap::m_kInv[4][4] =
{
  {1.27968707244633, -0.366414485833771, -0.0415206295795327, -0.366414485833771},
  {-0.366414485833771, 1.27968707244633, -0.366414485833771, -0.0415206295795327},
  {-0.0415206295795327, -0.366414485833771, 1.27968707244633, -0.366414485833771},
  {-0.366414485833771, -0.0415206295795327, -0.366414485833771, 1.27968707244633}
};

CorrelatedShadowingPropagationLossModel::ShadowingMap::ShadowingMap () :
  m_correlationDistance (110)
{
  NS_LOG_FUNCTION_NOARGS ();

  // The generation of new variables and positions along the grid is handled
  // by the GetLoss function. Here, we only create the normal random variable.
  m_shadowingValue = CreateObject<NormalRandomVariable> ();
  m_shadowingValue->SetAttribute ("Mean", DoubleValue (0.0));
  m_shadowingValue->SetAttribute ("Variance", DoubleValue (16.0));
}

CorrelatedShadowingPropagationLossModel::ShadowingMap::~ShadowingMap ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

double
CorrelatedShadowingPropagationLossModel::ShadowingMap::GetLoss
  (CorrelatedShadowingPropagationLossModel::Position position)
{
  NS_LOG_FUNCTION (this << position.x << position.y);

  // Verify whether this position is already in the shadowingMap. Since the
  // Position implementation overloads the == operator, this comparison
  // between doubles is ok and we can use std::map's find function.
  std::map<CorrelatedShadowingPropagationLossModel::Position,
           double>::const_iterator it;
  it = m_shadowingMap.find (position);

  // If it's not found (i.e, if find returns the end of the map), we need to
  // generate the value at the specified position.
  if (it == m_shadowingMap.end ())
    {
      // Get the coordinates of the position
      double x = position.x;
      double y = position.y;
      int xcoord =
        ((x > 0) - (x < 0)) * ((std::fabs (x) + m_correlationDistance / 2) / m_correlationDistance);
      int ycoord =
        ((y > 0) - (y < 0)) * ((std::fabs (y) + m_correlationDistance / 2) / m_correlationDistance);

      // Verify whether there already are the 4 surrounding positions in the
      // map
      double xmin = xcoord * m_correlationDistance - m_correlationDistance / 2;
      double xmax = xcoord * m_correlationDistance + m_correlationDistance / 2;
      double ymin = ycoord * m_correlationDistance - m_correlationDistance / 2;
      double ymax = ycoord * m_correlationDistance + m_correlationDistance / 2;

      CorrelatedShadowingPropagationLossModel::Position lowerLeft (xmin, ymin);
      CorrelatedShadowingPropagationLossModel::Position upperLeft (xmin, ymax);
      CorrelatedShadowingPropagationLossModel::Position lowerRight (xmax, ymin);
      CorrelatedShadowingPropagationLossModel::Position upperRight (xmax, ymax);

      NS_LOG_DEBUG ("Generating a new shadowing value in the following quadrant:");
      NS_LOG_DEBUG ("xmin " << xmin << ", xmax " << xmax <<
                    ", ymin " << ymin << ", ymax " << ymax);

      // Use the map's insert method to insert the coordinates of the 4
      // surrounding positions (if they are already there, they won't be
      // substituted thanks to the map's implementation).
      // TODO: Avoid useless generation of ShadowingMap values. This can be
      // done by performing some checks (and not leveraging the map
      // implementation)
      double q11 = m_shadowingValue->GetValue ();
      NS_LOG_DEBUG ("Lower left corner: " << q11);
      m_shadowingMap[lowerLeft] = q11;
      double q12 = m_shadowingValue->GetValue ();
      NS_LOG_DEBUG ("Upper left corner: " << q12);
      m_shadowingMap[upperLeft] = q12;
      double q21 = m_shadowingValue->GetValue ();
      NS_LOG_DEBUG ("Lower right corner: " << q21);
      m_shadowingMap[lowerRight] = q21;
      double q22 = m_shadowingValue->GetValue ();
      NS_LOG_DEBUG ("Upper right corner: " << q22);
      m_shadowingMap[upperRight] = q22;

      NS_LOG_DEBUG (q11 << " " << q12 << " " << q21 << " " << q22 << " ");

      // The c matrix contains the positions of the 4 vertices
      double c[2][4] = {{xmin, xmax, xmax, xmin}, {ymin, ymin, ymax, ymax}};

      // For the following procedure, reference:
      // S. Schlegel et al., "On the Interpolation of Data with Normally
      // Distributed Uncertainty for Visualization", IEEE Transactions on
      // Visualization and Computer Graphics, vol. 18, no. 12, Dec. 2012.

      // Compute the phi coefficients
      double phi1 = 0;
      double phi2 = 0;
      double phi3 = 0;
      double phi4 = 0;

      for (int j = 0; j < 4; j++)
        {
          double distance = sqrt ((c[0][j] - x) * (c[0][j] - x) + (c[1][j] - y) * (c[1][j] - y));

          NS_LOG_DEBUG ("Distance: " << distance);

          double k = std::exp (-distance / m_correlationDistance);
          phi1 = phi1 + m_kInv[0][j] * k;
          phi2 = phi2 + m_kInv[1][j] * k;
          phi3 = phi3 + m_kInv[2][j] * k;
          phi4 = phi4 + m_kInv[3][j] * k;
        }

      NS_LOG_DEBUG ("Phi: " << phi1 << " " << phi2 << " " << phi3 << " " <<
                    phi4 << " ");

      double shadowing = q11 * phi1 + q21 * phi2 + q22 * phi3 + q12 * phi4;

      // Add the newly computed shadowing value to the shadowing map
      m_shadowingMap[position] = shadowing;
      NS_LOG_DEBUG ("Created new shadowing map: " << shadowing);
    }
  else
    {
      NS_LOG_DEBUG ("Shadowing map for this location already exists");
    }

  return m_shadowingMap[position];
}

/*****************************
 *  Position Implementation  *
 *****************************/

CorrelatedShadowingPropagationLossModel::Position::Position ()
{
}

CorrelatedShadowingPropagationLossModel::Position::Position (double x,
                                                             double y)
{
  this->x = x;
  this->y = y;
}

/*
 * Since Position holds two doubles, some tolerance must be taken into
 * account when comparing for equality.
 */
bool CorrelatedShadowingPropagationLossModel::Position::operator==
  (const CorrelatedShadowingPropagationLossModel::Position &other) const
{
  double EPSILON = 0.1;     // Arbitrary value for the tolerance, 10 cm.
  return ((fabs (this->x - other.x) < EPSILON)
          && (fabs (this->y - other.y) < EPSILON));
}

/*
 * In order to use Positions as keys in a map, we have to be able to order
 * them correctly.
 */
bool CorrelatedShadowingPropagationLossModel::Position::operator<
  (const CorrelatedShadowingPropagationLossModel::Position &other) const
{
  if (this->x != other.x)
    {
      return this->x < other.x;
    }
  return this->y < other.y;
}
}
}
