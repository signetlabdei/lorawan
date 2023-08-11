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

#include "urban-traffic-helper.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/periodic-sender.h"
#include "ns3/pointer.h"
#include "ns3/poisson-sender.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("UrbanTrafficHelper");

UrbanTrafficHelper::UrbanTrafficHelper()
{
    // Number of occurencies
    const std::vector<double> pdf = {20.947,
                                     2200.0,
                                     316.47,
                                     15.03,
                                     15.03,
                                     69.823,
                                     3845.0,
                                     384.5,
                                     3845.0,
                                     3845.0,
                                     26915.0,
                                     7690.0,
                                     11535.0};

    // Cumulative distribution
    double tot = 0.0;
    m_cdf = std::vector<double>(pdf.size());
    for (size_t i = 0; i < pdf.size(); ++i)
    {
        tot += pdf[i];
        m_cdf[i] = tot;
    }

    m_intervalProb = CreateObject<UniformRandomVariable>();
    m_intervalProb->SetAttribute("Min", DoubleValue(0));
    m_intervalProb->SetAttribute("Max", DoubleValue(m_cdf[12]));
}

UrbanTrafficHelper::~UrbanTrafficHelper()
{
    m_intervalProb = nullptr;
}

void
UrbanTrafficHelper::SetDeviceGroups(M2MDeviceGroups groups)
{
    /**
     * Note: with UniformRandomVariable, the low end of the range is
     * always included and the high end of the range is always excluded.
     */
    switch (groups)
    {
    case Commercial:
        m_intervalProb->SetAttribute("Min", DoubleValue(0));
        m_intervalProb->SetAttribute("Max", DoubleValue(m_cdf[5]));
        break;
    case InHouse:
        m_intervalProb->SetAttribute("Min", DoubleValue(m_cdf[5]));
        m_intervalProb->SetAttribute("Max", DoubleValue(m_cdf[12]));
        break;
    case All:
    default:
        m_intervalProb->SetAttribute("Min", DoubleValue(0));
        m_intervalProb->SetAttribute("Max", DoubleValue(m_cdf[12]));
        break;
    }
}

ApplicationContainer
UrbanTrafficHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
UrbanTrafficHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
UrbanTrafficHelper::InstallPriv(Ptr<Node> node) const
{
    NS_LOG_FUNCTION(this << node);

    double intervalProb = m_intervalProb->GetValue();

    Time interval = Minutes(10);
    uint8_t pktSize = 18;
    bool poisson = false;
    std::string type = "generic";

    Ptr<LoraApplication> app;

    /**
     * From [IEEE C802.16p-11/0102r2]
     *
     * ------------------------------------------------------------------------------------
     * |        Application       |   Density   | Interval | PacketSize |     Traffic     |
     * |  (Commercial, In-House)  | [nodes/km2] |   [s]    |    [B]     |                 |
     * ------------------------------------------------------------------------------------
     * | Credit machine (grocery) | 20.947      | 120      | 24         | Poisson         |
     * | Credit machine (shop)    | 2200.0      | 1800     | 24         | Poisson         |
     * | Roadway sign             | 316.47      | 30       | 1          | Uniform         |
     * | Traffic light            | 15.03       | 60       | 1          | Uniform         |
     * | Traffic sensor           | 15.03       | 60       | 1          | Poisson         |
     * | Movie rental machine     | 69.823      | 21600    | 38         | Poisson         |
     * ------------------------------------------------------------------------------------
     * | Home security system     | 3845.0      | 600      | 20         | Poisson/uniform |
     * | Elderly sensor device    | 384.5       | 20       | 43         | Poisson/uniform |
     * | Refrigerator             | 3845.0      | 3600     | 30         | Poisson/uniform |
     * | Freezer                  | 3845.0      | 86400    | 30         | Poisson/uniform |
     * | Other house appliance    | 26915.0     | 86400    | 8          | Poisson/uniform |
     * | PHEV charging station    | 7690.0      | 1400     | 32         | Poisson/uniform |
     * | Smart meter              | 11535.0     | 150      | 34         | Poisson/uniform |
     * ------------------------------------------------------------------------------------
     *
     * Total density: 56851.8 nodes/km2
     *
     * Discussion:
     * Credit machine (grocery) - reliability critical, duty-cycle limited on SF12
     * Credit machine (shop)    - reliability critical
     * Roadway sign             - duty-cycle limited on SF11 & SF12
     * Traffic light            - duty-cycle limited on SF12
     * Traffic sensor           - duty-cycle limited on SF12
     * Movie rental machine     - do they actually still exist?
     * Home security system     - reliability critical
     * Elderly sensor device    - reliability critical, duty-cycle limited from SF9
     *
     * We could implement packet fragmentation...
     */

    if (intervalProb < m_cdf[0]) // Credit machine (grocery)
    {
        interval = Minutes(2);
        pktSize = 24;
        poisson = true;
        type = "Credit machine (grocery)";
    }
    else if (intervalProb < m_cdf[1]) // Credit machine (shop)
    {
        interval = Minutes(30);
        pktSize = 24;
        poisson = true;
        type = "Credit machine (shop)";
    }
    else if (intervalProb < m_cdf[2]) // Roadway sign
    {
        interval = Seconds(30);
        pktSize = 1;
        poisson = false;
        type = "Roadway sign";
    }
    else if (intervalProb < m_cdf[3]) // Traffic light
    {
        interval = Minutes(1);
        pktSize = 1;
        poisson = false;
        type = "Traffic light";
    }
    else if (intervalProb < m_cdf[4]) // Traffic sensor
    {
        interval = Minutes(1);
        pktSize = 1;
        poisson = true;
        type = "Traffic sensor";
    }
    else if (intervalProb < m_cdf[5]) // Movie rental machine
    {
        interval = Hours(6);
        pktSize = 38;
        poisson = true;
        type = "Movie rental machine";
    }
    else if (intervalProb < m_cdf[6]) // Home security system
    {
        interval = Minutes(10);
        pktSize = 20;
        poisson = (bool)m_intervalProb->GetInteger(0, 1);
        type = "Home security system";
    }
    else if (intervalProb < m_cdf[7]) // Elderly sensor device
    {
        interval = Seconds(20);
        pktSize = 43;
        poisson = (bool)m_intervalProb->GetInteger(0, 1);
        type = "Elderly sensor device";
    }
    else if (intervalProb < m_cdf[8]) // Refrigerator
    {
        interval = Hours(1);
        pktSize = 30;
        poisson = (bool)m_intervalProb->GetInteger(0, 1);
        type = "Refrigerator";
    }
    else if (intervalProb < m_cdf[9]) // Freezer
    {
        interval = Days(1);
        pktSize = 30;
        poisson = (bool)m_intervalProb->GetInteger(0, 1);
        type = "Freezer";
    }
    else if (intervalProb < m_cdf[10]) // Other house appliance
    {
        interval = Days(1);
        pktSize = 8;
        poisson = (bool)m_intervalProb->GetInteger(0, 1);
        type = "Other house appliance";
    }
    else if (intervalProb < m_cdf[11]) // PHEV charging station
    {
        interval = Seconds(1400);
        pktSize = 32;
        poisson = (bool)m_intervalProb->GetInteger(0, 1);
        type = "PHEV charging station";
    }
    else // m_cdf[12], Smart meter
    {
        interval = Seconds(150);
        pktSize = 34;
        poisson = (bool)m_intervalProb->GetInteger(0, 1);
        type = "Smart meter";
    }

    if (poisson)
    {
        app = CreateObjectWithAttributes<PoissonSender>("Interval",
                                                        TimeValue(interval),
                                                        "PacketSize",
                                                        UintegerValue(pktSize));
    }
    else
    {
        app = CreateObjectWithAttributes<PeriodicSender>("Interval",
                                                         TimeValue(interval),
                                                         "PacketSize",
                                                         UintegerValue(pktSize));
    }

    NS_LOG_DEBUG("Created: " << type << " (" << interval.GetSeconds() << "s, " << (unsigned)pktSize
                             << "B, " << ((poisson) ? "poisson)" : "uniform)"));
    app->SetInitialDelay(Seconds(m_intervalProb->GetValue(0, interval.GetSeconds())));

    app->SetNode(node);
    node->AddApplication(app);

    return app;
}

} // namespace lorawan
} // namespace ns3
