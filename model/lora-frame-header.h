/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef LORA_FRAME_HEADER_H
#define LORA_FRAME_HEADER_H

#include "ns3/header.h"
#include "ns3/lora-device-address.h"
#include "ns3/mac-command.h"

namespace ns3 {
namespace lorawan {

/**
 * This class represents the Frame header (FHDR) used in a LoraWAN network.
 *
 * Although the specification divides the FHDR from the FPort field, this
 * implementation considers them as a unique entity (i.e., FPort is treated as
 * if it were a part of FHDR).
 *
 * \remark Prior to using it, this class needs to be informed of whether the
 * header is for an uplink or downlink message. This is necessary due to the
 * fact that UL and DL messages have subtly different structure and, hence,
 * serialization and deserialization schemes.
 */
class LoraFrameHeader : public Header
{
public:
  LoraFrameHeader ();
  ~LoraFrameHeader ();

  // Methods inherited from Header
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Return the size required for serialization of this header
   *
   * \return The serialized size in bytes
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * Serialize the header.
   *
   * See Page 15 of LoraWAN specification for a representation of fields.
   *
   * \param start A pointer to the buffer that will be filled with the
   * serialization.
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * Deserialize the contents of the buffer into a LoraFrameHeader object.
   *
   * \param start A pointer to the buffer we need to deserialize.
   * \return The number of consumed bytes.
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Print the header in a human-readable format.
   *
   * \param os The std::ostream on which to print the header.
   */
  virtual void Print (std::ostream &os) const;

  /**
   * State that this is an uplink message.
   *
   * This method needs to be called at least once before any serialization or
   * deserialization.
   */
  void SetAsUplink (void);

  /**
   * State that this is a downlink message.
   *
   * This method needs to be called at least once before any serialization or
   * deserialization.
   */
  void SetAsDownlink (void);

  /**
   * Set the FPort value.
   *
   * \param fPort The FPort to set.
   */
  void SetFPort (uint8_t fPort);

  /**
   * Get the FPort value.
   *
   * \return The FPort value.
   */
  uint8_t GetFPort (void) const;

  /**
   * Set the address.
   *
   * \param address The LoraDeviceAddress to set.
   */
  void SetAddress (LoraDeviceAddress address);

  /**
   * Get this header's device address value.
   *
   * \return The address value stored in this header.
   */
  LoraDeviceAddress GetAddress (void) const;

  /**
   * Set the Adr value.
   *
   * \param Adr The Adr to set.
   */
  void SetAdr (bool adr);

  /**
   * Get the Adr value.
   *
   * \return The Adr value.
   */
  bool GetAdr (void) const;

  /**
   * Set the AdrAckReq value.
   *
   * \param adrAckReq The AdrAckReq to set.
   */
  void SetAdrAckReq (bool adrAckReq);

  /**
   * Get the AdrAckReq value.
   *
   * \return The AdrAckReq value.
   */
  bool GetAdrAckReq (void) const;

  /**
   * Set the Ack bit.
   *
   * \param ack Whether or not to set the ACK bit.
   */
  void SetAck (bool ack);

  /**
   * Get the Ack bit value.
   *
   * \return True if the ACK bit is set, false otherwise.
   */
  bool GetAck (void) const;

  /**
   * Set the FPending value.
   *
   * \param fPending The FPending to set.
   */
  void SetFPending (bool fPending);

  /**
   * Get the FPending value.
   *
   * \return The FPending value.
   */
  bool GetFPending (void) const;

  /**
   * Get the FOptsLen value.
   *
   * \remark This value cannot be set since it's directly extracted from the
   * number and kind of MAC commands.
   *
   * \return The FOptsLen value.
   */
  uint8_t GetFOptsLen (void) const;

  /**
   * Set the FCnt value
   *
   * \param FCnt The FCnt to set.
   */
  void SetFCnt (uint16_t fCnt);
  /**
   * Get the FCnt value.
   *
   * \return The FCnt value.
   */
  uint16_t GetFCnt (void) const;

  /**
   * Return a pointer to a MacCommand, or 0 if the MacCommand does not exist
   * in this header.
   */
  template<typename T>
  inline Ptr<T> GetMacCommand (void);

  /**
   * Add a LinkCheckReq command.
   */
  void AddLinkCheckReq (void);

  /**
   * Add a LinkCheckAns command.
   *
   * \param margin The demodulation margin the LinkCheckReq packet was received with.
   * \param gwCnt The number of gateways the LinkCheckReq packet was received by.
   */
  void AddLinkCheckAns (uint8_t margin, uint8_t gwCnt);

  /**
   * Add a LinkAdrReq command.
   *
   * \param dataRate The data rate at which the receiver should transmit.
   * \param txPower The power at which the receiver should transmit, encoded according to the LoRaWAN specification of the region.
   * \param enabledChannels A list containing the indices of channels enabled by this command.
   * \param repetitions The number of repetitions the receiver should send when transmitting.
   */
  void AddLinkAdrReq (uint8_t dataRate, uint8_t txPower, std::list<int> enabledChannels, int repetitions);

  /**
   * Add a LinkAdrAns command.
   *
   * \param powerAck Whether the power can be set or not.
   * \param dataRateAck Whether the data rate can be set or not.
   * \param channelMaskAck Whether the channel mask is coherent with the device's current state or not.
   */
  void AddLinkAdrAns (bool powerAck, bool dataRateAck, bool channelMaskAck);

  /**
   * Add a DutyCycleReq command.
   *
   * This command accepts an 8-bit integer as dutyCycle. The actual dutyCycle
   * that will be implemented in the end-device will then be, in fraction form,
   * 1/2^(dutyCycle).
   *
   * \param dutyCycle The dutyCycle in 8-bit form.
   */
  void AddDutyCycleReq (uint8_t dutyCycle);

  /**
   * Add a DutyCycleAns command.
   */
  void AddDutyCycleAns (void);

  /**
   * Add a RxParamSetupReq command.
   *
   * \param rx1DrOffset The requested data rate offset for the first receive window.
   * \param rx2DataRate The requested data rate for the second receive window.
   * \param frequency The frequency at which to listen for the second receive window.
   */
  void AddRxParamSetupReq (uint8_t rx1DrOffset, uint8_t rx2DataRate, double frequency);

  /**
   * Add a RxParamSetupAns command.
   */
  void AddRxParamSetupAns ();

  /**
   * Add a DevStatusReq command.
   */
  void AddDevStatusReq ();

  /**
   * Add a NewChannelReq command.
   */
  void AddNewChannelReq (uint8_t chIndex, double frequency, uint8_t minDataRate,
                         uint8_t maxDataRate);

  /**
   * Return a list of pointers to all the MAC commands saved in this header.
   */
  std::list<Ptr<MacCommand> > GetCommands (void);

  /**
   * Add a predefined command to the list.
   */
  void AddCommand (Ptr<MacCommand> macCommand);

private:
  uint8_t m_fPort;

  LoraDeviceAddress m_address;

  bool m_adr;
  bool m_adrAckReq;
  bool m_ack;
  bool m_fPending;
  uint8_t m_fOptsLen;

  uint16_t m_fCnt;

  Buffer m_fOpts;

  /**
   * List containing all the MacCommand instances that are contained in this
   * LoraFrameHeader.
   */
  std::list< Ptr< MacCommand> > m_macCommands;

  bool m_isUplink;
};


template<typename T>
Ptr<T>
LoraFrameHeader::GetMacCommand ()
{
  // Iterate on MAC commands and try casting
  std::list< Ptr< MacCommand> >::const_iterator it;
  for (it = m_macCommands.begin (); it != m_macCommands.end (); ++it)
    {
      if ((*it)->GetObject<T> () != 0)
        {
          return (*it)->GetObject<T> ();
        }
    }

  // If no command was found, return 0
  return 0;
}
}

}
#endif
