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

#include "lorawan-mac-helper.h"

#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/lora-application.h"
#include "ns3/node-list.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LorawanMacHelper");

LorawanMacHelper::LorawanMacHelper()
    : m_region(LorawanMacHelper::EU)
{
    // By default, we create an ClassAEndDeviceLorawanMac.
    SetType("ns3::ClassAEndDeviceLorawanMac");
}

LorawanMacHelper::~LorawanMacHelper()
{
    m_addrGen = nullptr;
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
LorawanMacHelper::Install(Ptr<LoraNetDevice> device) const
{
    Ptr<LorawanMac> mac = m_mac.Create<LorawanMac>();
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
    device->SetMac(mac);
    return mac;
}

void
LorawanMacHelper::ConfigureForAlohaRegion(Ptr<LorawanMac> mac) const
{
    //////////////
    // SubBands //
    //////////////

    auto channelHelper = CreateObject<LogicalChannelManager>();
    channelHelper->AddSubBand(868000000, 868600000, 1, 14);

    //////////////////////
    // Default channels //
    //////////////////////

    Ptr<LogicalChannel> lc0 = Create<LogicalChannel>(868100000, 0, 5);
    channelHelper->AddChannel(0, lc0);

    mac->SetLogicalChannelManager(channelHelper);

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
        {
            edMac->SetDeviceAddress(m_addrGen->NextAddress());
        }
    }
}

void
LorawanMacHelper::ConfigureForEuRegion(Ptr<LorawanMac> mac) const
{
    //////////////
    // SubBands //
    //////////////

    auto channelHelper = CreateObject<LogicalChannelManager>();
    channelHelper->AddSubBand(863000000, 865000000, 0.001, 14);
    channelHelper->AddSubBand(865000000, 868000000, 0.01, 14);
    channelHelper->AddSubBand(868000000, 868600000, 0.01, 14);
    channelHelper->AddSubBand(868700000, 869200000, 0.001, 14);
    channelHelper->AddSubBand(869400000, 869650000, 0.1, 27);
    channelHelper->AddSubBand(869700000, 870000000, 0.01, 14);

    //////////////////////
    // Default channels //
    //////////////////////

    Ptr<LogicalChannel> lc0 = Create<LogicalChannel>(868100000, 0, 5);
    Ptr<LogicalChannel> lc1 = Create<LogicalChannel>(868300000, 0, 5);
    Ptr<LogicalChannel> lc2 = Create<LogicalChannel>(868500000, 0, 5);
    channelHelper->AddChannel(0, lc0);
    channelHelper->AddChannel(1, lc1);
    channelHelper->AddChannel(2, lc2);

    ////////////////////////
    // Addtional channels //
    ////////////////////////

    Ptr<LogicalChannel> lc3 = Create<LogicalChannel>(867100000, 0, 5);
    Ptr<LogicalChannel> lc4 = Create<LogicalChannel>(867300000, 0, 5);
    Ptr<LogicalChannel> lc5 = Create<LogicalChannel>(867500000, 0, 5);
    Ptr<LogicalChannel> lc6 = Create<LogicalChannel>(867700000, 0, 5);
    Ptr<LogicalChannel> lc7 = Create<LogicalChannel>(867900000, 0, 5);
    channelHelper->AddChannel(3, lc3);
    channelHelper->AddChannel(4, lc4);
    channelHelper->AddChannel(5, lc5);
    channelHelper->AddChannel(6, lc6);
    channelHelper->AddChannel(7, lc7);

    mac->SetLogicalChannelManager(channelHelper);

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
        {
            edMac->SetDeviceAddress(m_addrGen->NextAddress());
        }
    }
}

void
LorawanMacHelper::ConfigureForSingleChannelRegion(Ptr<LorawanMac> mac) const
{
    //////////////
    // SubBands //
    //////////////

    auto channelHelper = CreateObject<LogicalChannelManager>();
    channelHelper->AddSubBand(868000000, 868600000, 0.01, 14);
    channelHelper->AddSubBand(868700000, 869200000, 0.001, 14);
    channelHelper->AddSubBand(869400000, 869650000, 0.1, 27);

    //////////////////////
    // Default channels //
    //////////////////////

    Ptr<LogicalChannel> lc0 = Create<LogicalChannel>(868100000, 0, 5);
    channelHelper->AddChannel(0, lc0);

    mac->SetLogicalChannelManager(channelHelper);

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
        {
            edMac->SetDeviceAddress(m_addrGen->NextAddress());
        }
    }
}

std::vector<int>
LorawanMacHelper::SetSpreadingFactorsUp(NodeContainer endDevices,
                                        NodeContainer gateways,
                                        Ptr<LoraChannel> channel)
{
    NS_LOG_FUNCTION_NOARGS();

    std::vector<int> sfQuantity(6, 0);
    for (auto j = endDevices.Begin(); j != endDevices.End(); ++j)
    {
        auto node = *j;
        auto loraNetDevice = DynamicCast<LoraNetDevice>(node->GetDevice(0));
        NS_ASSERT(bool(loraNetDevice));
        auto position = node->GetObject<MobilityModel>();
        auto mac = DynamicCast<BaseEndDeviceLorawanMac>(loraNetDevice->GetMac());
        NS_ASSERT(bool(position) && bool(mac));

        // Try computing the distance from each gateway and find the best one
        auto bestGateway = gateways.Get(0);
        auto bestGatewayPosition = bestGateway->GetObject<MobilityModel>();
        // Assume devices transmit at 14 dBm erp
        double highestRxPower = channel->GetRxPower(14, position, bestGatewayPosition);
        for (auto currentGw = gateways.Begin() + 1; currentGw != gateways.End(); ++currentGw)
        {
            // Compute the power received from the current gateway
            auto curr = *currentGw;
            auto currPosition = curr->GetObject<MobilityModel>();
            double currentRxPower = channel->GetRxPower(14, position, currPosition); // dBm
            if (currentRxPower > highestRxPower)
            {
                bestGateway = curr;
                bestGatewayPosition = curr->GetObject<MobilityModel>();
                highestRxPower = currentRxPower;
            }
        }
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
        {
            datarate = 5; // SF7
        }
        else if (snrMargin > snrThresholds[1])
        {
            datarate = 4; // SF8
        }
        else if (snrMargin > snrThresholds[2])
        {
            datarate = 3; // SF9
        }
        else if (snrMargin > snrThresholds[3])
        {
            datarate = 2; // SF10
        }
        else if (snrMargin > snrThresholds[4])
        {
            datarate = 1; // SF11
        }

        mac->SetDataRate(datarate);
        sfQuantity[datarate]++;

        // Minimize power
        if (datarate != 6)
        {
            continue;
        }
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

} // namespace lorawan
} // namespace ns3
