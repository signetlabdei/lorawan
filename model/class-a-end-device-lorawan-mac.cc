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
 *         Martina Capuzzo <capuzzom@dei.unipd.it>
 */

#include "ns3/log.h"

namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("ClassAEndDeviceLorawanMac");

NS_OBJECT_ENSURE_REGISTERED (ClassAEndDeviceLorawanMac);

TypeId
ClassAEndDeviceLorawanMac::GetTypeId (void)
{
static TypeId tid = TypeId ("ns3::ClassAEndDeviceLorawanMac")
  .SetParent<EndDeviceLorawanMac> ()
  .SetGroupName ("lorawan")
  .AddConstructor<ClassAEndDeviceLorawanMac> ();
return tid;
}

ClassAEndDeviceLorawanMac::ClassAEndDeviceLorawanMac () :
{
  NS_LOG_FUNCTION (this);
}

ClassAEndDeviceLorawanMac::~ClassAEndDeviceLorawanMac ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

//////////////////////////
//  Receiving methods   //
//////////////////////////
void
ClassAEndDeviceLorawanMac::Receive (Ptr<Packet const> packet)
{
}

void
ClassAEndDeviceLorawanMac::FailedReception (Ptr<Packet const> packet)
{
}

void
ClassAEndDeviceLorawanMac::TxFinished (Ptr<const Packet> packet)
{

}

void
ClassAEndDeviceLorawanMac::OpenFirstReceiveWindow (void)
{
}

void
ClassAEndDeviceLorawanMac::CloseFirstReceiveWindow (void)
{
}

void
ClassAEndDeviceLorawanMac::OpenSecondReceiveWindow (void)
{
}

void
ClassAEndDeviceLorawanMac::CloseSecondReceiveWindow (void)
{
}

/////////////////////////
// Getters and Setters //
/////////////////////////

Time
ClassAEndDeviceLorawanMac::GetNextClassTransmissionDelay (void)
{
}

uint8_t
ClassAEndDeviceLorawanMac::GetReceiveWindow (void)
{
}

uint8_t
ClassAEndDeviceLorawanMac::GetFirstReceiveWindowDataRate (void)
{
}

void
ClassAEndDeviceLorawanMac::SetSecondReceiveWindowDataRate (uint8_t dataRate)
{
}

uint8_t
ClassAEndDeviceLorawanMac::GetSecondReceiveWindowDataRate (void)
{
}

void
ClassAEndDeviceLorawanMac::SetSecondReceiveWindowFrequency (double frequencyMHz)
{
}

double
ClassAEndDeviceLorawanMac::GetSecondReceiveWindowFrequency (void)
{
}

/////////////////////////
// MAC command methods //
/////////////////////////

void
ClassAEndDeviceLorawanMac::OnRxClassParamSetupReq (uint8_t rx1DrOffset, uint8_t rx2DataRate, double frequency)
{
}

} /* namespace lorawan */
} /* namespace ns3 */
