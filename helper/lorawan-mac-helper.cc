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
 *
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#include "ns3/lorawan-mac-helper.h"

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
LorawanMacHelper::SetRegion(enum LorawanMacHelper::Regions region)
{
    m_region = region;
}

void
LorawanMacHelper::SetAddressGenerator(Ptr<LoraDeviceAddressGenerator> addrGen)
{
    m_addrGen = addrGen;
}

Ptr<LorawanMac>
LorawanMacHelper::Create(Ptr<LoraNetDevice> device) const
{
    Ptr<LorawanMac> mac = m_mac.Create<LorawanMac>();
    mac->SetDevice(device);
    device->SetMac(mac);

    switch (m_region)
    {
    case LorawanMacHelper::EU:
        ConfigureForEuRegion(mac);
        break;
    case LorawanMacHelper::SingleChannel:
        ConfigureForSingleChannelRegion(mac);
        break;
    case LorawanMacHelper::ALOHA:
        ConfigureForAlohaRegion(mac);
        break;
    default:
        NS_LOG_ERROR("This region isn't supported yet!");
    }
    
    return mac;
}

void
LorawanMacHelper::ConfigureForAlohaRegion(Ptr<LorawanMac> mac) const
{
    //////////////
    // SubBands //
    //////////////

    LogicalLoraChannelHelper channelHelper;
    channelHelper.AddSubBand(868000000, 868600000, 1, 14);

    //////////////////////
    // Default channels //
    //////////////////////

    Ptr<LogicalLoraChannel> lc0 = CreateObject<LogicalLoraChannel>(868100000, 0, 5);
    channelHelper.AddChannel(0, lc0);

    mac->SetLogicalLoraChannelHelper(channelHelper);

    ///////////////////////////////////////////////
    // DataRate -> SF, DataRate -> Bandwidth     //
    // and DataRate -> MaxAppPayload conversions //
    ///////////////////////////////////////////////

    mac->SetSfForDataRate(std::vector<uint8_t>{12, 11, 10, 9, 8, 7});
    mac->SetBandwidthForDataRate(
        std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000});
    mac->SetMaxMacPayloadForDataRate(std::vector<uint32_t>{59, 59, 59, 123, 230, 230});

    ////////////////////////////////////////////
    // Configurations specific to end devices //
    ////////////////////////////////////////////

    if (auto edMac = DynamicCast<ClassAEndDeviceLorawanMac>(mac); edMac != nullptr)
    {
        /////////////////////////////////////////////////////
        // TxPower -> Transmission power in dBm conversion //
        /////////////////////////////////////////////////////

        edMac->SetTxDbmForTxPower(std::vector<double>{14, 12, 10, 8, 6, 4, 2, 0});

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

        /////////////
        // Address //
        /////////////

        if (m_addrGen)
            edMac->SetDeviceAddress(m_addrGen->NextAddress());
    }
}

void
LorawanMacHelper::ConfigureForEuRegion(Ptr<LorawanMac> mac) const
{
    //////////////
    // SubBands //
    //////////////

    LogicalLoraChannelHelper channelHelper;
    channelHelper.AddSubBand(863000000, 865000000, 0.001, 14);
    channelHelper.AddSubBand(865000000, 868000000, 0.01, 14);
    channelHelper.AddSubBand(868000000, 868600000, 0.01, 14);
    channelHelper.AddSubBand(868700000, 869200000, 0.001, 14);
    channelHelper.AddSubBand(869400000, 869650000, 0.1, 27);
    channelHelper.AddSubBand(869700000, 870000000, 0.01, 14);

    //////////////////////
    // Default channels //
    //////////////////////

    Ptr<LogicalLoraChannel> lc0 = CreateObject<LogicalLoraChannel>(868100000, 0, 5);
    Ptr<LogicalLoraChannel> lc1 = CreateObject<LogicalLoraChannel>(868300000, 0, 5);
    Ptr<LogicalLoraChannel> lc2 = CreateObject<LogicalLoraChannel>(868500000, 0, 5);
    channelHelper.AddChannel(0, lc0);
    channelHelper.AddChannel(1, lc1);
    channelHelper.AddChannel(2, lc2);

    ////////////////////////
    // Addtional channels //
    ////////////////////////

    Ptr<LogicalLoraChannel> lc3 = CreateObject<LogicalLoraChannel>(867100000, 0, 5);
    Ptr<LogicalLoraChannel> lc4 = CreateObject<LogicalLoraChannel>(867300000, 0, 5);
    Ptr<LogicalLoraChannel> lc5 = CreateObject<LogicalLoraChannel>(867500000, 0, 5);
    Ptr<LogicalLoraChannel> lc6 = CreateObject<LogicalLoraChannel>(867700000, 0, 5);
    Ptr<LogicalLoraChannel> lc7 = CreateObject<LogicalLoraChannel>(867900000, 0, 5);
    // channelHelper.AddChannel (3, lc3);
    // channelHelper.AddChannel (4, lc4);
    // channelHelper.AddChannel (5, lc5);
    // channelHelper.AddChannel (6, lc6);
    // channelHelper.AddChannel (7, lc7);

    mac->SetLogicalLoraChannelHelper(channelHelper);

    ///////////////////////////////////////////////
    // DataRate -> SF, DataRate -> Bandwidth     //
    // and DataRate -> MaxAppPayload conversions //
    ///////////////////////////////////////////////

    mac->SetSfForDataRate(std::vector<uint8_t>{12, 11, 10, 9, 8, 7});
    mac->SetBandwidthForDataRate(
        std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000});
    mac->SetMaxMacPayloadForDataRate(std::vector<uint32_t>{59, 59, 59, 123, 230, 230});

    ////////////////////////////////////////////
    // Configurations specific to end devices //
    ////////////////////////////////////////////

    if (auto edMac = DynamicCast<ClassAEndDeviceLorawanMac>(mac); edMac != nullptr)
    {
        ////////////////////////////////////////////////////////////
        // TxPower -> Transmission power in dBm e.r.p. conversion //
        ////////////////////////////////////////////////////////////

        edMac->SetTxDbmForTxPower(std::vector<double>{14, 12, 10, 8, 6, 4, 2, 0});

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

        /////////////
        // Address //
        /////////////

        if (m_addrGen)
            edMac->SetDeviceAddress(m_addrGen->NextAddress());
    }
}

void
LorawanMacHelper::ConfigureForSingleChannelRegion(Ptr<LorawanMac> mac) const
{
    //////////////
    // SubBands //
    //////////////

    LogicalLoraChannelHelper channelHelper;
    channelHelper.AddSubBand(868000000, 868600000, 0.01, 14);
    channelHelper.AddSubBand(868700000, 869200000, 0.001, 14);
    channelHelper.AddSubBand(869400000, 869650000, 0.1, 27);

    //////////////////////
    // Default channels //
    //////////////////////

    Ptr<LogicalLoraChannel> lc0 = CreateObject<LogicalLoraChannel>(868100000, 0, 5);
    channelHelper.AddChannel(0, lc0);

    mac->SetLogicalLoraChannelHelper(channelHelper);

    ///////////////////////////////////////////////
    // DataRate -> SF, DataRate -> Bandwidth     //
    // and DataRate -> MaxAppPayload conversions //
    ///////////////////////////////////////////////

    mac->SetSfForDataRate(std::vector<uint8_t>{12, 11, 10, 9, 8, 7});
    mac->SetBandwidthForDataRate(
        std::vector<double>{125000, 125000, 125000, 125000, 125000, 125000});
    mac->SetMaxMacPayloadForDataRate(std::vector<uint32_t>{59, 59, 59, 123, 230, 230});

    ////////////////////////////////////////////
    // Configurations specific to end devices //
    ////////////////////////////////////////////

    if (auto edMac = DynamicCast<ClassAEndDeviceLorawanMac>(mac); edMac != nullptr)
    {
        /////////////////////////////////////////////////////
        // TxPower -> Transmission power in dBm conversion //
        /////////////////////////////////////////////////////

        edMac->SetTxDbmForTxPower(std::vector<double>{14, 12, 10, 8, 6, 4, 2, 0});

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

        /////////////
        // Address //
        /////////////

        if (m_addrGen)
            edMac->SetDeviceAddress(m_addrGen->NextAddress());
    }
}

std::vector<int>
LorawanMacHelper::SetSpreadingFactorsUp(NodeContainer endDevices,
                                        NodeContainer gateways,
                                        Ptr<LoraChannel> channel)
{
    NS_LOG_FUNCTION_NOARGS();

    std::vector<int> sfQuantity(6, 0);
    for (NodeContainer::Iterator j = endDevices.Begin(); j != endDevices.End(); ++j)
    {
        Ptr<Node> object = *j;
        Ptr<MobilityModel> position = object->GetObject<MobilityModel>();
        NS_ASSERT(bool(position) != 0);
        Ptr<NetDevice> netDevice = object->GetDevice(0);
        Ptr<LoraNetDevice> loraNetDevice = netDevice->GetObject<LoraNetDevice>();
        NS_ASSERT(bool(loraNetDevice) != 0);
        Ptr<ClassAEndDeviceLorawanMac> mac =
            loraNetDevice->GetMac()->GetObject<ClassAEndDeviceLorawanMac>();
        NS_ASSERT(bool(mac) != 0);

        // Try computing the distance from each gateway and find the best one
        Ptr<Node> bestGateway = gateways.Get(0);
        Ptr<MobilityModel> bestGatewayPosition = bestGateway->GetObject<MobilityModel>();

        // Assume devices transmit at 14 dBm erp
        double highestRxPower = channel->GetRxPower(14, position, bestGatewayPosition);

        for (NodeContainer::Iterator currentGw = gateways.Begin() + 1; currentGw != gateways.End();
             ++currentGw)
        {
            // Compute the power received from the current gateway
            Ptr<Node> curr = *currentGw;
            Ptr<MobilityModel> currPosition = curr->GetObject<MobilityModel>();
            double currentRxPower = channel->GetRxPower(14, position, currPosition); // dBm

            if (currentRxPower > highestRxPower)
            {
                bestGateway = curr;
                bestGatewayPosition = curr->GetObject<MobilityModel>();
                highestRxPower = currentRxPower;
            }
        }

        // NS_LOG_DEBUG ("Rx Power: " << highestRxPower);
        double rxPower = highestRxPower;

        std::vector<double> snrThresholds = {-7.5, -10, -12.5, -15, -17.5, -20}; // dB
        double noise = -174.0 + 10 * log10(125000.0) + 6;                        // dBm
        double snr = rxPower - noise;                                            // dB

        double prob_H = 0.98;
        // dB, desired thermal gain for 0.98 PDR with rayleigh fading
        double deviceMargin = 10 * log10(-1 / log(prob_H));
        double snrMargin = snr - deviceMargin;

        uint8_t datarate = 0; // SF12 by default
        if (snrMargin > snrThresholds[0])
            datarate = 5; // SF7
        else if (snrMargin > snrThresholds[1])
            datarate = 4; // SF8
        else if (snrMargin > snrThresholds[2])
            datarate = 3; // SF9
        else if (snrMargin > snrThresholds[3])
            datarate = 2; // SF10
        else if (snrMargin > snrThresholds[4])
            datarate = 1; // SF11

        mac->SetDataRate(datarate);
        sfQuantity[datarate]++;

        // Minimize power
        if (datarate != 6)
            continue;
        for (int j = 14; j >= 0; j -= 2)
        {
            snrMargin =
                channel->GetRxPower(14, position, bestGatewayPosition) - noise - deviceMargin;
            if (snrMargin > snrThresholds[0])
            {
                mac->SetTransmissionPower(14 - j);
                break;
            }
        }
    } // end loop on nodes

    return sfQuantity;

} //  end function

std::vector<int>
LorawanMacHelper::SetSpreadingFactorsGivenDistribution(NodeContainer endDevices,
                                                       NodeContainer gateways,
                                                       std::vector<double> distribution)
{
    NS_LOG_FUNCTION_NOARGS();

    std::vector<int> sfQuantity(7, 0);
    Ptr<UniformRandomVariable> uniformRV = CreateObject<UniformRandomVariable>();
    std::vector<double> cumdistr(6);
    cumdistr[0] = distribution[0];
    for (int i = 1; i < 7; ++i)
    {
        cumdistr[i] = distribution[i] + cumdistr[i - 1];
    }

    NS_LOG_DEBUG("Distribution: " << distribution[0] << " " << distribution[1] << " "
                                  << distribution[2] << " " << distribution[3] << " "
                                  << distribution[4] << " " << distribution[5]);
    NS_LOG_DEBUG("Cumulative distribution: " << cumdistr[0] << " " << cumdistr[1] << " "
                                             << cumdistr[2] << " " << cumdistr[3] << " "
                                             << cumdistr[4] << " " << cumdistr[5]);

    for (NodeContainer::Iterator j = endDevices.Begin(); j != endDevices.End(); ++j)
    {
        Ptr<Node> object = *j;
        Ptr<MobilityModel> position = object->GetObject<MobilityModel>();
        NS_ASSERT(bool(position) != 0);
        Ptr<NetDevice> netDevice = object->GetDevice(0);
        Ptr<LoraNetDevice> loraNetDevice = netDevice->GetObject<LoraNetDevice>();
        NS_ASSERT(bool(loraNetDevice) != 0);
        Ptr<ClassAEndDeviceLorawanMac> mac =
            loraNetDevice->GetMac()->GetObject<ClassAEndDeviceLorawanMac>();
        NS_ASSERT(bool(mac) != 0);

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
