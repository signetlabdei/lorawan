/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef ONE_SHOT_SENDER_H
#define ONE_SHOT_SENDER_H

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
 * Packet sender application to send a single packet
 */
class OneShotSender : public Application
{
  public:
    OneShotSender();           //!< Default constructor
    ~OneShotSender() override; //!< Destructor

    /**
     * Construct a new OneShotSender object with provided send time.
     *
     * @param sendTime The Time of sending.
     */
    OneShotSender(Time sendTime);

    /**
     *  Register this type.
     *  @return The object TypeId.
     */
    static TypeId GetTypeId();

    /**
     * Send a packet using the LoraNetDevice's Send method.
     */
    void SendPacket();

    /**
     * Set the time at which this app will send a packet.
     *
     * @param sendTime The Time of sending.
     */
    void SetSendTime(Time sendTime);

    /**
     * Start the application by scheduling the first SendPacket event.
     */
    void StartApplication() override;

    /**
     * Stop the application.
     */
    void StopApplication() override;

  private:
    Time m_sendTime;       //!< The time at which to send the packet.
    EventId m_sendEvent;   //!< The sending event.
    Ptr<LorawanMac> m_mac; //!< The MAC layer of this node.
};

} // namespace lorawan

} // namespace ns3
#endif /* ONE_SHOT_APPLICATION */
