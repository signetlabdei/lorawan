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

#include "lora-application.h"
#include "ns3/lorawan-mac.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {
namespace lorawan {

class PoissonSender : public LoraApplication
{
public:
  PoissonSender ();
  ~PoissonSender ();

  static TypeId GetTypeId (void);

  /**
   * Set the sending interval
   * \param interval the interval between two packet sendings
   */
  void SetInterval (Time interval);

  /**
   * Get the sending inteval
   * \returns the interval between two packet sends
   */
  Time GetInterval (void) const;

  /**
   * Set the initial delay of this application
   */
  void SetInitialDelay (Time delay);

  /**
   * Set packet size
   */
  void SetPacketSize (uint8_t size);

  /**
   * Get packet size
   */
  uint8_t GetPacketSize (void) const;

  /** 
   * True if the application is currently running
   */
  bool IsRunning (void);

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

  /**
   * The average interval between to consecutive send events
   */
  Time m_avgInterval;

  /**
   * The initial delay of this application
   */
  Time m_initialDelay;

  /**
   * The sending event scheduled as next
   */
  EventId m_sendEvent;

  /**
   * The MAC layer of this node
   */
  Ptr<LorawanMac> m_mac;

  /**
   * The packet size.
   */
  uint8_t m_basePktSize;

  Ptr<ExponentialRandomVariable> m_interval; //!< Random variable modeling packet inter-send time
};

} // namespace lorawan

} // namespace ns3
#endif /* POISSON_SENDER_H */
