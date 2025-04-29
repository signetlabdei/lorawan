/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LORA_TAG_H
#define LORA_TAG_H

#include "ns3/tag.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Tag used to save various data about a packet, like its Spreading Factor and data about
 * interference.
 */
class LoraTag : public Tag
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    /**
     * Create a LoraTag with a given spreading factor and collision.
     *
     * @param sf The Spreading Factor.
     * @param destroyedBy The spreading factor this tag's packet was destroyed by.
     */
    LoraTag(uint8_t sf = 0, uint8_t destroyedBy = 0);

    ~LoraTag() override; //!< Destructor

    void Serialize(TagBuffer i) const override;
    void Deserialize(TagBuffer i) override;
    uint32_t GetSerializedSize() const override;
    void Print(std::ostream& os) const override;

    /**
     * Read which Spreading Factor this packet was transmitted with.
     *
     * @return This tag's packet's spreading factor.
     */
    uint8_t GetSpreadingFactor() const;

    /**
     * Read which Spreading Factor this packet was destroyed by.
     *
     * @return The spreading factor this packet was destroyed by.
     */
    uint8_t GetDestroyedBy() const;

    /**
     * Read the power this packet arrived with.
     *
     * @return This tag's packet received power.
     */
    double GetReceivePower() const;

    /**
     * Set which Spreading Factor this packet was transmitted with.
     *
     * @param sf The Spreading Factor.
     */
    void SetSpreadingFactor(uint8_t sf);

    /**
     * Set which Spreading Factor this packet was destroyed by.
     *
     * @param sf The Spreading Factor.
     */
    void SetDestroyedBy(uint8_t sf);

    /**
     * Set the power this packet was received with.
     *
     * @param receivePower The power, in dBm.
     */
    void SetReceivePower(double receivePower);

    /**
     * Set the frequency of the packet.
     *
     * This value works in two ways:
     * - It is used by the gateway to signal to the network server the frequency of the uplink
     * packet
     * - It is used by the network server to signal to the gateway the frequency of a downlink
     * packet.
     *
     * @param frequencyHz The frequency value [Hz].
     */
    void SetFrequency(uint32_t frequencyHz);

    /**
     * Get the frequency of the packet.
     *
     * This value works in two ways:
     * - It is used by the gateway to signal to the network server the frequency of the uplink
     * packet
     * - It is used by the network server to signal to the gateway the frequency of a downlink
     * packet.
     *
     * @return The frequency value [Hz].
     */
    uint32_t GetFrequency() const;

    /**
     * Get the data rate for this packet.
     *
     * @return The data rate that needs to be employed for this packet.
     */
    uint8_t GetDataRate() const;

    /**
     * Set the data rate for this packet.
     *
     * @param dataRate The data rate.
     */
    void SetDataRate(uint8_t dataRate);

  private:
    uint8_t m_sf;           //!< The Spreading Factor used by the packet.
    uint8_t m_destroyedBy;  //!< The Spreading Factor that destroyed the packet.
    double m_receivePower;  //!< The reception power of this packet.
    uint8_t m_dataRate;     //!< The data rate that needs to be used to send this packet.
    uint32_t m_frequencyHz; //!< The frequency [Hz] of this packet
};
} // namespace lorawan
} // namespace ns3
#endif
