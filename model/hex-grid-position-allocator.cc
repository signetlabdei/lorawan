/*
 * Copyright (c) 2022 Orange SA
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
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 */

#include "hex-grid-position-allocator.h"

#include "ns3/double.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("HexGridPositionAllocator");

NS_OBJECT_ENSURE_REGISTERED(HexGridPositionAllocator);

TypeId
HexGridPositionAllocator::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::HexGridPositionAllocator")
            .SetParent<PositionAllocator>()
            .AddConstructor<HexGridPositionAllocator>()
            .SetGroupName("Lora")
            .AddAttribute("distance",
                          "The distance between two nodes",
                          DoubleValue(6000),
                          MakeDoubleAccessor(&HexGridPositionAllocator::SetDistance),
                          MakeDoubleChecker<double>())
            .AddAttribute("Z",
                          "Vertical position of nodes",
                          DoubleValue(0.0),
                          MakeDoubleAccessor(&HexGridPositionAllocator::SetZ),
                          MakeDoubleChecker<double>());
    return tid;
}

HexGridPositionAllocator::HexGridPositionAllocator()
{
    NS_LOG_FUNCTION_NOARGS();

    ResetCoordinates();
}

HexGridPositionAllocator::~HexGridPositionAllocator()
{
    NS_LOG_FUNCTION_NOARGS();
}

Vector
HexGridPositionAllocator::GetNext() const
{
    Vector position = ObtainCurrentPosition();
    // Shift coordinates to point next position
    // Standard case (Most common when going infinite)
    if (m_hex < m_ring - 1)
    {
        m_hex++;
        return position;
    }
    // Sector finished (but not ring!)
    if (m_sector < 5)
    {
        m_sector++;
        m_hex = 0;
        return position;
    }
    // Ring finished
    m_ring++;
    m_sector = 0;
    m_hex = 0;
    return position;
}

int64_t
HexGridPositionAllocator::AssignStreams(int64_t stream)
{
    return 0;
}

void
HexGridPositionAllocator::SetDistance(double distance)
{
    NS_ASSERT(distance > 0);
    m_d = distance;
    ResetCoordinates();
}

void
HexGridPositionAllocator::SetZ(double z)
{
    NS_ASSERT(z >= 0);
    m_z = z;
    ResetCoordinates();
}

Vector
HexGridPositionAllocator::ObtainCurrentPosition() const
{
    NS_LOG_DEBUG("Coordinates: ring=" << m_ring << ", sector=" << m_sector << ", hex=" << m_hex);

    // First vector
    double v1_x = -std::sin(p3 * m_sector) * m_d * m_ring;
    double v1_y = std::cos(p3 * m_sector) * m_d * m_ring;

    // Second vector
    double v2_x = -std::sin(p3 * (m_sector + 2)) * m_d * m_hex;
    double v2_y = std::cos(p3 * (m_sector + 2)) * m_d * m_hex;

    // Sum vectors
    double x = v1_x + v2_x;
    double y = v1_y + v2_y;

    Vector position(x, y, m_z);

    NS_LOG_DEBUG("New position: " << position);

    return position;
}

void
HexGridPositionAllocator::ResetCoordinates()
{
    // Special initialization to manage the first assignment
    m_ring = 0;
    m_sector = 5;
    m_hex = 0;
}

} // namespace ns3
