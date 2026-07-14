/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "lorawan-mac-header.h"

#include <bitset>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LorawanMacHeader");

LorawanMacHeader::LorawanMacHeader()
    : m_major(0)
{
}

LorawanMacHeader::~LorawanMacHeader()
{
}

TypeId
LorawanMacHeader::GetTypeId()
{
    static TypeId tid =
        TypeId("LorawanMacHeader").SetParent<Header>().AddConstructor<LorawanMacHeader>();
    return tid;
}

TypeId
LorawanMacHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
LorawanMacHeader::GetSerializedSize() const
{
    NS_LOG_FUNCTION_NOARGS();

    return 1; // This header only consists in 8 bits
}

void
LorawanMacHeader::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION_NOARGS();

    // The header we need to fill
    uint8_t header = 0;

    // The FType
    header |= uint8_t(m_fType) << 5;

    // Do nothing for the bits that are RFU

    // The major version bits
    header |= m_major;

    // Write the byte
    start.WriteU8(header);

    NS_LOG_DEBUG("Serialization of MAC header: " << std::bitset<8>(header));
}

uint32_t
LorawanMacHeader::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION_NOARGS();

    // Save the byte on a temporary variable
    uint8_t byte;
    byte = start.ReadU8();

    // Get the 2 least significant bits to have the Major
    m_major = byte & 0b11;

    // Move the three most significant bits to the least significant positions
    // to get the FType
    m_fType = FType(byte >> 5);

    return 1; // the number of bytes consumed.
}

void
LorawanMacHeader::Print(std::ostream& os) const
{
    os << "FType=" << unsigned(m_fType);
    os << ", Major=" << unsigned(m_major);
}

void
LorawanMacHeader::SetFType(FType fType)
{
    NS_LOG_FUNCTION(this << fType);

    m_fType = fType;
}

LorawanMacHeader::FType
LorawanMacHeader::GetFType() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_fType;
}

void
LorawanMacHeader::SetMajor(uint8_t major)
{
    NS_LOG_FUNCTION_NOARGS();

    NS_ASSERT(0 <= major && major < 4);

    m_major = major;
}

uint8_t
LorawanMacHeader::GetMajor() const
{
    NS_LOG_FUNCTION_NOARGS();

    return m_major;
}

bool
LorawanMacHeader::IsUplink() const
{
    NS_LOG_FUNCTION_NOARGS();

    return (m_fType == FType::JOIN_REQUEST) || (m_fType == FType::UNCONFIRMED_DATA_UP) ||
           (m_fType == FType::CONFIRMED_DATA_UP);
}

bool
LorawanMacHeader::IsConfirmed() const
{
    NS_LOG_FUNCTION_NOARGS();

    return (m_fType == FType::CONFIRMED_DATA_DOWN) || (m_fType == FType::CONFIRMED_DATA_UP);
}

std::ostream&
operator<<(std::ostream& os, const LorawanMacHeader::FType& fType)
{
    switch (fType)
    {
    case LorawanMacHeader::FType::JOIN_REQUEST:
        return (os << "JOIN_REQUEST");
    case LorawanMacHeader::FType::JOIN_ACCEPT:
        return (os << "JOIN_ACCEPT");
    case LorawanMacHeader::FType::UNCONFIRMED_DATA_UP:
        return (os << "UNCONFIRMED_DATA_UP");
    case LorawanMacHeader::FType::UNCONFIRMED_DATA_DOWN:
        return (os << "UNCONFIRMED_DATA_DOWN");
    case LorawanMacHeader::FType::CONFIRMED_DATA_UP:
        return (os << "CONFIRMED_DATA_UP");
    case LorawanMacHeader::FType::CONFIRMED_DATA_DOWN:
        return (os << "CONFIRMED_DATA_DOWN");
    case LorawanMacHeader::FType::PROPRIETARY:
        return (os << "PROPRIETARY");
    }
    NS_FATAL_ERROR("Invalid LoRaWAN MAC Header FType");
}

} // namespace lorawan
} // namespace ns3
