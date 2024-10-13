/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
 * \ingroup lorawan
 *
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
    /**
     *  Register this type.
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    LogicalLoraChannel();           //!< Default constructor
    ~LogicalLoraChannel() override; //!< Destructor

    /**
     * Construct a new LogicalLoraChannel object initializing the carrier frequency.
     *
     * \param frequency The carrier frequency [MHz].
     */
    LogicalLoraChannel(double frequency);

    /**
     * Constructor providing initialization of frequency and data rate limits.
     *
     * \param frequency This channel's frequency [MHz].
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

    // void SetFrequency (double frequencyMHz);

    /**
     * Set the minimum data rate that is allowed on this channel.
     *
     * \param minDataRate The minimum data rate value.
     */
    void SetMinimumDataRate(uint8_t minDataRate);

    /**
     * Set the maximum data rate that is allowed on this channel.
     *
     * \param maxDataRate The maximum data rate value.
     */
    void SetMaximumDataRate(uint8_t maxDataRate);

    /**
     * Get the minimum data rate that is allowed on this channel.
     *
     * \return The minimum data rate value.
     */
    uint8_t GetMinimumDataRate() const;

    /**
     * Get the maximum data rate that is allowed on this channel.
     *
     * \return The maximum data rate value.
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
     *
     * \return True if the channel can be used for uplink, false otherwise.
     */
    bool IsEnabledForUplink() const;

  private:
    double m_frequency;      //!< The central frequency of this channel, in MHz.
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
