/*
 * Copyright (c) 2021 Alessandro Aimi
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
 * Author: Alessandro Aimi <alessandro.aimi@orange.com>
 *                         <alessandro.aimi@cnam.fr>
 *
 */

#include "recv-window-manager.h"

#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("RecvWindowManager");

NS_OBJECT_ENSURE_REGISTERED(RecvWindowManager);

TypeId
RecvWindowManager::GetTypeId()
{
    static TypeId tid = TypeId("ns3::RecvWindowManager")
                            .SetParent<Object>()
                            .SetGroupName("lorawan")
                            .AddConstructor<RecvWindowManager>();
    return tid;
}

RecvWindowManager::RecvWindowManager()
    : m_win({{Seconds(1), 12, Seconds(0), 868100000}, {Seconds(2), 12, Seconds(0), 869525000}}),
      m_phy(nullptr)
{
    NS_LOG_FUNCTION(this);
}

RecvWindowManager::~RecvWindowManager()
{
    NS_LOG_FUNCTION(this);
}

void
RecvWindowManager::Start()
{
    NS_LOG_FUNCTION(this);
    NS_ASSERT_MSG(bool(m_phy), "No physical layer was set.");
    // Schedule the opening of the first receive window
    Simulator::Schedule(m_win[FIRST].delay, &RecvWindowManager::OpenWin, this, FIRST);
    // Schedule the opening of the second receive window
    m_second = Simulator::Schedule(m_win[SECOND].delay, &RecvWindowManager::OpenWin, this, SECOND);
}

void
RecvWindowManager::ForceSleep()
{
    NS_LOG_FUNCTION(this);
    m_closing.Cancel();
    if (m_phy->GetState() == EndDeviceLoraPhy::STANDBY)
    {
        m_phy->SwitchToSleep();
    }
}

void
RecvWindowManager::Stop()
{
    NS_LOG_FUNCTION(this);
    ForceSleep();
    m_second.Cancel();
}

bool
RecvWindowManager::NoMoreWindows()
{
    return m_second.IsExpired();
}

void
RecvWindowManager::SetRx1Delay(Time d)
{
    NS_LOG_FUNCTION(this << d);
    m_win[FIRST].delay = d;
    m_win[SECOND].delay = d + Seconds(1);
}

void
RecvWindowManager::SetSf(WinId id, uint8_t sf)
{
    NS_LOG_FUNCTION(this << id << sf);
    m_win[id].sf = sf;
}

void
RecvWindowManager::SetDuration(WinId id, Time d)
{
    NS_LOG_FUNCTION(this << id << d);
    m_win[id].duration = d;
}

void
RecvWindowManager::SetFrequency(WinId id, double f)
{
    NS_LOG_FUNCTION(this << id << f);
    m_win[id].frequency = f;
}

void
RecvWindowManager::SetPhy(Ptr<EndDeviceLoraPhy> phy)
{
    m_phy = phy;
}

void
RecvWindowManager::SetNoRecvCallback(NoRecvCallback cb)
{
    m_noRecvCallback = cb;
}

void
RecvWindowManager::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_win.clear();
    m_phy = nullptr;
    m_closing.Cancel();
    m_second.Cancel();
}

void
RecvWindowManager::OpenWin(WinId id)
{
    NS_LOG_FUNCTION(this << id);
    // Set reception window parameters
    m_phy->SetRxSpreadingFactor(m_win[id].sf);
    m_phy->SetRxFrequency(m_win[id].frequency);
    NS_LOG_DEBUG("Opening reception window with parameters: freq="
                 << m_win[id].frequency << "Hz, SF=" << unsigned(m_win[id].sf) << ".");
    // Set Phy in Standby mode
    m_phy->SwitchToStandby();
    // Schedule closure
    m_closing = Simulator::Schedule(m_win[id].duration, &RecvWindowManager::CloseWin, this, id);
}

void
RecvWindowManager::CloseWin(WinId id)
{
    NS_LOG_FUNCTION(this << id);
    // Check the Phy layer's state:
    // - RX -> We have received a preamble.
    // - STANDBY -> Nothing was detected.
    // We should never be in TX or SLEEP mode at this point
    switch (m_phy->GetState())
    {
    case EndDeviceLoraPhy::TX:
        NS_ABORT_MSG("PHY was in TX mode when attempting to "
                     << "close a receive window.");
    case EndDeviceLoraPhy::SLEEP:
        NS_ABORT_MSG("PHY was in already in SLEEP mode when attempting to "
                     << "close a receive window.");
    case EndDeviceLoraPhy::RX:
        // PHY is receiving: let it finish
        NS_LOG_DEBUG("PHY is receiving: Receive will handle the result.");
        return;
    case EndDeviceLoraPhy::STANDBY:
        // No reception, turn PHY layer to sleep
        m_phy->SwitchToSleep();
        if (id == SECOND && !m_noRecvCallback.IsNull())
        {
            m_noRecvCallback();
        }
    }
}

} // namespace lorawan
} // namespace ns3
