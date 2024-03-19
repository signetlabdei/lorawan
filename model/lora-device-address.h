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

#ifndef LORA_DEVICE_ADDRESS_H
#define LORA_DEVICE_ADDRESS_H

#include "ns3/address.h"

#include <string>

namespace ns3
{
namespace lorawan
{

/**
 * \ingroup lorawan
 *
 * \brief Class representing the NetworkId component of a LoraDeviceAddress (7 bits).
 */
class NwkID
{
  public:
    /**
     * \brief Construct a new NwkID object
     *
     * \todo add warning as in Set()
     *
     * \param nwkId the network id value [0:127]
     */
    NwkID(uint8_t nwkId = 0);

    /**
     * \brief Set the NwkID, starting from a 8-bit representation of a 7-bit integer.
     *
     * This method will ignore the most significant bit of the argument. This
     * means that all arguments such that nwkId > 127 will actually be
     * considered as nwkId mod 127.
     *
     * \param nwkId The Network Id value to set.
     */
    void Set(uint8_t nwkId);

    /**
     * \brief Get an uint8_t representation of the 7-bit network ID.
     *
     * \return The Network Id.
     */
    uint8_t Get() const;

  private:
    uint8_t m_nwkId; //!< 8-bit integer representation of the network id
};

/**
 * \ingroup lorawan
 *
 * \brief Class representing the Network Address component of a LoraDeviceAddress (25
 * bits)
 */
class NwkAddr
{
  public:
    /**
     * \brief Construct a new NwkAddr object
     *
     * \todo add warning as in Set()
     *
     * \param nwkId network addr value [0:2^25-1]
     */
    NwkAddr(uint32_t nwkId = 0);

    /**
     * \brief Set the NwkAddr, starting from a 32-bit representation of a 25-bit integer.
     *
     * This method will ignore the most significant bits of the argument. This
     * means that all arguments such that nwkAddr > 2^25-1 will actually be
     * considered as nwkAddr mod 2^25-1.
     *
     * \param nwkAddr The Network Address to set.
     */
    void Set(uint32_t nwkAddr);

    /**
     * \brief Get an uint32_t representation of the 25-bit network address.
     *
     * \return The Network Address.
     */
    uint32_t Get() const;

  private:
    uint32_t m_nwkAddr; //!< 8-bit integer representation of the network id
};

/**
 * \ingroup lorawan
 *
 * \brief This class represents the device address of a LoraWAN End Device.
 */
class LoraDeviceAddress
{
  public:
    LoraDeviceAddress(); //!< Default constructor

    /**
     * \brief Build a new address from a 32-bit integer.
     *
     * \param address full numeric value of the address
     */
    LoraDeviceAddress(uint32_t address);

    /**
     * \brief Build a new address from a network id and network address.
     *
     * \param nwkId network id numeric value
     * \param nwkAddr network address numeric value
     */
    LoraDeviceAddress(uint8_t nwkId, uint32_t nwkAddr);

    /**
     * \brief Build a new address from a network id and network address.
     *
     * \param nwkId network id object
     * \param nwkAddr network address object
     */
    LoraDeviceAddress(NwkID nwkId, NwkAddr nwkAddr);

    /**
     * \brief Convert this address to a buffer.
     *
     * \param buf [out] buffer to fill with serialized address.
     */
    void Serialize(uint8_t buf[4]) const;

    /**
     * \brief Convert the input buffer into a new address.
     *
     * \param buf [in] buffer containing serialized address.
     * \return the LoraDeviceAddress object
     */
    static LoraDeviceAddress Deserialize(const uint8_t buf[4]);

    /**
     * \brief Convert from an ordinary address to a LoraDeviceAddress instance.
     *
     * \param address reference to ordinary Address object
     * \return the LoraDeviceAddress object
     */
    static LoraDeviceAddress ConvertFrom(const Address& address);

    /**
     * \brief Set the address as a 32 bit integer.
     *
     * \param address full numeric value of the address
     */
    void Set(uint32_t address);

    /**
     * \brief Set the address, combining a network id and a network address.
     *
     * Note that nwkId is 7 bits long, and this function expects the 7 least
     * significant bits to contain the nwkId. Similarly for the nwkAddr, the 25
     * least significant bits of the uint32 are those that are expected to
     * contain the nwkAddr.
     *
     * \todo not implemented
     *
     * \param nwkId network id numeric value
     * \param nwkAddr network address numeric value
     */
    void Set(uint8_t nwkId, uint32_t nwkAddr);

    /**
     * \brief Get the address in 32-bit integer form.
     *
     * \return full numeric value of the address
     */
    uint32_t Get() const;

    /**
     * \brief Get the NwkID of this device.
     *
     * \remark The NwkID bit-by-bit representation will be contained in the 7
     * least significant bits of the returned uint8_t.
     *
     * \return An 8-bit representation of the Network Id of this Device Address.
     */
    uint8_t GetNwkID();

    /**
     * \brief Set the NwkID of this device.
     *
     * \remark The NwkID is expected to be contained on the 7 least significant
     * bits of the uint8_t.
     *
     * \param nwkId The network id to set.
     */
    void SetNwkID(uint8_t nwkId);

    /**
     * \brief Get the NwkAddr of this device.
     *
     * \remark The NwkAddr will be contained on the 25 least significant bits of
     * the uint32_t.
     *
     * \return A 32-bit representation of the Network Address of this Device Address.
     */
    uint32_t GetNwkAddr();

    /**
     * \brief Set the NwkAddr of this device.
     *
     * \remark The NwkAddr is expected to be contained on the least significant
     * bits of the uint32_t.
     *
     * \param nwkAddr The network address to set.
     */
    void SetNwkAddr(uint32_t nwkAddr);

    /**
     * \brief Print the address bit-by-bit to a human-readable string.
     *
     * \return The string containing the network address.
     */
    std::string Print() const;

    /**
     * \brief Equality comparison operator
     * \param other address to compare
     * \return true if the addresses are equal
     */
    bool operator==(const LoraDeviceAddress& other) const;
    /**
     * \brief Inequality comparison operator
     * \param other address to compare
     * \return true if the addresses are different
     */
    bool operator!=(const LoraDeviceAddress& other) const;
    /**
     * \brief Less-then comparison operator
     * \param other address to compare
     * \return true if the first address is less than the second
     */
    bool operator<(const LoraDeviceAddress& other) const;
    /**
     * \brief Greater-then comparison operator
     * \param other address to compare
     * \return true if the first address is greater than the second
     */
    bool operator>(const LoraDeviceAddress& other) const;

  private:
    /**
     * \brief Convert this instance of LoraDeviceAddress to an Address
     *
     * \return the Address object
     */
    Address ConvertTo() const;

    /**
     * \brief Get a new address type id.
     * \return the new address type id
     */
    static uint8_t GetType();

    NwkID m_nwkId;     //!< The network Id of this address
    NwkAddr m_nwkAddr; //!< The network address of this address
};

/**
 * \brief Operator overload to correctly handle logging when an address is passed as
 * an argument.
 */
std::ostream& operator<<(std::ostream& os, const LoraDeviceAddress& address);

} // namespace lorawan
} // namespace ns3
#endif
