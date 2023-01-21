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
 * 23/12/2022
 * Modified by: Alessandro Aimi <alessandro.aimi@orange.com>
 *                              <alessandro.aimi@cnam.fr>
 */

#ifndef LORA_NET_DEVICE_H
#define LORA_NET_DEVICE_H

#include "ns3/lora-channel.h"
#include "ns3/lora-phy.h"
#include "ns3/lorawan-mac.h"
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
    static TypeId GetTypeId(void);

    // Constructor and destructor
    LoraNetDevice();
    virtual ~LoraNetDevice();

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
    Ptr<LorawanMac> GetMac(void) const;

    /**
     * Get the LoraPhy instance that is linked to this NetDevice.
     *
     * \return the phy we are currently using.
     */
    Ptr<LoraPhy> GetPhy(void) const;

    // From class NetDevice.
    virtual void SetNode(Ptr<Node> node);
    virtual Ptr<Node> GetNode(void) const;

  protected:
    void DoInitialize() override;

  private:
    /**
     * Return the LoraChannel this device is connected to.
     * Used by attribute system
     */
    Ptr<LoraChannel> DoGetChannel(void) const;

    /**
     * Complete the configuration of this LoRa device by connecting all lower
     * components (PHY, MAC, Channel) together.
     */
    void CompleteConfig(void);

    // Member variables
    Ptr<Node> m_node;      //!< The Node this NetDevice is connected to.
    Ptr<LoraPhy> m_phy;    //!< The LoraPhy this NetDevice is connected to.
    Ptr<LorawanMac> m_mac; //!< The LorawanMac this NetDevice is connected to.
    bool m_configComplete; //!< Whether the configuration was already completed.

    /**
     * Upper layer callback used for notification of new data packet arrivals.
     */
    NetDevice::ReceiveCallback m_receiveCallback;

    /**
     * Inherited by NetDevice. Some of these have little meaning for a LoRaWAN
     * network device (since, for instance, IP is not used in the standard)
     *
     * Set to private to avoid exposing them.
     */
    virtual Ptr<Channel> GetChannel(void) const;
    virtual void SetIfIndex(const uint32_t index);
    virtual uint32_t GetIfIndex(void) const;
    virtual void SetAddress(Address address);
    virtual Address GetAddress(void) const;
    virtual bool SetMtu(const uint16_t mtu);
    virtual uint16_t GetMtu(void) const;
    virtual bool IsLinkUp(void) const;
    virtual void AddLinkChangeCallback(Callback<void> callback);
    virtual bool IsBroadcast(void) const;
    virtual Address GetBroadcast(void) const;
    virtual bool IsMulticast(void) const;
    virtual Address GetMulticast(Ipv4Address multicastGroup) const;
    virtual Address GetMulticast(Ipv6Address addr) const;
    virtual bool IsBridge(void) const;
    virtual bool IsPointToPoint(void) const;
    virtual bool Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
    virtual bool SendFrom(Ptr<Packet> packet,
                          const Address& source,
                          const Address& dest,
                          uint16_t protocolNumber);
    virtual bool NeedsArp(void) const;
    virtual void SetReceiveCallback(NetDevice::ReceiveCallback cb);
    virtual void SetPromiscReceiveCallback(PromiscReceiveCallback cb);
    virtual bool SupportsSendFrom(void) const;
};

} // namespace lorawan

} // namespace ns3
#endif /* LORA_NET_DEVICE_H */
