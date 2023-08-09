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

#ifndef LOGICAL_CHANNEL_MANAGER_H
#define LOGICAL_CHANNEL_MANAGER_H

#include "ns3/logical-channel.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/packet.h"
#include "ns3/sub-band.h"

#include <iterator>
#include <list>
#include <map>
#include <vector>

namespace ns3
{
namespace lorawan
{

/**
 * This class supports LorawanMac instances by managing a list of the logical
 * channels that the device is supposed to be using, and establishes their
 * relationship with SubBands.
 *
 * This class also takes into account duty cycle limitations, by updating a list
 * of SubBand objects and providing methods to query whether transmission on a
 * set channel is admissible or not.
 */
class LogicalChannelManager : public Object
{
  public:
    static TypeId GetTypeId();

    LogicalChannelManager();
    ~LogicalChannelManager() override;

    /**
     * Get the time it is necessary to wait before transmitting again, according
     * to the aggregate duty cycle parameter and the duration of the last packet.
     *
     * \param aggregatedDutyCycle The parameter
     * \return The aggregate waiting time.
     */
    Time GetAggregatedWaitingTime(double aggregatedDutyCycle);

    /**
     * Get the time it is necessary to wait for before transmitting on a given
     * channel.
     *
     * \remark This function does not take into account aggregate waiting time.
     * Check on this should be performed before calling this function.
     *
     * \param channel A pointer to the channel we want to know the waiting time
     * for.
     * \return A Time instance containing the waiting time before transmission is
     * allowed on the channel.
     */
    Time GetWaitingTime(const Ptr<LogicalChannel> channel);

    /**
     * Preemptively register the transmission of a packet.
     *
     * \param duration The duration of the transmission event.
     * \param channel The channel the transmission is made on.
     */
    void AddEvent(Time duration, Ptr<LogicalChannel> channel);

    /**
     * Get the list of LogicalChannels currently registered on this helper.
     *
     * \return A list of the managed channels.
     */
    std::vector<Ptr<LogicalChannel>> GetChannelList();

    /**
     * Get the list of LogicalChannels currently registered on this helper
     * that have been enabled for Uplink transmission with the channel mask.
     *
     * \return A list of the managed channels enabled for Uplink transmission.
     */
    std::vector<Ptr<LogicalChannel>> GetEnabledChannelList();

    /**
     *  Get a pointer to the LogicalChannel at a certain index.
     *
     *  \param chIndex The index of the channel to get.
     */
    Ptr<LogicalChannel> GetChannel(uint8_t chIndex);

    /**
     * Add a new channel at a fixed index.
     *
     * \param chIndex The index of the channel to substitute.
     * \param logicalChannel A pointer to the channel to add to the list.
     */
    void AddChannel(uint8_t chIndex, Ptr<LogicalChannel> logicalChannel);

    /**
     * Set a different reply frequency of a channel.
     *
     * \param chIndex The index of the channel to change the reply freq. of.
     * \param replyFrequency Frequency (Hz) to be set as reply freq.
     */
    void SetReplyFrequency(uint8_t chIndex, double replyFrequency);

    /**
     * Add a new SubBand to this helper.
     *
     * \param firstFrequency The first frequency of the subband, in Hz.
     * \param lastFrequency The last frequency of the subband, in Hz.
     * \param dutyCycle The duty cycle that needs to be enforced on this subband.
     * \param maxTxPowerDbm The maximum transmission power [dBm] that can be used
     * on this SubBand.
     */
    void AddSubBand(double firstFrequency,
                    double lastFrequency,
                    double dutyCycle,
                    double maxTxPowerDbm);

    /**
     * Add a new SubBand.
     *
     * \param subBand A pointer to the SubBand that needs to be added.
     */
    void AddSubBand(Ptr<SubBand> subBand);

    /**
     * Remove a channel.
     *
     * \param chIndex Index of the channel we want to remove.
     */
    void RemoveChannel(uint8_t chIndex);

    /**
     * Returns the maximum transmission power [dBm] that is allowed on a channel.
     *
     * \param logicalChannel The power for which to check the maximum allowed
     * transmission power.
     * \return The power in dBm.
     */
    double GetTxPowerForChannel(Ptr<LogicalChannel> logicalChannel);

    /**
     * Get the SubBand a channel belongs to.
     *
     * \param channel The channel whose SubBand we want to get.
     * \return The SubBand the channel belongs to.
     */
    Ptr<SubBand> GetSubBandFromChannel(const Ptr<LogicalChannel> channel);

    /**
     * Get the SubBand a frequency belongs to.
     *
     * \param frequency The frequency we want to check.
     * \return The SubBand the frequency belongs to.
     */
    Ptr<SubBand> GetSubBandFromFrequency(double frequency);

    /**
     * Disable the channel at a specified index.
     *
     * \param chIndex The index of the channel to disable.
     */
    void DisableChannel(uint8_t chIndex);

  protected:
    void DoDispose() override;

  private:
    /**
     * A list of the SubBands that are currently registered within this helper.
     */
    std::list<Ptr<SubBand>> m_subBandList;

    /**
     * A vector of the LogicalChannels that are currently registered within
     * this helper. This vector represents the node's channel mask. The first N
     * channels are the default ones for a fixed region.
     */
    std::map<uint8_t, Ptr<LogicalChannel>> m_channelList;

    Time m_lastTxDuration; //!< Duration of the last frame (seconds).

    Time m_lastTxStart; //!< Timestamp of the last trasmission start.
};
} // namespace lorawan

} // namespace ns3
#endif /* LOGICAL_CHANNEL_MANAGER_H */
