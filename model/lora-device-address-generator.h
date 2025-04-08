/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LORA_DEVICE_ADDRESS_GENERATOR_H
#define LORA_DEVICE_ADDRESS_GENERATOR_H

#include "lora-device-address.h"

#include "ns3/object.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * This class generates sequential LoraDeviceAddress instances.
 */
class LoraDeviceAddressGenerator : public Object
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Initialise the base NwkID and the first NwkAddr to be used by the
     * generator.
     *
     * The first call to NextAddress() or GetAddress() will return these values.
     *
     * @param nwkId The first network id.
     * @param nwkAddr The first address.
     */
    LoraDeviceAddressGenerator(const uint8_t nwkId = 0, const uint32_t nwkAddr = 0);

    /**
     * Get the first address from the next network.
     *
     * This resets the address to the base address that was used for
     * initialization.
     *
     * @return The LoraDeviceAddress address of the next network.
     */
    LoraDeviceAddress NextNetwork();

    /**
     * Allocate the next LoraDeviceAddress.
     *
     * This operation is a post-increment, meaning that the first address
     * allocated will be the one that was initially configured.
     *
     * This keeps the nwkId constant, only incrementing nwkAddr.
     *
     * @return The LoraDeviceAddress address.
     */
    LoraDeviceAddress NextAddress();

    /**
     * Get the LoraDeviceAddress that will be allocated upon a call to
     * NextAddress.
     *
     * Does not change the internal state; is just used to peek at the next
     * address that will be allocated upon a call to NextAddress.
     *
     * @return The LoraDeviceAddress.
     */
    LoraDeviceAddress GetNextAddress();

  private:
    NwkID m_currentNwkId;     //!< The current Network Id value
    NwkAddr m_currentNwkAddr; //!< The current Network Address value
};
} // namespace lorawan
} // namespace ns3
#endif
