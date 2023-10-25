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

#ifndef LOGICAL_LORA_CHANNEL_H
#define LOGICAL_LORA_CHANNEL_H

#include "sub-band.h"

#include "ns3/object.h"

namespace ns3
{
namespace lorawan
{

class SubBand;

/**
 * This class represents a logical LoRaWAN channel.
 *
 * A logical channel is characterized by a central frequency and a range of data
 * rates that can be sent on it.
 *
 * Furthermore, a LogicalLoraChannel can be marked as enabled or disabled for
 * uplink transmission.
 */
class LogicalLoraChannel : public Object
{
  public:
    static TypeId GetTypeId();

    LogicalLoraChannel();
    ~LogicalLoraChannel() override;

    LogicalLoraChannel(double frequency);

    /**
     * Constructor providing initialization of frequency and data rate limits.
     *
     * \param frequency This channel's frequency.
     * \param minDataRate This channel's minimum data rate.
     * \param maxDataRate This channel's maximum data rate.
     */
    LogicalLoraChannel(double frequency, uint8_t minDataRate, uint8_t maxDataRate);

    /**
     * Get the frequency (MHz).
     *
     * \return The center frequency of this channel.
     */
    double GetFrequency() const;

    /**
     * Set the frequency (MHz).
     *
     * \param frequencyMHz The center frequency this channel should be at.
     */
    // void SetFrequency (double frequencyMHz);

    /**
     * Set the minimum Data Rate that is allowed on this channel.
     */
    void SetMinimumDataRate(uint8_t minDataRate);

    /**
     * Set the maximum Data Rate that is allowed on this channel.
     */
    void SetMaximumDataRate(uint8_t maxDataRate);

    /**
     * Get the minimum Data Rate that is allowed on this channel.
     */
    uint8_t GetMinimumDataRate() const;

    /**
     * Get the maximum Data Rate that is allowed on this channel.
     */
    uint8_t GetMaximumDataRate() const;

    /**
     * Set this channel as enabled for uplink.
     */
    void SetEnabledForUplink();

    /**
     * Set this channel as disabled for uplink.
     */
    void DisableForUplink();

    /**
     * Test whether this channel is marked as enabled for uplink.
     */
    bool IsEnabledForUplink() const;

  private:
    /**
     * The central frequency of this channel, in MHz.
     */
    double m_frequency;

    /**
     * The minimum Data Rate that is allowed on this channel.
     */
    uint8_t m_minDataRate;

    /**
     * The maximum Data Rate that is allowed on this channel.
     */
    uint8_t m_maxDataRate;

    /**
     * Whether this channel can be used for uplink or not.
     */
    bool m_enabledForUplink;
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
