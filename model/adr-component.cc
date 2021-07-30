/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 University of Padova
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
 * Author: Matteo Perin <matteo.perin.2@studenti.unipd.it
 */

#include "ns3/adr-component.h"

namespace ns3 {
namespace lorawan {

////////////////////////////////////////
// LinkAdrRequest commands management //
////////////////////////////////////////

NS_LOG_COMPONENT_DEFINE ("AdrComponent");

NS_OBJECT_ENSURE_REGISTERED (AdrComponent);

TypeId AdrComponent::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::AdrComponent")
    .SetGroupName ("lorawan")
    .AddConstructor<AdrComponent> ()
    .SetParent<NetworkControllerComponent> ()
    .AddAttribute ("MultipleGwCombiningMethod",
                   "Whether to average the received power of gateways or to use the maximum",
                   EnumValue (AdrComponent::AVERAGE),
                   MakeEnumAccessor (&AdrComponent::tpAveraging),
                   MakeEnumChecker (AdrComponent::AVERAGE,
                                    "avg",
                                    AdrComponent::MAXIMUM,
                                    "max",
                                    AdrComponent::MINIMUM,
                                    "min"))
    .AddAttribute ("MultiplePacketsCombiningMethod",
                   "Whether to average SNRs from multiple packets or to use the maximum",
                   EnumValue (AdrComponent::AVERAGE),
                   MakeEnumAccessor (&AdrComponent::historyAveraging),
                   MakeEnumChecker (AdrComponent::AVERAGE,
                                    "avg",
                                    AdrComponent::MAXIMUM,
                                    "max",
                                    AdrComponent::MINIMUM,
                                    "min"))
    .AddAttribute ("HistoryRange",
                   "Number of packets to use for averaging",
                   IntegerValue (4),
                   MakeIntegerAccessor (&AdrComponent::historyRange),
                   MakeIntegerChecker<int> (0, 100))
    .AddAttribute ("ChangeTransmissionPower",
                   "Whether to toggle the transmission power or not",
                   BooleanValue (true),
                   MakeBooleanAccessor (&AdrComponent::m_toggleTxPower),
                   MakeBooleanChecker ())
  ;
  return tid;
}

AdrComponent::AdrComponent ()
{
}

AdrComponent::~AdrComponent ()
{
}

void AdrComponent::OnReceivedPacket (Ptr<const Packet> packet,
                                     Ptr<EndDeviceStatus> status,
                                     Ptr<NetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (this->GetTypeId () << packet << networkStatus);

  // We will only act just before reply, when all Gateways will have received
  // the packet, since we need their respective received power.
}

void
AdrComponent::BeforeSendingReply (Ptr<EndDeviceStatus> status,
                                  Ptr<NetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (this << status << networkStatus);

  Ptr<Packet> myPacket = status->GetLastPacketReceivedFromDevice ()->Copy ();
  LorawanMacHeader mHdr;
  LoraFrameHeader fHdr;
  fHdr.SetAsUplink ();
  myPacket->RemoveHeader (mHdr);
  myPacket->RemoveHeader (fHdr);

  //Execute the ADR algotithm only if the request bit is set
  if (fHdr.GetAdr ())
    {
      if (int(status->GetReceivedPacketList ().size ()) < historyRange)
        {
          NS_LOG_ERROR ("Not enough packets received by this device (" << status->GetReceivedPacketList ().size () << ") for the algorithm to work (need " << historyRange << ")");
        }
      else
        {
          NS_LOG_DEBUG ("New ADR request");

          //Get the SF used by the device
          uint8_t spreadingFactor = status->GetFirstReceiveWindowSpreadingFactor ();

          //Get the device transmission power (dBm)
          uint8_t transmissionPower = status->GetMac ()->GetTransmissionPower ();

          //New parameters for the end-device
          uint8_t newDataRate;
          uint8_t newTxPower;

          //ADR Algorithm
          AdrImplementation (&newDataRate,
                             &newTxPower,
                             status);

          // Change the power back to the default if we don't want to change it
          if (!m_toggleTxPower)
            {
              newTxPower = transmissionPower;
            }

          if (newDataRate != SfToDr (spreadingFactor) || newTxPower != transmissionPower)
            {
              //Create a list with mandatory channel indexes
              int channels[] = {0, 1, 2};
              std::list<int> enabledChannels (channels,
                                              channels + sizeof(channels) /
                                              sizeof(int));

              //Repetitions Setting
              const int rep = 1;

              NS_LOG_DEBUG ("Sending LinkAdrReq with DR = " << (unsigned)newDataRate << " and TP = " << (unsigned)newTxPower << " dBm");

              status->m_reply.frameHeader.AddLinkAdrReq (newDataRate,
                                                         GetTxPowerIndex (newTxPower),
                                                         enabledChannels,
                                                         rep);
              status->m_reply.frameHeader.SetAsDownlink ();
              status->m_reply.macHeader.SetMType (LorawanMacHeader::UNCONFIRMED_DATA_DOWN);

              status->m_reply.needsReply = true;
            }
          else
            {
              NS_LOG_DEBUG ("Skipped request");
            }
        }
    }
  else
    {
      // Do nothing
    }
}

void AdrComponent::OnFailedReply (Ptr<EndDeviceStatus> status,
                                  Ptr<NetworkStatus> networkStatus)
{
  NS_LOG_FUNCTION (this->GetTypeId () << networkStatus);
}

void AdrComponent::AdrImplementation (uint8_t *newDataRate,
                                      uint8_t *newTxPower,
                                      Ptr<EndDeviceStatus> status)
{
  //Compute the maximum or median SNR, based on the boolean value historyAveraging
  double m_SNR = 0;
  switch (historyAveraging)
    {
    case AdrComponent::AVERAGE:
      m_SNR = GetAverageSNR (status->GetReceivedPacketList (),
                             historyRange);
      break;
    case AdrComponent::MAXIMUM:
      m_SNR = GetMaxSNR (status->GetReceivedPacketList (),
                         historyRange);
      break;
    case AdrComponent::MINIMUM:
      m_SNR = GetMinSNR (status->GetReceivedPacketList (),
                         historyRange);
    }

  NS_LOG_DEBUG ("m_SNR = " << m_SNR);

  //Get the SF used by the device
  uint8_t spreadingFactor = status->GetFirstReceiveWindowSpreadingFactor ();

  NS_LOG_DEBUG ("SF = " << (unsigned)spreadingFactor);

  //Get the device data rate and use it to get the SNR demodulation treshold
  double req_SNR = treshold[SfToDr (spreadingFactor)];

  NS_LOG_DEBUG ("Required SNR = " << req_SNR);

  //Get the device transmission power (dBm)
  double transmissionPower = status->GetMac ()->GetTransmissionPower ();

  NS_LOG_DEBUG ("Transmission Power = " << transmissionPower);

  //Compute the SNR margin taking into consideration the SNR of
  //previously received packets
  double margin_SNR = m_SNR - req_SNR;

  NS_LOG_DEBUG ("Margin = " << margin_SNR);

  //Number of steps to decrement the SF (thereby increasing the Data Rate)
  //and the TP.
  int steps = std::floor (margin_SNR / 3);

  NS_LOG_DEBUG ("steps = " << steps);

  //If the number of steps is positive (margin_SNR is positive, so its
  //decimal value is high) increment the data rate, if there are some
  //leftover steps after reaching the maximum possible data rate
  //(corresponding to the minimum SF) decrement the transmission power as
  //well for the number of steps left.
  //If, on the other hand, the number of steps is negative (margin_SNR is
  //negative, so its decimal value is low) increase the transmission power
  //(note that the SF is not incremented as this particular algorithm
  //expects the node itself to raise its SF whenever necessary).
  while (steps > 0 && spreadingFactor > min_spreadingFactor)
    {
      spreadingFactor--;
      steps--;
      NS_LOG_DEBUG ("Decreased SF by 1");
    }
  while (steps > 0 && transmissionPower > min_transmissionPower)
    {
      transmissionPower -= 2;
      steps--;
      NS_LOG_DEBUG ("Decreased Ptx by 2");
    }
  while (steps < 0 && transmissionPower < max_transmissionPower)
    {
      transmissionPower += 2;
      steps++;
      NS_LOG_DEBUG ("Increased Ptx by 2");
    }

  *newDataRate = SfToDr (spreadingFactor);
  *newTxPower = transmissionPower;
}

uint8_t AdrComponent::SfToDr (uint8_t sf)
{
  switch (sf)
    {
    case 12:
      return 0;
      break;
    case 11:
      return 1;
      break;
    case 10:
      return 2;
      break;
    case 9:
      return 3;
      break;
    case 8:
      return 4;
      break;
    default:
      return 5;
      break;
    }
}

double AdrComponent::RxPowerToSNR (double transmissionPower)
{
  //The following conversion ignores interfering packets
  return transmissionPower + 174 - 10 * log10 (B) - NF;
}

//Get the maximum received power (it considers the values in dB!)
double AdrComponent::GetMinTxFromGateways (EndDeviceStatus::GatewayList gwList)
{
  EndDeviceStatus::GatewayList::iterator it = gwList.begin ();
  double min = it->second.rxPower;

  for (; it != gwList.end (); it++)
    {
      if (it->second.rxPower < min)
        {
          min = it->second.rxPower;
        }
    }

  return min;
}

//Get the maximum received power (it considers the values in dB!)
double AdrComponent::GetMaxTxFromGateways (EndDeviceStatus::GatewayList gwList)
{
  EndDeviceStatus::GatewayList::iterator it = gwList.begin ();
  double max = it->second.rxPower;

  for (; it != gwList.end (); it++)
    {
      if (it->second.rxPower > max)
        {
          max = it->second.rxPower;
        }
    }

  return max;
}

//Get the maximum received power
double AdrComponent::GetAverageTxFromGateways (EndDeviceStatus::GatewayList gwList)
{
  double sum = 0;

  for (EndDeviceStatus::GatewayList::iterator it = gwList.begin (); it != gwList.end (); it++)
    {
      NS_LOG_DEBUG ("Gateway at " << it->first << " has TP " << it->second.rxPower);
      sum += it->second.rxPower;
    }

  double average = sum / gwList.size ();

  NS_LOG_DEBUG ("TP (average) = " << average);

  return average;
}

double
AdrComponent::GetReceivedPower (EndDeviceStatus::GatewayList gwList)
{
  switch (tpAveraging)
    {
    case AdrComponent::AVERAGE:
      return GetAverageTxFromGateways (gwList);
    case AdrComponent::MAXIMUM:
      return GetMaxTxFromGateways (gwList);
    case AdrComponent::MINIMUM:
      return GetMinTxFromGateways (gwList);
    default:
      return -1;
    }
}

// TODO Make this more elegant
double AdrComponent::GetMinSNR (EndDeviceStatus::ReceivedPacketList packetList,
                                int historyRange)
{
  double m_SNR;

  //Take elements from the list starting at the end
  auto it = packetList.rbegin ();
  double min = RxPowerToSNR (GetReceivedPower (it->second.gwList));

  for (int i = 0; i < historyRange; i++, it++)
    {
      m_SNR = RxPowerToSNR (GetReceivedPower (it->second.gwList));

      NS_LOG_DEBUG ("Received power: " << GetReceivedPower (it->second.gwList));
      NS_LOG_DEBUG ("m_SNR = " << m_SNR);

      if (m_SNR < min)
        {
          min = m_SNR;
        }
    }

  NS_LOG_DEBUG ("SNR (min) = " << min);

  return min;
}

double AdrComponent::GetMaxSNR (EndDeviceStatus::ReceivedPacketList packetList,
                                int historyRange)
{
  double m_SNR;

  //Take elements from the list starting at the end
  auto it = packetList.rbegin ();
  double max = RxPowerToSNR (GetReceivedPower (it->second.gwList));

  for (int i = 0; i < historyRange; i++, it++)
    {
      m_SNR = RxPowerToSNR (GetReceivedPower (it->second.gwList));

      NS_LOG_DEBUG ("Received power: " << GetReceivedPower (it->second.gwList));
      NS_LOG_DEBUG ("m_SNR = " << m_SNR);

      if (m_SNR > max)
        {
          max = m_SNR;
        }
    }

  NS_LOG_DEBUG ("SNR (max) = " << max);

  return max;
}

double AdrComponent::GetAverageSNR (EndDeviceStatus::ReceivedPacketList packetList,
                                    int historyRange)
{
  double sum = 0;
  double m_SNR;

  //Take elements from the list starting at the end
  auto it = packetList.rbegin ();
  for (int i = 0; i < historyRange; i++, it++)
    {
      m_SNR = RxPowerToSNR (GetReceivedPower (it->second.gwList));

      NS_LOG_DEBUG ("Received power: " << GetReceivedPower (it->second.gwList));
      NS_LOG_DEBUG ("m_SNR = " << m_SNR);

      sum += m_SNR;
    }

  double average = sum / historyRange;

  NS_LOG_DEBUG ("SNR (average) = " << average);

  return average;
}

int AdrComponent::GetTxPowerIndex (int txPower)
{
  if (txPower >= 16)
    {
      return 0;
    }
  else if (txPower >= 14)
    {
      return 1;
    }
  else if (txPower >= 12)
    {
      return 2;
    }
  else if (txPower >= 10)
    {
      return 3;
    }
  else if (txPower >= 8)
    {
      return 4;
    }
  else if (txPower >= 6)
    {
      return 5;
    }
  else if (txPower >= 4)
    {
      return 6;
    }
  else
    {
      return 7;
    }
}
}
}
