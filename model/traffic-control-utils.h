/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef TRAFFIC_CONTROL_UTILS_H
#define TRAFFIC_CONTROL_UTILS_H

#include <unordered_map>

namespace ns3
{

class TrafficControlUtils
{
    using devices_t = std::vector<std::pair<uint32_t, double>>;
    using output_t = std::unordered_map<uint32_t, uint8_t>;

    // Optimization input data
    struct datamodel_t
    {
        // Capacity value to be assigned
        double limit;
        // Device offered traffic
        std::vector<double> deltas;
        // Number of devices
        int bound;
    };

  public:
    static double OptimizeDutyCycleMaxMin(const devices_t& devs,
                                          const double limit,
                                          output_t& output);

    static void OptimizeDutyCycleMax(const devices_t& devs, const double limit, output_t& output);

  private:
    TrafficControlUtils()
    {
    }

    virtual ~TrafficControlUtils()
    {
    }

    // Hard coded parameters
    static const std::vector<double> m_dutycycles;
};

} // namespace ns3

#endif /* TRAFFIC_CONTROL_UTILS_H */
