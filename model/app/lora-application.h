/*
 * Copyright (c) 2022 Orange SA
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
 */

#ifndef LORA_APPLICATION_H
#define LORA_APPLICATION_H

#include "ns3/application.h"
#include "ns3/base-end-device-lorawan-mac.h"

namespace ns3
{
namespace lorawan
{

class LoraApplication : public Application
{
  public:
    /**
     * \brief Get the type ID.
     * \return the object TypeId
     */
    static TypeId GetTypeId();
    LoraApplication();
    ~LoraApplication() override;

    /**
     * Set the sending interval
     * \param interval the interval between two packet sendings
     */
    void SetInterval(Time interval);

    /**
     * Get the sending inteval
     * \returns the interval between two packet sends
     */
    Time GetInterval() const;

    /**
     * Set the initial delay of this application
     */
    void SetInitialDelay(Time delay);

    /**
     * Set packet size
     */
    void SetPacketSize(uint8_t size);

    /**
     * Get packet size
     */
    uint8_t GetPacketSize() const;

    /**
     * True if the application is currently running
     */
    bool IsRunning();

  protected:
    void DoInitialize() override;
    void DoDispose() override;

    /**
     * Start the application by scheduling the first SendPacket event
     */
    void StartApplication() override;

    /**
     * Stop the application
     */
    void StopApplication() override;

    /**
     * Send a packet using the LoraNetDevice's Send method
     */
    virtual void SendPacket();

    /**
     * The average interval between to consecutive send events
     */
    Time m_avgInterval;

    /**
     * The initial delay of this application
     */
    Time m_initialDelay;

    /**
     * The sending event scheduled as next
     */
    EventId m_sendEvent;

    /**
     * The packet size.
     */
    uint8_t m_basePktSize;

    /**
     * The MAC layer of this node
     */
    Ptr<BaseEndDeviceLorawanMac> m_mac;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_APPLICATION_H */
