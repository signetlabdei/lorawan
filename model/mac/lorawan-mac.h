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
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef LORAWAN_MAC_H
#define LORAWAN_MAC_H

#include "ns3/logical-channel-manager.h"
#include "ns3/lora-phy.h"
#include "ns3/object.h"
#include "ns3/packet.h"

#include <array>

namespace ns3
{
namespace lorawan
{

class LoraPhy;

/**
 * Class representing the LoRaWAN MAC layer.
 *
 * This class is meant to be extended differently based on whether the layer
 * belongs to an End Device or a Gateway, while holding some functionality that
 * is common to both.
 */
class LorawanMac : public Object
{
  public:
    typedef std::array<std::array<uint8_t, 6>, 8> ReplyDataRateMatrix;

    /**
     * \param mac a pointer to the mac which is calling this callback
     * \param packet the packet received
     * \returns true if the callback could handle the packet successfully, false
     *          otherwise.
     */
    typedef Callback<bool, Ptr<LorawanMac>, Ptr<const Packet>> ReceiveCallback;

    static TypeId GetTypeId();

    LorawanMac();
    ~LorawanMac() override;

    /**
     * Send a packet.
     *
     * \param packet The packet to send.
     */
    virtual void Send(Ptr<Packet> packet) = 0;

    /**
     * Perform actions after sending a packet.
     *
     * \param packet The packet that just finished transmission.
     */
    virtual void TxFinished(Ptr<const Packet> packet) = 0;

    /**
     * Receive a packet from the lower layer.
     *
     * \param packet the received packet
     */
    virtual void Receive(Ptr<const Packet> packet) = 0;

    /**
     * Function called by lower layers to inform this layer that reception of a
     * packet we were locked on failed.
     *
     * \param packet the packet we failed to receive
     */
    virtual void FailedReception(Ptr<const Packet> packet) = 0;

    /**
     * Set the number of PHY preamble symbols this MAC is set to use.
     *
     * \param nPreambleSymbols The number of preamble symbols to use (typically 8).
     */
    void SetNPreambleSymbols(uint16_t nPreambleSymbols);

    /**
     * Get the number of PHY preamble symbols this MAC is set to use.
     *
     * \return The number of preamble symbols to use (typically 8).
     */
    uint16_t GetNPreambleSymbols() const;

    /**
     * Get the symbol time based on the datarate
     * \param dr The datarate
     * \return The symbol time
     */
    Time GetTSym(uint8_t dr);

    /**
     * \param cb callback to invoke whenever a packet has been received and must
     *        be forwarded to the higher layers.
     *
     * Set the callback to be used to notify higher layers when a packet has been
     * received.
     */
    virtual void SetReceiveCallback(ReceiveCallback cb);

    /**
     * Set the device this MAC layer is installed on.
     *
     * \param device The NetDevice this MAC layer will refer to.
     */
    void SetDevice(Ptr<NetDevice> device);

    /**
     * Get the device this MAC layer is installed on.
     *
     * \return The NetDevice this MAC layer will refer to.
     */
    Ptr<NetDevice> GetDevice();

    /**
     * Set the underlying PHY layer
     *
     * \param phy the phy layer
     */
    void SetPhy(Ptr<LoraPhy> phy);

    /**
     * Get the underlying PHY layer
     *
     * \return The PHY layer that this MAC is connected to.
     */
    Ptr<LoraPhy> GetPhy();

    /**
     * Get the logical lora channel helper associated with this MAC.
     *
     * \return The instance of LogicalChannelManager that this MAC is using.
     */
    Ptr<LogicalChannelManager> GetLogicalChannelManager();

    /**
     * Set the LogicalChannelManager this MAC instance will use.
     *
     * \param helper The instance of the helper to use.
     */
    void SetLogicalChannelManager(Ptr<LogicalChannelManager> helper);

    /**
     * Get the SF corresponding to a data rate, based on this MAC's region.
     *
     * \param dataRate The Data Rate we need to convert to a Spreading Factor
     * value.
     * \return The SF that corresponds to a Data Rate in this MAC's region, or 0
     * if the dataRate is not valid.
     */
    uint8_t GetSfFromDataRate(uint8_t dataRate);

    /**
     * Get the BW corresponding to a data rate, based on this MAC's region
     *
     * \param dataRate The Data Rate we need to convert to a bandwidth value.
     * \return The bandwidth that corresponds to the parameter Data Rate in this
     * MAC's region, or 0 if the dataRate is not valid.
     */
    double GetBandwidthFromDataRate(uint8_t dataRate);

    /**
     * Get the transmission power in dBm that corresponds, in this region, to the
     * encoded 8-bit txPower.
     *
     * \param txPower The 8-bit encoded txPower to convert.
     *
     * \return The corresponding transmission power in dBm, or 0 if the encoded
     * power was not recognized as valid.
     */
    double GetDbmForTxPower(uint8_t txPower);

    /**
     * Set the vector to use to check up correspondence between SF and DataRate.
     *
     * \param sfForDataRate A vector that contains at position i the SF that
     * should correspond to DR i.
     */
    void SetSfForDataRate(std::vector<uint8_t> sfForDataRate);

    /**
     * Set the vector to use to check up correspondence between bandwidth and
     * DataRate.
     *
     * \param bandwidthForDataRate A vector that contains at position i the
     * bandwidth that should correspond to DR i in this MAC's region.
     */
    void SetBandwidthForDataRate(std::vector<double> bandwidthForDataRate);

    /**
     * Set the maximum App layer payload for a set DataRate.
     *
     * \param maxMacPayloadForDataRate A vector that contains at position i the
     * maximum Application layer payload that should correspond to DR i in this
     * MAC's region.
     */
    void SetMaxMacPayloadForDataRate(std::vector<uint32_t> maxMacPayloadForDataRate);

    /**
     * Set the vector to use to check up which transmission power in Dbm
     * corresponds to a certain TxPower value in this MAC's region.
     *
     * \param txDbmForTxPower A vector that contains at position i the
     * transmission power in dBm that should correspond to a TXPOWER value of i in
     * this MAC's region.
     */
    void SetTxDbmForTxPower(std::vector<double> txDbmForTxPower);

    /**
     * Set the matrix to use when deciding with which DataRate to respond. Region
     * based.
     *
     * \param replyDataRateMatrix A matrix containing the reply DataRates, based
     * on the sending DataRate and on the value of the RX1DROffset parameter.
     */
    void SetReplyDataRateMatrix(ReplyDataRateMatrix replyDataRateMatrix);

  protected:
    void DoDispose() override;

    ReceiveCallback m_receiveCallback; ///<! Callback to forward to upper layers

    /**
     * The tx parameters to use for transmitting.
     */
    LoraPhyTxParameters m_txParams;

    Ptr<LoraPhy> m_phy;      //!< The PHY instance that sits under this MAC layer.
    Ptr<NetDevice> m_device; //!< The device this MAC layer is installed on.
    Ptr<LogicalChannelManager> m_channelManager; //!< The channel manager assigned to this MAC.

    /**
     * A vector holding the SF each Data Rate corresponds to.
     */
    std::vector<uint8_t> m_sfForDataRate;

    /**
     * A vector holding the bandwidth each Data Rate corresponds to.
     */
    std::vector<double> m_bandwidthForDataRate;

    /**
     * A vector holding the maximum app payload size that corresponds to a
     * certain DataRate.
     */
    std::vector<uint32_t> m_maxMacPayloadForDataRate;

    /**
     * A vector holding the power that corresponds to a certain TxPower value.
     */
    std::vector<double> m_txDbmForTxPower;

    /**
     * The matrix that decides the DR the GW will use in a reply based on the ED's
     * sending DR and on the value of the RX1DROffset parameter.
     */
    ReplyDataRateMatrix m_replyDataRateMatrix;

    /**
     * The trace source that is fired when a packet cannot be sent because of duty
     * cycle limitations.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>> m_cannotSendBecauseDutyCycle;

    /**
     * Trace source that is fired when a new packet is sent by the MAC layer.
     */
    TracedCallback<Ptr<const Packet>> m_sentNewPacket;

    /**
     * Trace source that is fired when a packet reaches the MAC layer from PHY.
     */
    TracedCallback<Ptr<const Packet>> m_receivedPacket;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORAWAN_MAC_H */
