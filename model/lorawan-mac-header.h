/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LORAWAN_MAC_HEADER_H
#define LORAWAN_MAC_HEADER_H

#include "ns3/header.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * This class represents the Mac header of a LoRaWAN packet.
 */
class LorawanMacHeader : public Header
{
  public:
    /**
     * The message type.
     *
     * The enum value corresponds to the value that will be written in the header
     * by the Serialize method.
     */
    enum MType
    {
        JOIN_REQUEST = 0,
        JOIN_ACCEPT = 1,
        UNCONFIRMED_DATA_UP = 2,
        UNCONFIRMED_DATA_DOWN = 3,
        CONFIRMED_DATA_UP = 4,
        CONFIRMED_DATA_DOWN = 5,
        PROPRIETARY = 7
    };

    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    LorawanMacHeader();           //!< Default constructor
    ~LorawanMacHeader() override; //!< Destructor

    // Pure virtual methods from Header that need to be implemented by this class
    TypeId GetInstanceTypeId() const override;
    uint32_t GetSerializedSize() const override;

    /**
     * Serialize the header.
     *
     * See Page 15 of LoRaWAN specification for a representation of fields.
     *
     * @param start A pointer to the buffer that will be filled with the
     * serialization.
     */
    void Serialize(Buffer::Iterator start) const override;

    /**
     * Deserialize the header.
     *
     * @param start A pointer to the buffer we need to deserialize.
     * @return The number of consumed bytes.
     */
    uint32_t Deserialize(Buffer::Iterator start) override;

    /**
     * Print the header in a human readable format.
     *
     * @param os The std::ostream on which to print the header.
     */
    void Print(std::ostream& os) const override;

    /**
     * Set the message type.
     *
     * @param mtype The message type of this header.
     */
    void SetMType(enum MType mtype);

    /**
     * Get the message type from the header.
     *
     * @return The uint8_t corresponding to this header's message type.
     */
    uint8_t GetMType() const;

    /**
     * Set the major version of this header.
     *
     * @param major The uint8_t corresponding to this header's major version.
     */
    void SetMajor(uint8_t major);

    /**
     * Get the major version from the header.
     *
     * @return The uint8_t corresponding to this header's major version.
     */
    uint8_t GetMajor() const;

    /**
     * Check whether this header is for an uplink message.
     *
     * @return True if the message is meant to be sent from an end device to a gateway, false
     * otherwise.
     */
    bool IsUplink() const;

    /**
     * Check whether this header is for a confirmed message, i.e. a message asking from
     * reception acknowledgment from the received.
     *
     * @return True is the message MType is of the confirmed variant, false otherwise.
     */
    bool IsConfirmed() const;

  private:
    /**
     * The Message Type.
     */
    uint8_t m_mtype;

    /**
     * The major version this header is using.
     */
    uint8_t m_major;
};
} // namespace lorawan

} // namespace ns3
#endif
