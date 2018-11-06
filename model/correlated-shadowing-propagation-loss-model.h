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

#ifndef CORRELATED_SHADOWING_PROPAGATION_LOSS_MODEL_H
#define CORRELATED_SHADOWING_PROPAGATION_LOSS_MODEL_H

#include "ns3/propagation-loss-model.h"
#include "ns3/mobility-model.h"
#include "ns3/vector.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {
class MobilityModel;
namespace lorawan {

class CorrelatedShadowingPropagationLossModel : public PropagationLossModel
{

public:
  class Position
  {
public:
    Position ();
    Position (double x, double y);
    double x;
    double y;

    bool operator== (const Position &other) const;
    bool operator< (const Position &other) const;
  };

  class ShadowingMap : public
                       SimpleRefCount<CorrelatedShadowingPropagationLossModel::ShadowingMap>
  {
public:
    /**
     * Constructor.
     * This initializes the shadowing map with a grid of independent
     * shadowing values, one m_correlationDistance meters apart from the next
     * one. The result is something like:
     *  o---o---o---o---o
     *  |   |   |   |   |
     *  o---o---o---o---o
     *  |   |   |   |   |
     *  o---o---o---o---o
     *  |   |   |   |   |
     *  o---o---o---o---o
     *  where at each o we have an independently generated shadowing value.
     *  We can then interpolate the 4 values surrounding any point in space
     *  in order to get a correlated shadowing value. After generating this
     *  value, we will add it to the map so that we don't have to compute it
     *  twice. Also, since interpolation is a deterministic operation, we are
     *  guaranteed that, as long as the grid doesn't change, also two values
     *  generated in the same square will be correlated.
     */
    ShadowingMap ();

    ~ShadowingMap ();

    /**
     * Get the loss for a certain position.
     * If this position is not already in the map, add it by computing the
     * interpolation of neighboring shadowing values belonging to the grid.
     */
    double GetLoss (CorrelatedShadowingPropagationLossModel::Position position);

private:
    /**
     * For each Position, this map gives a corresponding loss.
     * The map contains a basic grid that is initialized at construction
     * time, and then newly computed values are added as they are created.
     */
    std::map<CorrelatedShadowingPropagationLossModel::Position, double>
    m_shadowingMap;

    /**
     * The distance after which two samples are to be considered almost
     * uncorrelated
     */
    double m_correlationDistance;

    /**
     * The normal random variable that is used to obtain shadowing values.
     */
    Ptr<NormalRandomVariable> m_shadowingValue;

    /**
     * The inverted K matrix.
     * This matrix is used to compute the coefficients to be used when
     * interpolating the vertices of a grid square.
     */
    static const double m_kInv[4][4];
  };

  static TypeId GetTypeId (void);

  /**
   * Constructor.
   */
  CorrelatedShadowingPropagationLossModel ();

  /**
   * Set the correlation distance for newly created ShadowingMap instances
   */
  void SetCorrelationDistance (double distance);

  /**
   * Get the correlation distance that is currently being used.
   */
  double GetCorrelationDistance (void);

private:
  virtual double DoCalcRxPower (double txPowerDbm,
                                Ptr<MobilityModel> a,
                                Ptr<MobilityModel> b) const;

  virtual int64_t DoAssignStreams (int64_t stream);

  double m_correlationDistance;     //!< The correlation distance for the ShadowingMap

  /**
   * Map linking a square to a ShadowingMap.
   * Each square of the shadowing grid has a corresponding ShadowingMap, and a
   * square is identified by a pair of coordinates. Coordinates are computed as
   * such:
   *
   *  o---------o---------o---------o---------o---------o
   *  |         |         |    '    |         |         |
   *  |  (-2,2) |  (-1,2) |  (0,2)  |  (1,2)  |  (2,2)  |
   *  |         |         |    '    |         |         |
   *  o---------o---------o----+----o---------o---------o
   *  |         |         |    '    |         |         |
   *  |  (-2,1) |  (-1,1) |  (0,1)  |  (1,1)  |  (2,1)  |
   *  |         |         |    '    |         |         |
   *  o---------o---------o----+----o---------o---------o
   *  |         |         |    '    |         |         |
   *  |--(-2,0)-+--(-1,0)-+--(0,0)--+--(1,0)--+--(2,0)--|
   *  |         |         |    '    |         |         |
   *  o---------o---------o----+----o---------o---------o
   *  |         |         |    '    |         |         |
   *  | (-2,-1) | (-1,-1) | (0,-1)  | (1,-1)  | (2,-1)  |
   *  |         |         |    '    |         |         |
   *  o---------o---------o----+----o---------o---------o
   *  |         |         |    '    |         |         |
   *  | (-2,-2) | (-1,-2) | (0,-2)  | (1,-2)  | (2,-2)  |
   *  |         |         |    '    |         |         |
   *  o---------o---------o---------o---------o---------o
   *
   *  For each one of these coordinates, a ShadowingMap is computed. That is,
   *  each one of the points belonging to the same square sees the same
   *  shadowing for the points around it. This is one level of correlation for
   *  the shadowing, i.e. close nodes transmitting to the same point will see
   *  the same shadowing since they are using the same shadowing map.
   *  Further, the ShadowingMap will be "smooth": when transmitting from point
   *  a to points b and c, the shadowing experienced by b and c will be similar
   *  if they are close (ideally, within a correlation distance).
   */
  mutable std::map<std::pair<int, int>, Ptr<ShadowingMap> > m_shadowingGrid;
};

}

}
#endif
