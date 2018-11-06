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
 * Authors: Davide Magrin <magrinda@dei.unipd.it>,
 *          Michele Luvisotto <michele.luvisotto@dei.unipd.it>
 *          Stefano Romagnolo <romagnolostefano93@gmail.com>
 */

#ifndef END_DEVICE_LORA_PHY_H
#define END_DEVICE_LORA_PHY_H

#include "ns3/object.h"
#include "ns3/traced-value.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/mobility-model.h"
#include "ns3/node.h"
#include "ns3/lora-phy.h"

namespace ns3 {
namespace lorawan {

class LoraChannel;

/**
 * Receive notifications about PHY events.
 */
class EndDeviceLoraPhyListener
{
public:
  virtual ~EndDeviceLoraPhyListener ();

  /**
   * We have received the first bit of a packet. We decided
   * that we could synchronize on this packet. It does not mean
   * we will be able to successfully receive completely the
   * whole packet. It means that we will report a BUSY status until
   * one of the following happens:
   *   - NotifyRxEndOk
   *   - NotifyRxEndError
   *   - NotifyTxStart
   *
   * \param duration the expected duration of the packet reception.
   */
  virtual void NotifyRxStart () = 0;

  /**
   * We are about to send the first bit of the packet.
   * We do not send any event to notify the end of
   * transmission. Listeners should assume that the
   * channel implicitely reverts to the idle state
   * unless they have received a cca busy report.
   *
   * \param duration the expected transmission duration.
   * \param txPowerDbm the nominal tx power in dBm
   */
  virtual void NotifyTxStart (double txPowerDbm) = 0;

  /**
   * Notify listeners that we went to sleep
   */
  virtual void NotifySleep (void) = 0;

  /**
   * Notify listeners that we woke up
   */
  virtual void NotifyStandby (void) = 0;
};

/**
 * Class representing a LoRa transceiver.
 *
 * This class inherits some functionality by LoraPhy, like the GetOnAirTime
 * function, and extends it to represent the behavior of a LoRa chip, like the
 * SX1272.
 *
 * Additional behaviors featured in this class include a State member variable
 * that expresses the current state of the device (SLEEP, TX, RX or STANDBY),
 * and a frequency and Spreading Factor this device is listening to when in
 * STANDBY mode. After transmission and reception, the device returns
 * automatically to STANDBY mode. The decision of when to go into SLEEP mode
 * is delegateed to an upper layer, which can modify the state of the device
 * through the public SwitchToSleep and SwitchToStandby methods. In SLEEP
 * mode, the device cannot lock on a packet and start reception.
 *
 * Peculiarities about the error model and about how errors are handled are
 * supposed to be handled by classes extending this one, like
 * SimpleEndDeviceLoraPhy or SpectrumEndDeviceLoraPhy. These classes need to
 */
class EndDeviceLoraPhy : public LoraPhy
{
public:
  /**
   * An enumeration of the possible states of an EndDeviceLoraPhy.
   * It makes sense to define a state for End Devices since there's only one
   * demodulator which can either send, receive, stay idle or go in a deep
   * sleep state.
   */
  enum State
  {
    /**
     * The PHY layer is sleeping.
     * During sleep, the device is not listening for incoming messages.
     */
    SLEEP,

    /**
     * The PHY layer is in STANDBY.
     * When the PHY is in this state, it's listening to the channel, and
     * it's also ready to transmit data passed to it by the MAC layer.
     */
    STANDBY,

    /**
     * The PHY layer is sending a packet.
     * During transmission, the device cannot receive any packet or send
     * any additional packet.
     */
    TX,

    /**
     * The PHY layer is receiving a packet.
     * While the device is locked on an incoming packet, transmission is
     * not possible.
     */
    RX
  };

  static TypeId GetTypeId (void);

  // Constructor and destructor
  EndDeviceLoraPhy ();
  virtual ~EndDeviceLoraPhy ();

  // Implementation of LoraPhy's pure virtual functions
  virtual void StartReceive (Ptr<Packet> packet, double rxPowerDbm,
                             uint8_t sf, Time duration, double frequencyMHz) = 0;

  // Implementation of LoraPhy's pure virtual functions
  virtual void EndReceive (Ptr<Packet> packet,
                           Ptr<LoraInterferenceHelper::Event> event) = 0;

  // Implementation of LoraPhy's pure virtual functions
  virtual void Send (Ptr<Packet> packet, LoraTxParameters txParams,
                     double frequencyMHz, double txPowerDbm) = 0;

  // Implementation of LoraPhy's pure virtual functions
  virtual bool IsOnFrequency (double frequencyMHz);

  // Implementation of LoraPhy's pure virtual functions
  virtual bool IsTransmitting (void);

  /**
   * Set the frequency this EndDevice will listen on.
   *
   * Should a packet be transmitted on a frequency different than that the
   * EndDeviceLoraPhy is listening on, the packet will be discarded.
   *
   * \param The frequency [MHz] to listen to.
   */
  void SetFrequency (double frequencyMHz);

  /**
   * Set the Spreading Factor this EndDevice will listen for.
   *
   * The EndDeviceLoraPhy object will not be able to lock on transmissions that
   * use a different SF than the one it's listening for.
   *
   * \param sf The spreading factor to listen for.
   */
  void SetSpreadingFactor (uint8_t sf);

  /**
   * Get the Spreading Factor this EndDevice is listening for.
   *
   * \return The Spreading Factor we are listening for.
   */
  uint8_t GetSpreadingFactor (void);

  /**
   * Return the state this End Device is currently in.
   *
   * \return The state this EndDeviceLoraPhy is currently in.
   */
  EndDeviceLoraPhy::State GetState (void);

  /**
   * Switch to the STANDBY state.
   */
  void SwitchToStandby (void);

  /**
   * Switch to the SLEEP state.
   */
  void SwitchToSleep (void);

  /**
   * Add the input listener to the list of objects to be notified of PHY-level
   * events.
   *
   * \param listener the new listener
   */
  void RegisterListener (EndDeviceLoraPhyListener *listener);

  /**
   * Remove the input listener from the list of objects to be notified of
   * PHY-level events.
   *
   * \param listener the listener to be unregistered
   */
  void UnregisterListener (EndDeviceLoraPhyListener *listener);

  static const double sensitivity[6]; //!< The sensitivity vector of this device to different SFs


protected:
  /**
   * Switch to the RX state
   */
  void SwitchToRx ();

  /**
   * Switch to the TX state
   */
  void SwitchToTx (double txPowerDbm);

  /**
   * Trace source for when a packet is lost because it was using a SF different from
   * the one this EndDeviceLoraPhy was configured to listen for.
   */
  TracedCallback<Ptr<const Packet>, uint32_t> m_wrongSf;

  /**
   * Trace source for when a packet is lost because it was transmitted on a
   * frequency different from the one this EndDeviceLoraPhy was configured to
   * listen on.
   */
  TracedCallback<Ptr<const Packet>, uint32_t> m_wrongFrequency;

  TracedValue<State> m_state; //!< The state this PHY is currently in.

  // static const double sensitivity[6]; //!< The sensitivity vector of this device to different SFs

  double m_frequency; //!< The frequency this device is listening on

  uint8_t m_sf; //!< The Spreading Factor this device is listening for

  /**
   * typedef for a list of EndDeviceLoraPhyListener
   */
  typedef std::vector<EndDeviceLoraPhyListener *> Listeners;
  /**
   * typedef for a list of EndDeviceLoraPhyListener iterator
   */
  typedef std::vector<EndDeviceLoraPhyListener *>::iterator ListenersI;

  Listeners m_listeners; //!< PHY listeners
};

} /* namespace ns3 */

}
#endif /* END_DEVICE_LORA_PHY_H */
