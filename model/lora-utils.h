/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Romagnolo Stefano <romagnolostefano93@gmail.com>
 */

#ifndef LORA_UTILS_H
#define LORA_UTILS_H

#include "ns3/nstime.h"
#include "ns3/uinteger.h"

namespace ns3
{
namespace lorawan
{

/**
 * Convert from dBm to Watts.
 *
 * @param dbm The power in dBm.
 *
 * @return The equivalent Watts for the given dBm value.
 */
double DbmToW(double dbm);
/**
 * Convert from dB to ratio.
 *
 * @param db The dB value.
 *
 * @return The equivalent ratio from the given dB value.
 */
double DbToRatio(double db);
/**
 * Convert from Watts to dBm.
 *
 * @param w The power in Watts.
 *
 * @return The equivalent dBm for the given Watts.
 */
double WToDbm(double w);
/**
 * Convert from ratio to dB.
 *
 * @param ratio The ratio value.
 *
 * @return The equivalent dB from the given ratio value.
 */
double RatioToDb(double ratio);

} // namespace lorawan

} // namespace ns3
#endif /* LORA_UTILS_H */
