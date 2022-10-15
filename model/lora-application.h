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

#ifndef LORA_APPLICATION_H
#define LORA_APPLICATION_H

#include "ns3/application.h"

namespace ns3 {
namespace lorawan {

class LoraApplication : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  LoraApplication ();
  virtual ~LoraApplication ();

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

protected:
  /**
   * Start the application by scheduling the first SendPacket event
   */
  virtual void StartApplication (void);

  /**
   * Stop the application
   */
  virtual void StopApplication (void);

  /**
   * Send a packet using the LoraNetDevice's Send method
   */
  virtual void SendPacket (void);

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
   * The packet size.
   */
  uint8_t m_basePktSize;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_APPLICATION_H */
