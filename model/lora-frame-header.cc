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

#include "ns3/lora-frame-header.h"
#include "ns3/log.h"
#include <bitset>

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("LoraFrameHeader");

// Initialization list
LoraFrameHeader::LoraFrameHeader () :
  m_fPort     (0),
  m_address   (LoraDeviceAddress (0,0)),
  m_adr       (0),
  m_adrAckReq (0),
  m_ack       (0),
  m_fPending  (0),
  m_fOptsLen  (0),
  m_fCnt      (0)
{
}

LoraFrameHeader::~LoraFrameHeader ()
{
}

TypeId
LoraFrameHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("LoraFrameHeader")
    .SetParent<Header> ()
    .AddConstructor<LoraFrameHeader> ()
  ;
  return tid;
}

TypeId
LoraFrameHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
LoraFrameHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Sizes in bytes:
  // 4 for DevAddr + 1 for FCtrl + 2 for FCnt + 1 for FPort + 0-15 for FOpts
  uint32_t size = 8 + m_fOptsLen;

  NS_LOG_INFO ("LoraFrameHeader serialized size: " << size);

  return size;
}

void
LoraFrameHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION_NOARGS ();

  // Device Address field
  start.WriteU32 (m_address.Get ());

  // fCtrl field
  uint8_t fCtrl = 0;
  fCtrl |= uint8_t (m_adr << 6 & 0b1000000);
  fCtrl |= uint8_t (m_adrAckReq << 5 & 0b100000);
  fCtrl |= uint8_t (m_ack << 4 & 0b10000);
  fCtrl |= uint8_t (m_fPending << 3 & 0b1000);
  fCtrl |= m_fOptsLen & 0b111;
  start.WriteU8 (fCtrl);

  // FCnt field
  start.WriteU16 (m_fCnt);

  // FOpts field
  for (auto it = m_macCommands.begin (); it != m_macCommands.end (); it++)
    {
      NS_LOG_DEBUG ("Serializing a MAC command");
      (*it)->Serialize (start);
    }

  // FPort
  start.WriteU8 (m_fPort);
}

uint32_t
LoraFrameHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION_NOARGS ();

  // Empty the list of MAC commands
  m_macCommands.clear ();

  // Read from buffer and save into local variables
  m_address.Set (start.ReadU32 ());
  uint8_t fCtl = start.ReadU8 ();
  m_adr = (fCtl >> 6) & 0b1;
  m_adrAckReq = (fCtl >> 5) & 0b1;
  m_ack = (fCtl >> 4) & 0b1;
  m_fPending = (fCtl >> 3) & 0b1;
  m_fOptsLen = fCtl & 0b111;
  m_fCnt = start.ReadU16 ();

  NS_LOG_DEBUG ("Deserialized data: ");
  NS_LOG_DEBUG ("Address: " << m_address.Print ());
  NS_LOG_DEBUG ("ADR: " << unsigned(m_adr));
  NS_LOG_DEBUG ("ADRAckReq: " << unsigned (m_adrAckReq));
  NS_LOG_DEBUG ("Ack: " << unsigned (m_ack));
  NS_LOG_DEBUG ("fPending: " << unsigned (m_fPending));
  NS_LOG_DEBUG ("fOptsLen: " << unsigned (m_fOptsLen));
  NS_LOG_DEBUG ("fCnt: " << unsigned (m_fCnt));

  // Deserialize MAC commands
  NS_LOG_DEBUG ("Starting deserialization of MAC commands");
  for (uint8_t byteNumber = 0; byteNumber < m_fOptsLen;)
    {
      uint8_t cid = start.PeekU8 ();
      NS_LOG_DEBUG ("CID: " << unsigned(cid));

      // Divide Uplink and Downlink messages
      // This needs to be done because they have the same CID, and the context
      // about where this message will be Serialized/Deserialized (i.e., at the
      // ED or at the NS) is umportant.
      if (m_isUplink)
        {
          switch (cid)
            {
            // In the case of Uplink messages, the NS will deserialize the
            // request for a link check
            case (0x02):
              {
                NS_LOG_DEBUG ("Creating a LinkCheckReq command");
                Ptr<LinkCheckReq> command = Create <LinkCheckReq> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x03):
              {
                NS_LOG_DEBUG ("Creating a LinkAdrAns command");
                Ptr<LinkAdrAns> command = Create <LinkAdrAns> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x04):
              {
                NS_LOG_DEBUG ("Creating a DutyCycleAns command");
                Ptr<DutyCycleAns> command = Create <DutyCycleAns> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x05):
              {
                NS_LOG_DEBUG ("Creating a RxParamSetupAns command");
                Ptr<RxParamSetupAns> command = Create <RxParamSetupAns> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x06):
              {
                NS_LOG_DEBUG ("Creating a DevStatusAns command");
                Ptr<DevStatusAns> command = Create <DevStatusAns> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x07):
              {
                NS_LOG_DEBUG ("Creating a NewChannelAns command");
                Ptr<NewChannelAns> command = Create <NewChannelAns> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x08):
              {
                NS_LOG_DEBUG ("Creating a RxTimingSetupAns command");
                Ptr<RxTimingSetupAns> command = Create <RxTimingSetupAns> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x09):
              {
                NS_LOG_DEBUG ("Creating a TxParamSetupAns command");
                Ptr<TxParamSetupAns> command = Create <TxParamSetupAns> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x0A):
              {
                NS_LOG_DEBUG ("Creating a DlChannelAns command");
                Ptr<DlChannelAns> command = Create <DlChannelAns> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            default:
              {
                NS_LOG_ERROR ("CID not recognized during deserialization");
              }
            }
        }
      else
        {
          switch (cid)
            {
            // In the case of Downlink messages, the ED will deserialize the
            // answer to a link check
            case (0x02):
              {
                NS_LOG_DEBUG ("Creating a LinkCheckAns command");
                Ptr<LinkCheckAns> command = Create <LinkCheckAns> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x03):
              {
                NS_LOG_DEBUG ("Creating a LinkAdrReq command");
                Ptr<LinkAdrReq> command = Create <LinkAdrReq> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x04):
              {
                NS_LOG_DEBUG ("Creating a DutyCycleReq command");
                Ptr<DutyCycleReq> command = Create <DutyCycleReq> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x05):
              {
                NS_LOG_DEBUG ("Creating a RxParamSetupReq command");
                Ptr<RxParamSetupReq> command = Create <RxParamSetupReq> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x06):
              {
                NS_LOG_DEBUG ("Creating a DevStatusReq command");
                Ptr<DevStatusReq> command = Create <DevStatusReq> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x07):
              {
                NS_LOG_DEBUG ("Creating a NewChannelReq command");
                Ptr<NewChannelReq> command = Create <NewChannelReq> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x08):
              {
                NS_LOG_DEBUG ("Creating a RxTimingSetupReq command");
                Ptr<RxTimingSetupReq> command = Create <RxTimingSetupReq> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            case (0x09):
              {
                NS_LOG_DEBUG ("Creating a TxParamSetupReq command");
                Ptr<TxParamSetupReq> command = Create <TxParamSetupReq> ();
                byteNumber += command->Deserialize (start);
                m_macCommands.push_back (command);
                break;
              }
            default:
              {
                NS_LOG_ERROR ("CID not recognized during deserialization");
              }
            }
        }
    }

  m_fPort = uint8_t (start.ReadU8 ());

  return 8 + m_fOptsLen;       // the number of bytes consumed.
}

void
LoraFrameHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION_NOARGS ();

  os << "Address=" << m_address.Print () << std::endl;
  os << "ADR=" << m_adr << std::endl;
  os << "ADRAckReq=" << m_adrAckReq << std::endl;
  os << "ACK=" << m_ack << std::endl;
  os << "FPending=" << m_fPending << std::endl;
  os << "FOptsLen=" << unsigned(m_fOptsLen) << std::endl;
  os << "FCnt=" << unsigned(m_fCnt) << std::endl;

  for (auto it = m_macCommands.begin (); it != m_macCommands.end (); it++)
    {
      (*it)->Print (os);
    }

  os << "FPort=" << unsigned(m_fPort) << std::endl;
}

void
LoraFrameHeader::SetAsUplink (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_isUplink = true;
}

void
LoraFrameHeader::SetAsDownlink (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  m_isUplink = false;
}

void
LoraFrameHeader::SetFPort (uint8_t fPort)
{
  m_fPort = fPort;
}

uint8_t
LoraFrameHeader::GetFPort (void) const
{
  return m_fPort;
}

void
LoraFrameHeader::SetAddress (LoraDeviceAddress address)
{
  m_address = address;
}

LoraDeviceAddress
LoraFrameHeader::GetAddress (void) const
{
  return m_address;
}

void
LoraFrameHeader::SetAdr (bool adr)
{
  NS_LOG_FUNCTION (this << adr);
  m_adr = adr;
}
bool
LoraFrameHeader::GetAdr (void) const
{
  return m_adr;
}

void
LoraFrameHeader::SetAdrAckReq (bool adrAckReq)
{
  m_adrAckReq = adrAckReq;
}
bool
LoraFrameHeader::GetAdrAckReq (void) const
{
  return m_adrAckReq;
}

void
LoraFrameHeader::SetAck (bool ack)
{
  NS_LOG_FUNCTION (this << ack);
  m_ack = ack;
}
bool
LoraFrameHeader::GetAck (void) const
{
  return m_ack;
}

void
LoraFrameHeader::SetFPending (bool fPending)
{
  m_fPending = fPending;
}
bool
LoraFrameHeader::GetFPending (void) const
{
  return m_fPending;
}

uint8_t
LoraFrameHeader::GetFOptsLen (void) const
{
  // Sum the serialized lenght of all commands in the list
  uint8_t fOptsLen = 0;
  std::list< Ptr< MacCommand> >::const_iterator it;
  for (it = m_macCommands.begin (); it != m_macCommands.end (); it++)
    {
      fOptsLen = fOptsLen + (*it)->GetSerializedSize ();
    }
  return fOptsLen;
}

void
LoraFrameHeader::SetFCnt (uint16_t fCnt)
{
  m_fCnt = fCnt;
}

uint16_t
LoraFrameHeader::GetFCnt (void) const
{
  return m_fCnt;
}

void
LoraFrameHeader::AddLinkCheckReq (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  Ptr<LinkCheckReq> command = Create<LinkCheckReq> ();
  m_macCommands.push_back (command);

  NS_LOG_DEBUG ("Command SerializedSize: " << unsigned(command->GetSerializedSize ()));
  m_fOptsLen += command->GetSerializedSize ();
}

void
LoraFrameHeader::AddLinkCheckAns (uint8_t margin, uint8_t gwCnt)
{
  NS_LOG_FUNCTION (this << unsigned(margin) << unsigned(gwCnt));

  Ptr<LinkCheckAns> command = Create<LinkCheckAns> (margin, gwCnt);
  m_macCommands.push_back (command);

  m_fOptsLen += command->GetSerializedSize ();
}

void
LoraFrameHeader::AddLinkAdrReq (uint8_t dataRate, uint8_t txPower, std::list<int> enabledChannels, int repetitions)
{
  NS_LOG_FUNCTION (this << unsigned (dataRate) << txPower << repetitions);

  uint16_t channelMask = 0;
  for (auto it = enabledChannels.begin (); it != enabledChannels.end (); it++)
    {
      NS_ASSERT ((*it) < 16 && (*it) > -1);

      channelMask |= 0b1 << (*it);
    }

  // TODO Implement chMaskCntl field

  NS_LOG_DEBUG ("Creating LinkAdrReq with: DR = " << unsigned(dataRate) << " and txPower = " << unsigned(txPower));

  Ptr<LinkAdrReq> command = Create<LinkAdrReq> (dataRate, txPower, channelMask, 0, repetitions);
  m_macCommands.push_back (command);

  m_fOptsLen += command->GetSerializedSize ();
}

void
LoraFrameHeader::AddLinkAdrAns (bool powerAck, bool dataRateAck, bool channelMaskAck)
{
  NS_LOG_FUNCTION (this << powerAck << dataRateAck << channelMaskAck);

  Ptr<LinkAdrAns> command = Create<LinkAdrAns> (powerAck, dataRateAck, channelMaskAck);
  m_macCommands.push_back (command);

  m_fOptsLen += command->GetSerializedSize ();
}

void
LoraFrameHeader::AddDutyCycleReq (uint8_t dutyCycle)
{
  NS_LOG_FUNCTION (this << unsigned (dutyCycle));

  Ptr<DutyCycleReq> command = Create<DutyCycleReq> (dutyCycle);

  m_macCommands.push_back (command);

  m_fOptsLen += command->GetSerializedSize ();
}

void
LoraFrameHeader::AddDutyCycleAns (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<DutyCycleAns> command = Create<DutyCycleAns> ();

  m_macCommands.push_back (command);

  m_fOptsLen += command->GetSerializedSize ();
}

void
LoraFrameHeader::AddRxParamSetupReq (uint8_t rx1DrOffset, uint8_t rx2DataRate, double frequency)
{
  NS_LOG_FUNCTION (this << unsigned (rx1DrOffset) << unsigned (rx2DataRate) <<
                   frequency);

  // Evaluate whether to eliminate this assert in case new offsets can be defined.
  NS_ASSERT (0 <= rx1DrOffset && rx1DrOffset <= 5);

  Ptr<RxParamSetupReq> command = Create<RxParamSetupReq> (rx1DrOffset,
                                                          rx2DataRate,
                                                          frequency);

  m_macCommands.push_back (command);

  m_fOptsLen += command->GetSerializedSize ();
}

void
LoraFrameHeader::AddRxParamSetupAns (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<RxParamSetupAns> command = Create<RxParamSetupAns> ();

  m_macCommands.push_back (command);

  m_fOptsLen += command->GetSerializedSize ();
}

void
LoraFrameHeader::AddDevStatusReq (void)
{
  NS_LOG_FUNCTION (this);

  Ptr<DevStatusReq> command = Create<DevStatusReq> ();

  m_macCommands.push_back (command);

  m_fOptsLen += command->GetSerializedSize ();
}

void
LoraFrameHeader::AddNewChannelReq (uint8_t chIndex, double frequency,
                                   uint8_t minDataRate, uint8_t maxDataRate)
{
  NS_LOG_FUNCTION (this);

  Ptr<NewChannelReq> command = Create<NewChannelReq> (chIndex, frequency,
                                                      minDataRate, maxDataRate);

  m_macCommands.push_back (command);

  m_fOptsLen += command->GetSerializedSize ();
}

std::list<Ptr<MacCommand> >
LoraFrameHeader::GetCommands (void)
{
  NS_LOG_FUNCTION_NOARGS ();

  return m_macCommands;
}

void
LoraFrameHeader::AddCommand (Ptr<MacCommand> macCommand)
{
  NS_LOG_FUNCTION (this << macCommand);

  m_macCommands.push_back (macCommand);
  m_fOptsLen += macCommand->GetSerializedSize ();
}

}
}
