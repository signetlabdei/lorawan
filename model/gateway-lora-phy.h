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
 */

#ifndef GATEWAY_LORA_PHY_H
#define GATEWAY_LORA_PHY_H

#include "ns3/object.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/mobility-model.h"
#include "ns3/node.h"
#include "ns3/lora-phy.h"
#include "ns3/traced-value.h"
#include <list>

namespace ns3 {
namespace lorawan {

class LoraChannel;

/**
 * Class modeling a Lora SX1301 chip.
 *
 * This class models the behaviour of the chip employed in Lora gateways. These
 * chips are characterized by the presence of 8 receive paths, or parallel
 * receivers, which can be employed to listen to different channels
 * simultaneously. This characteristic of the chip is modeled using the
 * ReceivePath class, which describes a single parallel receiver. GatewayLoraPhy
 * essentially holds and manages a collection of these objects.
 */
class GatewayLoraPhy : public LoraPhy
{
public:
  static TypeId GetTypeId (void);

  GatewayLoraPhy ();
  virtual ~GatewayLoraPhy ();

  virtual void StartReceive (Ptr<Packet> packet, double rxPowerDbm, uint8_t sf, Time duration,
                             double frequencyMHz) = 0;

  virtual void EndReceive (Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event) = 0;

  virtual void Send (Ptr<Packet> packet, LoraTxParameters txParams, double frequencyMHz,
                     double txPowerDbm) = 0;

  virtual void TxFinished (Ptr<Packet> packet);

  bool IsTransmitting (void);

  virtual bool IsOnFrequency (double frequencyMHz);

  /**
   * Add a reception path, locked on a specific frequency.
   */
  void AddReceptionPath ();

  /**
   * Reset the list of reception paths.
   *
   * This method deletes all currently available ReceptionPath objects.
   */
  void ResetReceptionPaths (void);

  /**
   * Add a frequency to the list of frequencies we are listening to.
   */
  void AddFrequency (double frequencyMHz);

  /**
   * A vector containing the sensitivities required to correctly decode
   * different spreading factors.
   */
  static const double sensitivity[6];

protected:
  /**
   * This class represents a configurable reception path.
   *
   * Differently from EndDeviceLoraPhys, these do not need to be configured to
   * listen for a certain SF. ReceptionPaths be either locked on an event or
   * free.
   */
  class ReceptionPath : public SimpleRefCount<GatewayLoraPhy::ReceptionPath>
  {

  public:
    /**
     * Constructor.
     */
    ReceptionPath ();

    ~ReceptionPath ();

    /**
     * Query whether this reception path is available to lock on a signal.
     *
     * \return True if its current state is free, false if it's currently locked.
     */
    bool IsAvailable (void);

    /**
     * Set this reception path as available.
     *
     * This function sets the m_available variable as true, and deletes the
     * LoraInterferenceHelper Event this ReceivePath was previously locked on.
     */
    void Free (void);

    /**
     * Set this reception path as not available and lock it on the
     * provided event.
     *
     * \param event The LoraInterferenceHelper Event to lock on.
     */
    void LockOnEvent (Ptr<LoraInterferenceHelper::Event> event);

    /**
     * Set the event this reception path is currently on.
     *
     * \param event the event to lock this ReceptionPath on.
     */
    void SetEvent (Ptr<LoraInterferenceHelper::Event> event);

    /**
     * Get the event this reception path is currently on.
     *
     * \returns 0 if no event is currently being received, a pointer to
     * the event otherwise.
     */
    Ptr<LoraInterferenceHelper::Event> GetEvent (void);

    /**
     * Get the EventId of the EndReceive call associated to this ReceptionPath's
     * packet.
     */
    EventId GetEndReceive (void);

    /**
     * Set the EventId of the EndReceive call associated to this ReceptionPath's
     * packet.
     */
    void SetEndReceive (EventId endReceiveEventId);

  private:
    /**
     * Whether this reception path is available to lock on a signal or not.
     */
    bool m_available;

    /**
     * The event this reception path is currently locked on.
     */
    Ptr<LoraInterferenceHelper::Event> m_event;

    /**
     * The EventId associated of the call to EndReceive that is scheduled to
     * happen when the packet this ReceivePath is locked on finishes reception.
     */
    EventId m_endReceiveEventId;
  };

  /**
   * A list containing the various parallel receivers that are managed by this
   * Gateway.
   */
  std::list<Ptr<ReceptionPath>> m_receptionPaths;

  /**
   * The number of occupied reception paths.
   */
  TracedValue<int> m_occupiedReceptionPaths;

  /**
   * Trace source that is fired when a packet cannot be received because all
   * available ReceivePath instances are busy.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet>, uint32_t> m_noMoreDemodulators;

  /**
   * Trace source that is fired when a packet cannot be received because
   * the Gateway is in transmission state.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<const Packet>, uint32_t> m_noReceptionBecauseTransmitting;

  bool m_isTransmitting; //!< Flag indicating whether a transmission is going on

  std::list<double> m_frequencies;
};

} // namespace lorawan

} // namespace ns3
#endif /* GATEWAY_LORA_PHY_H */
