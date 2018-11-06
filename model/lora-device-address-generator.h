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

#ifndef LORA_DEVICE_ADDRESS_GENERATOR_H
#define LORA_DEVICE_ADDRESS_GENERATOR_H

#include "ns3/lora-device-address.h"
#include "ns3/object.h"

namespace ns3 {
namespace lorawan {

/**
 * This class generates sequential LoraDeviceAddress instances.
 */
class LoraDeviceAddressGenerator : public Object
{
public:
  static TypeId GetTypeId (void);

  /**
   * Initialise the base NwkID and the first NwkAddr to be used by the
   * generator.
   *
   * The first call to NextAddress() or GetAddress() will return these values.
   *
   * \param nwkId The first network id.
   * \param nwkAddr The first address.
   */
  LoraDeviceAddressGenerator (const uint8_t nwkId = 0,
                              const uint32_t nwkAddr = 0);

  /**
   * Get the first address from the next network.
   *
   * This resets the address to the base address that was used for
   * initialization.
   *
   * \return the LoraDeviceAddress address of the next network
   */
  LoraDeviceAddress NextNetwork (void);

  /**
   * Allocate the next LoraDeviceAddress.
   *
   * This operation is a post-increment, meaning that the first address
   * allocated will be the one that was initially configured.
   *
   * This keeps the nwkId constant, only incrementing nwkAddr.
   *
   * \return the LoraDeviceAddress address
   */
  LoraDeviceAddress NextAddress (void);

  /**
   * Get the LoraDeviceAddress that will be allocated upon a call to
   * NextAddress.
   *
   * Does not change the internal state; is just used to peek at the next
   * address that will be allocated upon a call to NextAddress
   *
   * \return the LoraDeviceAddress
   */
  LoraDeviceAddress GetNextAddress (void);

private:
  NwkID m_currentNwkId; //!< The current Network Id value
  NwkAddr m_currentNwkAddr; //!< The current Network Address value
};
} //namespace ns3
}
#endif
