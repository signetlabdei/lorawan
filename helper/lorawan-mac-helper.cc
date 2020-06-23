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

#include "ns3/lorawan-mac-helper.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/lora-net-device.h"
#include "ns3/log.h"
#include "ns3/random-variable-stream.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("LorawanMacHelper");

LorawanMacHelper::LorawanMacHelper () : m_region (LorawanMacHelper::EU)
{
}

void
LorawanMacHelper::Set (std::string name, const AttributeValue &v)
{
  m_mac.Set (name, v);
}

void
LorawanMacHelper::SetDeviceType (enum DeviceType dt)
{
  NS_LOG_FUNCTION (this << dt);
  switch (dt)
    {
    case GW:
      m_mac.SetTypeId ("ns3::GatewayLorawanMac");
      break;
    case ED_A:
      m_mac.SetTypeId ("ns3::ClassAEndDeviceLorawanMac");
      break;
    }
  m_deviceType = dt;
}

void
LorawanMacHelper::SetAddressGenerator (Ptr<LoraDeviceAddressGenerator> addrGen)
{
  NS_LOG_FUNCTION (this);

  m_addrGen = addrGen;
}

void
LorawanMacHelper::SetRegion (enum LorawanMacHelper::Regions region)
{
  m_region = region;
}

Ptr<LorawanMac>
LorawanMacHelper::Create (Ptr<Node> node, Ptr<NetDevice> device) const
{
  Ptr<LorawanMac> mac = m_mac.Create<LorawanMac> ();
  mac->SetDevice (device);

  // If we are operating on an end device, add an address to it
  if (m_deviceType == ED_A && m_addrGen != 0)
    {
      mac->GetObject<ClassAEndDeviceLorawanMac> ()->SetDeviceAddress (m_addrGen->NextAddress ());
    }

  // Add a basic list of channels based on the region where the device is
  // operating
  if (m_deviceType == ED_A)
    {
      Ptr<ClassAEndDeviceLorawanMac> edMac = mac->GetObject<ClassAEndDeviceLorawanMac> ();
      switch (m_region)
        {
          case LorawanMacHelper::EU: {
            ConfigureForEuRegion (edMac);
            break;
          }
          case LorawanMacHelper::SingleChannel: {
            ConfigureForSingleChannelRegion (edMac);
            break;
          }
          case LorawanMacHelper::ALOHA: {
            ConfigureForAlohaRegion (edMac);
            break;
          }
          default: {
            NS_LOG_ERROR ("This region isn't supported yet!");
            break;
          }
        }
    }
  else
    {
      Ptr<GatewayLorawanMac> gwMac = mac->GetObject<GatewayLorawanMac> ();
      switch (m_region)
        {
          case LorawanMacHelper::EU: {
            ConfigureForEuRegion (gwMac);
            break;
          }
          case LorawanMacHelper::SingleChannel: {
            ConfigureForSingleChannelRegion (gwMac);
            break;
          }
          case LorawanMacHelper::ALOHA: {
            ConfigureForAlohaRegion (gwMac);
            break;
          }
          default: {
            NS_LOG_ERROR ("This region isn't supported yet!");
            break;
          }
        }
    }
  return mac;
}

void
LorawanMacHelper::ConfigureForAlohaRegion (Ptr<ClassAEndDeviceLorawanMac> edMac) const
{
  NS_LOG_FUNCTION_NOARGS ();

  ApplyCommonAlohaConfigurations (edMac);

  /////////////////////////////////////////////////////
  // TxPower -> Transmission power in dBm conversion //
  /////////////////////////////////////////////////////
  edMac->SetTxDbmForTxPower (std::vector<double>{16, 14, 12, 10, 8, 6, 4, 2});

  ////////////////////////////////////////////////////////////
  // Matrix to know which DataRate the GW will respond with //
  ////////////////////////////////////////////////////////////
  LorawanMac::ReplyDataRateMatrix matrix = {{{{0, 0, 0, 0, 0, 0}},
                                             {{1, 0, 0, 0, 0, 0}},
                                             {{2, 1, 0, 0, 0, 0}},
                                             {{3, 2, 1, 0, 0, 0}},
                                             {{4, 3, 2, 1, 0, 0}},
                                             {{5, 4, 3, 2, 1, 0}},
                                             {{6, 5, 4, 3, 2, 1}},
                                             {{7, 6, 5, 4, 3, 2}}}};
  edMac->SetReplyDataRateMatrix (matrix);

  /////////////////////
  // Preamble length //
  /////////////////////
  edMac->SetNPreambleSymbols (8);

  //////////////////////////////////////
  // Second receive window parameters //
  //////////////////////////////////////
  edMac->SetSecondReceiveWindowDataRate (0);
  edMac->SetSecondReceiveWindowFrequency (869.525);
}

void
LorawanMacHelper::ConfigureForAlohaRegion (Ptr<GatewayLorawanMac> gwMac) const
{
  NS_LOG_FUNCTION_NOARGS ();

  ///////////////////////////////
  // ReceivePath configuration //
  ///////////////////////////////
  Ptr<GatewayLoraPhy> gwPhy =
      gwMac->GetDevice ()->GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<GatewayLoraPhy> ();

  ApplyCommonAlohaConfigurations (gwMac);

  if (gwPhy) // If cast is successful, there's a GatewayLoraPhy
    {
      NS_LOG_DEBUG ("Resetting reception paths");
      gwPhy->ResetReceptionPaths ();

      int receptionPaths = 0;
      int maxReceptionPaths = 1;
      while (receptionPaths < maxReceptionPaths)
        {
          gwPhy->GetObject<GatewayLoraPhy> ()->AddReceptionPath ();
          receptionPaths++;
        }
      gwPhy->AddFrequency (868.1);
    }
}

void
LorawanMacHelper::ApplyCommonAlohaConfigurations (Ptr<LorawanMac> lorawanMac) const
{
  NS_LOG_FUNCTION_NOARGS ();

  //////////////
  // SubBands //
  //////////////

  LogicalLoraChannelHelper channelHelper;
  channelHelper.AddSubBand (868, 868.6, 1, 14);

  //////////////////////
  // Default channels //
  //////////////////////
  Ptr<LogicalLoraChannel> lc1 = CreateObject<LogicalLoraChannel> (868.1, 0, 5);
  channelHelper.AddChannel (lc1);

  lorawanMac->SetLogicalLoraChannelHelper (channelHelper);

  ///////////////////////////////////////////////
  // DataRate -> SF, DataRate -> Bandwidth     //
  // and DataRate -> MaxAppPayload conversions //
  ///////////////////////////////////////////////
  lorawanMac->SetSfForDataRate (std::vector<uint8_t>{12, 11, 10, 9, 8, 7, 7});
  lorawanMac->SetBandwidthForDataRate (
      std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000, 250000});
  lorawanMac->SetMaxAppPayloadForDataRate (
      std::vector<uint32_t>{59, 59, 59, 123, 230, 230, 230, 230});
}

void
LorawanMacHelper::ConfigureForEuRegion (Ptr<ClassAEndDeviceLorawanMac> edMac) const
{
  NS_LOG_FUNCTION_NOARGS ();

  ApplyCommonEuConfigurations (edMac);

  /////////////////////////////////////////////////////
  // TxPower -> Transmission power in dBm conversion //
  /////////////////////////////////////////////////////
  edMac->SetTxDbmForTxPower (std::vector<double>{16, 14, 12, 10, 8, 6, 4, 2});

  ////////////////////////////////////////////////////////////
  // Matrix to know which DataRate the GW will respond with //
  ////////////////////////////////////////////////////////////
  LorawanMac::ReplyDataRateMatrix matrix = {{{{0, 0, 0, 0, 0, 0}},
                                             {{1, 0, 0, 0, 0, 0}},
                                             {{2, 1, 0, 0, 0, 0}},
                                             {{3, 2, 1, 0, 0, 0}},
                                             {{4, 3, 2, 1, 0, 0}},
                                             {{5, 4, 3, 2, 1, 0}},
                                             {{6, 5, 4, 3, 2, 1}},
                                             {{7, 6, 5, 4, 3, 2}}}};
  edMac->SetReplyDataRateMatrix (matrix);

  /////////////////////
  // Preamble length //
  /////////////////////
  edMac->SetNPreambleSymbols (8);

  //////////////////////////////////////
  // Second receive window parameters //
  //////////////////////////////////////
  edMac->SetSecondReceiveWindowDataRate (0);
  edMac->SetSecondReceiveWindowFrequency (869.525);
}

void
LorawanMacHelper::ConfigureForEuRegion (Ptr<GatewayLorawanMac> gwMac) const
{
  NS_LOG_FUNCTION_NOARGS ();

  ///////////////////////////////
  // ReceivePath configuration //
  ///////////////////////////////
  Ptr<GatewayLoraPhy> gwPhy =
      gwMac->GetDevice ()->GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<GatewayLoraPhy> ();

  ApplyCommonEuConfigurations (gwMac);

  if (gwPhy) // If cast is successful, there's a GatewayLoraPhy
    {
      NS_LOG_DEBUG ("Resetting reception paths");
      gwPhy->ResetReceptionPaths ();

      std::vector<double> frequencies;
      frequencies.push_back (868.1);
      frequencies.push_back (868.3);
      frequencies.push_back (868.5);

      for (auto &f : frequencies)
        {
          gwPhy->AddFrequency (f);
        }

      int receptionPaths = 0;
      int maxReceptionPaths = 8;
      while (receptionPaths < maxReceptionPaths)
        {
          gwPhy->GetObject<GatewayLoraPhy> ()->AddReceptionPath ();
          receptionPaths++;
        }
    }
}

void
LorawanMacHelper::ApplyCommonEuConfigurations (Ptr<LorawanMac> lorawanMac) const
{
  NS_LOG_FUNCTION_NOARGS ();

  //////////////
  // SubBands //
  //////////////

  LogicalLoraChannelHelper channelHelper;
  channelHelper.AddSubBand (868, 868.6, 0.01, 14);
  channelHelper.AddSubBand (868.7, 869.2, 0.001, 14);
  channelHelper.AddSubBand (869.4, 869.65, 0.1, 27);

  //////////////////////
  // Default channels //
  //////////////////////
  Ptr<LogicalLoraChannel> lc1 = CreateObject<LogicalLoraChannel> (868.1, 0, 5);
  Ptr<LogicalLoraChannel> lc2 = CreateObject<LogicalLoraChannel> (868.3, 0, 5);
  Ptr<LogicalLoraChannel> lc3 = CreateObject<LogicalLoraChannel> (868.5, 0, 5);
  channelHelper.AddChannel (lc1);
  channelHelper.AddChannel (lc2);
  channelHelper.AddChannel (lc3);

  lorawanMac->SetLogicalLoraChannelHelper (channelHelper);

  ///////////////////////////////////////////////
  // DataRate -> SF, DataRate -> Bandwidth     //
  // and DataRate -> MaxAppPayload conversions //
  ///////////////////////////////////////////////
  lorawanMac->SetSfForDataRate (std::vector<uint8_t>{12, 11, 10, 9, 8, 7, 7});
  lorawanMac->SetBandwidthForDataRate (
      std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000, 250000});
  lorawanMac->SetMaxAppPayloadForDataRate (
      std::vector<uint32_t>{59, 59, 59, 123, 230, 230, 230, 230});
}

///////////////////////////////

void
LorawanMacHelper::ConfigureForSingleChannelRegion (Ptr<ClassAEndDeviceLorawanMac> edMac) const
{
  NS_LOG_FUNCTION_NOARGS ();

  ApplyCommonSingleChannelConfigurations (edMac);

  /////////////////////////////////////////////////////
  // TxPower -> Transmission power in dBm conversion //
  /////////////////////////////////////////////////////
  edMac->SetTxDbmForTxPower (std::vector<double>{16, 14, 12, 10, 8, 6, 4, 2});

  ////////////////////////////////////////////////////////////
  // Matrix to know which DataRate the GW will respond with //
  ////////////////////////////////////////////////////////////
  LorawanMac::ReplyDataRateMatrix matrix = {{{{0, 0, 0, 0, 0, 0}},
                                             {{1, 0, 0, 0, 0, 0}},
                                             {{2, 1, 0, 0, 0, 0}},
                                             {{3, 2, 1, 0, 0, 0}},
                                             {{4, 3, 2, 1, 0, 0}},
                                             {{5, 4, 3, 2, 1, 0}},
                                             {{6, 5, 4, 3, 2, 1}},
                                             {{7, 6, 5, 4, 3, 2}}}};
  edMac->SetReplyDataRateMatrix (matrix);

  /////////////////////
  // Preamble length //
  /////////////////////
  edMac->SetNPreambleSymbols (8);

  //////////////////////////////////////
  // Second receive window parameters //
  //////////////////////////////////////
  edMac->SetSecondReceiveWindowDataRate (0);
  edMac->SetSecondReceiveWindowFrequency (869.525);
}

void
LorawanMacHelper::ConfigureForSingleChannelRegion (Ptr<GatewayLorawanMac> gwMac) const
{
  NS_LOG_FUNCTION_NOARGS ();

  ///////////////////////////////
  // ReceivePath configuration //
  ///////////////////////////////
  Ptr<GatewayLoraPhy> gwPhy =
      gwMac->GetDevice ()->GetObject<LoraNetDevice> ()->GetPhy ()->GetObject<GatewayLoraPhy> ();

  ApplyCommonEuConfigurations (gwMac);

  if (gwPhy) // If cast is successful, there's a GatewayLoraPhy
    {
      NS_LOG_DEBUG ("Resetting reception paths");
      gwPhy->ResetReceptionPaths ();

      std::vector<double> frequencies;
      frequencies.push_back (868.1);

      for (auto &f : frequencies)
        {
          gwPhy->AddFrequency (f);
        }

      int receptionPaths = 0;
      int maxReceptionPaths = 8;
      while (receptionPaths < maxReceptionPaths)
        {
          gwPhy->GetObject<GatewayLoraPhy> ()->AddReceptionPath ();
          receptionPaths++;
        }
    }
}

void
LorawanMacHelper::ApplyCommonSingleChannelConfigurations (Ptr<LorawanMac> lorawanMac) const
{
  NS_LOG_FUNCTION_NOARGS ();

  //////////////
  // SubBands //
  //////////////

  LogicalLoraChannelHelper channelHelper;
  channelHelper.AddSubBand (868, 868.6, 0.01, 14);
  channelHelper.AddSubBand (868.7, 869.2, 0.001, 14);
  channelHelper.AddSubBand (869.4, 869.65, 0.1, 27);

  //////////////////////
  // Default channels //
  //////////////////////
  Ptr<LogicalLoraChannel> lc1 = CreateObject<LogicalLoraChannel> (868.1, 0, 5);
  channelHelper.AddChannel (lc1);

  lorawanMac->SetLogicalLoraChannelHelper (channelHelper);

  ///////////////////////////////////////////////
  // DataRate -> SF, DataRate -> Bandwidth     //
  // and DataRate -> MaxAppPayload conversions //
  ///////////////////////////////////////////////
  lorawanMac->SetSfForDataRate (std::vector<uint8_t>{12, 11, 10, 9, 8, 7, 7});
  lorawanMac->SetBandwidthForDataRate (
      std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000, 250000});
  lorawanMac->SetMaxAppPayloadForDataRate (
      std::vector<uint32_t>{59, 59, 59, 123, 230, 230, 230, 230});
}

std::vector<int>
LorawanMacHelper::SetSpreadingFactorsUp (NodeContainer endDevices, NodeContainer gateways,
                                         Ptr<LoraChannel> channel)
{
  NS_LOG_FUNCTION_NOARGS ();

  std::vector<int> sfQuantity (7, 0);
  for (NodeContainer::Iterator j = endDevices.Begin (); j != endDevices.End (); ++j)
    {
      Ptr<Node> object = *j;
      Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
      NS_ASSERT (position != 0);
      Ptr<NetDevice> netDevice = object->GetDevice (0);
      Ptr<LoraNetDevice> loraNetDevice = netDevice->GetObject<LoraNetDevice> ();
      NS_ASSERT (loraNetDevice != 0);
      Ptr<ClassAEndDeviceLorawanMac> mac =
          loraNetDevice->GetMac ()->GetObject<ClassAEndDeviceLorawanMac> ();
      NS_ASSERT (mac != 0);

      // Try computing the distance from each gateway and find the best one
      Ptr<Node> bestGateway = gateways.Get (0);
      Ptr<MobilityModel> bestGatewayPosition = bestGateway->GetObject<MobilityModel> ();

      // Assume devices transmit at 14 dBm
      double highestRxPower = channel->GetRxPower (14, position, bestGatewayPosition);

      for (NodeContainer::Iterator currentGw = gateways.Begin () + 1; currentGw != gateways.End ();
           ++currentGw)
        {
          // Compute the power received from the current gateway
          Ptr<Node> curr = *currentGw;
          Ptr<MobilityModel> currPosition = curr->GetObject<MobilityModel> ();
          double currentRxPower = channel->GetRxPower (14, position, currPosition); // dBm

          if (currentRxPower > highestRxPower)
            {
              bestGateway = curr;
              bestGatewayPosition = curr->GetObject<MobilityModel> ();
              highestRxPower = currentRxPower;
            }
        }

      // NS_LOG_DEBUG ("Rx Power: " << highestRxPower);
      double rxPower = highestRxPower;

      // Get the ED sensitivity
      Ptr<EndDeviceLoraPhy> edPhy = loraNetDevice->GetPhy ()->GetObject<EndDeviceLoraPhy> ();
      const double *edSensitivity = edPhy->sensitivity;

      if (rxPower > *edSensitivity)
        {
          mac->SetDataRate (5);
          sfQuantity[0] = sfQuantity[0] + 1;
        }
      else if (rxPower > *(edSensitivity + 1))
        {
          mac->SetDataRate (4);
          sfQuantity[1] = sfQuantity[1] + 1;
        }
      else if (rxPower > *(edSensitivity + 2))
        {
          mac->SetDataRate (3);
          sfQuantity[2] = sfQuantity[2] + 1;
        }
      else if (rxPower > *(edSensitivity + 3))
        {
          mac->SetDataRate (2);
          sfQuantity[3] = sfQuantity[3] + 1;
        }
      else if (rxPower > *(edSensitivity + 4))
        {
          mac->SetDataRate (1);
          sfQuantity[4] = sfQuantity[4] + 1;
        }
      else if (rxPower > *(edSensitivity + 5))
        {
          mac->SetDataRate (0);
          sfQuantity[5] = sfQuantity[5] + 1;
        }
      else // Device is out of range. Assign SF12.
        {
          // NS_LOG_DEBUG ("Device out of range");
          mac->SetDataRate (0);
          sfQuantity[6] = sfQuantity[6] + 1;
          // NS_LOG_DEBUG ("sfQuantity[6] = " << sfQuantity[6]);
        }

      /*

      // Get the Gw sensitivity
      Ptr<NetDevice> gatewayNetDevice = bestGateway->GetDevice (0);
      Ptr<LoraNetDevice> gatewayLoraNetDevice = gatewayNetDevice->GetObject<LoraNetDevice> ();
      Ptr<GatewayLoraPhy> gatewayPhy = gatewayLoraNetDevice->GetPhy ()->GetObject<GatewayLoraPhy> ();
      const double *gwSensitivity = gatewayPhy->sensitivity;

      if(rxPower > *gwSensitivity)
        {
          mac->SetDataRate (5);
          sfQuantity[0] = sfQuantity[0] + 1;

        }
      else if (rxPower > *(gwSensitivity+1))
        {
          mac->SetDataRate (4);
          sfQuantity[1] = sfQuantity[1] + 1;

        }
      else if (rxPower > *(gwSensitivity+2))
        {
          mac->SetDataRate (3);
          sfQuantity[2] = sfQuantity[2] + 1;

        }
      else if (rxPower > *(gwSensitivity+3))
        {
          mac->SetDataRate (2);
          sfQuantity[3] = sfQuantity[3] + 1;
        }
      else if (rxPower > *(gwSensitivity+4))
        {
          mac->SetDataRate (1);
          sfQuantity[4] = sfQuantity[4] + 1;
        }
      else if (rxPower > *(gwSensitivity+5))
        {
          mac->SetDataRate (0);
          sfQuantity[5] = sfQuantity[5] + 1;

        }
      else // Device is out of range. Assign SF12.
        {
          mac->SetDataRate (0);
          sfQuantity[6] = sfQuantity[6] + 1;

        }
        */

    } // end loop on nodes

  return sfQuantity;

} //  end function

std::vector<int>
LorawanMacHelper::SetSpreadingFactorsGivenDistribution (NodeContainer endDevices,
                                                        NodeContainer gateways,
                                                        std::vector<double> distribution)
{
  NS_LOG_FUNCTION_NOARGS ();

  std::vector<int> sfQuantity (7, 0);
  Ptr<UniformRandomVariable> uniformRV = CreateObject<UniformRandomVariable> ();
  std::vector<double> cumdistr (6);
  cumdistr[0] = distribution[0];
  for (int i = 1; i < 7; ++i)
    {
      cumdistr[i] = distribution[i] + cumdistr[i - 1];
    }

  NS_LOG_DEBUG ("Distribution: " << distribution[0] << " " << distribution[1] << " "
                                 << distribution[2] << " " << distribution[3] << " "
                                 << distribution[4] << " " << distribution[5]);
  NS_LOG_DEBUG ("Cumulative distribution: " << cumdistr[0] << " " << cumdistr[1] << " "
                                            << cumdistr[2] << " " << cumdistr[3] << " "
                                            << cumdistr[4] << " " << cumdistr[5]);

  for (NodeContainer::Iterator j = endDevices.Begin (); j != endDevices.End (); ++j)
    {
      Ptr<Node> object = *j;
      Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
      NS_ASSERT (position != 0);
      Ptr<NetDevice> netDevice = object->GetDevice (0);
      Ptr<LoraNetDevice> loraNetDevice = netDevice->GetObject<LoraNetDevice> ();
      NS_ASSERT (loraNetDevice != 0);
      Ptr<ClassAEndDeviceLorawanMac> mac =
          loraNetDevice->GetMac ()->GetObject<ClassAEndDeviceLorawanMac> ();
      NS_ASSERT (mac != 0);

      double prob = uniformRV->GetValue (0, 1);

      // NS_LOG_DEBUG ("Probability: " << prob);
      if (prob < cumdistr[0])
        {
          mac->SetDataRate (5);
          sfQuantity[0] = sfQuantity[0] + 1;
        }
      else if (prob > cumdistr[0] && prob < cumdistr[1])
        {
          mac->SetDataRate (4);
          sfQuantity[1] = sfQuantity[1] + 1;
        }
      else if (prob > cumdistr[1] && prob < cumdistr[2])
        {
          mac->SetDataRate (3);
          sfQuantity[2] = sfQuantity[2] + 1;
        }
      else if (prob > cumdistr[2] && prob < cumdistr[3])
        {
          mac->SetDataRate (2);
          sfQuantity[3] = sfQuantity[3] + 1;
        }
      else if (prob > cumdistr[3] && prob < cumdistr[4])
        {
          mac->SetDataRate (1);
          sfQuantity[4] = sfQuantity[4] + 1;
        }
      else
        {
          mac->SetDataRate (0);
          sfQuantity[5] = sfQuantity[5] + 1;
        }

    } // end loop on nodes

  return sfQuantity;

} //  end function

} // namespace lorawan
} // namespace ns3
