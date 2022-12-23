/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef CHIRPSTACK_HELPER_H
#define CHIRPSTACK_HELPER_H

#include "ns3/node-container.h"
#include "ns3/loragw_hal.h"

namespace ns3 {
namespace lorawan {

/**
 * This class can be used to install devices and gateways on a real 
 * chirpstack network server using the REST API. 
 * Requires libcurlpp and libcurl installed.
 */
class ChirpstackHelper
{
  using str = std::string;
  using query_t = std::vector<std::pair<str, str>>;

  struct session_t
  {
    // Tenant information
    str tenantId;
    str devProfId;
    str appId;

    // Session keys
    str netKey;
    str appKey;
  };

public:
  ChirpstackHelper ();

  ~ChirpstackHelper ();

  int InitConnection (Ipv4Address ip, uint16_t port);

  void CloseConnection (int signal) const;

  int Register (NodeContainer c) const;

  int Register (Ptr<Node> node) const;

private:
  int NewTenant (const str &name);

  int NewDeviceProfile (const str &name);

  int NewApplication (const str &name);

  int RegisterPriv (Ptr<Node> node) const;

  int NewDevice (Ptr<Node> node) const;

  int NewGateway (Ptr<Node> node) const;

  int POST (const str &path, const str &body, str &out) const;

  int DELETE (const str &path, str &out) const;

  str m_url;
  std::list<str> m_header;
  
  session_t m_session;

  static const struct coord_s m_center;
};

} // namespace lorawan

} // namespace ns3
#endif /* CHIRPSTACK_HELPER_H */
