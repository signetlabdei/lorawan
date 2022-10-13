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
  virtual ~LoraApplication (){};

  /**
   * Set the sending interval
   * \param interval the interval between two packet sendings
   */
  virtual void SetInterval (Time interval) = 0;

  /**
   * Get the sending inteval
   * \returns the interval between two packet sends
   */
  virtual Time GetInterval (void) const = 0;

  /**
   * Set the initial delay of this application
   */
  virtual void SetInitialDelay (Time delay) = 0;

  /**
   * Set packet size
   */
  virtual void SetPacketSize (uint8_t size) = 0;

  /**
   * Get packet size
   */
  virtual uint8_t GetPacketSize (void) const = 0;

  /** 
   * True if the application is currently running
   */
  virtual bool IsRunning (void) = 0;

private:
  /**
   * Start the application by scheduling the first SendPacket event
   */
  virtual void StartApplication (void) = 0;

  /**
   * Stop the application
   */
  virtual void StopApplication (void) = 0;

  /**
   * Send a packet using the LoraNetDevice's Send method
   */
  virtual void SendPacket (void) = 0;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_APPLICATION_H */
