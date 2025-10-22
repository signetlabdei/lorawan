/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "lora-tag.h"

#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3
{
namespace lorawan
{

NS_OBJECT_ENSURE_REGISTERED(LoraTag);

const uint8_t LoraTag::SYNC_WORD_LORAWAN = 0x34;

TypeId
LoraTag::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LoraTag").SetParent<Tag>().SetGroupName("lorawan").AddConstructor<LoraTag>();
    return tid;
}

TypeId
LoraTag::GetInstanceTypeId() const
{
    return GetTypeId();
}

LoraTag::LoraTag(uint8_t sf, uint8_t destroyedBy)
    : m_sf(sf),
      m_sync(SYNC_WORD_LORAWAN),
      m_destroyedBy(destroyedBy),
      m_receivePower(0),
      m_dataRate(0)
{
}

LoraTag::~LoraTag()
{
}

uint32_t
LoraTag::GetSerializedSize() const
{
    // 4 * uint8_t: (m_sf, m_sync, m_destroyedBy, m_dataRate)
    // 1 * uint32_t: (m_frequencyHz)
    // 1 * double: (m_receivePower)
    return 4 * sizeof(uint8_t) + 1 * sizeof(uint32_t) + 1 * sizeof(double);
}

void
LoraTag::Serialize(TagBuffer i) const
{
    i.WriteU8(m_sf);
    i.WriteU8(m_sync);
    i.WriteU8(m_destroyedBy);
    i.WriteDouble(m_receivePower);
    i.WriteU8(m_dataRate);
    i.WriteU32(m_frequencyHz);
}

void
LoraTag::Deserialize(TagBuffer i)
{
    m_sf = i.ReadU8();
    m_sync = i.ReadU8();
    m_destroyedBy = i.ReadU8();
    m_receivePower = i.ReadDouble();
    m_dataRate = i.ReadU8();
    m_frequencyHz = i.ReadU32();
}

void
LoraTag::Print(std::ostream& os) const
{
    os << m_sf << " " << m_destroyedBy << " " << m_receivePower << " " << m_dataRate;
}

uint8_t
LoraTag::GetSpreadingFactor() const
{
    return m_sf;
}

uint8_t
LoraTag::GetDestroyedBy() const
{
    return m_destroyedBy;
}

double
LoraTag::GetReceivePower() const
{
    return m_receivePower;
}

void
LoraTag::SetDestroyedBy(uint8_t sf)
{
    m_destroyedBy = sf;
}

void
LoraTag::SetSpreadingFactor(uint8_t sf)
{
    m_sf = sf;
}

void
LoraTag::SetReceivePower(double receivePower)
{
    m_receivePower = receivePower;
}

void
LoraTag::SetFrequency(uint32_t frequencyHz)
{
    m_frequencyHz = frequencyHz;
}

uint32_t
LoraTag::GetFrequency() const
{
    return m_frequencyHz;
}

void
LoraTag::SetSyncWord(uint8_t syncWord)
{
    m_sync = syncWord;
}

uint8_t
LoraTag::GetSyncWord() const
{
    return m_sync;
}

uint8_t
LoraTag::GetDataRate() const
{
    return m_dataRate;
}

void
LoraTag::SetDataRate(uint8_t dataRate)
{
    m_dataRate = dataRate;
}

} // namespace lorawan
} // namespace ns3
