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

#include "ns3/log.h"
#include "ns3/hex-grid-position-allocator.h"
#include "ns3/double.h"

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("HexGridPositionAllocator");

  NS_OBJECT_ENSURE_REGISTERED (HexGridPositionAllocator);

  TypeId
  HexGridPositionAllocator::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::HexGridPositionAllocator")
      .SetParent<PositionAllocator> ()
      .AddConstructor<HexGridPositionAllocator> ()
      .SetGroupName ("Lora")
      .AddAttribute ("Radius", "The radius of a single hexagon",
                     DoubleValue (6000),
                     MakeDoubleAccessor (&HexGridPositionAllocator::m_radius),
                     MakeDoubleChecker<double> ());

    return tid;
  }

  HexGridPositionAllocator::HexGridPositionAllocator () :
    m_radius (6000)
  {
    NS_LOG_FUNCTION_NOARGS ();

    // Create the first position
    m_positions.push_back (Vector (0.0,0.0,0.0));

    // Add rings
    for (int i = 0; i < 20; i++) {
      m_positions = AddRing(m_positions);
    }

    // Set the iterator
    m_next = m_positions.begin();
  }

  HexGridPositionAllocator::HexGridPositionAllocator (double radius) :
    m_radius (radius)
  {
    NS_LOG_FUNCTION_NOARGS ();

    // Create the first position
    m_positions.push_back (Vector (0.0,0.0,0.0));

    // Add a couple rings
    // Add rings
    for (int i = 0; i < 20; i++) {
      m_positions = AddRing(m_positions);
    }

    // Set the iterator
    m_next = m_positions.begin();
  }

  HexGridPositionAllocator::~HexGridPositionAllocator ()
  {
    NS_LOG_FUNCTION_NOARGS ();
  }

  const double HexGridPositionAllocator::pi = std::acos(-1);

  double
  HexGridPositionAllocator::GetRadius (void)
  {
    return m_radius;
  }

  void
  HexGridPositionAllocator::SetRadius (double radius)
  {
    m_radius = radius;
  }

  Vector
  HexGridPositionAllocator::GetNext (void) const
  {
    // TODO: Check that there is a next element
    Vector position = *m_next;
    m_next++;
    return position;
  }

  int64_t
  HexGridPositionAllocator::AssignStreams (int64_t stream)
  {
    return 0;
  }

  std::vector<Vector>
  HexGridPositionAllocator::AddRing (std::vector<Vector> positions)
  {
    NS_LOG_FUNCTION (this);

    // Make a copy of the vector
    std::vector<Vector> copy = positions;

    // Iterate on the given vector
    for (std::vector<Vector>::iterator it = positions.begin ();
         it != positions.end (); it++)
      {
        // Get the current position
        Vector currentPosition = *it;
        NS_LOG_DEBUG ("Current position " << currentPosition);

        // Iterate to create the 6 surrounding positions
        // The angle is with respect to a vertical line
        Vector newPosition;
        for (double angle = 0; angle < 2*pi; angle+=pi/3)
          {
            newPosition = Vector (currentPosition.x+2*m_radius*std::sin(angle),
                                  currentPosition.y+2*m_radius*std::cos(angle),
                                  currentPosition.z);
            NS_LOG_DEBUG ("New position: " << newPosition);

            // If the newly created position is not already in the copy, add it
            bool found = false;
            for (std::vector<Vector>::iterator it = copy.begin ();
                 it != copy.end (); it++)
              {
                // If the vector is already in the vector
                // 1 is an EPSILON used to determine whether two floats are equal
                if (CalculateDistance(newPosition, *it) < 10)
                  {
                    found = true;
                    break;
                  }
              }
            if (found == false)
              {
                NS_LOG_DEBUG ("Adding position " << newPosition);
                copy.push_back (newPosition);
              }
          }
      }
    return copy;
  }
} // namespace ns3
