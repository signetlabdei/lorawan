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

#ifndef LORA_TAG_H
#define LORA_TAG_H

#include "ns3/lora-phy.h"
#include "ns3/nstime.h"
#include "ns3/tag.h"

namespace ns3
{
namespace lorawan
{

/**
 * Tag used to save various data about a packet, like its Spreading Factor and
 * data about interference.
 */
class LoraTag : public Tag
{
  public:
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    /**
     * Create a LoraTag with a given spreading factor and collision.
     *
     * \param sf The Spreading Factor.
     * \param destroyedBy The SF this tag's packet was destroyed by.
     */
    LoraTag();

    ~LoraTag() override;

    uint32_t GetSerializedSize() const override;
    void Serialize(TagBuffer i) const override;
    void Deserialize(TagBuffer i) override;
    void Print(std::ostream& os) const override;

    /**
     * Set the tx parameters for this packet.
     *
     * \param params The tx parameters.
     */
    void SetTxParameters(LoraPhyTxParameters params);

    /**
     * Get the tx parameters for this packet.
     *
     * \return The tx parameters of this packet.
     */
    LoraPhyTxParameters GetTxParameters() const;

    /**
     * Set the data rate of the packet.
     *
     * This value works in two ways:
     * - It is used by the GW to signal to the NS the data rate of the uplink
         packet
     * - It is used by the NS to signal to the GW the data rate of a downlink
         packet
     */
    void SetDataRate(uint8_t datarate);

    /**
     * Get the data rate of the packet.
     */
    uint8_t GetDataRate() const;

    /**
     * Set the frequency of the packet.
     *
     * This value works in two ways:
     * - It is used by the GW to signal to the NS the frequency of the uplink
         packet
     * - It is used by the NS to signal to the GW the freqeuncy of a downlink
         packet
     */
    void SetFrequency(double frequency);

    /**
     * Get the frequency of the packet.
     */
    double GetFrequency() const;

    /**
     * Set which Spreading Factor this packet was destroyed by.
     *
     * \param sf The Spreading Factor.
     */
    void SetDestroyedBy(uint8_t sf);

    /**
     * Read which Spreading Factor this packet was destroyed by.
     *
     * \return The SF this packet was destroyed by.
     */
    uint8_t GetDestroyedBy() const;

    /**
     * Set the reception time for this packet.
     *
     * \param receptionTime The reception time.
     */
    void SetReceptionTime(Time receptionTime);

    /**
     * Get the reception time (demodulation end) for this packet.
     *
     * \return The timestamp at demodulation end of this packet.
     */
    Time GetReceptionTime() const;

    /**
     * Set the power this packet was received with.
     *
     * \param receivePower The power, in dBm.
     */
    void SetReceivePower(double receivePower);

    /**
     * Read the power this packet arrived with.
     *
     * \return This tag's packet received power.
     */
    double GetReceivePower() const;

    /**
     * Set the SNR for this packet.
     *
     * \param snr The SNR.
     */
    void SetSnr(double snr);

    /**
     * Get the SNR for this packet.
     *
     * \return The SNR measured during demodulation of this packet.
     */
    double GetSnr() const;

  private:
    LoraPhyTxParameters m_params; //!< The PHY transmission parameters of this packet
    uint8_t m_dataRate;           //!< The data rate of this packet
    double m_frequency;           //!< The frequency of this packet
    uint8_t m_destroyedBy;        //!< The Spreading Factor that destroyed the packet.
    Time m_receptionTime;         //!< The time at which reception was completed for this packet
    double m_receivePower;        //!< The reception power of this packet.
    double m_snr;                 //!< The SNR of this packet during demodulation
};
} // namespace lorawan
} // namespace ns3
#endif
