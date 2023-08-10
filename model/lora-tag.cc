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
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "lora-tag.h"

#include "ns3/tag.h"
#include "ns3/uinteger.h"

namespace ns3
{
namespace lorawan
{

NS_OBJECT_ENSURE_REGISTERED(LoraTag);

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

LoraTag::LoraTag()
    : m_frequency(0),
      m_destroyedBy(0),
      m_receptionTime(0),
      m_receivePower(0),
      m_snr(0)
{
}

LoraTag::~LoraTag()
{
}

uint32_t
LoraTag::GetSerializedSize() const
{
    return 4 + 1 + sizeof(double) + 1 + sizeof(int64_t) + sizeof(double) + sizeof(double);
}

void
LoraTag::Serialize(TagBuffer i) const
{
    // LoraPhyTxParameters (4 bytes total)
    i.WriteU8(m_params.sf);
    uint8_t p = 0;
    // headerDisabled (1 bit)
    p |= uint8_t(m_params.headerDisabled << 7 & 0b10000000);
    // codingRate - 1 (2 bits)
    p |= uint8_t((m_params.codingRate - 1) << 5 & 0b1100000);
    // bandwidthHz / 125000 - 1 (2 bits)
    p |= uint8_t(uint8_t(m_params.bandwidthHz / 125000 - 1) << 3 & 0b11000);
    // crcEnabled (1 bit)
    p |= uint8_t(m_params.crcEnabled << 2 & 0b100);
    // lowDataRateOptimizationEnabled (1 bit)
    p |= uint8_t(m_params.lowDataRateOptimizationEnabled << 1 & 0b10);
    i.WriteU8(p);
    i.WriteU16(m_params.nPreamble);

    i.WriteU8(m_dataRate);
    i.WriteDouble(m_frequency);
    i.WriteU8(m_destroyedBy);

    // Reception completed timestamp
    int64_t t = m_receptionTime.GetNanoSeconds();
    i.Write((const uint8_t*)&t, 8);

    i.WriteDouble(m_receivePower);
    i.WriteDouble(m_snr);
}

void
LoraTag::Deserialize(TagBuffer i)
{
    // LoraPhyTxParameters
    m_params.sf = i.ReadU8();
    uint8_t p = i.ReadU8();
    m_params.headerDisabled = bool((p >> 7) & 0b1);
    m_params.codingRate = uint8_t((p >> 5) & 0b11) + 1;
    m_params.bandwidthHz = (double((p >> 3) & 0b11) + 1) * 125000;
    m_params.crcEnabled = bool((p >> 2) & 0b1);
    m_params.lowDataRateOptimizationEnabled = bool((p >> 1) & 0b1);
    m_params.nPreamble = i.ReadU16();

    m_dataRate = i.ReadU8();
    m_frequency = i.ReadDouble();
    m_destroyedBy = i.ReadU8();

    int64_t t;
    i.Read((uint8_t*)&t, 8);
    m_receptionTime = NanoSeconds(t);

    m_receivePower = i.ReadDouble();
    m_snr = i.ReadDouble();
}

void
LoraTag::Print(std::ostream& os) const
{
    os << "txParams: " << m_params << ", dataRate=" << (unsigned)m_dataRate
       << ", frequency=" << m_frequency << ", destroyedBy=" << (unsigned)m_destroyedBy
       << ", receptionTime=" << m_receptionTime << ", rxPower=" << m_receivePower
       << ", snr=" << m_snr;
}

void
LoraTag::SetTxParameters(LoraPhyTxParameters params)
{
    m_params = params;
}

LoraPhyTxParameters
LoraTag::GetTxParameters() const
{
    return m_params;
}

void
LoraTag::SetDataRate(uint8_t dataRate)
{
    m_dataRate = dataRate;
}

uint8_t
LoraTag::GetDataRate() const
{
    return m_dataRate;
}

void
LoraTag::SetFrequency(double frequency)
{
    m_frequency = frequency;
}

double
LoraTag::GetFrequency() const
{
    return m_frequency;
}

void
LoraTag::SetDestroyedBy(uint8_t sf)
{
    m_destroyedBy = sf;
}

uint8_t
LoraTag::GetDestroyedBy() const
{
    return m_destroyedBy;
}

void
LoraTag::SetReceptionTime(Time receptionTime)
{
    m_receptionTime = receptionTime;
}

Time
LoraTag::GetReceptionTime() const
{
    return m_receptionTime;
}

void
LoraTag::SetReceivePower(double receivePower)
{
    m_receivePower = receivePower;
}

double
LoraTag::GetReceivePower() const
{
    return m_receivePower;
}

void
LoraTag::SetSnr(double snr)
{
    m_snr = snr;
}

double
LoraTag::GetSnr() const
{
    return m_snr;
}

} // namespace lorawan
} // namespace ns3
