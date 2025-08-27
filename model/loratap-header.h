/*
 * Copyright (c) 2025 ML!PA Consulting GmbH
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Marian Buschsieweke <marian.buschsieweke@posteo.net>
 */

#ifndef LORATAP_HEADER_H
#define LORATAP_HEADER_H

#include "lora-tag.h"

#include "ns3/header.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * This class represents the LoRaTap header as defined in
 * https://github.com/eriknl/LoRaTap/blob/master/loratap.h, for compatibility
 * with the wireshark disscector. The format looks like this:
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |    Version    |    Padding    |         Header Length         |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |                   Channel Frequency [Hz]                      |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   Bandwidth   |       SF      |   RSSI[PKT]   |  RSSI[MAX]    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |   RSSI[CUR]   |      SNR      |   Sync Word   |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Version:
 *     Version of the LoRaTap header. We use version 0.
 *
 * Padding:
 *     Unused
 *
 * Header Length:
 *     Length of the LoRaTap header. Big Endian.
 *
 * Channel Frequency [Hz]:
 *     Frequency the frame was send/received at in Hz. Big Endian.
 *
 * Bandwitdth:
 *     Channel Bandwidth in steps of 125 kHz. (0 ↦ 0 kHz, 1 ↦ 125 kHz, ...)
 *
 * SF:
 *     Spreading factor
 *
 * RSSI[PKT]:
 *     Packet RSSI in dBm starting from -139 dBm. If SNR is negative in steps
 *     of 0.25 dBm, otherwise in steps of 1 dBm. 0xff means unspecified.
 *
 *     0xff ↦ n.d.
 *     SNR ≥ 0: 0x00 ↦ -139 dBm, 0x01 ↦ -138 dBm, 0x02 ↦ -137 dBm, ...
 *     SNR < 0: 0x00 ↦ -139 dBm, 0x01 ↦ -138.75 dBm, 0x02 ↦ -138.5 dBm, ...
 *
 * RSSI[MAX]:
 *     LoRa receiver maximum RSSI in dBm from -139 dBm.
 *
 *     0 ↦ -139 dBm, 1 ↦ -138 dBm, ...
 *
 * RSSI[CUR]:
 *     Current RSSI in dBm starting from -139 dBm. 0xff means unspecified
 *
 *     0x00 ↦ -139 dBm, 0x01 ↦ -138 dBm, ..., 0xfe ↦ 115 dBm, 0xff ↦ n.d.
 *
 * SNR:
 *     Signal to noise ratio. Twos' complement in steps of 0.25 dB.
 *
 *     0x00 ↦ 0.00 dB, 0x01 ↦ 0.25 dB, ... 0x7f ↦ 31.75 dB,
 *     0x80 ↦ -32 dB, ..., 0xff ↦ -0.25 dB
 *
 * Sync Word:
 *     LoRa radio sync word. (0x34 = LoRaWAN)
 *
 *
 */
class LoraTapHeader : public Header
{
  public:
    LoraTapHeader(); //!< Default constructor
    /**
     * Constructor initializing LoraTapHeader with the details from the given
     * tag.
     *
     * @param tag The LoRa Tag to read the transmission details from
     */
    LoraTapHeader(const LoraTag& tag);
    ~LoraTapHeader() override; //!< Destructor

    // Methods inherited from Header

    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    /**
     * Return the size required for serialization of this header.
     *
     * @return The serialized size in bytes.
     */
    uint32_t GetSerializedSize() const override;

    /**
     * Serialize the header.
     *
     * See Page 15 of LoraWAN specification for a representation of fields.
     *
     * @param start A pointer to the buffer that will be filled with the
     * serialization.
     */
    void Serialize(Buffer::Iterator start) const override;

    /**
     * Deserialize the contents of the buffer into a LoraTapHeader object.
     *
     * @param start A pointer to the buffer we need to deserialize.
     * @return The number of consumed bytes.
     */
    uint32_t Deserialize(Buffer::Iterator start) override;

    /**
     * Print the header in a human-readable format.
     *
     * @param os The std::ostream on which to print the header.
     */
    void Print(std::ostream& os) const override;

    /**
     * Set the channel frequency used.
     *
     * @param frequencyHz Channel frequency in Hz
     */
    void SetChannelFrequency(uint32_t frequencyHz);

    /**
     * Get the used channel frequency
     */
    uint32_t GetChannelFrequency() const;

    /**
     * Set the channel bandwidth
     *
     * @param bandwidthHz Channel bandwidth in Hz
     *
     * @details This is internally rounded to the closest multiple of 125 kHz
     */
    void SetChannelBandwidth(uint32_t bandwidthHz);

    /**
     * Get the channel bandwidth in Hz
     */
    uint32_t GetChannelBandwidth() const;

    /**
     * Set the spreading factor
     *
     * @param sf The spreacing factor
     */
    void SetSpreadingFactor(uint8_t sf);

    /**
     * Get the spreading factor
     */
    uint8_t GetSpreadingFactor() const;

    /**
     * Set the packet RSSI value in dBm
     *
     * @param rssi The packet RSSI value in dBm
     */
    void SetRssiPacket(double rssi);

    /**
     * Clear the packet RSSI value
     *
     * Store that no packet RSSI info is available
     */
    void ClearRssiPacket();

    /**
     * Is a packet RSSI value provided?
     */
    bool HasRssiPacket() const;

    /**
     * Get the packet RSSI value in dBm
     */
    double GetRssiPacket() const;

    /**
     * Set the maximum receiver RSSI value in dBm
     *
     * @param rssi The maximum receiver RSSI value in dBm
     */
    void SetRssiReceiverMax(double rssi);

    /**
     * Clear the maximum receiver RSSI value
     *
     * Store that no maximum receiver RSSI info is available
     */
    void ClearRssiReceiverMax();

    /**
     * Is a maximum receiver RSSI value provided?
     */
    bool HasRssiReceiverMax() const;

    /**
     * Get the maximum receiver RSSI value in dBm
     */
    double GetRssiReceiverMax() const;

    /**
     * Set the current receiver RSSI value in dBm
     *
     * @param rssi The current receiver RSSI value in dBm
     */
    void SetRssiReceiverCurrent(double rssi);

    /**
     * Clear the current receiver RSSI value
     *
     * Store that no current receiver RSSI info is available
     */
    void ClearRssiReceiverCurrent();

    /**
     * Is a current receiver RSSI value provided?
     */
    bool HasRssiReceiverCurrent() const;

    /**
     * Get the current receiver RSSI value in dBm
     */
    double GetRssiReceiverCurrent() const;

    /**
     * Set the signal to noise ratio in milli-dB
     *
     * @param snr SNR in dB
     */
    void SetSignalToNoise(double snr);

    /**
     * Get the signal to noise ration in milli-dB
     */
    double GetSignalToNoise() const;

    /**
     * Set the sync word (e. g. 0x34 for LoRaWAN)
     */
    void SetSyncWord(uint8_t syncWord);

    /**
     * Get the sync word
     */
    uint8_t GetSyncWord() const;

  private:
    uint32_t m_frequencyHz;    //!< Channel frequency in Hz
    uint8_t m_bandwidthHz;     //!< Channel bandwidth in multiples of 125 kHz
    uint8_t m_spreadingFactor; //!< Spreading factor
    // We cannot go for serialization friendly format for packet RSSI, as
    // serialization format depends on SNR.
    double m_rssiPkt;      //!< Packet RSSI in dBm from -139 dBm in steps of 0.25
                           //!< dBm or 1 dBm, depending on SNR
    uint8_t m_rssiMax;     //!< Maximum receiver RSSI in dBm from -139 dBm
    uint8_t m_rssiCurrent; //!< Current receiver RSSI in dBm from -139 dBm
    int8_t m_snr;          //!< signal to noise ratio in steps of 0.25 dB
    uint8_t m_sync;        //!< sync word
};

} // namespace lorawan

} // namespace ns3
#endif
