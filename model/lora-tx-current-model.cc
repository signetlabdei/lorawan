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
#include "ns3/log.h"

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
                          DoubleValue(0.10),
                          MakeDoubleAccessor(&LinearLoraTxCurrentModel::SetEta,
                                             &LinearLoraTxCurrentModel::GetEta),
                          MakeDoubleChecker<double>())
            .AddAttribute("Voltage",
                          "The supply voltage (in Volts).",
                          DoubleValue(3.3),
                          MakeDoubleAccessor(&LinearLoraTxCurrentModel::SetVoltage,
                                             &LinearLoraTxCurrentModel::GetVoltage),
                          MakeDoubleChecker<double>())
            .AddAttribute("StandbyCurrent",
                          "The current in the STANDBY state (in Watts).",
                          DoubleValue(0.0014), // idle mode = 1.4mA
                          MakeDoubleAccessor(&LinearLoraTxCurrentModel::SetStandbyCurrent,
                                             &LinearLoraTxCurrentModel::GetStandbyCurrent),
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

void
LinearLoraTxCurrentModel::SetEta(double eta)
{
    NS_LOG_FUNCTION(this << eta);
    m_eta = eta;
}

void
LinearLoraTxCurrentModel::SetVoltage(double voltage)
{
    NS_LOG_FUNCTION(this << voltage);
    m_voltage = voltage;
}

void
LinearLoraTxCurrentModel::SetStandbyCurrent(double idleCurrent)
{
    NS_LOG_FUNCTION(this << idleCurrent);
    m_idleCurrent = idleCurrent;
}

double
LinearLoraTxCurrentModel::GetEta() const
{
    return m_eta;
}

double
LinearLoraTxCurrentModel::GetVoltage() const
{
    return m_voltage;
}

double
LinearLoraTxCurrentModel::GetStandbyCurrent() const
{
    return m_idleCurrent;
}

double
LinearLoraTxCurrentModel::CalcTxCurrent(double txPowerDbm) const
{
    NS_LOG_FUNCTION(this << txPowerDbm);
    return DbmToW(txPowerDbm) / (m_voltage * m_eta) + m_idleCurrent;
}

NS_OBJECT_ENSURE_REGISTERED(ConstantLoraTxCurrentModel);

TypeId
ConstantLoraTxCurrentModel::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::ConstantLoraTxCurrentModel")
            .SetParent<LoraTxCurrentModel>()
            .SetGroupName("Lora")
            .AddConstructor<ConstantLoraTxCurrentModel>()
            .AddAttribute("TxCurrent",
                          "The radio Tx current in Ampere.",
                          DoubleValue(0.028), // transmit at 0dBm = 28mA
                          MakeDoubleAccessor(&ConstantLoraTxCurrentModel::SetTxCurrent,
                                             &ConstantLoraTxCurrentModel::GetTxCurrent),
                          MakeDoubleChecker<double>());
    return tid;
}

ConstantLoraTxCurrentModel::ConstantLoraTxCurrentModel()
{
    NS_LOG_FUNCTION(this);
}

ConstantLoraTxCurrentModel::~ConstantLoraTxCurrentModel()
{
    NS_LOG_FUNCTION(this);
}

void
ConstantLoraTxCurrentModel::SetTxCurrent(double txCurrent)
{
    NS_LOG_FUNCTION(this << txCurrent);
    m_txCurrent = txCurrent;
}

double
ConstantLoraTxCurrentModel::GetTxCurrent() const
{
    return m_txCurrent;
}

double
ConstantLoraTxCurrentModel::CalcTxCurrent(double txPowerDbm) const
{
    NS_LOG_FUNCTION(this << txPowerDbm);
    return m_txCurrent;
}

} // namespace lorawan
} // namespace ns3
