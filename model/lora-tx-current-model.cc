/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Romagnolo Stefano <romagnolostefano93@gmail.com>
 */

#include "lora-tx-current-model.h"

#include "lora-utils.h"

#include "ns3/double.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraTxCurrentModel");

NS_OBJECT_ENSURE_REGISTERED(LoraTxCurrentModel);

TypeId
LoraTxCurrentModel::GetTypeId()
{
    static TypeId tid = TypeId("ns3::LoraTxCurrentModel").SetParent<Object>().SetGroupName("Lora");
    return tid;
}

LoraTxCurrentModel::LoraTxCurrentModel()
{
}

LoraTxCurrentModel::~LoraTxCurrentModel()
{
}

// Similarly to the wifi case
NS_OBJECT_ENSURE_REGISTERED(LinearLoraTxCurrentModel);

TypeId
LinearLoraTxCurrentModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LinearLoraTxCurrentModel")
            .SetParent<LoraTxCurrentModel>()
            .SetGroupName("Lora")
            .AddConstructor<LinearLoraTxCurrentModel>()
            .AddAttribute("Eta",
                          "The efficiency of the power amplifier.",
                          DoubleValue(0.452750), // see class description
                          MakeDoubleAccessor(&LinearLoraTxCurrentModel::m_eta),
                          MakeDoubleChecker<double>())
            .AddAttribute("Voltage",
                          "The supply voltage (in Volts).",
                          DoubleValue(3.3),
                          MakeDoubleAccessor(&LinearLoraTxCurrentModel::m_voltage),
                          MakeDoubleChecker<double>())
            .AddAttribute("BaseCurrent",
                          "The TX baseline current (in Ampere) at 0W output.",
                          DoubleValue(0.014646), // see class description
                          MakeDoubleAccessor(&LinearLoraTxCurrentModel::m_baseCurrent),
                          MakeDoubleChecker<double>());
    return tid;
}

LinearLoraTxCurrentModel::LinearLoraTxCurrentModel()
{
    NS_LOG_FUNCTION(this);
}

LinearLoraTxCurrentModel::~LinearLoraTxCurrentModel()
{
    NS_LOG_FUNCTION(this);
}

double
LinearLoraTxCurrentModel::CalcTxCurrent(double txPowerDbm) const
{
    NS_LOG_FUNCTION(this << txPowerDbm);
    return DbmToW(txPowerDbm) / (m_voltage * m_eta) + m_baseCurrent;
}

} // namespace lorawan
} // namespace ns3
