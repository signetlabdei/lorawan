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

#ifndef URBAN_TRAFFIC_HELPER_H
#define URBAN_TRAFFIC_HELPER_H

#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/random-variable-stream.h"
#include "ns3/string.h"

namespace ns3
{
namespace lorawan
{

/**
 * This class can be used to install a range of realistic sender applications
 * on a wide range of nodes. Traffic types and their distribution are from
 * [IEEE C802.16p-11/0102r2] for the urban scenario
 */

enum M2MDeviceGroups
{
    All,
    InHouse,
    Commercial
};

class UrbanTrafficHelper
{
  public:
    UrbanTrafficHelper();

    ~UrbanTrafficHelper();

    ApplicationContainer Install(NodeContainer c) const;

    ApplicationContainer Install(Ptr<Node> node) const;

    void SetDeviceGroups(M2MDeviceGroups groups);

  private:
    Ptr<Application> InstallPriv(Ptr<Node> node) const;

    Ptr<UniformRandomVariable> m_intervalProb;

    std::vector<double> m_cdf;
};

} // namespace lorawan

} // namespace ns3
#endif /* URBAN_TRAFFIC_HELPER_H */
