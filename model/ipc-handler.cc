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

#include "ipc-handler.h"

#define MAX_ACTION_SIZE 256

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("IpcHandler");

IpcHandler::IpcHandler()
    : m_closed(false)
{
    // Connect to socket
    NS_LOG_INFO("Opening connection.");
    m_zqmContext = zmq_ctx_new();
    m_zqmSocket = zmq_socket(m_zqmContext, ZMQ_PAIR);
    int rc = zmq_connect(m_zqmSocket, "ipc:///tmp/feeds/0.ipc");
    NS_ASSERT_MSG(rc == 0, "zmq_connect error: " << zmq_strerror(zmq_errno()));
}

IpcHandler::~IpcHandler()
{
    if (!m_closed)
    {
        NS_LOG_INFO("Closing connection.");
        zmq_close(m_zqmSocket);
        zmq_ctx_destroy(m_zqmContext);
    }
}

IpcHandler::action_t
IpcHandler::GetAction(state_t s, reward_t r, bool terminal)
{
    std::string msg = "{\'state\': " + s + ", \'reward\': " + r +
                      ", \'terminal\': " + (terminal ? "True" : "False") + "}";
    NS_LOG_INFO("Sending: " << msg);

    int rc = zmq_send(m_zqmSocket, msg.c_str(), msg.size(), 0);
    NS_ASSERT_MSG(rc != -1, "zmq_send error: " << zmq_strerror(zmq_errno()));
    NS_LOG_INFO(rc << " bytes sent");

    if (terminal)
    {
        NS_LOG_INFO("Reached terminal state, closing connection.");
        zmq_close(m_zqmSocket);
        zmq_ctx_destroy(m_zqmContext);
        m_closed = true;
        return std::string("0");
    }

    char buf[MAX_ACTION_SIZE];
    rc = zmq_recv(m_zqmSocket, buf, MAX_ACTION_SIZE, 0);
    NS_ASSERT_MSG(rc != -1, "zmq_recvmsg error: " << zmq_strerror(zmq_errno()));
    NS_LOG_INFO(rc << " bytes received");
    action_t a = std::string(buf);

    NS_LOG_INFO("Received: " << a);
    return a;
}

std::string
IpcHandler::FullPrecision(double n)
{
    std::stringstream stream;
    stream.precision(17);
    stream << n;
    std::string s = stream.str();
    return s;
}

} // namespace lorawan
} // namespace ns3