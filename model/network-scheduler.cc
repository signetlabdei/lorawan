#include "network-scheduler.h"

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("NetworkScheduler");

  NS_OBJECT_ENSURE_REGISTERED (NetworkScheduler);

  TypeId
  NetworkScheduler::GetTypeId(void) {
    static TypeId tid = TypeId ("ns3::NetworkScheduler")
      .SetParent<Object> ()
      .AddConstructor<NetworkScheduler> ()
      .AddTraceSource("ReceiveWindowOpened",
                      "Trace source that is fired when a receive window opportunity happens.",
                      MakeTraceSourceAccessor (&NetworkScheduler::m_receiveWindowOpened),
                      "ns3::Packet::TracedCallback")
      .SetGroupName ("lorawan");
    return tid;
  }

  NetworkScheduler::NetworkScheduler() {}

  NetworkScheduler::NetworkScheduler(Ptr<NetworkStatus> status,
                                     Ptr<NetworkController> controller) :
    m_status (status),
    m_controller (controller)
  {
  }

  NetworkScheduler::~NetworkScheduler() {}

  void
  NetworkScheduler::OnReceivedPacket(Ptr<const Packet> packet)
  {
    NS_LOG_FUNCTION (packet);

    // Create a copy of the packet
    Ptr<Packet> myPacket = packet->Copy();

    // TODO Check if this packet is a duplicate:
    // It's possible that we already received the same packet from another
    // gateway.
    // - Extract the address
    LoraMacHeader macHeader;
    LoraFrameHeader frameHeader;
    myPacket->RemoveHeader(macHeader);
    myPacket->RemoveHeader(frameHeader);
    LoraDeviceAddress deviceAddress = frameHeader.GetAddress ();

    // Schedule OnReceiveWindowOpportunity event
    Simulator::Schedule (Seconds(1),
                         &NetworkScheduler::OnReceiveWindowOpportunity,
                         this,
                         deviceAddress,
                         1); // This will be the first receive window
  }

  void
  NetworkScheduler::OnReceiveWindowOpportunity(LoraDeviceAddress deviceAddress, int window)
  {
    NS_LOG_FUNCTION (deviceAddress);

    NS_LOG_DEBUG ("Opening receive window nubmer " << window << " for device "
                  << deviceAddress);

    // Check whether this device needs a response by querying m_status
    bool needsReply = m_status->NeedsReply (deviceAddress);

    if (needsReply)
    {
      NS_LOG_INFO ("A reply is needed");
    }

    // Check whether we can send a reply to the device, again by using
    // NetworkStatus
    Address gwAddress = m_status->GetBestGatewayForDevice (deviceAddress);

    NS_LOG_DEBUG ("Found available gateway with address: " << gwAddress);

    if (gwAddress == Address ())
      {
        // No suitable GW was found
        // TODO Schedule other receive window
      }
    else
      {
        m_controller->BeforeSendingReply (m_status->GetEndDeviceStatus
                                          (deviceAddress));

        // Send the reply through that gateway
        m_status->SendThroughGateway (m_status->GetReplyForDevice
                                      (deviceAddress, window),
                                      gwAddress);
      }
  }
}
