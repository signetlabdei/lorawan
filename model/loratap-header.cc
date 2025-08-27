/*
 * Copyright (c) 2025 ML!PA Consulting GmbH
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Marian Buschsieweke <marian.buschsieweke@posteo.net>
 */

#include "loratap-header.h"

#include "ns3/assert.h"
#include "ns3/log.h"

#include <cmath>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraTapHeader");

static const uint8_t RSSI_NOT_SPECIFIED = 0xff;
// lowest RSSI value that can be encoded
static const int16_t RSSI_FLOOR = -139;
static const uint8_t SYNC_LORAWAN = 0x34;
static const uint8_t LORATAP_VERSION = 0;
// Bandwidth resolution is in steps of 125 kHz
const uint32_t BANDWIDTH_RESOLUTION = 125000;
// See ASCI packet diagram in header: Version 0 is a fixed length header of
// 15 bytes
static const uint16_t LORATAP_HEADER_LENGTH = 15;

// Initialization list
LoraTapHeader::LoraTapHeader()
    : m_frequencyHz(0),
      m_bandwidthHz(0),
      m_spreadingFactor(0),
      m_rssiPkt(NAN),
      m_rssiMax(RSSI_NOT_SPECIFIED),
      m_rssiCurrent(RSSI_NOT_SPECIFIED),
      m_snr(0),
      m_sync(SYNC_LORAWAN)
{
}

LoraTapHeader::LoraTapHeader(const LoraTag& tag)
    : m_frequencyHz(tag.GetFrequency()),
      m_spreadingFactor(tag.GetSpreadingFactor()),
      m_rssiPkt(tag.GetReceivePower()),
      m_rssiMax(RSSI_NOT_SPECIFIED),
      m_rssiCurrent(RSSI_NOT_SPECIFIED),
      m_snr(tag.GetSignalToNoise()),
      m_sync(tag.GetSyncWord())
{
    SetChannelBandwidth(tag.GetChannelBandwidth());
}

LoraTapHeader::~LoraTapHeader()
{
}

TypeId
LoraTapHeader::GetTypeId()
{
    static TypeId tid = TypeId("LoraTapHeader").SetParent<Header>().AddConstructor<LoraTapHeader>();
    return tid;
}

TypeId
LoraTapHeader::GetInstanceTypeId() const
{
    return GetTypeId();
}

uint32_t
LoraTapHeader::GetSerializedSize() const
{
    NS_LOG_FUNCTION(this);
    return LORATAP_HEADER_LENGTH;
}

void
LoraTapHeader::Serialize(Buffer::Iterator start) const
{
    NS_LOG_FUNCTION(this);

    uint8_t rssi = ((int32_t)m_rssiPkt - RSSI_FLOOR);
    if (m_snr < 0)
    {
        int32_t tmp = m_rssiPkt * 4.0;
        tmp -= 4 * RSSI_FLOOR;
        rssi = tmp;
    }
    start.WriteU8(LORATAP_VERSION);
    start.WriteU8(0); // Padding
    start.WriteHtonU16(LORATAP_HEADER_LENGTH);
    start.WriteHtonU32(m_frequencyHz);
    start.WriteU8(m_bandwidthHz);
    start.WriteU8(m_spreadingFactor);
    start.WriteU8(rssi);
    start.WriteU8(m_rssiMax);
    start.WriteU8(m_rssiCurrent);
    start.WriteU8(m_snr);
    start.WriteU8(m_sync);
}

uint32_t
LoraTapHeader::Deserialize(Buffer::Iterator start)
{
    NS_LOG_FUNCTION(this);

    NS_ASSERT(LORATAP_VERSION == start.ReadU8());
    (void)start.ReadU8(); // Padding
    NS_ASSERT(LORATAP_HEADER_LENGTH == start.ReadNtohU16());
    m_frequencyHz = start.ReadNtohU32();
    m_bandwidthHz = start.ReadU8();
    m_spreadingFactor = start.ReadU8();
    uint8_t rssi = start.ReadU8();
    m_rssiMax = start.ReadU8();
    m_rssiCurrent = start.ReadU8();
    m_snr = start.ReadU8();
    m_sync = start.ReadU8();

    if (m_snr < 0)
    {
        m_rssiPkt = rssi;
        m_rssiPkt *= 0.25;
        m_rssiPkt += RSSI_FLOOR;
    }
    else
    {
        m_rssiPkt = (int32_t)rssi + (int32_t)RSSI_FLOOR;
    }

    return LORATAP_HEADER_LENGTH;
}

void
LoraTapHeader::Print(std::ostream& os) const
{
    os << "Freq=" << GetChannelFrequency() << " Hz";
    os << ", Bandwidth=" << GetChannelBandwidth() << " kHz";
    os << ", SF=" << GetSpreadingFactor();
    os << ", RSSI[PKT]=" << GetRssiPacket() << " dBm";
    os << ", RSSI[MAX]=" << GetRssiReceiverMax() << " dBm";
    os << ", RSSI[CUR]=" << GetRssiReceiverCurrent() << " dBm";
    os << ", SNR=" << GetSignalToNoise() << " dB";
    os << ", SYNC=" << GetSyncWord();
}

void
LoraTapHeader::SetChannelFrequency(uint32_t frequencyHz)
{
    m_frequencyHz = frequencyHz;
}

uint32_t
LoraTapHeader::GetChannelFrequency() const
{
    return m_frequencyHz;
}

void
LoraTapHeader::SetChannelBandwidth(uint32_t bandwidthHz)
{
    // scientific rounding
    m_bandwidthHz = (bandwidthHz + BANDWIDTH_RESOLUTION / 2) / BANDWIDTH_RESOLUTION;
}

uint32_t
LoraTapHeader::GetChannelBandwidth() const
{
    return m_bandwidthHz * BANDWIDTH_RESOLUTION;
}

void
LoraTapHeader::SetSpreadingFactor(uint8_t sf)
{
    m_spreadingFactor = sf;
}

uint8_t
LoraTapHeader::GetSpreadingFactor() const
{
    return m_spreadingFactor;
}

void
LoraTapHeader::SetRssiPacket(double rssi)
{
    m_rssiPkt = rssi;
}

void
LoraTapHeader::ClearRssiPacket()
{
    m_rssiPkt = NAN;
}

bool
LoraTapHeader::HasRssiPacket() const
{
    return !std::isnan(m_rssiPkt);
}

double
LoraTapHeader::GetRssiPacket() const
{
    return m_rssiPkt;
}

void
LoraTapHeader::SetRssiReceiverMax(double rssi)
{
    int32_t tmp = rssi;
    tmp -= RSSI_FLOOR;
    m_rssiMax = tmp;
}

void
LoraTapHeader::ClearRssiReceiverMax()
{
    m_rssiMax = RSSI_NOT_SPECIFIED;
}

bool
LoraTapHeader::HasRssiReceiverMax() const
{
    return m_rssiMax != RSSI_NOT_SPECIFIED;
}

double
LoraTapHeader::GetRssiReceiverMax() const
{
    if (!HasRssiReceiverMax())
    {
        return NAN;
    }

    return (int32_t)m_rssiMax + (int32_t)RSSI_FLOOR;
}

void
LoraTapHeader::SetRssiReceiverCurrent(double rssi)
{
    const double min = RSSI_FLOOR;
    const double max = (int32_t)RSSI_FLOOR + (int32_t)UINT8_MAX;

    if (rssi <= min)
    {
        m_rssiCurrent = 0;
        return;
    }

    if (rssi >= max)
    {
        m_rssiCurrent = UINT8_MAX;
        return;
    }

    int32_t tmp = rssi;
    tmp -= RSSI_FLOOR;
    m_rssiCurrent = tmp;
}

void
LoraTapHeader::ClearRssiReceiverCurrent()
{
    m_rssiCurrent = RSSI_NOT_SPECIFIED;
}

bool
LoraTapHeader::HasRssiReceiverCurrent() const
{
    return m_rssiCurrent != RSSI_NOT_SPECIFIED;
}

double
LoraTapHeader::GetRssiReceiverCurrent() const
{
    if (!HasRssiReceiverCurrent())
    {
        return NAN;
    }

    return (int32_t)m_rssiCurrent + (int32_t)RSSI_FLOOR;
}

void
LoraTapHeader::SetSignalToNoise(double snr)
{
    const double max = INT8_MAX * 4;
    const double min = INT8_MIN * 4;

    if (snr >= max)
    {
        m_snr = INT8_MAX;
        return;
    }

    if (snr <= min)
    {
        m_snr = INT8_MIN;
        return;
    }

    m_snr = (int8_t)(4.0 * snr);
}

double
LoraTapHeader::GetSignalToNoise() const
{
    return 0.25 * m_snr;
}

void
LoraTapHeader::SetSyncWord(uint8_t syncWord)
{
    m_sync = syncWord;
}

uint8_t
LoraTapHeader::GetSyncWord() const
{
    return m_sync;
}

} // namespace lorawan
} // namespace ns3
