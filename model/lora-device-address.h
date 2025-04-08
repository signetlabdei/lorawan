/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LORA_DEVICE_ADDRESS_H
#define LORA_DEVICE_ADDRESS_H

#include "ns3/address.h"

#include <string>

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Class representing the NetworkId component of a LoraDeviceAddress (7 bits).
 */
class NwkID
{
  public:
    /**
     * Construct a new NwkID object.
     *
     * @param nwkId The network id value [0:127].
     *
     * @todo Add warning as in Set().
     */
    NwkID(uint8_t nwkId = 0);

    /**
     * Set the NwkID, starting from a 8-bit representation of a 7-bit integer.
     *
     * This method will ignore the most significant bit of the argument. This
     * means that all arguments such that nwkId > 127 will actually be
     * considered as nwkId mod 127.
     *
     * @param nwkId The Network Id value to set.
     */
    void Set(uint8_t nwkId);

    /**
     * Get an uint8_t representation of the 7-bit network ID.
     *
     * @return The Network Id.
     */
    uint8_t Get() const;

  private:
    uint8_t m_nwkId; //!< 8-bit integer representation of the network id
};

/**
 * @ingroup lorawan
 *
 * Class representing the Network Address component of a LoraDeviceAddress (25
 * bits)
 */
class NwkAddr
{
  public:
    /**
     * Construct a new NwkAddr object.
     *
     * @param nwkId Network addr value [0:2^25-1].
     *
     * @todo Add warning as in Set().
     */
    NwkAddr(uint32_t nwkId = 0);

    /**
     * Set the NwkAddr, starting from a 32-bit representation of a 25-bit integer.
     *
     * This method will ignore the most significant bits of the argument. This
     * means that all arguments such that nwkAddr > 2^25-1 will actually be
     * considered as nwkAddr mod 2^25-1.
     *
     * @param nwkAddr The Network Address to set.
     */
    void Set(uint32_t nwkAddr);

    /**
     * Get an uint32_t representation of the 25-bit network address.
     *
     * @return The Network Address.
     */
    uint32_t Get() const;

  private:
    uint32_t m_nwkAddr; //!< 8-bit integer representation of the network id
};

/**
 * @ingroup lorawan
 *
 * This class represents the device address of a LoraWAN end device.
 */
class LoraDeviceAddress
{
  public:
    LoraDeviceAddress(); //!< Default constructor

    /**
     * Build a new address from a 32-bit integer.
     *
     * @param address Full numeric value of the address.
     */
    LoraDeviceAddress(uint32_t address);

    /**
     * Build a new address from a network id and network address.
     *
     * @param nwkId Network id numeric value.
     * @param nwkAddr Network address numeric value.
     */
    LoraDeviceAddress(uint8_t nwkId, uint32_t nwkAddr);

    /**
     * Build a new address from a network id and network address.
     *
     * @param nwkId Network id object.
     * @param nwkAddr Network address object.
     */
    LoraDeviceAddress(NwkID nwkId, NwkAddr nwkAddr);

    /**
     * Convert this address to a buffer.
     *
     * @param buf [out] buffer to fill with serialized address.
     */
    void Serialize(uint8_t buf[4]) const;

    /**
     * Convert the input buffer into a new address.
     *
     * @param buf [in] buffer containing serialized address.
     * @return The LoraDeviceAddress object.
     */
    static LoraDeviceAddress Deserialize(const uint8_t buf[4]);

    /**
     * Convert from an ordinary address to a LoraDeviceAddress instance.
     *
     * @param address Reference to ordinary Address object.
     * @return The LoraDeviceAddress object.
     */
    static LoraDeviceAddress ConvertFrom(const Address& address);

    /**
     * Set the address as a 32 bit integer.
     *
     * @param address Full numeric value of the address.
     */
    void Set(uint32_t address);

    /**
     * Set the address, combining a network id and a network address.
     *
     * Note that nwkId is 7 bits long, and this function expects the 7 least
     * significant bits to contain the nwkId. Similarly for the nwkAddr, the 25
     * least significant bits of the uint32 are those that are expected to
     * contain the nwkAddr.
     *
     * @param nwkId Network id numeric value.
     * @param nwkAddr Network address numeric value.
     *
     * @todo Not implemented, this is a placeholder for future implementation.
     */
    void Set(uint8_t nwkId, uint32_t nwkAddr);

    /**
     * Get the address in 32-bit integer form.
     *
     * @return Full numeric value of the address.
     */
    uint32_t Get() const;

    /**
     * Get the NwkID of this device.
     *
     * @remark The NwkID bit-by-bit representation will be contained in the 7
     * least significant bits of the returned uint8_t.
     *
     * @return An 8-bit representation of the Network Id of this Device Address.
     */
    uint8_t GetNwkID();

    /**
     * Set the NwkID of this device.
     *
     * @remark The NwkID is expected to be contained on the 7 least significant
     * bits of the uint8_t.
     *
     * @param nwkId The network id to set.
     */
    void SetNwkID(uint8_t nwkId);

    /**
     * Get the NwkAddr of this device.
     *
     * @remark The NwkAddr will be contained on the 25 least significant bits of
     * the uint32_t.
     *
     * @return A 32-bit representation of the Network Address of this Device Address.
     */
    uint32_t GetNwkAddr();

    /**
     * Set the NwkAddr of this device.
     *
     * @remark The NwkAddr is expected to be contained on the least significant
     * bits of the uint32_t.
     *
     * @param nwkAddr The network address to set.
     */
    void SetNwkAddr(uint32_t nwkAddr);

    /**
     * Print the address bit-by-bit to a human-readable string.
     *
     * @return The string containing the network address.
     */
    std::string Print() const;

    /**
     * Equality comparison operator
     * @param other Address to compare.
     * @return True if the addresses are equal.
     */
    bool operator==(const LoraDeviceAddress& other) const;
    /**
     * Inequality comparison operator
     * @param other Address to compare.
     * @return True if the addresses are different.
     */
    bool operator!=(const LoraDeviceAddress& other) const;
    /**
     * Less-then comparison operator
     * @param other Address to compare.
     * @return True if the first address is less than the second.
     */
    bool operator<(const LoraDeviceAddress& other) const;
    /**
     * Greater-then comparison operator
     * @param other Address to compare.
     * @return True if the first address is greater than the second.
     */
    bool operator>(const LoraDeviceAddress& other) const;

  private:
    /**
     * Convert this instance of LoraDeviceAddress to an Address.
     *
     * @return The Address object.
     */
    Address ConvertTo() const;

    /**
     * Get a new address type id.
     * @return The new address type id.
     */
    static uint8_t GetType();

    NwkID m_nwkId;     //!< The network Id of this address
    NwkAddr m_nwkAddr; //!< The network address of this address
};

/**
 * Operator overload to correctly handle logging when an address is passed as
 * an argument.
 */
std::ostream& operator<<(std::ostream& os, const LoraDeviceAddress& address);

} // namespace lorawan
} // namespace ns3
#endif
