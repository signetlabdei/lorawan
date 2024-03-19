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

#ifndef LORA_PHY_H
#define LORA_PHY_H

#include "lora-channel.h"
#include "lora-interference-helper.h"

#include "ns3/callback.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/object.h"

#include <list>

namespace ns3
{
namespace lorawan
{

class LoraChannel;

/**
 * \ingroup lorawan
 *
 * Structure to collect all parameters that are used to compute the duration of
 * a packet (excluding payload length).
 */
struct LoraTxParameters
{
    uint8_t sf = 7;                              //!< Spreading Factor
    bool headerDisabled = false;                 //!< Whether to use implicit header mode
    uint8_t codingRate = 1;                      //!< Code rate (obtained as 4/(codingRate+4))
    double bandwidthHz = 125000;                 //!< Bandwidth in Hz
    uint32_t nPreamble = 8;                      //!< Number of preamble symbols
    bool crcEnabled = true;                      //!< Whether Cyclic Redundancy Check is enabled
    bool lowDataRateOptimizationEnabled = false; //!< Whether Low Data Rate Optimization is enabled
};

/**
 * Allow logging of LoraTxParameters like with any other data type.
 */
std::ostream& operator<<(std::ostream& os, const LoraTxParameters& params);

/**
 * \ingroup lorawan
 *
 * \brief Base class for PHY layers implementing the LoRa modulation scheme.
 *
 * This class features common callbacks and defines the interfaces that are used
 * to send and receive packets at the PHY layer. Furthermore, it features an
 * implementation of the GetOnAirTime function, used to compute the actual
 * duration of a packet based on a series of parameters that are collected in
 * LoraTxParameters objects.
 */
class LoraPhy : public Object
{
  public:
    /**
     *  Register this type.
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    LoraPhy();           //!< Default constructor
    ~LoraPhy() override; //!< Destructor

    /**
     * Type definition for a callback for when a packet is correctly received.
     *
     * This callback can be set by an upper layer that wishes to be informed of
     * correct reception events.
     */
    typedef Callback<void, Ptr<const Packet>> RxOkCallback;

    /**
     * Type definition for a callback for when a packet reception fails.
     *
     * This callback can be set by an upper layer that wishes to be informed of
     * failed reception events.
     */
    typedef Callback<void, Ptr<const Packet>> RxFailedCallback;

    /**
     * Type definition for a callback to call when a packet has finished sending.
     *
     * This callback is used by the MAC layer, to determine when to open a receive
     * window.
     */
    typedef Callback<void, Ptr<const Packet>> TxFinishedCallback;

    /**
     * \brief Start receiving a packet.
     *
     * This method is typically called by LoraChannel.
     *
     * \param packet The packet that is arriving at this PHY layer.
     * \param rxPowerDbm The power of the arriving packet (assumed to be constant for the whole
     * reception).
     * \param sf The Spreading Factor of the arriving packet.
     * \param duration The on air time of this packet.
     * \param frequencyMHz The frequency this packet is being transmitted on.
     */
    virtual void StartReceive(Ptr<Packet> packet,
                              double rxPowerDbm,
                              uint8_t sf,
                              Time duration,
                              double frequencyMHz) = 0;

    /**
     * \brief Finish reception of a packet.
     *
     * This method is scheduled by StartReceive, based on the packet duration. By
     * passing a LoraInterferenceHelper Event to this method, the class will be
     * able to identify the packet that is being received among all those that
     * were registered as interference by StartReceive.
     *
     * \param packet The received packet.
     * \param event The event that is tied to this packet in the
     * LoraInterferenceHelper.
     */
    virtual void EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event) = 0;

    /**
     * \brief Instruct the PHY to send a packet according to some parameters.
     *
     * \param packet The packet to send.
     * \param txParams The desired transmission parameters.
     * \param frequencyMHz The frequency on which to transmit.
     * \param txPowerDbm The power in dBm with which to transmit the packet.
     */
    virtual void Send(Ptr<Packet> packet,
                      LoraTxParameters txParams,
                      double frequencyMHz,
                      double txPowerDbm) = 0;

    /**
     * \brief Whether this device is transmitting or not.
     *
     * \returns true if the device is currently transmitting a packet, false
     * otherwise.
     */
    virtual bool IsTransmitting() = 0;

    /**
     * \brief Whether this device is listening on the specified frequency or not.
     *
     * \param frequency The frequency to query.
     * \returns true if the device is listening on that frequency, false
     * otherwise.
     */
    virtual bool IsOnFrequency(double frequency) = 0;

    /**
     * \brief Set the callback to call upon successful reception of a packet.
     *
     * This method is typically called by an upper MAC layer that wants to be
     * notified after the successful reception of a packet.
     *
     * \param callback The RxOkCallback instance.
     */
    void SetReceiveOkCallback(RxOkCallback callback);

    /**
     * \brief Set the callback to call upon failed reception of a packet we were
     * previously locked on.
     *
     * This method is typically called by an upper MAC layer that wants to be
     * notified after the failed reception of a packet.
     *
     * \param callback The RxFailedCallback instance.
     */
    void SetReceiveFailedCallback(RxFailedCallback callback);

    /**
     * \brief Set the callback to call after transmission of a packet.
     *
     * This method is typically called by an upper MAC layer that wants to be
     * notified after the transmission of a packet.
     *
     * \param callback The TxFinishedCallback instance.
     */
    void SetTxFinishedCallback(TxFinishedCallback callback);

    /**
     * \brief Get the mobility model associated to this PHY.
     *
     * \return The MobilityModel associated to this PHY.
     */
    Ptr<MobilityModel> GetMobility();

    /**
     * \brief Set the mobility model associated to this PHY.
     *
     * \param mobility The mobility model to associate to this PHY.
     */
    void SetMobility(Ptr<MobilityModel> mobility);

    /**
     * \brief Set the LoraChannel instance PHY transmits on.
     *
     * Typically, there is only one instance per simulation.
     *
     * \param channel The LoraChannel instance this PHY will transmit on.
     */
    void SetChannel(Ptr<LoraChannel> channel);

    /**
     * \brief Get the channel instance associated to this PHY.
     *
     * \return The LoraChannel instance this PHY transmits on.
     */
    Ptr<LoraChannel> GetChannel() const;

    /**
     * \brief Get the NetDevice associated to this PHY.
     *
     * \return The NetDevice associated to this PHY.
     */
    Ptr<NetDevice> GetDevice() const;

    /**
     * \brief Set the NetDevice that owns this PHY.
     *
     * \param device The NetDevice this PHY will reference as its owner.
     */
    void SetDevice(Ptr<NetDevice> device);

    /**
     * \brief Compute the symbol time from SF and BW.
     *
     * \param txParams The parameters for transmission
     * \return TSym, the time required to send a LoRa modulation symbol.
     */
    static Time GetTSym(LoraTxParameters txParams);

    /**
     * \brief Compute the time that a packet with certain characteristics will take to be
     * transmitted.
     *
     * Besides from the ones saved in LoraTxParameters, the packet's payload
     * (obtained through a GetSize () call to account for the presence of Headers
     * and Trailers, too) also influences the packet transmit time.
     *
     * \param packet The packet that needs to be transmitted.
     * \param txParams The set of parameters that will be used for transmission.
     * \return The time necessary to transmit the packet.
     */
    static Time GetOnAirTime(Ptr<Packet> packet, LoraTxParameters txParams);

  private:
    Ptr<MobilityModel> m_mobility; //!< The mobility model associated to this PHY.

  protected:
    // Member objects

    Ptr<NetDevice> m_device; //!< The net device this PHY is attached to.

    Ptr<LoraChannel> m_channel; //!< The channel this PHY transmits on.

    LoraInterferenceHelper m_interference; //!< The LoraInterferenceHelper associated to this PHY.

    // Trace sources

    /**
     * The trace source fired when a packet is sent.
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_startSending;

    /**
     * The trace source fired when a packet begins the reception process from the
     * medium.
     */
    TracedCallback<Ptr<const Packet>> m_phyRxBeginTrace;

    /**
     * The trace source fired when a packet reception ends.
     */
    TracedCallback<Ptr<const Packet>> m_phyRxEndTrace;

    /**
     * The trace source fired when a packet was correctly received.
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_successfullyReceivedPacket;

    /**
     * The trace source fired when a packet cannot be received because its power
     * is below the sensitivity threshold.
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_underSensitivity;

    /**
     * The trace source fired when a packet cannot be correctly received because
     * of interference.
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_interferedPacket;

    // Callbacks

    /**
     * The callback to perform upon correct reception of a packet.
     */
    RxOkCallback m_rxOkCallback;

    /**
     * The callback to perform upon failed reception of a packet we were locked on.
     */
    RxFailedCallback m_rxFailedCallback;

    /**
     * The callback to perform upon the end of a transmission.
     */
    TxFinishedCallback m_txFinishedCallback;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_PHY_H */
