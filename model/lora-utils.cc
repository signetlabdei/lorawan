/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 * Author: Romagnolo Stefano <romagnolostefano93@gmail.com>
 */

#include "lora-utils.h"
#include <cmath>

namespace ns3 {
namespace lorawan {

double
DbToRatio (double dB)
{
  double ratio = std::pow (10.0, dB / 10.0);
  return ratio;
}

double
DbmToW (double dBm)
{
  double mW = std::pow (10.0, dBm / 10.0);
  return mW / 1000.0;
}

double
WToDbm (double w)
{
  return 10.0 * std::log10 (w * 1000.0);
}

double
RatioToDb (double ratio)
{
  return 10.0 * std::log10 (ratio);
}

}
} //namespace ns3
