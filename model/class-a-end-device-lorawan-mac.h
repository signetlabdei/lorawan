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
 *         Martina Capuzzo <capuzzom@dei.unipd.it>
 *
 * Modified by: Peggy Anderson <peggy.anderson@usask.ca>
 */

#ifndef CLASS_A_END_DEVICE_LORAWAN_MAC_H
#define CLASS_A_END_DEVICE_LORAWAN_MAC_H

#include "end-device-lorawan-mac.h" // EndDeviceLorawanMac
#include "lora-frame-header.h"      // RxParamSetupReq
#include "lorawan-mac.h"            // Packet
// #include "ns3/random-variable-stream.h"
#include "lora-device-address.h"

// #include "ns3/traced-value.h"

namespace ns3
{
namespace lorawan
{

/**
 * \ingroup lorawan
 *
 * Class representing the MAC layer of a Class A LoRaWAN device.
 */
class ClassAEndDeviceLorawanMac : public EndDeviceLorawanMac
{
  public:
    /**
     *  Register this type.
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    ClassAEndDeviceLorawanMac();
    ~ClassAEndDeviceLorawanMac() override;

    /////////////////////
    // Sending methods //
    /////////////////////

    /**
     * \brief Add headers and send a packet with the sending function of the physical layer.
     *
     * \param packet the packet to send
     */
    void SendToPhy(Ptr<Packet> packet) override;

    //////////////////////////
    //  Receiving methods   //
    //////////////////////////

    /**
     * \brief Receive a packet.
     *
     * This method is typically registered as a callback in the underlying PHY
     * layer so that it's called when a packet is going up the stack.
     *
     * \param packet the received packet.
     */
    void Receive(Ptr<const Packet> packet) override;

    void FailedReception(Ptr<const Packet> packet) override;

    void TxFinished(Ptr<const Packet> packet) override;

    /**
     * \brief Perform operations needed to open the first receive window.
     */
    void OpenFirstReceiveWindow();

    /**
     * \brief Perform operations needed to open the second receive window.
     */
    void OpenSecondReceiveWindow();

    /**
     * \brief Perform operations needed to close the first receive window.
     */
    void CloseFirstReceiveWindow();

    /**
     * \brief Perform operations needed to close the second receive window.
     */
    void CloseSecondReceiveWindow();

    /////////////////////////
    // Getters and Setters //
    /////////////////////////

    /**
     * \brief Find the minimum waiting time before the next possible transmission based
     * on End Device's Class Type.
     *
     * \param waitingTime The minimum waiting time that has to be respected,
     * irrespective of the class (e.g., because of duty cycle limitations).
     * \return The Time value.
     */
    Time GetNextClassTransmissionDelay(Time waitingTime) override;

    /**
     * \brief Get the Data Rate that will be used in the first receive window.
     *
     * \return The Data Rate
     */
    uint8_t GetFirstReceiveWindowDataRate();

    /**
     * \brief Set the Data Rate to be used in the second receive window.
     *
     * \param dataRate The Data Rate.
     */
    void SetSecondReceiveWindowDataRate(uint8_t dataRate);

    /**
     * \brief Get the Data Rate that will be used in the second receive window.
     *
     * \return The Data Rate
     */
    uint8_t GetSecondReceiveWindowDataRate() const;

    /**
     * \brief Set the frequency that will be used for the second receive window.
     *
     * \param frequencyMHz the Frequency.
     */
    void SetSecondReceiveWindowFrequency(double frequencyMHz);

    /**
     * \brief Get the frequency that is used for the second receive window.
     *
     * \return The frequency, in MHz
     */
    double GetSecondReceiveWindowFrequency() const;

    /////////////////////////
    // MAC command methods //
    /////////////////////////

    /**
     * \brief Perform the actions that need to be taken when receiving a RxParamSetupReq
     * command based on the Device's Class Type.
     *
     * \param rxParamSetupReq The Parameter Setup Request, which contains:
     *                            - The offset to set.
     *                            - The data rate to use for the second receive window.
     *                            - The frequency to use for the second receive window.
     */
    void OnRxClassParamSetupReq(Ptr<RxParamSetupReq> rxParamSetupReq) override;

  private:
    Time m_receiveDelay1; //!< The interval between when a packet is done sending and when the first
                          //!< receive window is opened.

    /**
     * The interval between when a packet is done sending and when the second
     * receive window is opened.
     */
    Time m_receiveDelay2;

    /**
     * The event of the closing the first receive window.
     *
     * This Event will be canceled if there's a successful reception of a packet.
     */
    EventId m_closeFirstWindow;

    /**
     * The event of the closing the second receive window.
     *
     * This Event will be canceled if there's a successful reception of a packet.
     */
    EventId m_closeSecondWindow;

    /**
     * The event of the second receive window opening.
     *
     * This Event is used to cancel the second window in case the first one is
     * successful.
     */
    EventId m_secondReceiveWindow;

    /**
     * The frequency to listen on for the second receive window.
     */
    double m_secondReceiveWindowFrequency;

    /**
     * The Data Rate to listen for during the second downlink transmission.
     */
    uint8_t m_secondReceiveWindowDataRate;

    /**
     * The RX1DROffset parameter value
     */
    uint8_t m_rx1DrOffset;

}; /* ClassAEndDeviceLorawanMac */
} /* namespace lorawan */
} /* namespace ns3 */
#endif /* CLASS_A_END_DEVICE_LORAWAN_MAC_H */
