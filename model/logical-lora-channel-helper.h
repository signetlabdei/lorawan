/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LOGICAL_LORA_CHANNEL_HELPER_H
#define LOGICAL_LORA_CHANNEL_HELPER_H

#include "logical-lora-channel.h"
#include "sub-band.h"

#include "ns3/nstime.h"
#include "ns3/ptr.h"
#include "ns3/simple-ref-count.h"

#include <vector>

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * This class supports LorawanMac instances by managing a list of the logical
 * channels that the device is supposed to be using, and establishes their
 * relationship with SubBands.
 *
 * This class also takes into account duty cycle limitations, by updating a list
 * of SubBand objects and providing methods to query whether transmission on a
 * set channel is admissible or not.
 */
class LogicalLoraChannelHelper : public SimpleRefCount<LogicalLoraChannelHelper>
{
  public:
    /**
     * Construct a LogicalLoraChannelHelper of a certain size.
     *
     * @param size The maximum number of transmission channels that can be installed on this device
     * according to regional parameter specifications.
     */
    LogicalLoraChannelHelper(uint8_t size);

    ~LogicalLoraChannelHelper(); //!< Destructor

    /**
     * Get the time it is necessary to wait for before transmitting on a given channel.
     *
     * @param channel A pointer to the channel we want to know the wait time for.
     * @return A Time instance containing the wait time before transmission is allowed on the
     * channel.
     */
    Time GetWaitTime(Ptr<LogicalLoraChannel> channel) const;

    /**
     * Get the time it is necessary to wait for before transmitting on a given channel.
     *
     * @param frequencyHz The channel frequency [Hz] we want to know the wait time of for.
     * @return A Time instance containing the wait time before transmission is allowed on the
     * channel.
     */
    Time GetWaitTime(uint32_t frequencyHz) const;

    /**
     * Register the transmission of a packet.
     *
     * @param duration The duration of the transmission event.
     * @param channel The channel the transmission was made on.
     */
    void AddEvent(Time duration, Ptr<LogicalLoraChannel> channel);

    /**
     * Register the transmission of a packet.
     *
     * @param duration The duration of the transmission event.
     * @param frequencyHz The carrier frequency [Hz] the transmission was on.
     */
    void AddEvent(Time duration, uint32_t frequencyHz);

    /**
     * Get the frequency channel storage array of this device.
     *
     * By specifications, devices are required to hold an indexed structure
     * of a certain size (region-dependent) for storing transmission channels.
     *
     * @remark Empty index slots hold nullptr.
     *
     * @return An indexed vector of pointers to LogicalLoraChannels.
     */
    std::vector<Ptr<LogicalLoraChannel>> GetRawChannelArray() const;

    /**
     * Set a new channel at a fixed index.
     *
     * @param chIndex The index of the channel to substitute.
     * @param channel A pointer to the channel to add to the list.
     */
    void SetChannel(uint8_t chIndex, Ptr<LogicalLoraChannel> channel);

    /**
     * Add a new SubBand.
     *
     * @param subBand A pointer to the SubBand that needs to be added.
     */
    void AddSubBand(Ptr<SubBand> subBand);

    /**
     * Returns the maximum transmission power [dBm] that is allowed on a channel.
     *
     * @param channel The channel in question.
     * @return The power in dBm.
     */
    double GetTxPowerForChannel(Ptr<LogicalLoraChannel> channel) const;

    /**
     * Returns the maximum transmission power [dBm] that is allowed on a channel.
     *
     * @param frequencyHz The carrier frequency [Hz] of the channel in question.
     * @return The power in dBm.
     */
    double GetTxPowerForChannel(uint32_t frequencyHz) const;

    /**
     * Check if a frequency is valid, that is, if it belongs to any of the sub-bands registered in
     * this class.
     *
     * @param frequencyHz The frequency [Hz] to be evaluated.
     * @return Whether the input frequency belongs to any of the registered sub-bands.
     */
    bool IsFrequencyValid(uint32_t frequencyHz) const;

  private:
    /**
     * Get the SubBand a frequency belongs to, also used to test validity of a frequency.
     *
     * @param frequencyHz The frequency [Hz] we want to check.
     * @return The SubBand the frequency belongs to, nullptr if none.
     */
    Ptr<SubBand> GetSubBandFromFrequency(uint32_t frequencyHz) const;

    /**
     * A vector of the SubBands that are currently registered within this helper.
     */
    std::vector<Ptr<SubBand>> m_subBandList;

    /**
     * A vector of the LogicalLoraChannels that are currently registered within
     * this helper. This vector represents the node's channel mask. The first N
     * channels are the default ones for a fixed region.
     */
    std::vector<Ptr<LogicalLoraChannel>> m_channelVec;
};

} // namespace lorawan
} // namespace ns3

#endif /* LOGICAL_LORA_CHANNEL_HELPER_H */
