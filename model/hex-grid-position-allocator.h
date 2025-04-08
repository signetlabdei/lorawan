/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef HEX_GRID_POSITION_ALLOCATOR_H
#define HEX_GRID_POSITION_ALLOCATOR_H

#include "ns3/position-allocator.h"

#include <cmath>

namespace ns3
{

/**
 * @ingroup lorawan
 *
 * Position allocator for hexagonal tiling.
 *
 * Starting with a first hexagon in the axes' center, following tiles are added in outward
 * rings. The first position returned for a new ring is always the top one, followed by the others
 * in anti-clockwise rotation.
 *
 * Visual example with 10 tiles, indexed 0-9:
 *
 *                     _____
 *                    /     \
 *              _____/   8   \
 *             /     \   ˙   /
 *            /   9   \_____/
 *            \   ˙   /     \
 *       next  \_____/   1   \_____
 *         ˙   /     \   ˙   /     \
 *            /   2   \_____/   7   \
 *            \   ˙   /     \   ˙   /
 *             \_____/   0   \_____/
 *             /     \   ˙   /     \
 *            /   3   \_____/   5   \
 *            \   ˙   /     \   ˙   /
 *             \_____/   4   \_____/
 *                   \   ˙   /
 *                    \_____/
 *
 * The size of tiles can be configured by setting the radius \f$\rho_{i}\f$ of the circle
 * \b inscribed within hexagons (i.e., the internal circle).
 *
 * Let's say that we are placing access points, and we want to cover a square/circular area with
 * hexagonal tiles. This leaves us with two questions: (i) which value should we choose for the
 * internal radius? (ii) how many access point nodes (i.e., tiles) do we need to instantiate?
 *
 * For instance, to guarantee that no point is further than 1km from the center of any tile (i.e.,
 * to have no uncovered patches), we can choose the radius \f$\rho_{c}\f$ of the
 * \b circumscribed (external) circle of each hexagonal tile to be exactly 1km. Then, question
 * (i) can be solved by setting the internal radius to \f$\rho_{i}=\frac{\sqrt{3}}{2}\rho_{c}\f$
 * using the properties of equilateral triangles.
 *
 * To understand how many tiles need to be instantiated (that is, in this example, the number of
 * access point nodes), it is often useful to start from the complete area's diagonal or radius, and
 * to derive the number of complete rings \f$r\f$ required for coverage. Let's say we want to be
 * able to cover a distance of at least \f$d\f$ km from the center. One way to do this can be to
 * take the floor of \f$d\f$ divided by the distance between access points (the tiles' centers),
 * which happens to be \f$2\rho_{i}\f$, and adding one/two ring for good measure. More formally,
 * starting from a single central tile, we would need at least
 * \f$r=\left\lfloor\frac{d}{2\rho_{i}}\right\rfloor+1\f$ \b additional rings around it. Then,
 * the total number of tiles \f$n\f$ in a tiling of \f$r\f$ complete rings around a central tile
 * evaluates to \f$n=3r^{2}-3r+1\f$ providing a possible solution for question (ii).
 *
 * @todo Move this into the module .rst documentation
 */
class HexGridPositionAllocator : public PositionAllocator
{
  public:
    HexGridPositionAllocator();           //!< Default constructor
    ~HexGridPositionAllocator() override; //!< Destructor

    /**
     * Construct a new HexGridPositionAllocator object with given radius.
     *
     * @param radius The radius length of the circle inscribed in the hexagonal tiles.
     */
    HexGridPositionAllocator(double radius);

    Vector GetNext() const override;

    int64_t AssignStreams(int64_t stream) override;

    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Get the radius of the circle inscribed in the hexagonal tiles.
     *
     * @return The radius length.
     */
    double GetRadius() const;

    /**
     * Set the radius of the circle inscribed in the hexagonal tiles.
     *
     * @param radius The radius length.
     */
    void SetRadius(double radius);

  private:
    /**
     * This method adds to the given list of positions an outer ring of positions.
     *
     * @param positions The list of position around which to create the new positions.
     * @return The input list of position with an added outer ring.
     */
    std::vector<Vector> AddRing(std::vector<Vector> positions);

    std::vector<Vector> m_positions; //!< The current list of positions
    mutable std::vector<Vector>::const_iterator
        m_next;      //!< The iterator pointing to the next position to return
    double m_radius; //!< The radius of a cell (defined as the half the distance between two
                     //!< adjacent nodes, that is, the radius of the circle inscribed in each
                     //!< hexagonal tile)

    const static double pi; //!< Pi
};

} // namespace ns3

#endif /* PERIODIC_SENDER_HELPER_H */
