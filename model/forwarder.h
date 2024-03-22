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

#ifndef FORWARDER_H
#define FORWARDER_H

#include "lora-net-device.h"

#include "ns3/application.h"
#include "ns3/attribute.h"
#include "ns3/nstime.h"
#include "ns3/point-to-point-net-device.h"

namespace ns3
{
namespace lorawan
{

/**
 * \ingroup lorawan
 *
 * This application forwards packets between NetDevices:
 * LoraNetDevice -> PointToPointNetDevice and vice versa.
 */
class Forwarder : public Application
{
  public:
    Forwarder();           //!< Default constructor
    ~Forwarder() override; //!< Destructor

    /**
     *  Register this type.
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Sets the device to use to communicate with the end devices.
     *
     * \param loraNetDevice The LoraNetDevice on this node.
     */
    void SetLoraNetDevice(Ptr<LoraNetDevice> loraNetDevice);

    /**
     * Sets the P2P device to use to communicate with the network server.
     *
     * \param pointToPointNetDevice The P2PNetDevice on this node.
     */
    void SetPointToPointNetDevice(Ptr<PointToPointNetDevice> pointToPointNetDevice);

    /**
     * Receive a packet from the LoraNetDevice.
     *
     * \param loraNetDevice The LoraNetDevice we received the packet from.
     * \param packet The packet we received.
     * \param protocol The protocol number associated to this packet.
     * \param sender The address of the sender.
     * \return True if we can handle the packet, false otherwise.
     */
    bool ReceiveFromLora(Ptr<NetDevice> loraNetDevice,
                         Ptr<const Packet> packet,
                         uint16_t protocol,
                         const Address& sender);

    /**
     * Receive a packet from the PointToPointNetDevice.
     *
     * \copydoc ns3::NetDevice::ReceiveCallback
     */
    bool ReceiveFromPointToPoint(Ptr<NetDevice> device,
                                 Ptr<const Packet> packet,
                                 uint16_t protocol,
                                 const Address& sender);

    /**
     * Start the application.
     */
    void StartApplication() override;

    /**
     * Stop the application.
     */
    void StopApplication() override;

  private:
    Ptr<LoraNetDevice> m_loraNetDevice; //!< Pointer to the node's LoraNetDevice

    Ptr<PointToPointNetDevice> m_pointToPointNetDevice; //!< Pointer to the P2PNetDevice we use to
                                                        //!< communicate with the network server
};

} // namespace lorawan

} // namespace ns3
#endif /* FORWARDER */
