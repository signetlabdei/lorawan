/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
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
 * @ingroup lorawan
 *
 * Implements a sender application generating packets following a periodic point process.
 */
class PeriodicSender : public Application
{
  public:
    PeriodicSender();           //!< Default constructor
    ~PeriodicSender() override; //!< Destructor

    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Set the sending interval.
     *
     * @param interval The interval between two packet send instances.
     */
    void SetInterval(Time interval);

    /**
     * Get the sending interval.
     *
     * @return The interval between two packet sends.
     */
    Time GetInterval() const;

    /**
     * Set the initial delay of this application.
     *
     * @param delay The initial delay value.
     */
    void SetInitialDelay(Time delay);

    /**
     * Set packet size.
     *
     * @param size The base packet size value in bytes.
     */
    void SetPacketSize(uint8_t size);

    /**
     * Set to add randomness to the base packet size.
     *
     * On each call to SendPacket(), an integer number is picked from a random variable. That
     * integer number is then added to the base packet size to create the new packet.
     *
     * @param rv The random variable used to extract the additional number of packet bytes.
     * Extracted values can be negative, but if they are lower than the base packet size they
     * produce a runtime error. This check is left to the caller during definition of the random
     * variable.
     */
    void SetPacketSizeRandomVariable(Ptr<RandomVariableStream> rv);

    /**
     * Send a packet using the LoraNetDevice's Send method.
     */
    void SendPacket();

    /**
     * Start the application by scheduling the first SendPacket event.
     */
    void StartApplication() override;

    /**
     * Stop the application.
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
