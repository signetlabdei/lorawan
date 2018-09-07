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

#ifndef HEX_GRID_POSITION_ALLOCATOR_H
#define HEX_GRID_POSITION_ALLOCATOR_H

#include "ns3/position-allocator.h"
#include <cmath>

namespace ns3 {

  class HexGridPositionAllocator : public PositionAllocator
  {
  public:
    HexGridPositionAllocator ();
    HexGridPositionAllocator (double radius);

    ~HexGridPositionAllocator ();

    virtual Vector GetNext (void) const;

    virtual int64_t AssignStreams (int64_t stream);

    static TypeId GetTypeId (void);

    double GetRadius (void);

    void SetRadius (double radius);

  private:
    /**
     * This method adds to the given list of positions an outer ring of positions
     * \param position the list of position around which to create the other positions
     */
    std::vector<Vector> AddRing (std::vector<Vector> positions);

    /**
     * The list of current positions
     */
    std::vector<Vector> m_positions;

    /**
     * The iterator pointing to the next position to return
     */
    mutable std::vector<Vector>::const_iterator m_next;

    /**
     * The radius of a cell (defined as the half the distance between two
     * adjacent nodes)
     */
    double m_radius;

    const static double pi; //!< Pi
  };

} // namespace ns3

#endif /* PERIODIC_SENDER_HELPER_H */
