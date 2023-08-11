/*
 * Copyright (c) 2017 University of Padova
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
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "periodic-sender-helper.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/periodic-sender.h"
#include "ns3/pointer.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("PeriodicSenderHelper");

PeriodicSenderHelper::PeriodicSenderHelper()
{
    m_factory.SetTypeId("ns3::PeriodicSender");

    m_period = Minutes(10.0);

    // m_factory.Set ("PacketSizeRandomVariable", StringValue
    //                  ("ns3::ParetoRandomVariable[Bound=10|Shape=2.5]"));

    m_initialDelay = CreateObject<UniformRandomVariable>();
    m_initialDelay->SetAttribute("Min", DoubleValue(0));

    m_intervalProb = CreateObject<UniformRandomVariable>();
    m_intervalProb->SetAttribute("Min", DoubleValue(0));
    m_intervalProb->SetAttribute("Max", DoubleValue(1));

    m_pktSize = 10;

    m_intervalGenerator = nullptr;
    m_sizeGenerator = nullptr;
}

PeriodicSenderHelper::~PeriodicSenderHelper()
{
    m_initialDelay = nullptr;
    m_intervalProb = nullptr;
    m_intervalGenerator = nullptr;
    m_sizeGenerator = nullptr;
}

void
PeriodicSenderHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
PeriodicSenderHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
PeriodicSenderHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
PeriodicSenderHelper::InstallPriv(Ptr<Node> node) const
{
    NS_LOG_FUNCTION(this << node);

    Ptr<PeriodicSender> app = m_factory.Create<PeriodicSender>();

    Time interval;
    if (m_period == Seconds(0))
    {
        double intervalProb = m_intervalProb->GetValue();
        NS_LOG_DEBUG("IntervalProb = " << intervalProb);

        // Based on TR 45.820
        if (intervalProb < 0.4)
        {
            interval = Days(1);
        }
        else if (0.4 <= intervalProb && intervalProb < 0.8)
        {
            interval = Hours(2);
        }
        else if (0.8 <= intervalProb && intervalProb < 0.95)
        {
            interval = Hours(1);
        }
        else
        {
            interval = Minutes(30);
        }
    }
    else
    {
        interval = m_period;
    }
    // Overwrite interval if random variable for iterval was provided
    if (m_intervalGenerator)
    {
        interval = Seconds(m_intervalGenerator->GetValue());
    }

    app->SetInterval(interval);
    NS_LOG_DEBUG("Created an application with interval = " << interval.GetSeconds() << " seconds");
    app->SetInitialDelay(Seconds(m_initialDelay->GetValue(0, interval.GetSeconds())));

    app->SetPacketSize(m_pktSize);
    // Different on each device
    if (m_sizeGenerator)
    {
        app->SetPacketSize(m_sizeGenerator->GetInteger());
    }

    app->SetNode(node);
    node->AddApplication(app);

    return app;
}

void
PeriodicSenderHelper::SetPeriod(Time period)
{
    m_period = period;
}

void
PeriodicSenderHelper::SetPacketSize(uint8_t size)
{
    m_pktSize = size;
}

void
PeriodicSenderHelper::SetPacketSizeGenerator(Ptr<RandomVariableStream> rv)
{
    m_sizeGenerator = rv;
}

void
PeriodicSenderHelper::SetPeriodGenerator(Ptr<RandomVariableStream> rv)
{
    m_intervalGenerator = rv;
}

} // namespace lorawan
} // namespace ns3
