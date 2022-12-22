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
 * Author: Alessandro Aimi <alessandro.aimi@cnam.fr>
 *                         <alessandro.aimi@orange.com>
 */

#ifndef POISSON_SENDER_H
#define POISSON_SENDER_H

#include "ns3/lora-application.h"
#include "ns3/lorawan-mac.h"

namespace ns3 {
namespace lorawan {

class PoissonSender : public LoraApplication
{
public:
  PoissonSender ();
  ~PoissonSender ();

  static TypeId GetTypeId (void);

private:
  /**
   * Start the application by scheduling the first SendPacket event
   */
  void StartApplication (void);

  /**
   * Stop the application
   */
  void StopApplication (void);

  /**
   * Send a packet using the LoraNetDevice's Send method
   */
  void SendPacket (void);

  Ptr<ExponentialRandomVariable> m_interval; //!< Random variable modeling packet inter-send time

  /**
   * The MAC layer of this node
   */
  Ptr<LorawanMac> m_mac;
};

} // namespace lorawan

} // namespace ns3
#endif /* POISSON_SENDER_H */
