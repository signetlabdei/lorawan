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

#ifndef LORA_PHY_H
#define LORA_PHY_H

#include "ns3/lora-channel.h"
#include "ns3/lora-interference-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"

namespace ns3
{
namespace lorawan
{

class LoraChannel;

/**
 * Structure to collect all parameters that are used to compute the duration of
 * a packet (excluding payload length).
 */
struct LoraPhyTxParameters
{
    uint8_t sf = 12;                         //!< Spreading Factor
    bool headerDisabled = 0;                 //!< Whether to use implicit header mode
    uint8_t codingRate = 1;                  //!< Code rate (obtained as 4/(codingRate+4))
    double bandwidthHz = 125000;             //!< Bandwidth in Hz
    uint16_t nPreamble = 8;                  //!< Number of preamble symbols
    bool crcEnabled = 1;                     //!< Whether Cyclic Redundancy Check is enabled
    bool lowDataRateOptimizationEnabled = 0; //!< Whether Low Data Rate Optimization is enabled
};

/**
 * Allow logging of LoraPhyTxParameters like with any other data type.
 */
std::ostream& operator<<(std::ostream& os, const LoraPhyTxParameters& params);

/**
 * \ingroup lorawan
 *
 * Base class for PHY layers implementing the LoRa modulation scheme.
 *
 * This class features common callbacks and defines the interfaces that are used
 * to send and receive packets at the PHY layer. Furthermore, it features an
 * implementation of the GetTimeOnAir function, used to compute the actual
 * duration of a packet based on a series of parameters that are collected in
 * LoraPhyTxParameters objects.
 */
class LoraPhy : public Object
{
  public:
    // TypeId
    static TypeId GetTypeId();

    /**
     * Constructor and destructor
     */
    LoraPhy();
    ~LoraPhy() override;

    /**
     * Instruct the PHY to send a packet according to some parameters.
     *
     * \param packet The packet to send.
     * \param txParams The desired transmission parameters.
     * \param frequency The frequency on which to transmit.
     * \param txPowerDbm The power in dBm with which to transmit the packet.
     */
    virtual void Send(Ptr<Packet> packet,
                      LoraPhyTxParameters txParams,
                      double frequency,
                      double txPowerDbm) = 0;

    /**
     * Start receiving a packet.
     *
     * This method is typically called by LoraChannel.
     *
     * \param packet The packet that is arriving at this PHY layer.
     * \param rxPowerDbm The power of the arriving packet (assumed to be constant
     * for the whole reception).
     * \param sf The Spreading Factor of the arriving packet.
     * \param duration The on air time of this packet.
     * \param frequency The frequency this packet is being transmitted on.
     */
    virtual void StartReceive(Ptr<Packet> packet,
                              double rxPowerDbm,
                              uint8_t sf,
                              Time duration,
                              double frequency) = 0;

    /**
     * Whether this device is transmitting or not.
     *
     * \returns true if the device is currently transmitting a packet, false
     * otherwise.
     */
    virtual bool IsTransmitting() = 0;

    /**
     * Type definition for a callback for when a packet is correctly received.
     *
     * This callback can be set by an upper layer that wishes to be informed of
     * correct reception events.
     */
    typedef Callback<void, Ptr<const Packet>> RxOkCallback;

    /**
     * Set the callback to call upon successful reception of a packet.
     *
     * This method is typically called by an upper MAC layer that wants to be
     * notified after the successful reception of a packet.
     */
    void SetReceiveOkCallback(RxOkCallback callback);

    /**
     * Type definition for a callback for when a packet reception fails.
     *
     * This callback can be set by an upper layer that wishes to be informed of
     * failed reception events.
     */
    typedef Callback<void, Ptr<const Packet>> RxFailedCallback;

    /**
     * Set the callback to call upon failed reception of a packet we were
     * previously locked on.
     *
     * This method is typically called by an upper MAC layer that wants to be
     * notified after the failed reception of a packet.
     */
    void SetReceiveFailedCallback(RxFailedCallback callback);

    /**
     * Type definition for a callback to call when a packet has finished sending.
     *
     * This callback is used by the MAC layer, to determine when to open a receive
     * window.
     */
    typedef Callback<void, Ptr<const Packet>> TxFinishedCallback;

    /**
     * Set the callback to call after transmission of a packet.
     *
     * This method is typically called by an upper MAC layer that wants to be
     * notified after the transmission of a packet.
     */
    void SetTxFinishedCallback(TxFinishedCallback callback);

    /**
     * Sets the interference helper.
     *
     * \param helper the interference helper
     */
    virtual void SetInterferenceHelper(const Ptr<LoraInterferenceHelper> helper);

    /**
     * Set the LoraChannel instance PHY transmits on.
     *
     * Typically, there is only one instance per simulation.
     *
     * \param channel The LoraChannel instance this PHY will transmit on.
     */
    virtual void SetChannel(Ptr<LoraChannel> channel);

    /**
     * Get the channel instance associated to this PHY.
     *
     * \return The LoraChannel instance this PHY transmits on.
     */
    Ptr<LoraChannel> GetChannel() const;

    /**
     * Get the mobility model associated to this PHY.
     *
     * \return The MobilityModel associated to this PHY.
     */
    Ptr<MobilityModel> GetMobility() const;

    /**
     * Set the mobility model associated to this PHY.
     *
     * \param mobility The mobility model to associate to this PHY.
     */
    void SetMobility(Ptr<MobilityModel> mobility);

    /**
     * Get the NetDevice associated to this PHY.
     *
     * \return The NetDevice associated to this PHY.
     */
    Ptr<NetDevice> GetDevice() const;

    /**
     * Set the NetDevice that owns this PHY.
     *
     * \param device The NetDevice this PHY will reference as its owner.
     */
    void SetDevice(Ptr<NetDevice> device);

    /**
     * Compute the symbol time from SF and BW.
     *
     * \param sf The SF
     * \param bw The BW
     * \return TSym, the time required to send a LoRa modulation symbol.
     */
    static Time GetTSym(const LoraPhyTxParameters& txParams);

    /**
     * Compute the time that a packet with certain characteristics will take to be
     * transmitted.
     *
     * Besides from the ones saved in LoraPhyTxParameters, the packet's payload
     * (obtained through a GetSize () call to accout for the presence of Headers
     * and Trailers, too) also influences the packet transmit time.
     *
     * \param packet The packet that needs to be transmitted.
     * \param txParams The set of parameters that will be used for transmission.
     * \return The time necessary to transmit the packet.
     */
    static Time GetTimeOnAir(Ptr<const Packet> packet, const LoraPhyTxParameters& txParams);

    /**
     * Compute the Signal to Noise Ratio (SNR) from the transmission power
     * measured at packet reception.
     *
     * \param transmissionPower The reception transmission power (dBm)
     * \param bandwidth The bandwidth used to transmit (Hz)
     * \return The SNR value in dB.
     */
    static double RxPowerToSNR(double transmissionPower, double bandwidth = 125000);

  protected:
    void DoInitialize() override;
    void DoDispose() override;

    /**
     * Finish reception of a packet.
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

    // Member objects
    Ptr<NetDevice> m_device;                    //!< The net device this PHY is attached to.
    Ptr<LoraChannel> m_channel;                 //!< The channel this PHY transmits on.
    Ptr<LoraInterferenceHelper> m_interference; //!< The InterferenceHelper associated to this PHY.

    // Callbacks (communication with MAC layer)
    RxOkCallback m_rxOkCallback;             //! Callback to perform upon correct reception
    RxFailedCallback m_rxFailedCallback;     //! Callback to perform upon failed reception
    TxFinishedCallback m_txFinishedCallback; //! Callback to perform upon transmission end

    // Trace sources
    uint32_t m_nodeId; //!< Node Id to correctly format context in traced callbacks

    /**
     * The trace source fired when a packet is sent.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_startSending;

    /**
     * The trace source fired when a packet begins the reception process from the
     * medium.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>> m_phyRxBeginTrace;

    /**
     * The trace source fired when a packet reception ends.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>> m_phyRxEndTrace;

    /**
     * The trace source fired when a packet was correctly received.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_successfullyReceivedPacket;

    /**
     * The trace source fired when a packet cannot be received because its power
     * is below the sensitivity threshold.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_underSensitivity;

    /**
     * The trace source fired when a packet cannot be correctly received because
     * of interference.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_interferedPacket;

    /**
     * The trace source fired when a received packet is sniffed.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>> m_phySniffRxTrace;

    /**
     * The trace source fired when a transmitted packet is sniffed.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<Ptr<const Packet>> m_phySniffTxTrace;

  private:
    Ptr<MobilityModel> m_mobility; //!< The mobility model associated to this PHY.
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_PHY_H */
