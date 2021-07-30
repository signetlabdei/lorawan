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
 *
 * Modified by: Peggy Anderson <peggy.anderson@usask.ca>
 */

#include "ns3/end-device-lorawan-mac.h"
#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <algorithm>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("EndDeviceLorawanMac");

NS_OBJECT_ENSURE_REGISTERED (EndDeviceLorawanMac);

TypeId
EndDeviceLorawanMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EndDeviceLorawanMac")
    .SetParent<LorawanMac> ()
    .SetGroupName ("lorawan")
    .AddTraceSource ("RequiredTransmissions",
                     "Total number of transmissions required to deliver this packet",
                     MakeTraceSourceAccessor
                       (&EndDeviceLorawanMac::m_requiredTxCallback),
                     "ns3::TracedValueCallback::uint8_t")
    .AddAttribute ("DataRate",
                   "Data Rate currently employed by this end device",
                   UintegerValue (0),
                   MakeUintegerAccessor (&EndDeviceLorawanMac::m_dataRate),
                   MakeUintegerChecker<uint8_t> (0, 5))
    .AddTraceSource ("DataRate",
                     "Data Rate currently employed by this end device",
                     MakeTraceSourceAccessor
                       (&EndDeviceLorawanMac::m_dataRate),
                     "ns3::TracedValueCallback::uint8_t")
    .AddAttribute ("DRControl",
                   "Whether to request the NS to control this device's Data Rate",
                   BooleanValue (),
                   MakeBooleanAccessor (&EndDeviceLorawanMac::m_controlDataRate),
                   MakeBooleanChecker ())
    .AddTraceSource ("TxPower",
                     "Transmission power currently employed by this end device",
                     MakeTraceSourceAccessor
                       (&EndDeviceLorawanMac::m_txPower),
                     "ns3::TracedValueCallback::Double")
    .AddTraceSource ("LastKnownLinkMargin",
                     "Last known demodulation margin in "
                     "communications between this end device "
                     "and a gateway",
                     MakeTraceSourceAccessor
                       (&EndDeviceLorawanMac::m_lastKnownLinkMargin),
                     "ns3::TracedValueCallback::Double")
    .AddTraceSource ("LastKnownGatewayCount",
                     "Last known number of gateways able to "
                     "listen to this end device",
                     MakeTraceSourceAccessor
                       (&EndDeviceLorawanMac::m_lastKnownGatewayCount),
                     "ns3::TracedValueCallback::Int")
    .AddTraceSource ("AggregatedDutyCycle",
                     "Aggregate duty cycle, in fraction form, "
                     "this end device must respect",
                     MakeTraceSourceAccessor
                       (&EndDeviceLorawanMac::m_aggregatedDutyCycle),
                     "ns3::TracedValueCallback::Double")
    .AddAttribute ("MaxTransmissions",
                   "Maximum number of transmissions for a packet",
                   IntegerValue (8),
                   MakeIntegerAccessor (&EndDeviceLorawanMac::m_maxNumbTx),
                   MakeIntegerChecker<uint8_t> ())
    .AddAttribute ("EnableEDDataRateAdaptation",
                   "Whether the End Device should up its Data Rate "
                   "in case it doesn't get a reply from the NS.",
                   BooleanValue (false),
                   MakeBooleanAccessor (&EndDeviceLorawanMac::m_enableDRAdapt),
                   MakeBooleanChecker ())
    .AddAttribute ("MType",
                   "Specify type of message will be sent by this ED.",
                   EnumValue (LorawanMacHeader::UNCONFIRMED_DATA_UP),
                   MakeEnumAccessor (&EndDeviceLorawanMac::m_mType),
                   MakeEnumChecker (LorawanMacHeader::UNCONFIRMED_DATA_UP,
                                    "Unconfirmed",
                                    LorawanMacHeader::CONFIRMED_DATA_UP,
                                    "Confirmed"))
    .AddConstructor<EndDeviceLorawanMac> ();
  return tid;
}

EndDeviceLorawanMac::EndDeviceLorawanMac ()
    : m_enableDRAdapt (false),
      m_maxNumbTx (8),
      m_dataRate (0),
      m_txPower (14),
      m_codingRate (1),
      // LoraWAN default
      m_headerDisabled (0),
      // LoraWAN default
      m_address (LoraDeviceAddress (0)),
      // LoraWAN default
      m_receiveWindowDurationInSymbols (8),
      // LoraWAN default
      m_controlDataRate (false),
      m_lastKnownLinkMargin (0),
      m_lastKnownGatewayCount (0),
      m_aggregatedDutyCycle (1),
      m_mType (LorawanMacHeader::CONFIRMED_DATA_UP),
      m_currentFCnt (0)
{
  NS_LOG_FUNCTION (this);

  // Initialize the random variable we'll use to decide which channel to
  // transmit on.
  m_uniformRV = CreateObject<UniformRandomVariable> ();

  // Void the transmission event
  m_nextTx = EventId ();
  m_nextTx.Cancel ();

  // Initialize structure for retransmission parameters
  m_retxParams = EndDeviceLorawanMac::LoraRetxParameters ();
  m_retxParams.retxLeft = m_maxNumbTx;
}

EndDeviceLorawanMac::~EndDeviceLorawanMac ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

////////////////////////
//  Sending methods   //
////////////////////////

void
EndDeviceLorawanMac::Send (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);

  // If it is not possible to transmit now because of the duty cycle,
  // or because we are receiving, schedule a tx/retx later

  Time netxTxDelay = GetNextTransmissionDelay ();
  if (netxTxDelay != Seconds (0))
    {
      postponeTransmission (netxTxDelay, packet);
      return;
    }

  // Pick a channel on which to transmit the packet
  Ptr<LogicalLoraChannel> txChannel = GetChannelForTx ();

  if (!(txChannel && m_retxParams.retxLeft > 0))
    {
      if (!txChannel)
        {
          m_cannotSendBecauseDutyCycle (packet);
        }
      else
        {
          NS_LOG_INFO ("Max number of transmission achieved: packet not transmitted.");
        }
    }
  else
  // the transmitting channel is available and we have not run out the maximum number of retransmissions
    {
      // Make sure we can transmit at the current power on this channel
      NS_ASSERT_MSG (m_txPower <= m_channelHelper.GetTxPowerForChannel (txChannel),
                     " The selected power is too hight to be supported by this channel.");
      DoSend (packet);
    }
}

void
EndDeviceLorawanMac::postponeTransmission (Time netxTxDelay, Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  // Delete previously scheduled transmissions if any.
  Simulator::Cancel (m_nextTx);
  m_nextTx = Simulator::Schedule (netxTxDelay, &EndDeviceLorawanMac::DoSend, this, packet);
  NS_LOG_WARN ("Attempting to send, but the aggregate duty cycle won't allow it. Scheduling a tx at a delay "
               << netxTxDelay.GetSeconds () << ".");
}


void
EndDeviceLorawanMac::DoSend (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this);
  // Checking if this is the transmission of a new packet
  if (packet != m_retxParams.packet)
    {
      NS_LOG_DEBUG ("Received a new packet from application. Resetting retransmission parameters.");
      m_currentFCnt++;
      NS_LOG_DEBUG ("APP packet: " << packet << ".");

      // Add the Lora Frame Header to the packet
      LoraFrameHeader frameHdr;
      ApplyNecessaryOptions (frameHdr);
      packet->AddHeader (frameHdr);

      NS_LOG_INFO ("Added frame header of size " << frameHdr.GetSerializedSize () <<
                   " bytes.");

      // Check that MACPayload length is below the allowed maximum
      if (packet->GetSize () > m_maxAppPayloadForDataRate.at (m_dataRate))
        {
          NS_LOG_WARN ("Attempting to send a packet larger than the maximum allowed"
                       << " size at this DataRate (DR" << unsigned(m_dataRate) <<
                       "). Transmission canceled.");
          return;
        }


      // Add the Lora Mac header to the packet
      LorawanMacHeader macHdr;
      ApplyNecessaryOptions (macHdr);
      packet->AddHeader (macHdr);

      // Reset MAC command list
      m_macCommandList.clear ();

      if (m_retxParams.waitingAck)
        {
          // Call the callback to notify about the failure
          uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
          m_requiredTxCallback (txs, false, m_retxParams.firstAttempt, m_retxParams.packet);
          NS_LOG_DEBUG (" Received new packet from the application layer: stopping retransmission procedure. Used " <<
                        unsigned(txs) << " transmissions out of a maximum of " << unsigned(m_maxNumbTx) << ".");
        }

      // Reset retransmission parameters
      resetRetransmissionParameters ();

      // If this is the first transmission of a confirmed packet, save parameters for the (possible) next retransmissions.
      if (m_mType == LorawanMacHeader::CONFIRMED_DATA_UP)
        {
          m_retxParams.packet = packet->Copy ();
          m_retxParams.retxLeft = m_maxNumbTx;
          m_retxParams.waitingAck = true;
          m_retxParams.firstAttempt = Simulator::Now ();
          m_retxParams.retxLeft = m_retxParams.retxLeft - 1;       // decreasing the number of retransmissions

          NS_LOG_DEBUG ("Message type is " << m_mType);
          NS_LOG_DEBUG ("It is a confirmed packet. Setting retransmission parameters and decreasing the number of transmissions left.");

          NS_LOG_INFO ("Added MAC header of size " << macHdr.GetSerializedSize () <<
                       " bytes.");

          // Sent a new packet
          NS_LOG_DEBUG ("Copied packet: " << m_retxParams.packet);
          m_sentNewPacket (m_retxParams.packet);

          // static_cast<ClassAEndDeviceLorawanMac*>(this)->SendToPhy (m_retxParams.packet);
          SendToPhy (m_retxParams.packet);
        }
      else
        {
          m_sentNewPacket (packet);
          // static_cast<ClassAEndDeviceLorawanMac*>(this)->SendToPhy (packet);
          SendToPhy (packet);
        }

    }
  // this is a retransmission
  else
    {
      if (m_retxParams.waitingAck)
        {

          // Remove the headers
          LorawanMacHeader macHdr;
          LoraFrameHeader frameHdr;
          packet->RemoveHeader(macHdr);
          packet->RemoveHeader(frameHdr);

          // Add the Lora Frame Header to the packet
          frameHdr = LoraFrameHeader ();
          ApplyNecessaryOptions (frameHdr);
          packet->AddHeader (frameHdr);

          NS_LOG_INFO ("Added frame header of size " << frameHdr.GetSerializedSize () <<
                       " bytes.");

          // Add the Lorawan Mac header to the packet
          macHdr = LorawanMacHeader ();
          ApplyNecessaryOptions (macHdr);
          packet->AddHeader (macHdr);
          m_retxParams.retxLeft = m_retxParams.retxLeft - 1;           // decreasing the number of retransmissions
          NS_LOG_DEBUG ("Retransmitting an old packet.");

          // static_cast<ClassAEndDeviceLorawanMac*>(this)->SendToPhy (m_retxParams.packet);
          SendToPhy (m_retxParams.packet);
        }
    }

}

void
EndDeviceLorawanMac::SendToPhy (Ptr<Packet> packet)
{ }

//////////////////////////
//  Receiving methods   //
//////////////////////////

void
EndDeviceLorawanMac::Receive (Ptr<Packet const> packet)
{ }

void
EndDeviceLorawanMac::FailedReception (Ptr<Packet const> packet)
{ }

void
EndDeviceLorawanMac::ParseCommands (LoraFrameHeader frameHeader)
{
  NS_LOG_FUNCTION (this << frameHeader);

  if (m_retxParams.waitingAck)
    {
      if (frameHeader.GetAck ())
        {
          NS_LOG_INFO ("The message is an ACK, not waiting for it anymore.");

          NS_LOG_DEBUG ("Reset retransmission variables to default values and cancel retransmission if already scheduled.");

          uint8_t txs = m_maxNumbTx - (m_retxParams.retxLeft);
          m_requiredTxCallback (txs, true, m_retxParams.firstAttempt, m_retxParams.packet);
          NS_LOG_DEBUG ("Received ACK packet after " << unsigned(txs) << " transmissions: stopping retransmission procedure. ");

          // Reset retransmission parameters
          resetRetransmissionParameters ();

        }
      else
        {
          NS_LOG_ERROR ("Received downlink message not containing an ACK while we were waiting for it!");
        }
    }

  std::list<Ptr<MacCommand> > commands = frameHeader.GetCommands ();
  std::list<Ptr<MacCommand> >::iterator it;
  for (it = commands.begin (); it != commands.end (); it++)
    {
      NS_LOG_DEBUG ("Iterating over the MAC commands...");
      enum MacCommandType type = (*it)->GetCommandType ();
      switch (type)
        {
        case (LINK_CHECK_ANS):
          {
            NS_LOG_DEBUG ("Detected a LinkCheckAns command.");

            // Cast the command
            Ptr<LinkCheckAns> linkCheckAns = (*it)->GetObject<LinkCheckAns> ();

            // Call the appropriate function to take action
            OnLinkCheckAns (linkCheckAns->GetMargin (), linkCheckAns->GetGwCnt ());

            break;
          }
        case (LINK_ADR_REQ):
          {
            NS_LOG_DEBUG ("Detected a LinkAdrReq command.");

            // Cast the command
            Ptr<LinkAdrReq> linkAdrReq = (*it)->GetObject<LinkAdrReq> ();

            // Call the appropriate function to take action
            OnLinkAdrReq (linkAdrReq->GetDataRate (), linkAdrReq->GetTxPower (),
                          linkAdrReq->GetEnabledChannelsList (),
                          linkAdrReq->GetRepetitions ());

            break;
          }
        case (DUTY_CYCLE_REQ):
          {
            NS_LOG_DEBUG ("Detected a DutyCycleReq command.");

            // Cast the command
            Ptr<DutyCycleReq> dutyCycleReq = (*it)->GetObject<DutyCycleReq> ();

            // Call the appropriate function to take action
            OnDutyCycleReq (dutyCycleReq->GetMaximumAllowedDutyCycle ());

            break;
          }
        case (RX_PARAM_SETUP_REQ):
          {
            NS_LOG_DEBUG ("Detected a RxParamSetupReq command.");

            // Cast the command
            Ptr<RxParamSetupReq> rxParamSetupReq = (*it)->GetObject<RxParamSetupReq> ();

            // Call the appropriate function to take action
            OnRxParamSetupReq (rxParamSetupReq);

            break;
          }
        case (DEV_STATUS_REQ):
          {
            NS_LOG_DEBUG ("Detected a DevStatusReq command.");

            // Cast the command
            Ptr<DevStatusReq> devStatusReq = (*it)->GetObject<DevStatusReq> ();

            // Call the appropriate function to take action
            OnDevStatusReq ();

            break;
          }
        case (NEW_CHANNEL_REQ):
          {
            NS_LOG_DEBUG ("Detected a NewChannelReq command.");

            // Cast the command
            Ptr<NewChannelReq> newChannelReq = (*it)->GetObject<NewChannelReq> ();

            // Call the appropriate function to take action
            OnNewChannelReq (newChannelReq->GetChannelIndex (), newChannelReq->GetFrequency (), newChannelReq->GetMinDataRate (), newChannelReq->GetMaxDataRate ());

            break;
          }
        case (RX_TIMING_SETUP_REQ):
          {
            break;
          }
        case (TX_PARAM_SETUP_REQ):
          {
            break;
          }
        case (DL_CHANNEL_REQ):
          {
            break;
          }
        default:
          {
            NS_LOG_ERROR ("CID not recognized");
            break;
          }
        }
    }

}

void
EndDeviceLorawanMac::ApplyNecessaryOptions (LoraFrameHeader& frameHeader)
{
  NS_LOG_FUNCTION_NOARGS ();

  frameHeader.SetAsUplink ();
  frameHeader.SetFPort (1);                     // TODO Use an appropriate frame port based on the application
  frameHeader.SetAddress (m_address);
  frameHeader.SetAdr (m_controlDataRate);
  frameHeader.SetAdrAckReq (0); // TODO Set ADRACKREQ if a member variable is true

  // FPending does not exist in uplink messages
  frameHeader.SetFCnt (m_currentFCnt);

  // Add listed MAC commands
  for (const auto &command : m_macCommandList)
    {
      NS_LOG_INFO ("Applying a MAC Command of CID " <<
                   unsigned(MacCommand::GetCIDFromMacCommand
                              (command->GetCommandType ())));

      frameHeader.AddCommand (command);
    }

}

void
EndDeviceLorawanMac::ApplyNecessaryOptions (LorawanMacHeader& macHeader)
{
  NS_LOG_FUNCTION_NOARGS ();

  macHeader.SetMType (m_mType);
  macHeader.SetMajor (1);
}

void
EndDeviceLorawanMac::SetMType (LorawanMacHeader::MType mType)
{
  m_mType = mType;
  NS_LOG_DEBUG ("Message type is set to " << mType);
}

LorawanMacHeader::MType
EndDeviceLorawanMac::GetMType (void)
{
  return m_mType;
}

void
EndDeviceLorawanMac::TxFinished (Ptr<const Packet> packet)
{ }

Time
EndDeviceLorawanMac::GetNextClassTransmissionDelay (Time waitingTime)
{
  NS_LOG_FUNCTION_NOARGS ();
  return waitingTime;
}

Time
EndDeviceLorawanMac::GetNextTransmissionDelay (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  //    Check duty cycle    //

  // Pick a random channel to transmit on
  std::vector<Ptr<LogicalLoraChannel> > logicalChannels;
  logicalChannels = m_channelHelper.GetEnabledChannelList ();                 // Use a separate list to do the shuffle
  //logicalChannels = Shuffle (logicalChannels);


  Time waitingTime = Time::Max ();

  // Try every channel
  std::vector<Ptr<LogicalLoraChannel> >::iterator it;
  for (it = logicalChannels.begin (); it != logicalChannels.end (); ++it)
    {
      // Pointer to the current channel
      Ptr<LogicalLoraChannel> logicalChannel = *it;
      double frequency = logicalChannel->GetFrequency ();

      waitingTime = std::min (waitingTime, m_channelHelper.GetWaitingTime (logicalChannel));

      NS_LOG_DEBUG ("Waiting time before the next transmission in channel with frequecy " <<
                    frequency << " is = " << waitingTime.GetSeconds () << ".");
    }

  waitingTime = GetNextClassTransmissionDelay (waitingTime);

  return waitingTime;
}

Ptr<LogicalLoraChannel>
EndDeviceLorawanMac::GetChannelForTx (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Pick a random channel to transmit on
  std::vector<Ptr<LogicalLoraChannel> > logicalChannels;
  logicalChannels = m_channelHelper.GetEnabledChannelList ();                 // Use a separate list to do the shuffle
  logicalChannels = Shuffle (logicalChannels);

  // Try every channel
  std::vector<Ptr<LogicalLoraChannel> >::iterator it;
  for (it = logicalChannels.begin (); it != logicalChannels.end (); ++it)
    {
      // Pointer to the current channel
      Ptr<LogicalLoraChannel> logicalChannel = *it;
      double frequency = logicalChannel->GetFrequency ();

      NS_LOG_DEBUG ("Frequency of the current channel: " << frequency);

      // Verify that we can send the packet
      Time waitingTime = m_channelHelper.GetWaitingTime (logicalChannel);

      NS_LOG_DEBUG ("Waiting time for current channel = " <<
                    waitingTime.GetSeconds ());

      // Send immediately if we can
      if (waitingTime == Seconds (0))
        {
          return *it;
        }
      else
        {
          NS_LOG_DEBUG ("Packet cannot be immediately transmitted on " <<
                        "the current channel because of duty cycle limitations.");
        }
    }
  return 0;                 // In this case, no suitable channel was found
}


std::vector<Ptr<LogicalLoraChannel> >
EndDeviceLorawanMac::Shuffle (std::vector<Ptr<LogicalLoraChannel> > vector)
{
  NS_LOG_FUNCTION_NOARGS ();

  int size = vector.size ();

  for (int i = 0; i < size; ++i)
    {
      uint16_t random = std::floor (m_uniformRV->GetValue (0, size));
      Ptr<LogicalLoraChannel> temp = vector.at (random);
      vector.at (random) = vector.at (i);
      vector.at (i) = temp;
    }

  return vector;
}

/////////////////////////
// Setters and Getters //
/////////////////////////

void EndDeviceLorawanMac::resetRetransmissionParameters ()
{
  m_retxParams.waitingAck = false;
  m_retxParams.retxLeft = m_maxNumbTx;
  m_retxParams.packet = 0;
  m_retxParams.firstAttempt = Seconds (0);

  // Cancel next retransmissions, if any
  Simulator::Cancel (m_nextTx);
}

void
EndDeviceLorawanMac::SetDataRateAdaptation (bool adapt)
{
  NS_LOG_FUNCTION (this << adapt);
  m_enableDRAdapt = adapt;
}

bool
EndDeviceLorawanMac::GetDataRateAdaptation (void)
{
  return m_enableDRAdapt;
}

void
EndDeviceLorawanMac::SetMaxNumberOfTransmissions (uint8_t maxNumbTx)
{
  NS_LOG_FUNCTION (this << unsigned(maxNumbTx));
  m_maxNumbTx = maxNumbTx;
  m_retxParams.retxLeft = maxNumbTx;
}

uint8_t
EndDeviceLorawanMac::GetMaxNumberOfTransmissions (void)
{
  NS_LOG_FUNCTION (this );
  return m_maxNumbTx;
}


void
EndDeviceLorawanMac::SetDataRate (uint8_t dataRate)
{
  NS_LOG_FUNCTION (this << unsigned (dataRate));

  m_dataRate = dataRate;
}

uint8_t
EndDeviceLorawanMac::GetDataRate (void)
{
  NS_LOG_FUNCTION (this);

  return m_dataRate;
}

void
EndDeviceLorawanMac::SetDeviceAddress (LoraDeviceAddress address)
{
  NS_LOG_FUNCTION (this << address);

  m_address = address;
}

LoraDeviceAddress
EndDeviceLorawanMac::GetDeviceAddress (void)
{
  NS_LOG_FUNCTION (this);

  return m_address;
}

void
EndDeviceLorawanMac::OnLinkCheckAns (uint8_t margin, uint8_t gwCnt)
{
  NS_LOG_FUNCTION (this << unsigned(margin) << unsigned(gwCnt));

  m_lastKnownLinkMargin = margin;
  m_lastKnownGatewayCount = gwCnt;
}

void
EndDeviceLorawanMac::OnLinkAdrReq (uint8_t dataRate, uint8_t txPower,
                                std::list<int> enabledChannels, int repetitions)
{
  NS_LOG_FUNCTION (this << unsigned (dataRate) << unsigned (txPower) <<
                   repetitions);

  // Three bools for three requirements before setting things up
  bool channelMaskOk = true;
  bool dataRateOk = true;
  bool txPowerOk = true;

  // Check the channel mask
  /////////////////////////
  // Check whether all specified channels exist on this device
  auto channelList = m_channelHelper.GetChannelList ();
  int channelListSize = channelList.size ();

  for (auto it = enabledChannels.begin (); it != enabledChannels.end (); it++)
    {
      if ((*it) > channelListSize)
        {
          channelMaskOk = false;
          break;
        }
    }

  // Check the dataRate
  /////////////////////
  // We need to know we can use it at all
  // To assess this, we try and convert it to a SF/BW combination and check if
  // those values are valid. Since GetSfFromDataRate and
  // GetBandwidthFromDataRate return 0 if the dataRate is not recognized, we
  // can check against this.
  uint8_t sf = GetSfFromDataRate (dataRate);
  double bw = GetBandwidthFromDataRate (dataRate);
  NS_LOG_DEBUG ("SF: " << unsigned (sf) << ", BW: " << bw);
  if (sf == 0 || bw == 0)
    {
      dataRateOk = false;
      NS_LOG_DEBUG ("Data rate non valid");
    }

  // We need to know we can use it in at least one of the enabled channels
  // Cycle through available channels, stop when at least one is enabled for the
  // specified dataRate.
  if (dataRateOk && channelMaskOk)                 // If false, skip the check
    {
      bool foundAvailableChannel = false;
      for (auto it = enabledChannels.begin (); it != enabledChannels.end (); it++)
        {
          NS_LOG_DEBUG ("MinDR: " << unsigned (channelList.at (*it)->GetMinimumDataRate ()));
          NS_LOG_DEBUG ("MaxDR: " << unsigned (channelList.at (*it)->GetMaximumDataRate ()));
          if (channelList.at (*it)->GetMinimumDataRate () <= dataRate
              && channelList.at (*it)->GetMaximumDataRate () >= dataRate)
            {
              foundAvailableChannel = true;
              break;
            }
        }

      if (!foundAvailableChannel)
        {
          dataRateOk = false;
          NS_LOG_DEBUG ("Available channel not found");
        }
    }

  // Check the txPower
  ////////////////////
  // Check whether we can use this transmission power
  if (GetDbmForTxPower (txPower) == 0)
    {
      txPowerOk = false;
    }

  NS_LOG_DEBUG ("Finished checking. " <<
                "ChannelMaskOk: " << channelMaskOk << ", " <<
                "DataRateOk: " << dataRateOk << ", " <<
                "txPowerOk: " << txPowerOk);

  // If all checks are successful, set parameters up
  //////////////////////////////////////////////////
  if (channelMaskOk && dataRateOk && txPowerOk)
    {
      // Cycle over all channels in the list
      for (uint32_t i = 0; i < m_channelHelper.GetChannelList ().size (); i++)
        {
          if (std::find (enabledChannels.begin (), enabledChannels.end (), i) != enabledChannels.end ())
            {
              m_channelHelper.GetChannelList ().at (i)->SetEnabledForUplink ();
              NS_LOG_DEBUG ("Channel " << i << " enabled");
            }
          else
            {
              m_channelHelper.GetChannelList ().at (i)->DisableForUplink ();
              NS_LOG_DEBUG ("Channel " << i << " disabled");
            }
        }

      // Set the data rate
      m_dataRate = dataRate;

      // Set the transmission power
      m_txPower = GetDbmForTxPower (txPower);
    }

  // Craft a LinkAdrAns MAC command as a response
  ///////////////////////////////////////////////
  m_macCommandList.push_back (CreateObject<LinkAdrAns> (txPowerOk, dataRateOk,
                                                        channelMaskOk));
}

void
EndDeviceLorawanMac::OnDutyCycleReq (double dutyCycle)
{
  NS_LOG_FUNCTION (this << dutyCycle);

  // Make sure we get a value that makes sense
  NS_ASSERT (0 <= dutyCycle && dutyCycle < 1);

  // Set the new duty cycle value
  m_aggregatedDutyCycle = dutyCycle;

  // Craft a DutyCycleAns as response
  NS_LOG_INFO ("Adding DutyCycleAns reply");
  m_macCommandList.push_back (CreateObject<DutyCycleAns> ());
}

void
EndDeviceLorawanMac::OnRxClassParamSetupReq (Ptr<RxParamSetupReq> rxParamSetupReq)
{ }

void
EndDeviceLorawanMac::OnRxParamSetupReq (Ptr<RxParamSetupReq> rxParamSetupReq)
{
  NS_LOG_FUNCTION (this << rxParamSetupReq);

  // static_cast<ClassAEndDeviceLorawanMac*>(this)->OnRxClassParamSetupReq (rxParamSetupReq);
  OnRxClassParamSetupReq (rxParamSetupReq);
}

void
EndDeviceLorawanMac::OnDevStatusReq (void)
{
  NS_LOG_FUNCTION (this);

  uint8_t battery = 10;                     // XXX Fake battery level
  uint8_t margin = 10;                     // XXX Fake margin

  // Craft a RxParamSetupAns as response
  NS_LOG_INFO ("Adding DevStatusAns reply");
  m_macCommandList.push_back (CreateObject<DevStatusAns> (battery, margin));
}

void
EndDeviceLorawanMac::OnNewChannelReq (uint8_t chIndex, double frequency, uint8_t minDataRate, uint8_t maxDataRate)
{
  NS_LOG_FUNCTION (this);


  bool dataRateRangeOk = true;                     // XXX Check whether the new data rate range is ok
  bool channelFrequencyOk = true;                     // XXX Check whether the frequency is ok

  // TODO Return false if one of the checks above failed
  // TODO Create new channel in the LogicalLoraChannelHelper

  SetLogicalChannel (chIndex, frequency, minDataRate, maxDataRate);

  NS_LOG_INFO ("Adding NewChannelAns reply");
  m_macCommandList.push_back (CreateObject<NewChannelAns> (dataRateRangeOk,
                                                           channelFrequencyOk));
}

void
EndDeviceLorawanMac::AddLogicalChannel (double frequency)
{
  NS_LOG_FUNCTION (this << frequency);

  m_channelHelper.AddChannel (frequency);
}

void
EndDeviceLorawanMac::AddLogicalChannel (Ptr<LogicalLoraChannel> logicalChannel)
{
  NS_LOG_FUNCTION (this << logicalChannel);

  m_channelHelper.AddChannel (logicalChannel);
}

void
EndDeviceLorawanMac::SetLogicalChannel (uint8_t chIndex, double frequency,
                                     uint8_t minDataRate, uint8_t maxDataRate)
{
  NS_LOG_FUNCTION (this << unsigned (chIndex) << frequency <<
                   unsigned (minDataRate) << unsigned(maxDataRate));

  m_channelHelper.SetChannel (chIndex, CreateObject<LogicalLoraChannel>
                                (frequency, minDataRate, maxDataRate));
}

void
EndDeviceLorawanMac::AddSubBand (double startFrequency, double endFrequency, double dutyCycle, double maxTxPowerDbm)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_channelHelper.AddSubBand (startFrequency, endFrequency, dutyCycle, maxTxPowerDbm);
}

double
EndDeviceLorawanMac::GetAggregatedDutyCycle (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_aggregatedDutyCycle;
}

void
EndDeviceLorawanMac::AddMacCommand (Ptr<MacCommand> macCommand)
{
  NS_LOG_FUNCTION (this << macCommand);

  m_macCommandList.push_back (macCommand);
}

uint8_t
EndDeviceLorawanMac::GetTransmissionPower (void)
{
  return m_txPower;
}
}
}
