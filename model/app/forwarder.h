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
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef FORWARDER_H
#define FORWARDER_H

#include "ns3/application.h"
#include "ns3/attribute.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/nstime.h"
#include "ns3/point-to-point-net-device.h"

namespace ns3
{
namespace lorawan
{

/**
 * This application forwards packets between :
 * GatewayLorawanMac <-> PointToPointNetDevice
 */
class Forwarder : public Application
{
  public:
    Forwarder();
    ~Forwarder() override;

    static TypeId GetTypeId();

    /**
     * Sets the mac to use to communicate with the EDs.
     *
     * \param mac The GatewayLorawanMac on this node.
     */
    void SetGatewayLorawanMac(Ptr<GatewayLorawanMac> mac);

    /**
     * Sets the P2P device to use to communicate with the NS.
     *
     * \param pointToPointNetDevice The P2PNetDevice on this node.
     */
    void SetPointToPointNetDevice(Ptr<PointToPointNetDevice> pointToPointNetDevice);

    /**
     * Receive a packet from the Lorawan Mac layer.
     *
     * \param packet The packet we received.
     */
    bool ReceiveFromLora(Ptr<LorawanMac> mac, Ptr<const Packet> packet);

    /**
     * Receive a packet from the PointToPointNetDevice
     */
    bool ReceiveFromPointToPoint(Ptr<NetDevice> pointToPointNetDevice,
                                 Ptr<const Packet> packet,
                                 uint16_t protocol,
                                 const Address& sender);

  protected:
    void DoDispose() override;

  private:
    /**
     * Start the application
     */
    void StartApplication() override;

    /**
     * Stop the application
     */
    void StopApplication() override;

    Ptr<LorawanMac> m_mac;                              //!< Pointer to the node's lorawan mac
    Ptr<PointToPointNetDevice> m_pointToPointNetDevice; //!< Pointer to the P2PNetDevice to the NS
};

} // namespace lorawan

} // namespace ns3
#endif /* FORWARDER */
