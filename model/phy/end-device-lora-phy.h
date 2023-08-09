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
 * Authors: Davide Magrin <magrinda@dei.unipd.it>,
 *          Michele Luvisotto <michele.luvisotto@dei.unipd.it>
 *          Stefano Romagnolo <romagnolostefano93@gmail.com>
 *
 * 17/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef END_DEVICE_LORA_PHY_H
#define END_DEVICE_LORA_PHY_H

#include "ns3/lora-device-address.h"
#include "ns3/lora-phy.h"
#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/traced-value.h"

namespace ns3
{
namespace lorawan
{

class LoraChannel;

/**
 * Receive notifications about PHY events.
 */
class EndDeviceLoraPhyListener
{
  public:
    virtual ~EndDeviceLoraPhyListener();

    /**
     * We have received the first bit of a packet. We decided
     * that we could synchronize on this packet. It does not mean
     * we will be able to successfully receive completely the
     * whole packet. It means that we will report a BUSY status until
     * one of the following happens:
     *   - NotifyRxEndOk
     *   - NotifyRxEndError
     *   - NotifyTxStart
     *
     * \param duration the expected duration of the packet reception.
     */
    virtual void NotifyRxStart() = 0;

    /**
     * We are about to send the first bit of the packet.
     * We do not send any event to notify the end of
     * transmission. Listeners should assume that the
     * channel implicitely reverts to the idle state
     * unless they have received a cca busy report.
     *
     * \param duration the expected transmission duration.
     * \param txPowerDbm the nominal tx power in dBm
     */
    virtual void NotifyTxStart(double txPowerDbm) = 0;

    /**
     * Notify listeners that we went to sleep
     */
    virtual void NotifySleep() = 0;

    /**
     * Notify listeners that we woke up
     */
    virtual void NotifyStandby() = 0;
};

/**
 * Class representing a LoRa transceiver.
 *
 * This class inherits some functionality by LoraPhy, like the GetTimeOnAir
 * function, and extends it to represent the behavior of a LoRa chip, like the
 * SX1272.
 *
 * Additional behaviors featured in this class include a State member variable
 * that expresses the current state of the device (SLEEP, TX, RX or STANDBY),
 * and a frequency and Spreading Factor this device is listening to when in
 * STANDBY mode. After transmission and reception, the device returns
 * automatically to STANDBY mode. The decision of when to go into SLEEP mode
 * is delegateed to an upper layer, which can modify the state of the device
 * through the public SwitchToSleep and SwitchToStandby methods. In SLEEP
 * mode, the device cannot lock on a packet and start reception.
 */
class EndDeviceLoraPhy : public LoraPhy
{
  public:
    /**
     * An enumeration of the possible states of an EndDeviceLoraPhy.
     * It makes sense to define a state for End Devices since there's only one
     * demodulator which can either send, receive, stay idle or go in a deep
     * sleep state.
     */
    enum State
    {
        /**
         * The PHY layer is sleeping.
         * During sleep, the device is not listening for incoming messages.
         */
        SLEEP,

        /**
         * The PHY layer is in STANDBY.
         * When the PHY is in this state, it's listening to the channel, and
         * it's also ready to transmit data passed to it by the MAC layer.
         */
        STANDBY,

        /**
         * The PHY layer is sending a packet.
         * During transmission, the device cannot receive any packet or send
         * any additional packet.
         */
        TX,

        /**
         * The PHY layer is receiving a packet.
         * While the device is locked on an incoming packet, transmission is
         * not possible.
         */
        RX
    };

    static TypeId GetTypeId();

    // Constructor and destructor
    EndDeviceLoraPhy();
    ~EndDeviceLoraPhy() override;

    // Implementation of LoraPhy's pure virtual functions
    void Send(Ptr<Packet> packet,
              LoraPhyTxParameters txParams,
              double frequency,
              double txPowerDbm) override;

    // Implementation of LoraPhy's pure virtual functions
    void StartReceive(Ptr<Packet> packet,
                      double rxPowerDbm,
                      uint8_t sf,
                      Time duration,
                      double frequency) override;

    // Implementation of LoraPhy's pure virtual functions
    bool IsTransmitting() override;

    /**
     * Switch to the STANDBY state.
     */
    void SwitchToStandby();

    /**
     * Switch to the SLEEP state.
     */
    void SwitchToSleep();

    /**
     * Set the frequency this EndDevice will listen on.
     *
     * Should a packet be transmitted on a frequency different than that the
     * EndDeviceLoraPhy is listening on, the packet will be discarded.
     *
     * \param frequency The frequency [Hz] to listen to.
     */
    void SetRxFrequency(double frequency);

    /**
     * Set the Spreading Factor this EndDevice will listen for.
     *
     * The EndDeviceLoraPhy object will not be able to lock on transmissions that
     * use a different SF than the one it's listening for.
     *
     * \param sf The spreading factor to listen for.
     */
    void SetRxSpreadingFactor(uint8_t sf);

    /**
     * Return the state this End Device is currently in.
     *
     * \return The state this EndDeviceLoraPhy is currently in.
     */
    EndDeviceLoraPhy::State GetState();

    /**
     * Add the input listener to the list of objects to be notified of PHY-level
     * events.
     *
     * \param listener the new listener
     */
    void RegisterListener(EndDeviceLoraPhyListener* listener);

    /**
     * Remove the input listener from the list of objects to be notified of
     * PHY-level events.
     *
     * \param listener the listener to be unregistered
     */
    void UnregisterListener(EndDeviceLoraPhyListener* listener);

    /**
     * Set the network address of this device.
     *
     * \param address The address to set.
     */
    void SetDeviceAddress(LoraDeviceAddress address);

  protected:
    void DoDispose() override;

    // Implementation of LoraPhy's pure virtual functions
    void EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event) override;

    /**
     * Compute the shorter duration of packets being filtered
     * early during reception for being uplink or for being
     * destined to another device
     */
    Time GetFilteredDuration(Ptr<const Packet> packet, Time duration) const;

    /**
     * Internal call when transmission finishes.
     */
    void TxFinished(Ptr<Packet> packet);

    /**
     * Switch to the RX state
     */
    void SwitchToRx();

    /**
     * Switch to the TX state
     */
    void SwitchToTx(double txPowerDbm);

    TracedValue<State> m_state; //!< The state this PHY is currently in.
    uint8_t m_rxSf;             //!< The Spreading Factor this device is listening for
    double m_rxFrequency;       //!< The frequency this device is listening on

    /**
     * The address of this device.
     * Set by MAC layer.
     */
    LoraDeviceAddress m_address;

    static const double sensitivity[6]; //!< The sensitivity vector of this device to different SFs

    std::vector<EndDeviceLoraPhyListener*> m_listeners; //!< PHY listeners

    /**
     * Trace source for when a packet is lost because it was using a SF different from
     * the one this EndDeviceLoraPhy was configured to listen for.
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_wrongSf;

    /**
     * Trace source for when a packet is lost because it was transmitted on a
     * frequency different from the one this EndDeviceLoraPhy was configured to
     * listen on.
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_wrongFrequency;
};

} // namespace lorawan

} // namespace ns3
#endif /* END_DEVICE_LORA_PHY_H */
