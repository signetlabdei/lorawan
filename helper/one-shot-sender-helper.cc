/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "one-shot-sender-helper.h"

#include "ns3/double.h"
#include "ns3/log.h"
#include "ns3/one-shot-sender.h"
#include "ns3/simulator.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("OneShotSenderHelper");

OneShotSenderHelper::OneShotSenderHelper()
{
    m_factory.SetTypeId("ns3::OneShotSender");
}

OneShotSenderHelper::~OneShotSenderHelper()
{
}

void
OneShotSenderHelper::SetSendTime(Time sendTime)
{
    m_sendTime = sendTime;
}

void
OneShotSenderHelper::SetAttribute(std::string name, const AttributeValue& value)
{
    m_factory.Set(name, value);
}

ApplicationContainer
OneShotSenderHelper::Install(Ptr<Node> node) const
{
    return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer
OneShotSenderHelper::Install(NodeContainer c) const
{
    ApplicationContainer apps;
    for (auto i = c.Begin(); i != c.End(); ++i)
    {
        apps.Add(InstallPriv(*i));
    }

    return apps;
}

Ptr<Application>
OneShotSenderHelper::InstallPriv(Ptr<Node> node) const
{
    NS_LOG_FUNCTION(this << node->GetId());

    Ptr<OneShotSender> app = m_factory.Create<OneShotSender>();

    app->SetSendTime(m_sendTime);

    app->SetNode(node);
    node->AddApplication(app);

    return app;
}
} // namespace lorawan
} // namespace ns3
