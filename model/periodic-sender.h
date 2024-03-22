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
 */

#ifndef PERIODIC_SENDER_H
#define PERIODIC_SENDER_H

#include "lorawan-mac.h"

#include "ns3/application.h"
#include "ns3/attribute.h"
#include "ns3/nstime.h"

namespace ns3
{
namespace lorawan
{

/**
 * \ingroup lorawan
 *
 * Implements a sender application generating packets following a periodic point process.
 */
class PeriodicSender : public Application
{
  public:
    PeriodicSender();
    ~PeriodicSender() override;

    /**
     *  Register this type.
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * \brief Set the sending interval
     *
     * \param interval the interval between two packet send instances
     */
    void SetInterval(Time interval);

    /**
     * \brief Get the sending interval
     *
     * \returns the interval between two packet sends
     */
    Time GetInterval() const;

    /**
     * \brief Set the initial delay of this application
     *
     * \param delay The initial delay value
     */
    void SetInitialDelay(Time delay);

    /**
     * \brief Set packet size
     *
     * \param size The base packet size value in bytes
     */
    void SetPacketSize(uint8_t size);

    /**
     * \brief Set to add randomness to the base packet size.
     *
     * On each call to SendPacket(), an integer number is picked from a random variable. That
     * integer number is then added to the base packet size to create the new packet.
     *
     * \param rv The random variable used to extract the additional number of packet bytes.
     * Extracted values can be negative, but if they are lower than the base packet size they
     * produce a runtime error. This check is left to the caller during definition of the random
     * variable.
     */
    void SetPacketSizeRandomVariable(Ptr<RandomVariableStream> rv);

    /**
     * \brief Send a packet using the LoraNetDevice's Send method
     */
    void SendPacket();

    /**
     * \brief Start the application by scheduling the first SendPacket event
     */
    void StartApplication() override;

    /**
     * \brief Stop the application
     */
    void StopApplication() override;

  private:
    Time m_interval;       //!< The interval between to consecutive send events.
    Time m_initialDelay;   //!< The initial delay of this application.
    EventId m_sendEvent;   //!< The sending event scheduled as next.
    Ptr<LorawanMac> m_mac; //!< The MAC layer of this node.
    uint8_t m_basePktSize; //!< The packet size.
    Ptr<RandomVariableStream>
        m_pktSizeRV; //!< The random variable that adds bytes to the packet size.
};

} // namespace lorawan

} // namespace ns3
#endif /* SENDER_APPLICATION */
