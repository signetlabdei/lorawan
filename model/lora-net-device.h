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

#ifndef LORA_NET_DEVICE_H
#define LORA_NET_DEVICE_H

#include "lora-channel.h"
#include "lora-phy.h"
#include "lorawan-mac.h"

#include "ns3/net-device.h"

namespace ns3
{
namespace lorawan
{

class LoraChannel;
class LoraPhy;
class LorawanMac;

/**
 * Hold together all LoRa related objects.
 *
 * This class holds together pointers to LoraChannel, LoraPhy and LorawanMac,
 * exposing methods through which Application instances can send packets. The
 * Application only needs to craft its packets, the NetDevice will take care of
 * calling the LorawanMac's Send method with the appropriate parameters.
 */
class LoraNetDevice : public NetDevice
{
  public:
    static TypeId GetTypeId();

    // Constructor and destructor
    LoraNetDevice();
    ~LoraNetDevice() override;

    /**
     * Set which LorawanMac instance is linked to this device.
     *
     * \param mac the mac layer to use.
     */
    void SetMac(Ptr<LorawanMac> mac);

    /**
     * Set which LoraPhy instance is linked to this device.
     *
     * \param phy the phy layer to use.
     */
    void SetPhy(Ptr<LoraPhy> phy);

    /**
     * Get the LorawanMac instance that is linked to this NetDevice.
     *
     * \return the mac we are currently using.
     */
    Ptr<LorawanMac> GetMac() const;

    /**
     * Get the LoraPhy instance that is linked to this NetDevice.
     *
     * \return the phy we are currently using.
     */
    Ptr<LoraPhy> GetPhy() const;

    /**
     * Send a packet through the LoRaWAN stack.
     *
     * \param packet The packet to send.
     */
    void Send(Ptr<Packet> packet);

    /**
     * This function is implemented to achieve compliance with the NetDevice
     * interface. Note that the dest and protocolNumber args are ignored.
     */
    bool Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) override;

    /**
     * Callback the Mac layer calls whenever a packet arrives and needs to be
     * forwarded up the stack.
     *
     * \param packet The packet that was received.
     */
    void Receive(Ptr<Packet> packet);

    // From class NetDevice. Some of these have little meaning for a LoRaWAN
    // network device (since, for instance, IP is not used in the standard)
    void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
    Ptr<Channel> GetChannel() const override;
    void SetNode(Ptr<Node> node) override;
    Ptr<Node> GetNode() const override;

    void SetIfIndex(const uint32_t index) override;
    uint32_t GetIfIndex() const override;
    void SetAddress(Address address) override;
    Address GetAddress() const override;
    bool SetMtu(const uint16_t mtu) override;
    uint16_t GetMtu() const override;
    bool IsLinkUp() const override;
    void AddLinkChangeCallback(Callback<void> callback) override;
    bool IsBroadcast() const override;
    Address GetBroadcast() const override;
    bool IsMulticast() const override;
    Address GetMulticast(Ipv4Address multicastGroup) const override;
    Address GetMulticast(Ipv6Address addr) const override;
    bool IsBridge() const override;
    bool IsPointToPoint() const override;
    bool SendFrom(Ptr<Packet> packet,
                  const Address& source,
                  const Address& dest,
                  uint16_t protocolNumber) override;
    bool NeedsArp() const override;
    void SetPromiscReceiveCallback(PromiscReceiveCallback cb) override;
    bool SupportsSendFrom() const override;

  protected:
    /**
     * Receive a packet from the lower layer and pass the
     * packet up the stack.
     *
     * \param packet The packet we need to forward.
     * \param from The from address.
     * \param to The to address.
     */
    void ForwardUp(Ptr<Packet> packet, Mac48Address from, Mac48Address to);

  private:
    /**
     * Return the LoraChannel this device is connected to.
     */
    Ptr<LoraChannel> DoGetChannel() const;

    /**
     * Complete the configuration of this LoRa device by connecting all lower
     * components (PHY, MAC, Channel) together.
     */
    void CompleteConfig();

    // Member variables
    Ptr<Node> m_node;      //!< The Node this NetDevice is connected to.
    Ptr<LoraPhy> m_phy;    //!< The LoraPhy this NetDevice is connected to.
    Ptr<LorawanMac> m_mac; //!< The LorawanMac this NetDevice is connected to.
    bool m_configComplete; //!< Whether the configuration was already completed.

    /**
     * Upper layer callback used for notification of new data packet arrivals.
     */
    NetDevice::ReceiveCallback m_receiveCallback;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_NET_DEVICE_H */
