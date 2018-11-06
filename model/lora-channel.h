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
 *
 * The structure of this class is inspired by the YansWifiChannel
 * contained in the WiFi module.
 */

#ifndef LORA_CHANNEL_H
#define LORA_CHANNEL_H

#include <vector>
#include "ns3/lora-phy.h"
#include "ns3/mobility-model.h"
#include "ns3/channel.h"
#include "ns3/net-device.h"
#include "ns3/propagation-loss-model.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/logical-lora-channel.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"

namespace ns3 {
class NetDevice;
class PropagationLossModel;
class PropagationDelayModel;
namespace lorawan {

class LoraPhy;
struct LoraTxParameters;

/**
 * A struct that holds meaningful parameters for transmission on a
 * LoraChannel.
 */
struct LoraChannelParameters
{
  double rxPowerDbm;     //!< The reception power.
  uint8_t sf;     //!< The Spreading Factor of this transmission.
  Time duration;     //!< The duration of the transmission.
  double frequencyMHz;     //!< The frequency [MHz] of this transmission.
};

/**
 * Allow logging of LoraChannelParameters like with any other data type.
 */
std::ostream &operator << (std::ostream &os, const LoraChannelParameters &params);

/**
 * The class that delivers packets among PHY layers.
 *
 * This class is tasked with taking packets that PHY layers want to send and,
 * based on some factors like the transmission power and the node positions,
 * computing the power at every receiver using a PropagationLossModel and
 * notifying them of the reception event after a delay based on some
 * PropagationDelayModel.
 */
class LoraChannel : public Channel
{
public:
  // TypeId
  static TypeId GetTypeId (void);

  // Constructor and destructor
  LoraChannel ();
  virtual ~LoraChannel ();

  // Inherited from Channel.
  virtual std::size_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (std::size_t i) const;

  /**
    * Construct a LoraChannel with a loss and delay model.
    *
    * \param loss The loss model to associate to this channel.
    * \param delay The delay model to associate to this channel.
    */
  LoraChannel (Ptr<PropagationLossModel> loss,
               Ptr<PropagationDelayModel> delay);

  /**
    * Connect a LoraPhy object to the LoraChannel.
    *
    * This method is needed so that the channel knows it has to notify this PHY
    * of incoming transmissions.
    *
    * \param phy The physical layer to add.
    */
  void Add (Ptr<LoraPhy> phy);

  /**
    * Remove a physical layer from the LoraChannel.
    *
    * This method removes a phy from the list of devices we have to notify.
    * Removing unused PHY layers from the channel can improve performance, since
    * it is not necessary to notify them about each transmission.
    *
    * \param phy The physical layer to remove.
    */
  void Remove (Ptr<LoraPhy> phy);

  /**
    * Send a packet in the channel.
    *
    * This method is typically invoked by a PHY that needs to send a packet.
    * Every connected Phy will be notified of this packet send through a call to
    * their StartReceive methods after a delay based on the channel's
    * PropagationDelayModel.
    *
    * \param sender The phy that is sending this packet.
    * \param packet The PHY layer packet that is being sent over the channel.
    * \param txPowerDbm The power of the transmission.
    * \param txParams The set of parameters that are used by the transmitter.
    * \param duration The on-air duration of this packet.
    * \param frequencyMHz The frequency this transmission will happen at.
    *
    * \internal
    *
    * When this method is called, the channel schedules an internal Receive call
    * that performs the actual call to the PHY's StartReceive function.
    */
  void Send (Ptr<LoraPhy> sender, Ptr<Packet> packet, double txPowerDbm,
             LoraTxParameters txParams, Time duration, double frequencyMHz)
  const;

  /**
    * Compute the received power when transmitting from a point to another one.
    *
    * This method can be used by external object to see the receive power of a
    * transmission from one point to another using this Channel's
    * PropagationLossModel.
    *
    * \param txPowerDbm The power the transmitter is using, in dBm.
    * \param senderMobility The mobility model of the sender.
    * \param receiverMobility The mobility model of the receiver.
    * \return The received power in dBm.
    */
  double GetRxPower (double txPowerDbm, Ptr<MobilityModel> senderMobility,
                     Ptr<MobilityModel> receiverMobility) const;

private:
  /**
    * Private method that is scheduled by LoraChannel's Send method to happen
    * after the channel delay, for each of the connected PHY layers.
    *
    * It's here that the Receive method of the PHY is called to initiate packet
    * reception at the PHY.
    *
    * \param i The index of the phy to start reception on.
    * \param packet The packet the phy will receive.
    * \param parameters The parameters that characterize this transmission
    */
  void Receive (uint32_t i, Ptr<Packet> packet,
                LoraChannelParameters parameters) const;

  /**
    * The vector containing the PHYs that are currently connected to the
    * channel.
    */
  std::vector<Ptr<LoraPhy> > m_phyList;

  /**
    * Pointer to the loss model.
    *
    * This loss model can be a concatenation of multiple loss models, obtained
    * via PropagationLossModel's SetNext method.
    */
  Ptr<PropagationLossModel> m_loss;

  /**
    * Pointer to the delay model.
    */
  Ptr<PropagationDelayModel> m_delay;

  /**
   * Callback for when a packet is being sent on the channel.
   */
  TracedCallback<Ptr<const Packet> > m_packetSent;

};

} /* namespace ns3 */

}
#endif /* LORA_CHANNEL_H */
