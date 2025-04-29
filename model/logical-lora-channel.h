/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LOGICAL_LORA_CHANNEL_H
#define LOGICAL_LORA_CHANNEL_H

#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * This class represents a logical LoRaWAN channel.
 *
 * A logical channel is characterized by a central frequency and a range of data
 * rates that can be sent on it.
 *
 * Furthermore, a LogicalLoraChannel can be marked as enabled or disabled for
 * uplink transmission.
 */
class LogicalLoraChannel : public SimpleRefCount<LogicalLoraChannel>
{
  public:
    /**
     * Constructor providing initialization of frequency and data rate limits.
     *
     * @param frequencyHz This channel's frequency [Hz].
     * @param minDataRate This channel's minimum data rate.
     * @param maxDataRate This channel's maximum data rate.
     */
    LogicalLoraChannel(uint32_t frequencyHz, uint8_t minDataRate, uint8_t maxDataRate);

    /**
     * Get the frequency (Hz).
     *
     * @return The center frequency of this channel.
     */
    uint32_t GetFrequency() const;

    /**
     * Get the minimum data rate that is allowed on this channel.
     *
     * @return The minimum data rate value.
     */
    uint8_t GetMinimumDataRate() const;

    /**
     * Get the maximum data rate that is allowed on this channel.
     *
     * @return The maximum data rate value.
     */
    uint8_t GetMaximumDataRate() const;

    /**
     * Set this channel as enabled for uplink.
     */
    void EnableForUplink();

    /**
     * Set this channel as disabled for uplink.
     */
    void DisableForUplink();

    /**
     * Test whether this channel is marked as enabled for uplink.
     *
     * @return True if the channel can be used for uplink, false otherwise.
     */
    bool IsEnabledForUplink() const;

  private:
    uint32_t m_frequencyHz;  //!< The central frequency of this channel, in Hz.
    uint8_t m_minDataRate;   //!< The minimum data rate that is allowed on this channel.
    uint8_t m_maxDataRate;   //!< The maximum data rate that is allowed on this channel.
    bool m_enabledForUplink; //!< Whether this channel can be used for uplink or not.
};

/**
 * Overload of the == operator to compare different instances of the same LogicalLoraChannel
 */
bool operator==(const Ptr<LogicalLoraChannel>& first, const Ptr<LogicalLoraChannel>& second);

/**
 * Overload the != operator to compare different instances of the same LogicalLoraChannel
 */
bool operator!=(const Ptr<LogicalLoraChannel>& first, const Ptr<LogicalLoraChannel>& second);

} // namespace lorawan

} // namespace ns3
#endif /* LOGICAL_LORA_CHANNEL_H */
