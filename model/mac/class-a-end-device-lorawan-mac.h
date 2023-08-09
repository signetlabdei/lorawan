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
 *
 * 11/01/2023
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef CLASS_A_END_DEVICE_LORAWAN_MAC_H
#define CLASS_A_END_DEVICE_LORAWAN_MAC_H

#include "ns3/base-end-device-lorawan-mac.h"
#include "ns3/recv-window-manager.h"

// #include "ns3/traced-value.h"

namespace ns3
{
namespace lorawan
{

/**
 * Class representing the MAC layer of a Class A LoRaWAN device.
 */
class ClassAEndDeviceLorawanMac : public BaseEndDeviceLorawanMac
{
    enum RxOutcome
    {
        ACK,
        RECV,
        FAIL,
        NONE
    };

  public:
    static TypeId GetTypeId();

    ClassAEndDeviceLorawanMac();
    ~ClassAEndDeviceLorawanMac() override;

    /**
     * Perform the actions that are required after a packet send.
     *
     * This function handles opening of the first receive window.
     */
    void TxFinished(Ptr<const Packet> packet) override;

    /**
     * Receive a packet.
     *
     * This method is typically registered as a callback in the underlying PHY
     * layer so that it's called when a packet is going up the stack.
     *
     * \param packet the received packet.
     */
    void Receive(Ptr<const Packet> packet) override;

    /**
     * Signal reception failure.
     *
     * This method is typically registered as a callback in the underlying PHY
     * layer so that it's called when a packet is going up the stack.
     *
     * \param packet the failed packet.
     */
    void FailedReception(Ptr<const Packet> packet) override;

    /**
     * Signal no reception during either reception window.
     *
     * This method is typically registered as a callback in the reception window
     * manager that it's called when the second reception window ends.
     */
    void NoReception();

    /////////////////////////
    // Getters and Setters //
    /////////////////////////

    /**
     * Set the Data Rate to be used in the second receive window.
     *
     * \param dataRate The Data Rate.
     */
    void SetSecondReceiveWindowDataRate(uint8_t dataRate);

    /**
     * Set the frequency that will be used for the second receive window.
     *
     * \param frequency the Frequency.
     */
    void SetSecondReceiveWindowFrequency(double frequency);

  protected:
    void DoInitialize() override;
    void DoDispose() override;

  private:
    /**
     * Add headers and send a packet with the sending function of the physical layer.
     *
     * \param packet the packet to send
     */
    void SendToPhy(Ptr<Packet> packet) override;

    /**
     * Find the minimum waiting time before the next possible transmission based
     * on End Device's transmission/reception process.
     */
    Time GetBusyTransmissionDelay() override;

    /**
     * Decide whether we can retransmit based on reception outcome.
     *
     * \param outcome Outcome of the reception.
     */
    void ManageRetransmissions(RxOutcome outcome);

    /**
     * Compute the time duration of a reception window based on its datarate.
     */
    Time GetReceptionWindowDuration(uint8_t datarate);

    /////////////////////////
    // MAC command methods //
    /////////////////////////

    /**
     * Perform the actions that need to be taken when receiving a RxParamSetupReq
     * command.
     *
     * \param rxParamSetupReq The Parameter Setup Request, which contains:
     *                            - The offset to set.
     *                            - The data rate to use for the second receive window.
     *                            - The frequency to use for the second receive window.
     */
    void OnRxParamSetupReq(Ptr<RxParamSetupReq> rxParamSetupReq) override;

    /**
     * Perform the actions that need to be taken when receiving a RxTimingSetupReq command.
     */
    void OnRxTimingSetupReq(Time delay) override;

    /**
     * The duration of a receive window in number of symbols. This should be
     * converted to time based or the reception parameter used.
     *
     * The downlink preamble transmitted by the gateways contains 8 symbols.
     * The receiver requires 5 symbols to detect the preamble and synchronize.
     * Therefore there must be a 5 symbols overlap between the receive window
     * and the transmitted preamble.
     * (Ref: Recommended SX1272/76 Settings for EU868 LoRaWAN Network Operation )
     */
    uint16_t m_recvWinSymb;

    /**
     * The RX1DROffset parameter value
     */
    uint8_t m_rx1DrOffset;

    /**
     * Last channel used for tx
     */
    Ptr<LogicalChannel> m_lastTxCh;

    /**
     * Reception window process manager.
     */
    Ptr<RecvWindowManager> m_rwm;

}; /* ClassAEndDeviceLorawanMac */

} /* namespace lorawan */
} /* namespace ns3 */

#endif /* CLASS_A_END_DEVICE_LORAWAN_MAC_H */
