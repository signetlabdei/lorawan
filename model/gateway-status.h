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

#ifndef GATEWAY_STATUS_H
#define GATEWAY_STATUS_H

#include "ns3/object.h"
#include "ns3/address.h"
#include "ns3/net-device.h"
#include "ns3/gateway-lorawan-mac.h"

namespace ns3 {
namespace lorawan {

class GatewayStatus : public Object
{
public:
  static TypeId GetTypeId (void);

  GatewayStatus ();
  GatewayStatus (Address address, Ptr<NetDevice> netDevice, Ptr<GatewayLorawanMac> gwMac);
  virtual ~GatewayStatus ();

  /**
   * Get this gateway's P2P link address.
   */
  Address GetAddress ();

  /**
   * Set this gateway's P2P link address.
   */
  void SetAddress (Address address);

  /**
   * Get the NetDevice through which it's possible to contact this gateway from the server.
   */
  Ptr<NetDevice> GetNetDevice ();

  /**
   * Set the NetDevice through which it's possible to contact this gateway from the server.
   */
  void SetNetDevice (Ptr<NetDevice> netDevice);

  /**
   * Get a pointer to this gateway's MAC instance.
   */
  Ptr<GatewayLorawanMac> GetGatewayMac (void);

  /**
   * Set a pointer to this gateway's MAC instance.
   */
  // void SetGatewayMac (Ptr<GatewayLorawanMac> gwMac);

  /**
   * Query whether or not this gateway is available for immediate transmission
   * on this frequency.
   *
   * \param frequency The frequency at which the gateway's availability should
   * be queried.
   * \return True if the gateway's available, false otherwise.
   */
  bool IsAvailableForTransmission (double frequency);

  void SetNextTransmissionTime (Time nextTransmissionTime);
  // Time GetNextTransmissionTime (void);

private:
  Address m_address;   //!< The Address of the P2PNetDevice of this gateway

  Ptr<NetDevice> m_netDevice;     //!< The NetDevice through which to reach this gateway from the server

  Ptr<GatewayLorawanMac> m_gatewayMac;     //!< The Mac layer of the gateway

  Time m_nextTransmissionTime;   //!< This gateway's next transmission time
};
}

}
#endif /* DEVICE_STATUS_H */
