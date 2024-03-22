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

#ifndef GATEWAY_STATUS_H
#define GATEWAY_STATUS_H

#include "gateway-lorawan-mac.h"

#include "ns3/address.h"
#include "ns3/net-device.h"
#include "ns3/object.h"

namespace ns3
{
namespace lorawan
{

/**
 * \ingroup lorawan
 *
 * This class represents the Network Server's knowledge about a Gateway in
 * the LoRaWAN network it is administering.
 *
 * The Network Server's NetworkStatus component contains a list of instances of
 * this class, one for gateway in the network. Each instance contains all
 * the parameters and information of the gateway. This class is used by the Network Server for
 * downlink scheduling and sending purposes. That is, to check the Gateway's availability for radio
 * transmission, and then to retrieve the correct Net Device to send the packet through.
 */
class GatewayStatus : public Object
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    GatewayStatus();
    /**
     * \brief Construct a new GatewayStatus object with values
     *
     * \param address The Address of the P2PNetDevice of the gateway connected to the NS
     * \param netDevice A Ptr to the NetDevice through which to reach this gateway from the server
     * \param gwMac A Ptr to the MAC layer of the gateway
     */
    GatewayStatus(Address address, Ptr<NetDevice> netDevice, Ptr<GatewayLorawanMac> gwMac);
    ~GatewayStatus() override;

    /**
     * Get this gateway's P2P link address.
     *
     * \return The Address instance
     */
    Address GetAddress();

    /**
     * Set this gateway's P2P link address.
     *
     * \param address The Address instance
     */
    void SetAddress(Address address);

    /**
     * Get the NetDevice through which it's possible to contact this gateway from the server.
     *
     * \return A Ptr to the NetDevice
     */
    Ptr<NetDevice> GetNetDevice();

    /**
     * Set the NetDevice through which it's possible to contact this gateway from the server.
     *
     * \param netDevice A Ptr to the NetDevice
     */
    void SetNetDevice(Ptr<NetDevice> netDevice);

    /**
     * Get a pointer to this gateway's MAC instance.
     *
     * \return A Ptr to the MAC layer object
     */
    Ptr<GatewayLorawanMac> GetGatewayMac();

    ///**
    // * Set a pointer to this gateway's MAC instance.
    // */
    // void SetGatewayMac (Ptr<GatewayLorawanMac> gwMac);

    /**
     * Query whether or not this gateway is available for immediate transmission
     * on this frequency.
     *
     * \param frequency The frequency at which the gateway's availability should
     * be queried.
     * \return True if the gateway's available, false otherwise.
     */
    bool IsAvailableForTransmission(double frequency);

    /**
     * \brief Set the time of the next scheduled transmission for the gateway
     *
     * \param nextTransmissionTime The Time value
     */
    void SetNextTransmissionTime(Time nextTransmissionTime);
    // Time GetNextTransmissionTime ();

  private:
    Address m_address; //!< The Address of the P2PNetDevice of this gateway

    Ptr<NetDevice>
        m_netDevice; //!< The NetDevice through which to reach this gateway from the server

    Ptr<GatewayLorawanMac> m_gatewayMac; //!< The Mac layer of the gateway

    Time m_nextTransmissionTime; //!< This gateway's next transmission time
};
} // namespace lorawan

} // namespace ns3
#endif /* DEVICE_STATUS_H */
