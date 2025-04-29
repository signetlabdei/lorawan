/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef LORA_FRAME_HEADER_H
#define LORA_FRAME_HEADER_H

#include "lora-device-address.h"
#include "mac-command.h"

#include "ns3/header.h"

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * This class represents the Frame header (FHDR) used in a LoraWAN network.
 *
 * Although the specification divides the FHDR from the FPort field, this
 * implementation considers them as a unique entity (i.e., FPort is treated as
 * if it were a part of FHDR).
 *
 * @remark Prior to using it, this class needs to be informed of whether the
 * header is for an uplink or downlink message. This is necessary due to the
 * fact that UL and DL messages have subtly different structure and, hence,
 * serialization and deserialization schemes.
 */
class LoraFrameHeader : public Header
{
  public:
    LoraFrameHeader();           //!< Default constructor
    ~LoraFrameHeader() override; //!< Destructor

    // Methods inherited from Header

    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;

    /**
     * Return the size required for serialization of this header.
     *
     * @return The serialized size in bytes.
     */
    uint32_t GetSerializedSize() const override;

    /**
     * Serialize the header.
     *
     * See Page 15 of LoraWAN specification for a representation of fields.
     *
     * @param start A pointer to the buffer that will be filled with the
     * serialization.
     */
    void Serialize(Buffer::Iterator start) const override;

    /**
     * Deserialize the contents of the buffer into a LoraFrameHeader object.
     *
     * @param start A pointer to the buffer we need to deserialize.
     * @return The number of consumed bytes.
     */
    uint32_t Deserialize(Buffer::Iterator start) override;

    /**
     * Print the header in a human-readable format.
     *
     * @param os The std::ostream on which to print the header.
     */
    void Print(std::ostream& os) const override;

    /**
     * State that this is an uplink message.
     *
     * This method needs to be called at least once before any serialization or
     * deserialization.
     */
    void SetAsUplink();

    /**
     * State that this is a downlink message.
     *
     * This method needs to be called at least once before any serialization or
     * deserialization.
     */
    void SetAsDownlink();

    /**
     * Set the FPort value.
     *
     * @param fPort The FPort to set.
     */
    void SetFPort(uint8_t fPort);

    /**
     * Get the FPort value.
     *
     * @return The FPort value.
     */
    uint8_t GetFPort() const;

    /**
     * Set the address.
     *
     * @param address The LoraDeviceAddress to set.
     */
    void SetAddress(LoraDeviceAddress address);

    /**
     * Get this header's device address value.
     *
     * @return The address value stored in this header.
     */
    LoraDeviceAddress GetAddress() const;

    /**
     * Set the value of the ADR bit field.
     *
     * @param adr Whether or not to set the ADR bit field.
     */
    void SetAdr(bool adr);

    /**
     * Get the value of the ADR bit field.
     *
     * @return True if the ADR bit is set, false otherwise.
     */
    bool GetAdr() const;

    /**
     * Set the value of the ADRACKReq bit field.
     *
     * @param adrAckReq Whether or not to set the ADRACKReq bit field.
     */
    void SetAdrAckReq(bool adrAckReq);

    /**
     * Get the value of the ADRACKReq bit field.
     *
     * @return True if the ADRACKReq bit is set, false otherwise.
     */
    bool GetAdrAckReq() const;

    /**
     * Set the value of the ACK bit field.
     *
     * @param ack Whether or not to set the ACK bit.
     */
    void SetAck(bool ack);

    /**
     * Get the value of the ACK bit field.
     *
     * @return True if the ACK bit is set, false otherwise.
     */
    bool GetAck() const;

    /**
     * Set the value of the FPending bit field.
     *
     * @param fPending Whether or not to set the FPending bit.
     */
    void SetFPending(bool fPending);

    /**
     * Get the value of the FPending bit field.
     *
     * @return True if the FPending bit is set, false otherwise.
     */
    bool GetFPending() const;

    /**
     * Get the FOptsLen value.
     *
     * @remark This value cannot be set since it's directly extracted from the
     * number and kind of MAC commands.
     *
     * @return The FOptsLen value.
     */
    uint8_t GetFOptsLen() const;

    /**
     * Set the FCnt value.
     *
     * @param fCnt The FCnt to set.
     */
    void SetFCnt(uint16_t fCnt);
    /**
     * Get the FCnt value.
     *
     * @return The FCnt value.
     */
    uint16_t GetFCnt() const;

    /**
     * Return a pointer to the first MacCommand of type T, or 0 if no such MacCommand exists
     * in this header.
     *
     * @return A pointer to a MacCommand of type T.
     */
    template <typename T>
    inline Ptr<T> GetMacCommand();

    /**
     * Add a LinkCheckReq command.
     */
    void AddLinkCheckReq();

    /**
     * Add a LinkCheckAns command.
     *
     * @param margin The demodulation margin the LinkCheckReq packet was received with.
     * @param gwCnt The number of gateways the LinkCheckReq packet was received by.
     */
    void AddLinkCheckAns(uint8_t margin, uint8_t gwCnt);

    /**
     * Add a LinkAdrReq command.
     *
     * @param dataRate The data rate at which the receiver should transmit.
     * @param txPower The power at which the receiver should transmit, encoded according to the
     * LoRaWAN specification of the region. @param enabledChannels A list containing the indices of
     * channels enabled by this command. @param repetitions The number of repetitions the receiver
     * should send when transmitting.
     */
    void AddLinkAdrReq(uint8_t dataRate,
                       uint8_t txPower,
                       std::list<int> enabledChannels,
                       int repetitions);

    /**
     * Add a LinkAdrAns command.
     *
     * @param powerAck Whether the power can be set or not.
     * @param dataRateAck Whether the data rate can be set or not.
     * @param channelMaskAck Whether the channel mask is coherent with the device's current state or
     * not.
     */
    void AddLinkAdrAns(bool powerAck, bool dataRateAck, bool channelMaskAck);

    /**
     * Add a DutyCycleReq command.
     *
     * This command accepts an 8-bit integer as dutyCycle. The actual dutyCycle
     * that will be implemented in the end-device will then be, in fraction form,
     * 1/2^(dutyCycle).
     *
     * @param dutyCycle The dutyCycle in 8-bit form.
     */
    void AddDutyCycleReq(uint8_t dutyCycle);

    /**
     * Add a DutyCycleAns command.
     */
    void AddDutyCycleAns();

    /**
     * Add a RxParamSetupReq command.
     *
     * @param rx1DrOffset The requested data rate offset for the first receive window.
     * @param rx2DataRate The requested data rate for the second receive window.
     * @param frequencyHz The frequency [Hz] at which to listen for the second receive window.
     */
    void AddRxParamSetupReq(uint8_t rx1DrOffset, uint8_t rx2DataRate, uint32_t frequencyHz);

    /**
     * Add a RxParamSetupAns command.
     */
    void AddRxParamSetupAns();

    /**
     * Add a DevStatusReq command.
     */
    void AddDevStatusReq();

    /**
     * Add a NewChannelReq command with provided fields.
     *
     * @param chIndex The ChIndex field.
     * @param frequencyHz The Frequency field in Hz.
     * @param minDataRate The MinDR field.
     * @param maxDataRate The MaxDR field.
     */
    void AddNewChannelReq(uint8_t chIndex,
                          uint32_t frequencyHz,
                          uint8_t minDataRate,
                          uint8_t maxDataRate);

    /**
     * Return a vector of pointers to all the MAC commands saved in this header.
     *
     * @return The vector of pointers to MacCommand objects.
     */
    std::vector<Ptr<MacCommand>> GetCommands();

    /**
     * Add a predefined command to the vector in this frame header.
     *
     * @param macCommand A pointer to the MacCommand object to add.
     */
    void AddCommand(Ptr<MacCommand> macCommand);

  private:
    uint8_t m_fPort; //!< The FPort field

    LoraDeviceAddress m_address; //!< The DevAddr field

    bool m_adr;         //!< The ADR field of the FCtrl
    bool m_adrAckReq;   //!< The ADRACKReq field of the FCtrl
    bool m_ack;         //!< The ACK field of the FCtrl
    bool m_fPending;    //!< The FPending/ClassB field of the FCtrl
    uint8_t m_fOptsLen; //!< The FOptsLen field of the FCtrl

    uint16_t m_fCnt; //!< The FCnt field

    Buffer m_fOpts;                             //!< The FOpts field
    std::vector<Ptr<MacCommand>> m_macCommands; //!< Vector containing all the MacCommand instances
                                                //!< that are contained in this LoraFrameHeader

    bool m_isUplink; //!< Whether this frame header is uplink or not
};

template <typename T>
Ptr<T>
LoraFrameHeader::GetMacCommand()
{
    // Iterate on MAC commands and try casting
    for (const auto& cmd : m_macCommands)
    {
        if (auto c = DynamicCast<T>(cmd); c)
        {
            return c;
        }
    }
    // If no command was found, return 0
    return nullptr;
}
} // namespace lorawan

} // namespace ns3
#endif
