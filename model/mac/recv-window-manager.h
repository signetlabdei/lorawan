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

#ifndef RECV_WINDOW_MANAGER_H
#define RECV_WINDOW_MANAGER_H

#include "ns3/end-device-lora-phy.h"
#include "ns3/nstime.h"
#include "ns3/object.h"

namespace ns3
{
namespace lorawan
{

/**
 * This class schedules the two LoRaWAN Class A reception windows
 * and manages the PHY state change SLEEP->STANDBY and vice versa
 */
class RecvWindowManager : public Object
{
    struct RecvWin_t
    {
        Time delay;
        uint8_t sf;
        Time duration;
        double frequency;
    };

  public:
    enum WinId
    {
        FIRST,
        SECOND
    };

    typedef Callback<void> NoRecvCallback;

    static TypeId GetTypeId();

    RecvWindowManager();
    ~RecvWindowManager() override;

    /* Start the reception windows scheduling process */
    void Start();
    /* Interrupt the process and ensure the device is put back to sleep */
    void Stop();
    /* Ensure the evice is put to sleep, but do not stop the process if
     * there are more reception windows to come */
    void ForceSleep();
    /* True if no more reception windows are scheduled to be opened */
    bool NoMoreWindows();

    /* Set RX1 delay (RX2 delay will be set to + 1s) */
    void SetRx1Delay(Time d);
    /* Set SF of window based on id */
    void SetSf(WinId id, uint8_t sf);
    /* Set duration of window based on id */
    void SetDuration(WinId id, Time d);
    /* Set frequency of window based on id */
    void SetFrequency(WinId id, double f);

    /* Set device physiscal layer */
    void SetPhy(Ptr<EndDeviceLoraPhy> phy);
    /* Set callback function to be called on expiration of second reception window */
    void SetNoRecvCallback(NoRecvCallback cb);

  protected:
    void DoDispose() override;

  private:
    /* Set frequency of window based on id */
    void OpenWin(WinId id);
    /* Set frequency of window based on id */
    void CloseWin(WinId id);

    std::vector<RecvWin_t> m_win;
    Ptr<EndDeviceLoraPhy> m_phy;
    EventId m_closing;
    EventId m_second;
    NoRecvCallback m_noRecvCallback;
};

} // namespace lorawan
} // namespace ns3

#endif /* RECV_WINDOW_MANAGER_H */
