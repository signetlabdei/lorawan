/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#ifndef IPC_HANDLER_H
#define IPC_HANDLER_H

#include "ns3/object.h"

#include <zmq.h>

namespace ns3
{
namespace lorawan
{

/**
 * Enables inter process communication
 * using the ZMQ protocol. Structured
 * to exchange state, reward, action
 * info for reinforcement learning.
 */

class IpcHandler
{
    using state_t = std::string;
    using reward_t = std::string;
    using action_t = std::string;

  public:
    IpcHandler();
    virtual ~IpcHandler();

    action_t GetAction(state_t s, reward_t r, bool terminal);

    static std::string FullPrecision(double n);

  private:
    void* m_zqmContext; // zmq context
    void* m_zqmSocket;  // zmq socket

    bool m_closed; // track connection state
};

} // namespace lorawan
} // namespace ns3

#endif
