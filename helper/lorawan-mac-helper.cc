/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "lorawan-mac-helper.h"

#include "ns3/end-device-lora-phy.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/log.h"
#include "ns3/lora-net-device.h"
#include "ns3/random-variable-stream.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LorawanMacHelper");

LorawanMacHelper::LorawanMacHelper()
    : m_region(LorawanMacHelper::EU)
{
}

void
LorawanMacHelper::Set(std::string name, const AttributeValue& v)
{
    m_mac.Set(name, v);
}

void
LorawanMacHelper::SetDeviceType(enum DeviceType dt)
{
    NS_LOG_FUNCTION(this << dt);
    switch (dt)
    {
    case GW:
        m_mac.SetTypeId("ns3::GatewayLorawanMac");
        break;
    case ED_A:
        m_mac.SetTypeId("ns3::ClassAEndDeviceLorawanMac");
        break;
    }
    m_deviceType = dt;
}

void
LorawanMacHelper::SetAddressGenerator(Ptr<LoraDeviceAddressGenerator> addrGen)
{
    NS_LOG_FUNCTION(this);

    m_addrGen = addrGen;
}

void
LorawanMacHelper::SetRegion(enum LorawanMacHelper::Regions region)
{
    m_region = region;
}

Ptr<LorawanMac>
LorawanMacHelper::Install(Ptr<Node> node, Ptr<NetDevice> device) const
{
    Ptr<LorawanMac> mac = m_mac.Create<LorawanMac>();
    mac->SetDevice(device);

    // If we are operating on an end device, add an address to it
    if (m_deviceType == ED_A && m_addrGen)
    {
        DynamicCast<ClassAEndDeviceLorawanMac>(mac)->SetDeviceAddress(m_addrGen->NextAddress());
    }

    // Add a basic list of channels based on the region where the device is
    // operating
    if (m_deviceType == ED_A)
    {
        Ptr<ClassAEndDeviceLorawanMac> edMac = DynamicCast<ClassAEndDeviceLorawanMac>(mac);
        switch (m_region)
        {
        case LorawanMacHelper::EU: {
            ConfigureForEuRegion(edMac);
            break;
        }
        case LorawanMacHelper::SingleChannel: {
            ConfigureForSingleChannelRegion(edMac);
            break;
        }
        case LorawanMacHelper::ALOHA: {
            ConfigureForAlohaRegion(edMac);
            break;
        }
        default: {
            NS_LOG_ERROR("This region isn't supported yet!");
            break;
        }
        }
    }
    else
    {
        Ptr<GatewayLorawanMac> gwMac = DynamicCast<GatewayLorawanMac>(mac);
        switch (m_region)
        {
        case LorawanMacHelper::EU: {
            ConfigureForEuRegion(gwMac);
            break;
        }
        case LorawanMacHelper::SingleChannel: {
            ConfigureForSingleChannelRegion(gwMac);
            break;
        }
        case LorawanMacHelper::ALOHA: {
            ConfigureForAlohaRegion(gwMac);
            break;
        }
        default: {
            NS_LOG_ERROR("This region isn't supported yet!");
            break;
        }
        }
    }
    return mac;
}

void
LorawanMacHelper::ConfigureForAlohaRegion(Ptr<ClassAEndDeviceLorawanMac> edMac) const
{
    NS_LOG_FUNCTION_NOARGS();

    ApplyCommonAlohaConfigurations(edMac);

    /////////////////////////////////////////////////////////
    // TxPower -> Transmission power in dBm ERP conversion //
    /////////////////////////////////////////////////////////
    edMac->SetTxDbmForTxPower(std::vector<double>{14, 12, 10, 8, 6, 4, 2, 0});

    ////////////////////////////////////////////////////////////
    // Matrix to know which data rate the gateway will respond with //
    ////////////////////////////////////////////////////////////
    LorawanMac::ReplyDataRateMatrix matrix = {{{{0, 0, 0, 0, 0, 0}},
                                               {{1, 0, 0, 0, 0, 0}},
                                               {{2, 1, 0, 0, 0, 0}},
                                               {{3, 2, 1, 0, 0, 0}},
                                               {{4, 3, 2, 1, 0, 0}},
                                               {{5, 4, 3, 2, 1, 0}},
                                               {{6, 5, 4, 3, 2, 1}},
                                               {{7, 6, 5, 4, 3, 2}}}};
    edMac->SetReplyDataRateMatrix(matrix);

    /////////////////////
    // Preamble length //
    /////////////////////
    edMac->SetNPreambleSymbols(8);

    //////////////////////////////////////
    // Second receive window parameters //
    //////////////////////////////////////
    edMac->SetSecondReceiveWindowDataRate(0);
    edMac->SetSecondReceiveWindowFrequency(869525000);
}

void
LorawanMacHelper::ConfigureForAlohaRegion(Ptr<GatewayLorawanMac> gwMac) const
{
    NS_LOG_FUNCTION_NOARGS();

    ///////////////////////////////
    // ReceivePath configuration //
    ///////////////////////////////
    Ptr<GatewayLoraPhy> gwPhy =
        DynamicCast<GatewayLoraPhy>(DynamicCast<LoraNetDevice>(gwMac->GetDevice())->GetPhy());

    ApplyCommonAlohaConfigurations(gwMac);

    if (gwPhy) // If cast is successful, there's a GatewayLoraPhy
    {
        NS_LOG_DEBUG("Resetting reception paths");
        gwPhy->ResetReceptionPaths();

        int receptionPaths = 0;
        int maxReceptionPaths = 1;
        while (receptionPaths < maxReceptionPaths)
        {
            DynamicCast<GatewayLoraPhy>(gwPhy)->AddReceptionPath();
            receptionPaths++;
        }
        gwPhy->AddFrequency(868100000);
    }
}

void
LorawanMacHelper::ApplyCommonAlohaConfigurations(Ptr<LorawanMac> lorawanMac) const
{
    NS_LOG_FUNCTION_NOARGS();

    //////////////
    // SubBands //
    //////////////

    auto channelHelper = Create<LogicalLoraChannelHelper>(1);
    channelHelper->AddSubBand(Create<SubBand>(868000000, 868600000, 1, 14));

    //////////////////////
    // Default channels //
    //////////////////////
    Ptr<LogicalLoraChannel> lc1 = Create<LogicalLoraChannel>(868100000, 0, 5);
    channelHelper->SetChannel(0, lc1);

    lorawanMac->SetLogicalLoraChannelHelper(channelHelper);

    ///////////////////////////////////////////////////////////
    // Data rate -> Spreading factor, Data rate -> Bandwidth //
    // and Data rate -> MaxAppPayload conversions            //
    ///////////////////////////////////////////////////////////
    lorawanMac->SetSfForDataRate(std::vector<uint8_t>{12, 11, 10, 9, 8, 7, 7});
    lorawanMac->SetBandwidthForDataRate(
        std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000, 250000});
    lorawanMac->SetMaxAppPayloadForDataRate(
        std::vector<uint32_t>{59, 59, 59, 123, 230, 230, 230, 230});
}

void
LorawanMacHelper::ConfigureForEuRegion(Ptr<ClassAEndDeviceLorawanMac> edMac) const
{
    NS_LOG_FUNCTION_NOARGS();

    ApplyCommonEuConfigurations(edMac);

    /////////////////////////////////////////////////////////
    // TxPower -> Transmission power in dBm ERP conversion //
    /////////////////////////////////////////////////////////
    edMac->SetTxDbmForTxPower(std::vector<double>{14, 12, 10, 8, 6, 4, 2, 0});

    ////////////////////////////////////////////////////////////
    // Matrix to know which data rate the gateway will respond with //
    ////////////////////////////////////////////////////////////
    LorawanMac::ReplyDataRateMatrix matrix = {{{{0, 0, 0, 0, 0, 0}},
                                               {{1, 0, 0, 0, 0, 0}},
                                               {{2, 1, 0, 0, 0, 0}},
                                               {{3, 2, 1, 0, 0, 0}},
                                               {{4, 3, 2, 1, 0, 0}},
                                               {{5, 4, 3, 2, 1, 0}},
                                               {{6, 5, 4, 3, 2, 1}},
                                               {{7, 6, 5, 4, 3, 2}}}};
    edMac->SetReplyDataRateMatrix(matrix);

    /////////////////////
    // Preamble length //
    /////////////////////
    edMac->SetNPreambleSymbols(8);

    //////////////////////////////////////
    // Second receive window parameters //
    //////////////////////////////////////
    edMac->SetSecondReceiveWindowDataRate(0);
    edMac->SetSecondReceiveWindowFrequency(869525000);
}

void
LorawanMacHelper::ConfigureForEuRegion(Ptr<GatewayLorawanMac> gwMac) const
{
    NS_LOG_FUNCTION_NOARGS();

    ///////////////////////////////
    // ReceivePath configuration //
    ///////////////////////////////
    Ptr<GatewayLoraPhy> gwPhy =
        DynamicCast<GatewayLoraPhy>(DynamicCast<LoraNetDevice>(gwMac->GetDevice())->GetPhy());

    ApplyCommonEuConfigurations(gwMac);

    if (gwPhy) // If cast is successful, there's a GatewayLoraPhy
    {
        NS_LOG_DEBUG("Resetting reception paths");
        gwPhy->ResetReceptionPaths();

        std::vector<uint32_t> frequencies;
        frequencies.push_back(868100000);
        frequencies.push_back(868300000);
        frequencies.push_back(868500000);

        for (auto& f : frequencies)
        {
            gwPhy->AddFrequency(f);
        }

        int receptionPaths = 0;
        int maxReceptionPaths = 8;
        while (receptionPaths < maxReceptionPaths)
        {
            DynamicCast<GatewayLoraPhy>(gwPhy)->AddReceptionPath();
            receptionPaths++;
        }
    }
}

void
LorawanMacHelper::ApplyCommonEuConfigurations(Ptr<LorawanMac> lorawanMac) const
{
    NS_LOG_FUNCTION_NOARGS();

    //////////////
    // SubBands //
    //////////////

    auto channelHelper = Create<LogicalLoraChannelHelper>(16);
    channelHelper->AddSubBand(Create<SubBand>(863000000, 865000000, 0.001, 14));
    channelHelper->AddSubBand(Create<SubBand>(865000000, 868000000, 0.01, 14));
    channelHelper->AddSubBand(Create<SubBand>(868000000, 868600000, 0.01, 14));
    channelHelper->AddSubBand(Create<SubBand>(868700000, 869200000, 0.001, 14));
    channelHelper->AddSubBand(Create<SubBand>(869400000, 869650000, 0.1, 27));
    channelHelper->AddSubBand(Create<SubBand>(869700000, 870000000, 0.01, 14));

    //////////////////////
    // Default channels //
    //////////////////////
    Ptr<LogicalLoraChannel> lc1 = Create<LogicalLoraChannel>(868100000, 0, 5);
    Ptr<LogicalLoraChannel> lc2 = Create<LogicalLoraChannel>(868300000, 0, 5);
    Ptr<LogicalLoraChannel> lc3 = Create<LogicalLoraChannel>(868500000, 0, 5);
    channelHelper->SetChannel(0, lc1);
    channelHelper->SetChannel(1, lc2);
    channelHelper->SetChannel(2, lc3);

    lorawanMac->SetLogicalLoraChannelHelper(channelHelper);

    ///////////////////////////////////////////////////////////
    // Data rate -> Spreading factor, Data rate -> Bandwidth //
    // and Data rate -> MaxAppPayload conversions            //
    ///////////////////////////////////////////////////////////
    lorawanMac->SetSfForDataRate(std::vector<uint8_t>{12, 11, 10, 9, 8, 7, 7});
    lorawanMac->SetBandwidthForDataRate(
        std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000, 250000});
    lorawanMac->SetMaxAppPayloadForDataRate(
        std::vector<uint32_t>{59, 59, 59, 123, 230, 230, 230, 230});
}

///////////////////////////////

void
LorawanMacHelper::ConfigureForSingleChannelRegion(Ptr<ClassAEndDeviceLorawanMac> edMac) const
{
    NS_LOG_FUNCTION_NOARGS();

    ApplyCommonSingleChannelConfigurations(edMac);

    /////////////////////////////////////////////////////////
    // TxPower -> Transmission power in dBm ERP conversion //
    /////////////////////////////////////////////////////////
    edMac->SetTxDbmForTxPower(std::vector<double>{14, 12, 10, 8, 6, 4, 2, 0});

    ////////////////////////////////////////////////////////////
    // Matrix to know which DataRate the gateway will respond with //
    ////////////////////////////////////////////////////////////
    LorawanMac::ReplyDataRateMatrix matrix = {{{{0, 0, 0, 0, 0, 0}},
                                               {{1, 0, 0, 0, 0, 0}},
                                               {{2, 1, 0, 0, 0, 0}},
                                               {{3, 2, 1, 0, 0, 0}},
                                               {{4, 3, 2, 1, 0, 0}},
                                               {{5, 4, 3, 2, 1, 0}},
                                               {{6, 5, 4, 3, 2, 1}},
                                               {{7, 6, 5, 4, 3, 2}}}};
    edMac->SetReplyDataRateMatrix(matrix);

    /////////////////////
    // Preamble length //
    /////////////////////
    edMac->SetNPreambleSymbols(8);

    //////////////////////////////////////
    // Second receive window parameters //
    //////////////////////////////////////
    edMac->SetSecondReceiveWindowDataRate(0);
    edMac->SetSecondReceiveWindowFrequency(869525000);
}

void
LorawanMacHelper::ConfigureForSingleChannelRegion(Ptr<GatewayLorawanMac> gwMac) const
{
    NS_LOG_FUNCTION_NOARGS();

    ///////////////////////////////
    // ReceivePath configuration //
    ///////////////////////////////
    Ptr<GatewayLoraPhy> gwPhy =
        DynamicCast<GatewayLoraPhy>(DynamicCast<LoraNetDevice>(gwMac->GetDevice())->GetPhy());

    ApplyCommonEuConfigurations(gwMac);

    if (gwPhy) // If cast is successful, there's a GatewayLoraPhy
    {
        NS_LOG_DEBUG("Resetting reception paths");
        gwPhy->ResetReceptionPaths();

        std::vector<uint32_t> frequencies;
        frequencies.push_back(868100000);

        for (auto& f : frequencies)
        {
            gwPhy->AddFrequency(f);
        }

        int receptionPaths = 0;
        int maxReceptionPaths = 8;
        while (receptionPaths < maxReceptionPaths)
        {
            gwPhy->AddReceptionPath();
            receptionPaths++;
        }
    }
}

void
LorawanMacHelper::ApplyCommonSingleChannelConfigurations(Ptr<LorawanMac> lorawanMac) const
{
    NS_LOG_FUNCTION_NOARGS();

    //////////////
    // SubBands //
    //////////////

    auto channelHelper = Create<LogicalLoraChannelHelper>(1);
    channelHelper->AddSubBand(Create<SubBand>(868000000, 868600000, 0.01, 14));

    //////////////////////
    // Default channels //
    //////////////////////
    Ptr<LogicalLoraChannel> lc1 = Create<LogicalLoraChannel>(868100000, 0, 5);
    channelHelper->SetChannel(0, lc1);

    lorawanMac->SetLogicalLoraChannelHelper(channelHelper);

    ///////////////////////////////////////////////////////////
    // Data rate -> Spreading factor, Data rate -> Bandwidth //
    // and Data rate -> MaxAppPayload conversions            //
    ///////////////////////////////////////////////////////////
    lorawanMac->SetSfForDataRate(std::vector<uint8_t>{12, 11, 10, 9, 8, 7, 7});
    lorawanMac->SetBandwidthForDataRate(
        std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000, 250000});
    lorawanMac->SetMaxAppPayloadForDataRate(
        std::vector<uint32_t>{59, 59, 59, 123, 230, 230, 230, 230});
}

std::vector<int>
LorawanMacHelper::SetSpreadingFactorsUp(NodeContainer endDevices,
                                        NodeContainer gateways,
                                        Ptr<LoraChannel> channel)
{
    NS_LOG_FUNCTION_NOARGS();

    std::vector<int> sfQuantity(7, 0);
    for (auto j = endDevices.Begin(); j != endDevices.End(); ++j)
    {
        Ptr<Node> object = *j;
        Ptr<MobilityModel> position = object->GetObject<MobilityModel>();
        NS_ASSERT(position);
        Ptr<NetDevice> netDevice = object->GetDevice(0);
        Ptr<LoraNetDevice> loraNetDevice = DynamicCast<LoraNetDevice>(netDevice);
        NS_ASSERT(loraNetDevice);
        Ptr<ClassAEndDeviceLorawanMac> mac =
            DynamicCast<ClassAEndDeviceLorawanMac>(loraNetDevice->GetMac());
        NS_ASSERT(mac);

        // Try computing the distance from each gateway and find the best one
        Ptr<Node> bestGateway = gateways.Get(0);
        Ptr<MobilityModel> bestGatewayPosition = bestGateway->GetObject<MobilityModel>();

        // Assume devices transmit at 14 dBm
        double highestRxPower = channel->GetRxPower(14, position, bestGatewayPosition);

        for (auto currentGw = gateways.Begin() + 1; currentGw != gateways.End(); ++currentGw)
        {
            // Compute the power received from the current gateway
            Ptr<Node> curr = *currentGw;
            Ptr<MobilityModel> currPosition = curr->GetObject<MobilityModel>();
            double currentRxPower = channel->GetRxPower(14, position, currPosition); // dBm

            if (currentRxPower > highestRxPower)
            {
                bestGateway = curr;
                bestGatewayPosition = currPosition;
                highestRxPower = currentRxPower;
            }
        }

        // NS_LOG_DEBUG ("Rx Power: " << highestRxPower);
        double rxPower = highestRxPower;

        // Get the end device sensitivity
        Ptr<EndDeviceLoraPhy> edPhy = DynamicCast<EndDeviceLoraPhy>(loraNetDevice->GetPhy());
        const double* edSensitivity = EndDeviceLoraPhy::sensitivity;

        if (rxPower > *edSensitivity)
        {
            mac->SetDataRate(5);
            sfQuantity[0] = sfQuantity[0] + 1;
        }
        else if (rxPower > *(edSensitivity + 1))
        {
            mac->SetDataRate(4);
            sfQuantity[1] = sfQuantity[1] + 1;
        }
        else if (rxPower > *(edSensitivity + 2))
        {
            mac->SetDataRate(3);
            sfQuantity[2] = sfQuantity[2] + 1;
        }
        else if (rxPower > *(edSensitivity + 3))
        {
            mac->SetDataRate(2);
            sfQuantity[3] = sfQuantity[3] + 1;
        }
        else if (rxPower > *(edSensitivity + 4))
        {
            mac->SetDataRate(1);
            sfQuantity[4] = sfQuantity[4] + 1;
        }
        else if (rxPower > *(edSensitivity + 5))
        {
            mac->SetDataRate(0);
            sfQuantity[5] = sfQuantity[5] + 1;
        }
        else // Device is out of range. Assign SF12.
        {
            // NS_LOG_DEBUG ("Device out of range");
            mac->SetDataRate(0);
            sfQuantity[6] = sfQuantity[6] + 1;
            // NS_LOG_DEBUG ("sfQuantity[6] = " << sfQuantity[6]);
        }

        /*

        // Get the Gw sensitivity
        Ptr<NetDevice> gatewayNetDevice = bestGateway->GetDevice (0);
        Ptr<LoraNetDevice> gatewayLoraNetDevice = DynamicCast<LoraNetDevice>(gatewayNetDevice);
        Ptr<GatewayLoraPhy> gatewayPhy = DynamicCast<GatewayLoraPhy>
        (gatewayLoraNetDevice->GetPhy ()); const double *gwSensitivity = gatewayPhy->sensitivity;

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
LorawanMacHelper::SetSpreadingFactorsGivenDistribution(NodeContainer endDevices,
                                                       NodeContainer gateways,
                                                       std::vector<double> distribution)
{
    NS_LOG_FUNCTION_NOARGS();
    NS_ASSERT(distribution.size() == 6);

    std::vector<int> sfQuantity(7, 0);
    Ptr<UniformRandomVariable> uniformRV = CreateObject<UniformRandomVariable>();
    std::vector<double> cumdistr(6);
    cumdistr[0] = distribution[0];
    for (int i = 1; i < 6; ++i)
    {
        cumdistr[i] = distribution[i] + cumdistr[i - 1];
    }

    NS_LOG_DEBUG("Distribution: " << distribution[0] << " " << distribution[1] << " "
                                  << distribution[2] << " " << distribution[3] << " "
                                  << distribution[4] << " " << distribution[5]);
    NS_LOG_DEBUG("Cumulative distribution: " << cumdistr[0] << " " << cumdistr[1] << " "
                                             << cumdistr[2] << " " << cumdistr[3] << " "
                                             << cumdistr[4] << " " << cumdistr[5]);

    for (auto j = endDevices.Begin(); j != endDevices.End(); ++j)
    {
        Ptr<Node> object = *j;
        Ptr<MobilityModel> position = object->GetObject<MobilityModel>();
        NS_ASSERT(position);
        Ptr<NetDevice> netDevice = object->GetDevice(0);
        Ptr<LoraNetDevice> loraNetDevice = DynamicCast<LoraNetDevice>(netDevice);
        NS_ASSERT(loraNetDevice);
        Ptr<ClassAEndDeviceLorawanMac> mac =
            DynamicCast<ClassAEndDeviceLorawanMac>(loraNetDevice->GetMac());
        NS_ASSERT(mac);

        double prob = uniformRV->GetValue(0, 1);

        // NS_LOG_DEBUG ("Probability: " << prob);
        if (prob < cumdistr[0])
        {
            mac->SetDataRate(5);
            sfQuantity[0] = sfQuantity[0] + 1;
        }
        else if (prob > cumdistr[0] && prob < cumdistr[1])
        {
            mac->SetDataRate(4);
            sfQuantity[1] = sfQuantity[1] + 1;
        }
        else if (prob > cumdistr[1] && prob < cumdistr[2])
        {
            mac->SetDataRate(3);
            sfQuantity[2] = sfQuantity[2] + 1;
        }
        else if (prob > cumdistr[2] && prob < cumdistr[3])
        {
            mac->SetDataRate(2);
            sfQuantity[3] = sfQuantity[3] + 1;
        }
        else if (prob > cumdistr[3] && prob < cumdistr[4])
        {
            mac->SetDataRate(1);
            sfQuantity[4] = sfQuantity[4] + 1;
        }
        else
        {
            mac->SetDataRate(0);
            sfQuantity[5] = sfQuantity[5] + 1;
        }

    } // end loop on nodes

    return sfQuantity;

} //  end function

} // namespace lorawan
} // namespace ns3
