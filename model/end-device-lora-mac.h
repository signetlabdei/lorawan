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
 *         Martina Capuzzo <capuzzom@dei.unipd.it>
 */

#ifndef END_DEVICE_LORA_MAC_H
#define END_DEVICE_LORA_MAC_H

#include "ns3/lora-mac.h"
#include "ns3/lora-mac-header.h"
#include "ns3/lora-frame-header.h"
#include "ns3/random-variable-stream.h"
#include "ns3/lora-device-address.h"
#include "ns3/traced-value.h"

namespace ns3 {

/**
  * Class representing the MAC layer of a LoRaWAN device.
  */
class EndDeviceLoraMac : public LoraMac
{
public:
  static TypeId GetTypeId (void);

  EndDeviceLoraMac ();
  virtual ~EndDeviceLoraMac ();

  /////////////////////
  // Sending methods //
  /////////////////////

  /**
   * Send a packet.
   *
   * The MAC layer of the ED will take care of using the right parameters.
   *
   * \param packet the packet to send
   */
  virtual void Send (Ptr<Packet> packet);

  /**
   * Checking if we are performing the transmission of a new packet or a retransmission, and call SendToPhy function.
   *
   * \param packet the packet to send
   */
  virtual void DoSend (Ptr<Packet> packet);

  /**
  * Add headers and send a packet with the sending function of the physical layer.
  *
  * \param packet the packet to send
  */
  virtual void SendToPhy (Ptr<Packet> packet);

  /**
   * Postpone transmission to the specified time and delete previously scheduled transmissions if present.
   *
   * \param nextTxDelay Delay at which the transmission will be performed.
   */
  virtual void postponeTransmission (Time nextTxDelay, Ptr<Packet>);


  ///////////////////////
  // Receiving methods //
  ///////////////////////

  /**
   * Receive a packet.
   *
   * This method is typically registered as a callback in the underlying PHY
   * layer so that it's called when a packet is going up the stack.
   *
   * \param packet the received packet.
   */
  virtual void Receive (Ptr<Packet const> packet);

  virtual void FailedReception (Ptr<Packet const> packet);

  /**
   * Perform the actions that are required after a packet send.
   *
   * This function handles opening of the first receive window.
   */
  void TxFinished (Ptr<const Packet> packet);

  /**
   * Perform operations needed to open the first receive window.
   */
  void OpenFirstReceiveWindow (void);

  /**
   * Perform operations needed to open the second receive window.
   */
  void OpenSecondReceiveWindow (void);

  /**
   * Perform operations needed to close the first receive window.
   */
  void CloseFirstReceiveWindow (void);

  /**
   * Perform operations needed to close the second receive window.
   */
  void CloseSecondReceiveWindow (void);

  /////////////////////////
  // Getters and Setters //
  /////////////////////////

  /**
  * Reset retransmission parameters contained in the structure LoraRetxParams
  */
  virtual void resetRetransmissionParameters ();

  /**
   * Enable data rate adaptation in the retransmitting procedure.
   *
   * \param adapt If the data rate adaptation is enabled or not.
   */
  void SetDataRateAdaptation (bool adapt);

  /**
   * Get if data rate adaptation is enabled or not.
   */
  bool GetDataRateAdaptation (void);

  /**
   * Set the maximum number of transmissions allowed.
   *
   * \param maxNumbTx The maximum number of transmissions allowed
   */
  void SetMaxNumberOfTransmissions (uint8_t maxNumbTx);

  /**
   * Set the maximum number of transmissions allowed.
   */
  uint8_t GetMaxNumberOfTransmissions (void);

  /**
   * Set the data rate this end device will use when transmitting. For End
   * Devices, this value is assumed to be fixed, and can be modified via MAC
   * commands issued by the GW.
   *
   * \param dataRate The dataRate to use when transmitting.
   */
  void SetDataRate (uint8_t dataRate);

  /**
   * Get the data rate this end device is set to use.
   *
   * \return The data rate this device uses when transmitting.
   */
  uint8_t GetDataRate (void);

  /**
   * Set the network address of this device.
   *
   * \param address The address to set.
   */
  void SetDeviceAddress (LoraDeviceAddress address);

  /**
   * Get the network address of this device.
   *
   * \return This device's address.
   */
  LoraDeviceAddress GetDeviceAddress (void);

  /**
   * Set the Data Rate to be used in the second receive window.
   *
   * \param dataRate The Data Rate.
   */
  void SetSecondReceiveWindowDataRate (uint8_t dataRate);

  /**
   * Get the Data Rate that will be used in the first receive window.
   *
   * \return The Data Rate
   */
  uint8_t GetFirstReceiveWindowDataRate (void);

  /**
   * Get the Data Rate that will be used in the second receive window.
   *
   * \return The Data Rate
   */
  uint8_t GetSecondReceiveWindowDataRate (void);

  /**
   * Set the frequency that will be used for the second receive window.
   *
   * \param frequencyMHz the Frequency.
   */
  void SetSecondReceiveWindowFrequency (double frequencyMHz);

  /**
   * Get the frequency that is used for the second receive window.
   *
   * @return The frequency, in MHz
   */
  double GetSecondReceiveWindowFrequency (void);

  /**
   * Set a value for the RX1DROffset parameter.
   *
   * This value decides the offset to use when deciding the DataRate of the
   * downlink transmission during the first receive window from the
   * replyDataRateMatrix.
   *
   * \param rx1DrOffset The value to set for the offset.
   */
  // void SetRx1DrOffset (uint8_t rx1DrOffset);

  /**
   * Get the value of the RX1DROffset parameter.
   *
   * \return The value of the RX1DROffset parameter.
   */
  // uint8_t GetRx1DrOffset (void);

  /**
   * Get the aggregated duty cycle.
   *
   * \return A time instance containing the aggregated duty cycle in fractional
   * form.
   */
  double GetAggregatedDutyCycle (void);

  /////////////////////////
  // MAC command methods //
  /////////////////////////

  /**
   * Add the necessary options and MAC commands to the LoraFrameHeader.
   *
   * \param frameHeader The frame header on which to apply the options.
   */
  void ApplyNecessaryOptions (LoraFrameHeader &frameHeader);

  /**
   * Add the necessary options and MAC commands to the LoraMacHeader.
   *
   * \param macHeader The mac header on which to apply the options.
   */
  void ApplyNecessaryOptions (LoraMacHeader &macHeader);

  /**
   * Set the message type to send when the Send method is called.
   */
  void SetMType (LoraMacHeader::MType mType);

  /**
 * Get the message type to send when the Send method is called.
 */
  LoraMacHeader::MType GetMType (void);

  /**
   * Parse and take action on the commands contained on this FrameHeader.
   */
  void ParseCommands (LoraFrameHeader frameHeader);

  /**
   * Perform the actions that need to be taken when receiving a LinkCheckAns command.
   *
   * \param margin The margin value of the command.
   * \param gwCnt The gateway count value of the command.
   */
  void OnLinkCheckAns (uint8_t margin, uint8_t gwCnt);

  /**
   * Perform the actions that need to be taken when receiving a LinkAdrReq command.
   *
   * \param dataRate The data rate value of the command.
   * \param txPower The transmission power value of the command.
   * \param enabledChannels A list of the enabled channels.
   * \param repetitions The number of repetitions prescribed by the command.
   */
  void OnLinkAdrReq (uint8_t dataRate, uint8_t txPower,
                     std::list<int> enabledChannels, int repetitions);

  /**
   * Perform the actions that need to be taken when receiving a DutyCycleReq command.
   *
   * \param dutyCycle The aggregate duty cycle prescribed by the command, in
   * fraction form.
   */
  void OnDutyCycleReq (double dutyCycle);

  /**
   * Perform the actions that need to be taken when receiving a RxParamSetupReq command.
   *
   * \param rx1DrOffset The offset to set.
   * \param rx2DataRate The data rate to use for the second receive window.
   * \param frequency The frequency to use for the second receive window.
   */
  void OnRxParamSetupReq (uint8_t rx1DrOffset, uint8_t rx2DataRate, double frequency);

  /**
   * Perform the actions that need to be taken when receiving a DevStatusReq command.
   */
  void OnDevStatusReq (void);

  /**
   * Perform the actions that need to be taken when receiving a NewChannelReq command.
   */
  void OnNewChannelReq (uint8_t chIndex, double frequency, uint8_t minDataRate,
                        uint8_t maxDataRate);

  ////////////////////////////////////
  // Logical channel administration //
  ////////////////////////////////////

  /**
   * Add a logical channel to the helper.
   *
   * \param frequency The channel's center frequency.
   */
  void AddLogicalChannel (double frequency);

  /**
   * Set a new logical channel in the helper.
   *
   * \param chIndex The channel's new index.
   * \param frequency The channel's center frequency.
   * \param minDataRate The minimum data rate allowed on the channel.
   * \param maxDataRate The maximum data rate allowed on the channel.
   */
  void SetLogicalChannel (uint8_t chIndex, double frequency,
                          uint8_t minDataRate, uint8_t maxDataRate);

  /**
   * Add a logical channel to the helper.
   *
   * \param frequency The channel's center frequency.
   */
  void AddLogicalChannel (Ptr<LogicalLoraChannel> logicalChannel);

  /**
   * Add a subband to the logical channel helper.
   *
   * \param startFrequency The SubBand's lowest frequency.
   * \param endFrequency The SubBand's highest frequency.
   * \param dutyCycle The SubBand's duty cycle, in fraction form.
   * \param maxTxPowerDbm The maximum transmission power allowed on the SubBand.
   */
  void AddSubBand (double startFrequency, double endFrequency, double dutyCycle,
                   double maxTxPowerDbm);

private:
  /**
  * Structure representing the parameters that will be used in the
  * retransmission procedure.
  */
  struct LoraRetxParameters
  {
    Time firstAttempt;
    Ptr<Packet> packet = 0;
    bool waitingAck = false;
    uint8_t retxLeft;
  };

  /**
   * Enable Data Rate adaptation during the retransmission procedure.
   */
  bool m_enableDRAdapt;

  /**
   * Maximum number of transmission allowed.
   */
  uint8_t m_maxNumbTx;

  /**
   * Randomly shuffle a Ptr<LogicalLoraChannel> vector.
   *
   * Used to pick a random channel on which to send the packet.
   */
  std::vector<Ptr<LogicalLoraChannel> > Shuffle (std::vector<Ptr<LogicalLoraChannel> > vector);

  /**
    * Find the minimum waiting time before the next possible transmission.
    */
  Time GetNextTransmissionDelay (void);


  /**
   * Find a suitable channel for transmission. The channel is chosen among the
   * ones that are available in the ED's LogicalLoraChannel, based on their duty
   * cycle limitations.
   */
  Ptr<LogicalLoraChannel> GetChannelForTx (void);

  /**
   * An uniform random variable, used by the Shuffle method to randomly reorder
   * the channel list.
   */
  Ptr<UniformRandomVariable> m_uniformRV;


/**
   * The total number of transmissions required.
   */
/*
TracedValue<uint8_t> m_requiredTx;
*/

  /**
   * The DataRate this device is using to transmit.
   */
  TracedValue<uint8_t> m_dataRate;

  /**
   * The transmission power this device is using to transmit.
   */
  TracedValue<double> m_txPower;

  /**
   * The coding rate used by this device.
   */
  uint8_t m_codingRate;

  /**
   * Whether or not the header is disabled for communications by this device.
   */
  bool m_headerDisabled;

  /**
   * The interval between when a packet is done sending and when the first
   * receive window is opened.
   */
  Time m_receiveDelay1;

  /**
   * The interval between when a packet is done sending and when the second
   * receive window is opened.
   */
  Time m_receiveDelay2;

  /**
   * The duration of a receive window.
   */
  Time m_receiveWindowDuration;

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
   * The event of retransmitting a packet in a consecutive moment if an ACK is not received.
   *
   * This Event is used to cancel the retransmission if the ACK is found in ParseCommand function and
   * if a newer packet is delivered from the application to be sent.
   */
  EventId m_nextTx;

  /**
   * The event of transmitting a packet in a consecutive moment, when the duty cycle let us transmit.
   *
   * This Event is used to cancel the transmission of this packet if a newer packet is delivered from the application to be sent.
   */
  EventId m_nextRetx;
  /**
   * The address of this device.
   */
  LoraDeviceAddress m_address;

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

  /**
   * List of the MAC commands that need to be applied to the next UL packet.
   */
  std::list<Ptr<MacCommand> > m_macCommandList;

  /**
   * The aggregated duty cycle this device needs to respect across all sub-bands.
   */
  TracedValue<double> m_aggregatedDutyCycle;

  /**
   * The message type to apply to packets sent with the Send method.
   */
  LoraMacHeader::MType m_mType;

  /* Structure containing the retransmission parameters
   * for this device.
   */
  struct LoraRetxParameters m_retxParams;

  /////////////////
  //  Callbacks  //
  /////////////////

  /**
   * The trace source fired when the transmission procedure is finished.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<uint8_t, bool, Time, Ptr<Packet> > m_requiredTxCallback;

};


} /* namespace ns3 */

#endif /* END_DEVICE_LORA_MAC_H */
