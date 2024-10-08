/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "lora-device-address-generator.h"

#include "ns3/log.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraDeviceAddressGenerator");

TypeId
LoraDeviceAddressGenerator::GetTypeId()
{
    static TypeId tid = TypeId("ns3::LoraDeviceAddressGenerator")
                            .SetParent<Object>()
                            .SetGroupName("lorawan")
                            .AddConstructor<LoraDeviceAddressGenerator>();
    return tid;
}

LoraDeviceAddressGenerator::LoraDeviceAddressGenerator(const uint8_t nwkId, const uint32_t nwkAddr)
{
    NS_LOG_FUNCTION(this << unsigned(nwkId) << nwkAddr);

    m_currentNwkId.Set(nwkId);
    m_currentNwkAddr.Set(nwkAddr);
}

LoraDeviceAddress
LoraDeviceAddressGenerator::NextNetwork()
{
    NS_LOG_FUNCTION_NOARGS();

    m_currentNwkId.Set(m_currentNwkId.Get() + 1);
    m_currentNwkAddr.Set(0);

    return LoraDeviceAddress(m_currentNwkId, m_currentNwkAddr);
}

LoraDeviceAddress
LoraDeviceAddressGenerator::NextAddress()
{
    NS_LOG_FUNCTION_NOARGS();

    NwkAddr oldNwkAddr = m_currentNwkAddr;
    m_currentNwkAddr.Set(m_currentNwkAddr.Get() + 1);

    return LoraDeviceAddress(m_currentNwkId, oldNwkAddr);
}

LoraDeviceAddress
LoraDeviceAddressGenerator::GetNextAddress()
{
    NS_LOG_FUNCTION_NOARGS();

    return LoraDeviceAddress(m_currentNwkId.Get(), m_currentNwkAddr.Get() + 1);
}
} // namespace lorawan
} // namespace ns3
