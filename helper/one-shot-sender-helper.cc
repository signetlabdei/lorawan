/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 */

#include "ns3/one-shot-sender-helper.h"
#include "ns3/one-shot-sender.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("OneShotSenderHelper");

OneShotSenderHelper::OneShotSenderHelper ()
{
  m_factory.SetTypeId ("ns3::OneShotSender");
}

OneShotSenderHelper::~OneShotSenderHelper ()
{
}

void
OneShotSenderHelper::SetSendTime (Time sendTime)
{
  m_sendTime = sendTime;
}

void
OneShotSenderHelper::SetAttribute (std::string name,
                                   const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
OneShotSenderHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
OneShotSenderHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
OneShotSenderHelper::InstallPriv (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node->GetId ());

  Ptr<OneShotSender> app = m_factory.Create<OneShotSender> ();

  app->SetSendTime (m_sendTime);

  app->SetNode (node);
  node->AddApplication (app);

  return app;
}
}
} // namespace ns3
