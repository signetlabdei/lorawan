/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 *         Martina Capuzzo <capuzzom@dei.unipd.it>
 *
 * Modified by: Peggy Anderson <peggy.anderson@usask.ca>
 */

#ifndef CLASS_A_END_DEVICE_LORAWAN_MAC_H
#define CLASS_A_END_DEVICE_LORAWAN_MAC_H

#include "end-device-lorawan-mac.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * Class representing the MAC layer of a Class A LoRaWAN device.
 */
class ClassAEndDeviceLorawanMac : public EndDeviceLorawanMac
{
  public:
    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    ClassAEndDeviceLorawanMac();           //!< Default constructor
    ~ClassAEndDeviceLorawanMac() override; //!< Destructor

    //////////////////////////
    //  Receiving methods   //
    //////////////////////////

    /**
     * Receive a packet.
     *
     * This method is typically registered as a callback in the underlying PHY
     * layer so that it's called when a packet is going up the stack.
     *
     * @param packet The received packet.
     */
    void Receive(Ptr<const Packet> packet) override;

    /**
     * Function called by lower layers to inform this layer that reception of a
     * packet we were locked on failed.
     *
     * @param packet The packet we failed to receive.
     */
    void FailedReception(Ptr<const Packet> packet) override;

    /**
     * Perform the actions that are required after a packet send.
     *
     * This function handles opening of the first receive window.
     *
     * @param packet The packet that has just been transmitted.
     */
    void TxFinished(Ptr<const Packet> packet) override;

    /////////////////////////
    // Getters and Setters //
    /////////////////////////

    /**
     * Get the data rate that will be used in the first receive window.
     *
     * @return The data rate.
     */
    uint8_t GetFirstReceiveWindowDataRate();

    /**
     * Set the data rate to be used in the second receive window.
     *
     * @param dataRate The data rate.
     */
    void SetSecondReceiveWindowDataRate(uint8_t dataRate);

    /**
     * Get the data rate that will be used in the second receive window.
     *
     * @return The data rate.
     */
    uint8_t GetSecondReceiveWindowDataRate() const;

    /**
     * Set the frequency that will be used for the second receive window.
     *
     * @param frequencyHz The Frequency.
     */
    void SetSecondReceiveWindowFrequency(uint32_t frequencyHz);

    /**
     * Get the frequency that is used for the second receive window.
     *
     * @return The frequency, in Hz.
     */
    uint32_t GetSecondReceiveWindowFrequency() const;

    /////////////////////////
    // MAC command methods //
    /////////////////////////

    void OnRxParamSetupReq(uint8_t rx1DrOffset, uint8_t rx2DataRate, double frequencyHz) override;

  protected:
    /////////////////////
    // Sending methods //
    /////////////////////

    /**
     * Compute a minimum wait time value based on the current busy state of the device.
     *
     * A Class A device is considered busy while it is in the process of sending and opening the 2
     * protocol-mandated reception windows.
     *
     * @return The wait Time value.
     */
    Time GetNextClassTransmissionDelay() const override;

    /**
     * Send a packet with the sending function of the physical layer.
     *
     * @param packet The packet to send.
     */
    void SendToPhy(Ptr<Packet> packet) override;

  private:
    enum RxOutcome
    {
        ACK,
        RECV,
        FAIL,
        NONE
    };

    /**
     * Perform operations needed to open the first receive window.
     */
    void OpenFirstReceiveWindow();

    /**
     * Perform operations needed to open the second receive window.
     */
    void OpenSecondReceiveWindow();

    /**
     * Perform operations needed to close the first receive window.
     */
    void CloseFirstReceiveWindow();

    /**
     * Perform operations needed to close the second receive window.
     */
    void CloseSecondReceiveWindow();

    /**
     * Decide whether we can retransmit based on reception outcome.
     *
     * \param outcome Outcome of the reception.
     */
    void ManageRetransmissions(RxOutcome outcome);

    EventId m_secondReceiveWindow; //!< The event of the second receive window opening, used
                                   //!< to cancel the second window in case the first one is
                                   //!< successful.

    // Reception window parameters

    Time m_receiveDelay1; //!< The interval between when a packet is done sending and when the first
                          //!< receive window is opened.
    uint32_t m_firstReceiveWindowFrequencyHz; //!< The frequency [Hz] to listen on for the first
                                              //!< receive window. This value is set dynamically to
                                              //!< the last uplink transmission frequency.
    uint8_t m_rx1DrOffset;                    //!< The RX1DROffset parameter value.

    Time m_receiveDelay2; //!< The interval between when a packet is done sending and when the
                          //!< second receive window is opened.
    uint32_t m_secondReceiveWindowFrequencyHz; //!< The frequency [Hz] to listen on for the second
                                               //!< receive window.
    uint8_t m_secondReceiveWindowDataRate;     //!< The data rate to listen for during the second
                                               //!< downlink transmission.

    // Rescheduling purposes

    bool m_isSecondWindowOpen; //!< Whether the second reception window is currently open
};

} /* namespace lorawan */
} /* namespace ns3 */

#endif /* CLASS_A_END_DEVICE_LORAWAN_MAC_H */
