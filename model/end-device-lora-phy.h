/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Authors: Davide Magrin <magrinda@dei.unipd.it>,
 *          Michele Luvisotto <michele.luvisotto@dei.unipd.it>
 *          Stefano Romagnolo <romagnolostefano93@gmail.com>
 */

#ifndef END_DEVICE_LORA_PHY_H
#define END_DEVICE_LORA_PHY_H

#include "lora-phy.h"

#include "ns3/traced-value.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Receive notifications about PHY events.
 */
class EndDeviceLoraPhyListener
{
  public:
    virtual ~EndDeviceLoraPhyListener(); //!< Destructor

    /**
     * We have received the first bit of a packet. We decided
     * that we could synchronize on this packet. It does not mean
     * we will be able to successfully receive completely the
     * whole packet. It means that we will report a BUSY status until
     * one of the following happens:
     *   - NotifyRxEndOk
     *   - NotifyRxEndError
     *   - NotifyTxStart
     */
    virtual void NotifyRxStart() = 0;

    /**
     * We are about to send the first bit of the packet.
     * We do not send any event to notify the end of
     * transmission. Listeners should assume that the
     * channel implicitly reverts to the idle state
     * unless they have received a cca busy report.
     *
     * @param txPowerDbm The nominal tx power in dBm.
     */
    virtual void NotifyTxStart(double txPowerDbm) = 0;

    /**
     * Notify listeners that we went to sleep.
     */
    virtual void NotifySleep() = 0;

    /**
     * Notify listeners that we woke up.
     */
    virtual void NotifyStandby() = 0;
};

/**
 * @ingroup lorawan
 *
 * Class representing a LoRa transceiver hardware state-machine of a SX1272 LoRa chip
 * (see SX1272/73 Datasheet, Rev. 4, Jan. 2019).
 *
 * This class inherits the base functions of LoraPhy, like the GetTimeOnAir function, and provides
 * Radio Abstraction Layer (RAL) functionality to parent classes. It also implements a ReceiveSingle
 * function for external classes to start a timed reception attempt.
 *
 * Internally, this class features a State member variable that expresses the current hardware state
 * of the device (SLEEP, STANDBY, TX, RX_ENABLED, RX_ACTIVE), and a structure representing the
 * chip configurations registers. After transmission and reception, the device returns automatically
 * to the STANDBY state. The decision of when to go into SLEEP is delegated to an external class,
 * which can modify the state of the device through the Sleep method.
 *
 * Even if could appear at first glance, these states are different from the operating modes defined
 * in the datasheet. Modes are higher level transceiver configurations that can involve multiple
 * internal states (see RX enabled/active for instance). Here, these modes are loosely mapped to
 * member functions for driving the PHY layer from a higher layer:
 *
 * - Send(): TX mode
 * - ReceiveSingle(): RXSINGLE mode
 * - Sleep(): SLEEP mode
 *
 * @todo Implementation of the RXCONTINUOUS mode
 * @todo Implementation of the CAD mode
 *
 * The datasheet tells us that you can go from any mode to any other mode, but for simplicity's sake
 * we assume that you are not able to interrupt an ongoing TX or RX window. Moreover, since it
 * currently has no practical use, we do not allow manually switching to STANDBY mode.
 *
 * Transitions marked with 'a' are automatic:
 * @verbatim
 *                          +-------+
 *                +-------- | SLEEP | --------+
 *               /          +-------+          \
 *              /               ^               \
 *             v                |                v
 * +------------+          +---------+          +----+
 * | RX_ENABLED |  <-----  | STANDBY |  ----->  | TX |
 * +------------+  --a-->  +---------+  <--a--  +----+
 *             \            ^
 *            a \        a /
 *               v        /
 *             +-----------+
 *             | RX_ACTIVE |
 *             +-----------+
 * @endverbatim
 *
 * Peculiarities about the radio error model and about how errors are supposed to be handled during
 * transmission and reception are left to classes extending this one, like SimpleEndDeviceLoraPhy or
 * SpectrumEndDeviceLoraPhy. These classes need to implement the pure virtual member function Send,
 * StartReceive and EndReceive.
 */
class EndDeviceLoraPhy : public LoraPhy
{
  public:
    /**
     * Type definition for a callback for when a packet reception hardware timeout expires.
     *
     * This callback can be set by an upper layer that wishes to be informed of reception timeout
     * events.
     */
    typedef Callback<void> RxTimeoutCallback;

    /**
     * An enumeration of the possible internal states of an EndDeviceLoraPhy. It makes
     * sense to define a state for End Devices since there's only one demodulator which can either
     * transmit, receive, be idle or go in a deep sleep state. See the description of the states for
     * more details on the possible transitions of the PHY state-machine.
     *
     * @note Even if could appear at first glance, these states are different from the operating
     * modes defined in the datasheet. Modes are higher level transceiver configurations that can
     * involve multiple internal states (see RX enabled/active for instance).
     */
    enum class State
    {
        /**
         * The PHY layer is in low-power sleep state. No reception or transmission can happen.
         * The only reachable states from this one are TX and RX_ENABLED.
         */
        SLEEP,

        /**
         * The PHY layer is in standby mode. This is the default in-between state where only the
         * chip's components common to both RX and TX are powered on. All other states are reachable
         * from this one with the exception of RX_ACTIVE.
         */
        STANDBY,

        /**
         * The PHY layer is transmitting a packet. During the transmission, the device is busy and
         * cannot receive any packet or send any additional packet. The only reachable state from
         * this one is STANDBY, and the switch should be automatically handled by the chip.
         */
        TX,

        /**
         * The PHY layer is listening to the channel for a valid transmission preamble. While the
         * device in this process, it is busy and transmission is not possible. The states
         * reachable from this one are RX_ACTIVE or STANDBY.
         *
         * If a premble is found, the PHY transitions to RX_ACTIVE and starts receiving a packet.
         * Otherwise, the PHY remains in this state until manually reset to STANDBY.
         *
         * In RXSINGLE mode an interrupt to STANDBY is usually scheduled after a certain amount of
         * time to create a reception window.
         *
         * @todo In RXCONTINUOUS mode, this must be done manually by the user.
         */
        RX_ENABLED,

        /**
         * The PHY layer is actively receiving a transmission after locking onto a preamble. While
         * the device in this process, it is busy and transmission is not possible. The states
         * reachable from this one are either STANDBY or RX_ENABLED, and the switch to either should
         * be automatically handled by the chip depending on the mode (RXSINGLE or RXCONTINUOUS).
         *
         * In RXSINGLE mode, the PHY is reset to STANDBY after reception ends.
         *
         * @todo In RXCONTINUOUS mode, the PHY goes back to RX_ENABLED instead.
         */
        RX_ACTIVE

        // NOTE: When extending/updating, please update operator<< accordingly.
    };

    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    EndDeviceLoraPhy();           //!< Default constructor
    ~EndDeviceLoraPhy() override; //!< Destructor

    static const double SENSITIVITY[6]; //!< The sensitivity vector of this device to different SFs

    // Forward LoraPhy's pure virtual function
    void Send(Ptr<Packet> packet,
              uint32_t frequencyHz,
              IQPolarity iqPolarity,
              const LoraTxParameters& txParams,
              double txPowerDbm) override = 0;

    // Forward LoraPhy's pure virtual function
    void StartReceive(Ptr<Packet> packet,
                      uint32_t frequencyHz,
                      IQPolarity iqPolarity,
                      uint8_t spreadingFactor,
                      double rxPowerDbm,
                      Time duration) override = 0;

    // Implementation of LoraPhy's pure virtual function
    bool IsTransmitting() const override;

    // Implementation of LoraPhy's pure virtual function
    bool IsOnFrequency(uint32_t frequencyHz) const override;

    /**
     * Return the internal state this end device is currently in.
     *
     * @return The internal state.
     */
    State GetState();

    /**
     * Set this PHY LoRa chip to sleep from standby after a transmission / reception.
     */
    void Sleep();

    /**
     * This function starts a reception attempt that will time-out if no transmission preamble is
     * detected. Input parameters are written in the PHY state and used for reception.
     *
     * @note Basic LoRa transceivers as the one modeled here are only able to listen for a distinct
     * spreading factor on a single frequency channel; all other transmissions will be discarded.
     *
     * @param frequencyHz Expected central frequency [Hz] of the incoming transmission
     * @param iqPolarity Whether to expect an uplink or downlink signal
     * @param spreadingFactor Expected Spreading Factor (SF) of the incoming transmission
     * @param bandwidthHz Expected bandwidth [Hz] of the incoming transmission
     * @param symbNumTimeout The reception timeout duration in number of symbols
     * @param rxTimeoutCallback Optional callback executed on reception timeout
     */
    void ReceiveSingle(uint32_t frequencyHz,
                       IQPolarity iqPolarity,
                       uint8_t spreadingFactor,
                       uint32_t bandwidthHz,
                       uint8_t symbNumTimeout,
                       RxTimeoutCallback rxTimeoutCallback);

    /**
     * Add the input listener to the list of objects to be notified of PHY-level
     * events.
     *
     * @param listener The new listener.
     */
    void RegisterListener(EndDeviceLoraPhyListener* listener);

    /**
     * Remove the input listener from the list of objects to be notified of
     * PHY-level events.
     *
     * @param listener The listener to be unregistered.
     */
    void UnregisterListener(EndDeviceLoraPhyListener* listener);

  protected:
    /**
     * Parameters affecting the internal PHY transmission / reception of a packet,
     * normally stored in the chip registers.
     *
     * Some parameters have slightly different meaning depending on the follow-up mode activated.
     *
     * For transmission, the symbol number timeout has no effect.
     *
     * @todo For reception, the payload length represents the maximum number of packet Bytes that
     * are accepted. The the premble length represents the minimum number of preamble symbols to
     * expect from the transmitter (if unknown, it should be set to max). With implicit header mode,
     * the payload length, coding rate and CRC validation must be also set explicitly.
     *
     * @note Currently, only frequencyHz, bandwidthHz, iqPolarity, spreadingFactor, symbNumTimeout
     * and txPowerDbm play a stateful role during device operation. The rest is implemented for
     * future expansions.
     */
    struct EndDeviceLoraRegisters
    {
        // Modulation parameters
        uint8_t spreadingFactor = 7;                //!< Symbol Spreading Factor (SF)
        uint32_t bandwidthHz = 125'000;             //!< Transmission bandwidth in Hz
        CodingRate codingRate = CodingRate::CR_4_5; //!< Transmission coding rate
        bool lowDataRateOptimize = false; //!< Low Data Rate Optimization (mandated for SF11/12)
        // PHY packet parameters
        uint16_t preambleLenSymb = 8; //!< Number of symbols in the packet preamble
        uint8_t payloadLenBytes = 1;  //!< Number of Bytes the packet payload
        bool implicitHeader = false;  //!< Whether to use implicit header mode
        bool crcEnabled = true;       //!< Whether Cyclic Redundancy Check (CRC) is enabled
        IQPolarity iqPolarity = IQPolarity::UP; //!< Whether to process an uplink or downlink signal
        // Base parameters
        uint32_t frequencyHz = 868'100'000; //!< The transmission central frequency [Hz]
        int8_t txPowerDbm = 14;     //!< The output power [dBm] to use for packet transmission
        uint8_t syncWord = 0x34;    //!< The LoRa sync. word (0x34 is reserved for LoRaWAN)
        uint8_t symbNumTimeout = 8; //!< The reception timeout duration in number of symbols
    };

    // Implementation of LoraPhy's pure virtual function
    void TxFinished(Ptr<const Packet> packet) override;

    // Radio Abstraction Layer

    /**
     * Request a switch to SLEEP mode.
     *
     * This only has an effect when in STANDBY state.
     */
    void RequestSleepMode();

    /**
     * This function puts the PHY in transmission mode. Parent classes must take care of scheduling
     * TxFinished() to automatically revert to STANDBY.
     *
     * @warning Before calling this function, parent classes must ensure that the device is in
     * SLEEP or STANDBY mode: Registers can only be accessed when in these modes.
     */
    void RequestTxMode();

    /**
     * This function reads RX parameters from the internal registers of the PHY hardware and begins
     * a reception attempt that will time-out if no transmission preamble is detected, automatically
     * reverting to STANDBY. Parent classes must take care of calling DoStartReceive() in case the
     * chip actually starts receiving from the channel during the reception attempt.
     *
     * @warning Before calling this function, parent classes must ensure that the device is in
     * SLEEP or STANDBY mode: Registers can only be accessed when in these modes.
     */
    void RequestRxSingleMode();

    /**
     * This function should be called by parent classes to signal that the PHY is starting to
     * demodulate a transmission. Internally, it cancels the RX attempt interrupt timeout and
     * switches from RX_ENABLED to RX_ACTIVE. Parent classes must take care of calling
     * DoEndReceive() before invoking any upper layer callback (RxOk, RxFailed)
     *
     * @warning Before calling this function, parent classes must ensure that the device is in
     * the RX_ENABLED state
     */
    void DoStartReceive();

    /**
     * This function should be called by parent classes to signal that the PHY is finishing to
     * demodulate a transmission. Internally, it switches from RX_ACTIVE to STANDBY
     *
     * @warning Before calling this function, parent classes must ensure that the device is in
     * the RX_ACTIVE state
     */
    void DoEndReceive();

    /**
     * Trace source for when a packet is lost because it was transmitted on a frequency different
     * from the one this EndDeviceLoraPhy was configured to listen on.
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_wrongFrequency;

    /**
     * Trace source for when a packet is lost because it was transmitted with a different I/Q
     * polarity (uplink or downlink) from the one this EndDeviceLoraPhy was configured to listen on.
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_wrongPolarity;

    /**
     * Trace source for when a packet is lost because it was using a spreading factor different from
     * the one this EndDeviceLoraPhy was configured to listen for.
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_wrongSf;

    EndDeviceLoraRegisters m_regs; //!< High level model of LoRa chip registers

  private:
    // Forward LoraPhy's pure virtual function
    void EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event) override = 0;

    /**
     * Callback for scheduling the end of an unsuccessful timed reception attempt.
     *
     * This is meant to model the hardware interrupt of real LoRa transceivers.
     */
    void RxTimeout();

    // Unsafe handles for hardware state switching

    /**
     * Switch to the SLEEP state.
     *
     * This fails if not in STANDBY state.
     */
    void SwitchToSleep();

    /**
     * Switch to the STANDBY state.
     *
     * @note This function is reserved for automatically switching to STANDBY after TX/RX
     *
     * This fails if in SLEEP state or already in STANDBY state.
     */
    void SwitchToStandBy();

    /**
     * Switch to the TX state.
     *
     * This fails if not in SLEEP or STANDBY state.
     */
    void SwitchToTx();

    /**
     * Switch to the RX_ENABLED state.
     *
     * This fails if not in SLEEP or STANDBY state.
     */
    void SwitchToRxEnabled();

    /**
     * Update the RX state to RX_ACTIVE on preamble lock.
     *
     * @note This function will cancel any scheduled RX timeout event
     *
     * This fails if not in RX_ENABLED state.
     */
    void SwitchToRxActive();

    /**
     * The callback to perform upon reception timeout. In reality, this is an hardware interrupt.
     */
    RxTimeoutCallback m_rxTimeoutCallback;

    TracedValue<State> m_state; //!< The state this PHY is currently in.
    EventId m_rxTimeoutEvent;   //!< Event for timed switch from RX_ENABLED to STANDBY

    std::vector<EndDeviceLoraPhyListener*> m_listeners; //!< PHY listeners
};

/**
 *  Overloaded operator to print the value of a EndDeviceLoraPhy::State.
 *
 *  @param os The output stream
 *  @param state The enum value of the PHY state
 *  @return The output stream with text value of the PHY state
 */
std::ostream& operator<<(std::ostream& os, const EndDeviceLoraPhy::State& state);

} // namespace lorawan
} // namespace ns3

#endif /* END_DEVICE_LORA_PHY_H */
