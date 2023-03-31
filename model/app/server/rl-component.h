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

#ifndef RL_COMPONENT_H
#define RL_COMPONENT_H

#include "ns3/ipc-handler.h"
#include "ns3/network-controller-components.h"

namespace ns3
{
namespace lorawan
{

/**
 * Component for channel allocation with an external
 * RL algorithm.
 */

class RlComponent : public NetworkControllerComponent
{
    /* Cluster PDR targets */
    using targets_t = std::vector<double>;

    /* Cluster membership of each device */
    using clusterMap_t = std::unordered_map<uint32_t, uint8_t>;

    /* State */
    struct state_t
    {
        std::vector<double> espVec;
        uint8_t cluster = 0;
        std::string serialize(void);
    };

    /* Reward */
    class reward_t // 1 instance in the class
    {
      public:
        void update(uint32_t dev, double mpe);
        std::string serialize(void);

      private:
        std::unordered_map<uint32_t, double> mpeMap;
        double value = 0;
    };

    /* Action */
    using action_t = uint8_t;

  public:
    static TypeId GetTypeId(void);

    RlComponent();
    virtual ~RlComponent();

    void OnReceivedPacket(Ptr<const Packet> packet,
                          Ptr<EndDeviceStatus> status,
                          Ptr<NetworkStatus> networkStatus);

    void BeforeSendingReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus);

    void SetTargets(targets_t targets);

  private:
    LoraFrameHeader GetFHeader(Ptr<const Packet> packet);

    IpcHandler ipc; // Inter process communication

    reward_t m_r;    // track reward
    bool m_terminal; // track terminal state

    clusterMap_t m_clusterMap; // cluster memberships
    targets_t m_targets;       // PDR targets
    Time m_start;              // start comms after
    Time m_end;                // stop comms after

    // map index to each gateway (auxiliary structure)
    std::map<Address, uint8_t> m_gwIdMap;

    // unused
    void OnFailedReply(Ptr<EndDeviceStatus> status, Ptr<NetworkStatus> networkStatus);
};

} // namespace lorawan
} // namespace ns3

#endif
