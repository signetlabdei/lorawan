/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef SUB_BAND_H
#define SUB_BAND_H

#include "logical-lora-channel.h"

#include "ns3/nstime.h"
#include "ns3/simple-ref-count.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Class representing a SubBand, i.e., a frequency band subject to some
 * regulations on duty cycle and transmission power.
 */
class SubBand : public SimpleRefCount<SubBand>
{
  public:
    /**
     * Create a new SubBand by specifying all of its properties.
     *
     * @param firstFrequencyHz The SubBand's lowest frequency [Hz].
     * @param lastFrequencyHz The SubBand's highest frequency [Hz].
     * @param dutyCycle The duty cycle (as a fraction) allowed on this SubBand.
     * @param maxTxPowerDbm The maximum transmission power [dBm] allowed on this SubBand.
     */
    SubBand(uint32_t firstFrequencyHz,
            uint32_t lastFrequencyHz,
            double dutyCycle,
            double maxTxPowerDbm);

    /**
     * Get the lowest frequency of the SubBand.
     *
     * @return The lowest frequency [Hz] of the SubBand.
     */
    uint32_t GetFirstFrequency() const;

    /**
     * Get the highest frequency of the SubBand.
     *
     * @return The highest frequency [Hz] of the SubBand.
     */
    uint32_t GetLastFrequency() const;

    /**
     * Get the duty cycle of the subband.
     *
     * @return The duty cycle (as a fraction) that needs to be enforced on this
     * SubBand.
     */
    double GetDutyCycle() const;

    /**
     * Update the next transmission time.
     *
     * This function is used by LogicalLoraChannelHelper, which computes the time
     * based on the SubBand's duty cycle and on the transmission duration.
     *
     * @param nextTime The future time from which transmission should be allowed
     * again.
     */
    void SetNextTransmissionTime(Time nextTime);

    /**
     * Returns the next time from which transmission on this subband will be
     * possible.
     *
     * @return The next time at which transmission in this SubBand will be
     * allowed.
     */
    Time GetNextTransmissionTime();

    /**
     * Return whether or not a frequency belongs to this SubBand.
     *
     * @param frequencyHz The frequency [Hz] we want to test against the current subband.
     * @return True if the frequency is between firstFrequencyHz and lastFrequencyHz,
     * false otherwise.
     */
    bool Contains(uint32_t frequencyHz) const;

    /**
     * Return whether or not a channel belongs to this SubBand.
     *
     * @param channel The channel we want to test against the current subband.
     * @return Whether the channel's center frequency is between the first and last frequency of the
     * sub-band, margins excluded.
     */
    bool Contains(Ptr<const LogicalLoraChannel> channel) const;

    /**
     * Set the maximum transmission power that is allowed on this SubBand.
     *
     * @param maxTxPowerDbm The maximum transmission power [dBm] to set.
     */
    void SetMaxTxPowerDbm(double maxTxPowerDbm);

    /**
     * Return the maximum transmission power that is allowed on this SubBand.
     *
     * @return The maximum transmission power, in dBm.
     */
    double GetMaxTxPowerDbm() const;

  private:
    uint32_t m_firstFrequencyHz; //!< Starting frequency of the subband, in Hz
    uint32_t m_lastFrequencyHz;  //!< Ending frequency of the subband, in Hz
    double m_dutyCycle;          //!< The duty cycle that needs to be enforced on this subband
    Time m_nextTransmissionTime; //!< The next time a transmission will be allowed in this subband
    double m_maxTxPowerDbm; //!< The maximum transmission power that is admitted on this subband
};
} // namespace lorawan
} // namespace ns3
#endif /* SUB_BAND_H */
