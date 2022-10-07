/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "udp-forwarder-helper.h"
#include "ns3/udp-forwarder.h"
#include "ns3/lora-net-device.h"
#include "ns3/csma-net-device.h"
#include "ns3/random-variable-stream.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/simulator.h"
#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("UdpForwarderHelper");

UdpForwarderHelper::UdpForwarderHelper ()
{
  m_factory.SetTypeId ("ns3::UdpForwarder");
}

UdpForwarderHelper::~UdpForwarderHelper ()
{
}

void
UdpForwarderHelper::SetAttribute (std::string name, const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
UdpForwarderHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpForwarderHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
UdpForwarderHelper::InstallPriv (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);

  Ptr<UdpForwarder> app = m_factory.Create<UdpForwarder> ();

  app->SetNode (node);
  node->AddApplication (app);

  // Link the Forwarder to the NetDevices
  for (uint32_t i = 0; i < node->GetNDevices (); i++)
    {
      Ptr<NetDevice> currentNetDevice = node->GetDevice (i);
      if (currentNetDevice->GetObject<LoraNetDevice> () != 0)
        {
          Ptr<LoraNetDevice> loraNetDevice = currentNetDevice->GetObject<LoraNetDevice> ();
          loraNetDevice->SetReceiveCallback (MakeCallback (&UdpForwarder::ReceiveFromLora, app));
        }
      else if (currentNetDevice->GetObject<CsmaNetDevice> () != 0)
        {
          Ptr<CsmaNetDevice> csmaNetDevice = currentNetDevice->GetObject<CsmaNetDevice> ();
          // Questa linea rompe tutto (con tutto si intende, la connessione csma)
          //csmaNetDevice->SetReceiveCallback (MakeCallback (&UdpForwarder::ReceiveFromCsma, app));
        }
      else
        {
          NS_LOG_ERROR ("Potential error: NetDevice is neither Lora nor Csma");
        }
    }

  return app;
}

} // namespace lorawan
} // namespace ns3
