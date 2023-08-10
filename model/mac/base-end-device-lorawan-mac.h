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
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef END_DEVICE_LORAWAN_MAC_H
#define END_DEVICE_LORAWAN_MAC_H

#include "ns3/LoRaMacCrypto.h"
#include "ns3/lora-device-address.h"
#include "ns3/lora-frame-header.h"
#include "ns3/lorawan-mac-header.h"
#include "ns3/lorawan-mac.h"
#include "ns3/mac-command.h"
#include "ns3/traced-value.h"

#define ADR_ACK_LIMIT 64
#define ADR_ACK_DELAY 32
#define MAX_ADR_ACK_CNT (ADR_ACK_LIMIT + 7 * ADR_ACK_DELAY + 1)

namespace ns3
{
namespace lorawan
{

/**
 * Class representing the MAC layer of a LoRaWAN device.
 */
class BaseEndDeviceLorawanMac : public LorawanMac
{
    struct LorawanMacTxContext
    {
        Time firstAttempt;
        Ptr<Packet> packet = nullptr;
        uint8_t nbTxLeft;
        bool waitingAck = false;
        bool busy = false;
    };

  public:
    static TypeId GetTypeId();

    BaseEndDeviceLorawanMac();
    ~BaseEndDeviceLorawanMac() override;

    /**
     * Send a packet.
     *
     * The MAC layer of the ED will take care of using the right parameters.
     *
     * \param packet the packet to send
     */
    void Send(Ptr<Packet> packet) override;

    /**
     * Perform the actions that are required after a packet send.
     *
     * This function handles opening of the first receive window.
     */
    void TxFinished(Ptr<const Packet> packet) override = 0;

    /**
     * Receive a packet.
     *
     * This method is typically registered as a callback in the underlying PHY
     * layer so that it's called when a packet is going up the stack.
     *
     * \param packet the received packet.
     */
    void Receive(Ptr<const Packet> packet) override = 0;

    /**
     * Signal reception failure.
     *
     * This method is typically registered as a callback in the underlying PHY
     * layer so that it's called when a packet is going up the stack.
     *
     * \param packet the failed packet.
     */
    void FailedReception(Ptr<const Packet> packet) override = 0;

    /**
     * Add a MAC command to the list of those that will be sent out in the next
     * packet.
     */
    void AddMacCommand(Ptr<MacCommand> macCommand);

    /////////////////////////
    // Getters and Setters //
    /////////////////////////

    /**
     * Set the network address of this device.
     *
     * \param address The address to set.
     */
    void SetDeviceAddress(LoraDeviceAddress address);

    /**
     * Get the network address of this device.
     *
     * \return This device's address.
     */
    LoraDeviceAddress GetDeviceAddress();

    /**
     * Set the message type to send when the Send method is called.
     */
    void SetFType(LorawanMacHeader::FType fType);

    /**
     * Get the message type to send when the Send method is called.
     */
    LorawanMacHeader::FType GetFType();

    /**
     * Set the data rate this end device will use when transmitting. For End
     * Devices, this value is assumed to be fixed, and can be modified via MAC
     * commands issued by the GW.
     *
     * \param dataRate The dataRate to use when transmitting.
     */
    void SetDataRate(uint8_t dataRate);

    /**
     * Get the data rate this end device is set to use.
     *
     * \return The data rate this device uses when transmitting.
     */
    uint8_t GetDataRate();

    /**
     * Set the transmission power this end device will use when transmitting.
     * For End Devices, this value is assumed to be fixed, and can be modified
     * via MAC commands issued by the GW.
     *
     * \param txPower The txPower to use when transmitting.
     */
    void SetTransmissionPower(uint8_t txPower);

    /**
     * Get the transmission power this end device is set to use.
     *
     * \return The transmission power this device uses when transmitting.
     */
    uint8_t GetTransmissionPower();

    /**
     * Get the aggregated duty cycle.
     *
     * \return A time instance containing the aggregated duty cycle in fractional
     * form.
     */
    double GetAggregatedDutyCycle();

    /**
     * Set the aggregated duty cycle.
     *
     * \param aggregatedDutyCycle A time instance containing the aggregated
     * duty cycle in fractional form.
     */
    void SetAggregatedDutyCycle(double aggregatedDutyCycle);

    /**
     * Set the number of transmissions ofr each uplink.
     *
     * \param nbTrans The number of transmissions ofr each uplink
     */
    void SetNumberOfTransmissions(uint8_t nbTrans);

    /**
     * Set the maximum number of transmissions allowed.
     */
    uint8_t GetNumberOfTransmissions() const;

    /**
     * Enable data rate adaptation in the retransmitting procedure.
     *
     * \param adapt If the data rate adaptation is enabled or not.
     */
    void SetADRBackoff(bool adapt);

    /**
     * Get if data rate adaptation is enabled or not.
     */
    bool GetADRBackoff() const;

  protected:
    void DoInitialize() override;
    void DoDispose() override;

    ///////////////////////////////
    // Protected sending methods //
    ///////////////////////////////

    /**
     * Postpone transmission to the specified time and delete previously scheduled transmissions if
     * present.
     *
     * \param nextTxDelay Delay at which the transmission will be performed.
     */
    virtual void postponeTransmission(Time nextTxDelay, Ptr<Packet>);

    /**
     * Find a suitable channel for transmission. The channel is chosen among the
     * ones that are available in the ED's LogicalChannel, based on their duty
     * cycle limitations.
     */
    Ptr<LogicalChannel> GetChannelForTx();

    ///////////////////////////
    // Protected MAC Actions //
    ///////////////////////////

    /**
     * Parse and take action on the commands contained on this FrameHeader.
     */
    void ApplyMACCommands(LoraFrameHeader fHdr, Ptr<const Packet> packet);

    ////////////////////////////////////////////
    // Protected Fields of the LoRaWAN header //
    ////////////////////////////////////////////

    /**
     * List of the MAC commands that need to be applied to the next UL packet.
     */
    std::list<Ptr<MacCommand>> m_fOpts;

    //////////////////////////////////
    // Protected MAC Layer settings //
    //////////////////////////////////

    /**
     * The DataRate this device is using to transmit.
     */
    TracedValue<uint8_t> m_dataRate;

    /**
     * The transmission power this device is using to transmit.
     */
    TracedValue<double> m_txPower;

    /**
     * Number of transmissions of each uplink frame.
     */
    uint8_t m_nbTrans;

    /////////////////////////////////
    // Protected MAC Layer context //
    /////////////////////////////////

    /* Counter for keepalive purposes */
    uint16_t m_ADRACKCnt;

    /**
     * The event of transmitting a packet in a consecutive moment, when the duty cycle let us
     * transmit or if an ACK is not received.
     *
     * This Event is also used to cancel the transmission of this packet if a newer packet is
     * delivered from the application to be sent.
     */
    EventId m_nextTx;

    /**
     * Structure representing the parameters that will be used in the
     * transmission procedure (packet tx + rx windows).
     */
    struct LorawanMacTxContext m_txContext;

    /////////////////////////
    // Protected Utilities //
    /////////////////////////

    /**
     * An uniform random variable, used by the Shuffle method to randomly reorder
     * the channel list.
     */
    Ptr<UniformRandomVariable> m_uniformRV;

    ////////////////////////////////
    // Protected Trace callbacks  //
    ////////////////////////////////

    /**
     * The trace source fired when the transmission procedure is finished.
     *
     * \see class CallBackTraceSource
     */
    TracedCallback<uint8_t, bool, Time, Ptr<Packet>> m_requiredTxCallback;

  private:
    /////////////////////////////
    // Private sending methods //
    /////////////////////////////

    /**
     * Checking if we are performing the transmission of a new packet or a retransmission,
     * add headers, and call SendToPhy function.
     *
     * \param packet the packet to send
     */
    virtual void DoSend(Ptr<Packet> packet);

    /**
     * Send a packet with the sending function of the physical layer.
     *
     * \param packet the packet to send
     */
    virtual void SendToPhy(Ptr<Packet> packet) = 0;

    /**
     * Find the minimum waiting time before the next possible transmission.
     */
    Time GetNextTransmissionDelay();

    /**
     * Find the minimum waiting time before the next possible transmission based
     * on End Device's Class Type.
     */
    virtual Time GetBusyTransmissionDelay() = 0;

    /* Check if we need to backoff parameters after long radio silence */
    void ExecuteADRBackoff();

    /**
     * Randomly shuffle a Ptr<LogicalChannel> vector.
     *
     * Used to pick a random channel on which to send the packet.
     */
    std::vector<Ptr<LogicalChannel>> Shuffle(std::vector<Ptr<LogicalChannel>> vector);

    /////////////////////////////////
    //  Private MAC layer actions  //
    /////////////////////////////////

    /**
     * Add the necessary options and MAC commands to the LoraFrameHeader.
     *
     * \param fHdr The frame header on which to apply the options.
     */
    void FillHeader(LoraFrameHeader& fHdr);

    /**
     * Add the necessary options and MAC commands to the LorawanMacHeader.
     *
     * \param mHdr The mac header on which to apply the options.
     */
    void FillHeader(LorawanMacHeader& mHdr);

    /* Add Message Integrity Code (4 Bytes) at the end of the packet */
    void AddMIC(Ptr<Packet> packet);

    /**
     * Manage the case of MAC commands being in the FRMPayload.
     *
     * \brief Serialized MAC commands from the payload are fist decrypted (if requested),
     *        piggybacked to the frame header, and then correctly deserialized into it.
     *
     * \param fHdr The packet frame header
     * \param cmds The serialized FRMPayload containing MAC commands
     * \param size Size of the serialized FRMPayload
     */
    void AppendCmdsFromFRMPayload(LoraFrameHeader& fHdr, Ptr<const Packet> packet);

    /**
     * Perform the actions that need to be taken when receiving a LinkCheckAns command.
     *
     * \param margin The margin value of the command.
     * \param gwCnt The gateway count value of the command.
     */
    void OnLinkCheckAns(uint8_t margin, uint8_t gwCnt);

    /**
     * Perform the actions that need to be taken when receiving a LinkAdrReq command.
     *
     * \param dataRate The data rate value of the command.
     * \param txPower The transmission power value of the command.
     * \param enabledChannels A list of the enabled channels.
     * \param repetitions The number of repetitions prescribed by the command.
     */
    void OnLinkAdrReq(uint8_t dataRate,
                      uint8_t txPower,
                      std::list<int> enabledChannels,
                      int repetitions);

    /**
     * Perform the actions that need to be taken when receiving a DutyCycleReq command.
     *
     * \param dutyCycle The aggregate duty cycle prescribed by the command, in
     * fraction form.
     */
    void OnDutyCycleReq(double dutyCycle);

    /**
     * Perform the actions that need to be taken when receiving a RxParamSetupReq command.
     *
     * \param rxParamSetupReq The Parameter Setup Request
     */
    virtual void OnRxParamSetupReq(Ptr<RxParamSetupReq> rxParamSetupReq) = 0;

    /**
     * Perform the actions that need to be taken when receiving a DevStatusReq command.
     */
    void OnDevStatusReq();

    /**
     * Perform the actions that need to be taken when receiving a NewChannelReq command.
     */
    void OnNewChannelReq(uint8_t chIndex,
                         double frequency,
                         uint8_t minDataRate,
                         uint8_t maxDataRate);

    /**
     * Perform the actions that need to be taken when receiving a RxTimingSetupReq command.
     */
    virtual void OnRxTimingSetupReq(Time delay) = 0;

    /**
     * Perform the actions that need to be taken when receiving a DlChannelReq command.
     */
    void OnDlChannelReq(uint8_t chIndex, double frequency);

    //////////////////////////////////////////
    // Private Fields of the LoRaWAN header //
    //////////////////////////////////////////

    /**
     * The frame type to apply to packets sent with the Send method.
     */
    LorawanMacHeader::FType m_fType;

    /**
     * The address of this device.
     */
    LoraDeviceAddress m_address;

    /**
     * Whether this device's data rate should be controlled by the NS.
     */
    bool m_ADRBit;

    /* Uplink only - request keepalive acknowledgement from the server */
    bool m_ADRACKReq;

    /**
     * Uplink frame counter of the device
     */
    uint16_t m_fCnt;

    ////////////////////////////////
    // Private MAC Layer settings //
    ////////////////////////////////

    /**
     * Enable Data Rate adaptation during the retransmission procedure.
     */
    bool m_enableADRBackoff;

    /**
     * Whether this device's should use cryptography according to specifications.
     */
    bool m_enableCrypto;
    /**
     * The aggregated duty cycle this device needs to respect across all sub-bands.
     */
    TracedValue<double> m_aggregatedDutyCycle;

    ///////////////////////////////
    // Private MAC Layer context //
    ///////////////////////////////

    /**
     * The last known link margin.
     *
     * This value is obtained (and updated) when a LinkCheckAns Mac command is
     * received.
     */
    TracedValue<double> m_lastKnownLinkMargin;

    /**
     * The last known gateway count (i.e., gateways that are in communication
     * range with this end device)
     *
     * This value is obtained (and updated) when a LinkCheckAns Mac command is
     * received.
     */
    TracedValue<int> m_lastKnownGatewayCount;

    ///////////////////////
    // Private Utilities //
    ///////////////////////

    /**
     * Class containing cryptographic keys and functions
     */
    LoRaMacCrypto* m_crypto;
};

} // namespace lorawan

} // namespace ns3
#endif /* END_DEVICE_LORAWAN_MAC_H */
