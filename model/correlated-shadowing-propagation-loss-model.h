/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef CORRELATED_SHADOWING_PROPAGATION_LOSS_MODEL_H
#define CORRELATED_SHADOWING_PROPAGATION_LOSS_MODEL_H

#include "ns3/mobility-model.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/random-variable-stream.h"
#include "ns3/vector.h"

namespace ns3
{
class MobilityModel;

namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Propagation loss model for spatially correlated shadowing in a city
 */
class CorrelatedShadowingPropagationLossModel : public PropagationLossModel
{
  public:
    /**
     * Stores x,y values and overrides critical operators.
     */
    class Position
    {
      public:
        Position(); //!< Default constructor

        /**
         * Construct a new Position object with values.
         *
         * @param x The x coordinate.
         * @param y The y coordinate.
         */
        Position(double x, double y);

        double x; //!< Stores the x coordinate.
        double y; //!< Stores the y coordinate.

        /**
         * Equality comparison operator
         *
         * @param other Second Position to compare this instance to.
         * @return True if the positions are equal.
         */
        bool operator==(const Position& other) const;
        /**
         * Less-then comparison operator
         *
         * @param other Second Position to compare this instance to.
         * @return True if either the x or y coordinate of first Position is less than the
         * respective one of the second Position
         */
        bool operator<(const Position& other) const;
    };

    /**
     * @ingroup lorawan
     *
     * This initializes the shadowing map with a grid of independent
     * shadowing values, one m_correlationDistance meters apart from the next
     * one. The result is something like:
     *
     *       o---o---o---o---o
     *       |   |   |   |   |
     *       o---o---o---o---o
     *       |   |   |   |   |
     *       o---o---o---o---o
     *       |   |   |   |   |
     *       o---o---o---o---o
     *
     * where at each o we have an independently generated shadowing value.
     * We can then interpolate the 4 values surrounding any point in space
     * in order to get a correlated shadowing value. After generating this
     * value, we will add it to the map so that we don't have to compute it
     * twice. Also, since interpolation is a deterministic operation, we are
     * guaranteed that, as long as the grid doesn't change, also two values
     * generated in the same square will be correlated.
     */
    class ShadowingMap
        : public SimpleRefCount<CorrelatedShadowingPropagationLossModel::ShadowingMap>
    {
      public:
        ShadowingMap();  //!< Default constructor
        ~ShadowingMap(); //!< Destructor

        /**
         * Get the loss for a certain position.
         *
         * If the position is not already in the map, add it by computing the
         * interpolation of neighboring shadowing values belonging to the grid.
         *
         * @param position The Position instance.
         * @return The loss as a double.
         */
        double GetLoss(CorrelatedShadowingPropagationLossModel::Position position);

      private:
        /**
         * For each Position, this map gives a corresponding loss.
         * The map contains a basic grid that is initialized at construction
         * time, and then newly computed values are added as they are created.
         */
        std::map<CorrelatedShadowingPropagationLossModel::Position, double> m_shadowingMap;

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

    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    CorrelatedShadowingPropagationLossModel(); //!< Default constructor

  private:
    double DoCalcRxPower(double txPowerDbm,
                         Ptr<MobilityModel> a,
                         Ptr<MobilityModel> b) const override;

    int64_t DoAssignStreams(int64_t stream) override;

    double m_correlationDistance; //!< The correlation distance for the ShadowingMap

    /**
     * Map linking a square to a ShadowingMap.
     * Each square of the shadowing grid has a corresponding ShadowingMap, and a
     * square is identified by a pair of coordinates. Coordinates are computed as
     * such:
     *
     *        o---------o---------o---------o---------o---------o
     *        |         |         |    '    |         |         |
     *        |  (-2,2) |  (-1,2) |  (0,2)  |  (1,2)  |  (2,2)  |
     *        |         |         |    '    |         |         |
     *        o---------o---------o----+----o---------o---------o
     *        |         |         |    '    |         |         |
     *        |  (-2,1) |  (-1,1) |  (0,1)  |  (1,1)  |  (2,1)  |
     *        |         |         |    '    |         |         |
     *        o---------o---------o----+----o---------o---------o
     *        |         |         |    '    |         |         |
     *        |--(-2,0)-+--(-1,0)-+--(0,0)--+--(1,0)--+--(2,0)--|
     *        |         |         |    '    |         |         |
     *        o---------o---------o----+----o---------o---------o
     *        |         |         |    '    |         |         |
     *        | (-2,-1) | (-1,-1) | (0,-1)  | (1,-1)  | (2,-1)  |
     *        |         |         |    '    |         |         |
     *        o---------o---------o----+----o---------o---------o
     *        |         |         |    '    |         |         |
     *        | (-2,-2) | (-1,-2) | (0,-2)  | (1,-2)  | (2,-2)  |
     *        |         |         |    '    |         |         |
     *        o---------o---------o---------o---------o---------o
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
    mutable std::map<std::pair<int, int>, Ptr<ShadowingMap>> m_shadowingGrid;
};

} // namespace lorawan

} // namespace ns3
#endif
