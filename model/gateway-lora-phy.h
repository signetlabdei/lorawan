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

#include "lora-phy.h"

#include "ns3/mobility-model.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/traced-value.h"

#include <list>

namespace ns3
{
namespace lorawan
{

class LoraChannel;

/**
 * \ingroup lorawan
 *
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
    /**
     *  Register this type.
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    GatewayLoraPhy();           //!< Default constructor
    ~GatewayLoraPhy() override; //!< Destructor

    void StartReceive(Ptr<Packet> packet,
                      double rxPowerDbm,
                      uint8_t sf,
                      Time duration,
                      double frequencyMHz) override = 0;

    void EndReceive(Ptr<Packet> packet, Ptr<LoraInterferenceHelper::Event> event) override = 0;

    void Send(Ptr<Packet> packet,
              LoraTxParameters txParams,
              double frequencyMHz,
              double txPowerDbm) override = 0;

    bool IsTransmitting() override;

    /**
     * Check whether the GatewayLoraPhy is currently listening to the specified frequency.
     *
     * \param frequencyMHz The value of the frequency [MHz].
     * \return True if the frequency is among the one being listened to, false otherwise.
     */
    bool IsOnFrequency(double frequencyMHz) override;

    /**
     * Add a reception path, locked on a specific frequency.
     */
    void AddReceptionPath();

    /**
     * Reset the list of reception paths.
     *
     * This method deletes all currently available ReceptionPath objects.
     */
    void ResetReceptionPaths();

    /**
     * Add a frequency to the list of frequencies we are listening to.
     *
     * \param frequencyMHz The value of the frequency [MHz].
     */
    void AddFrequency(double frequencyMHz);

    static const double sensitivity[6]; //!< A vector containing the sensitivities required to
                                        //!< correctly decode different spreading factors.

  protected:
    /**
     * Signals the end of a transmission by the GatewayLoraPhy.
     *
     * \param packet A pointer to the Packet transmitted.
     */
    void TxFinished(Ptr<const Packet> packet) override;

    /**
     * This class represents a configurable reception path.
     *
     * Differently from EndDeviceLoraPhys, these do not need to be configured to
     * listen for a certain spreading factor. ReceptionPaths be either locked on an event or
     * free.
     */
    class ReceptionPath : public SimpleRefCount<GatewayLoraPhy::ReceptionPath>
    {
      public:
        /**
         * Constructor.
         */
        ReceptionPath();
        ~ReceptionPath(); //!< Destructor

        /**
         * Query whether this reception path is available to lock on a signal.
         *
         * \return True if its current state is free, false if it's currently locked.
         */
        bool IsAvailable() const;

        /**
         * Set this reception path as available.
         *
         * This function sets the m_available variable as true, and deletes the
         * LoraInterferenceHelper Event this ReceivePath was previously locked on.
         */
        void Free();

        /**
         * Set this reception path as not available and lock it on the
         * provided event.
         *
         * \param event The LoraInterferenceHelper Event to lock on.
         */
        void LockOnEvent(Ptr<LoraInterferenceHelper::Event> event);

        /**
         * Set the event this reception path is currently on.
         *
         * \param event The event to lock this ReceptionPath on.
         */
        void SetEvent(Ptr<LoraInterferenceHelper::Event> event);

        /**
         * Get the event this reception path is currently on.
         *
         * \return 0 if no event is currently being received, a pointer to
         * the event otherwise.
         */
        Ptr<LoraInterferenceHelper::Event> GetEvent();

        /**
         * Get the EventId of the EndReceive call associated to this ReceptionPath's
         * packet.
         *
         * \return The EventId instance.
         */
        EventId GetEndReceive();

        /**
         * Set the EventId of the EndReceive call associated to this ReceptionPath's
         * packet.
         *
         * \param endReceiveEventId The EventId instance.
         */
        void SetEndReceive(EventId endReceiveEventId);

      private:
        bool m_available; //!< Whether this reception path is available to lock on a signal or not.
        Ptr<LoraInterferenceHelper::Event>
            m_event;                 //!< The event this reception path is currently locked on.
        EventId m_endReceiveEventId; //!< The EventId associated of the call to EndReceive that is
                                     //!< scheduled to happen when the packet this ReceivePath is
                                     //!< locked on finishes reception.
    };

    std::list<Ptr<ReceptionPath>> m_receptionPaths; //!< A list containing the various parallel
                                                    //!< receivers that are managed by this gateway.

    TracedValue<int> m_occupiedReceptionPaths; //!< The number of occupied reception paths.

    /**
     * Trace source fired when a packet cannot be received because all available ReceivePath
     * instances are busy.
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_noMoreDemodulators;

    /**
     * Trace source fired when a packet cannot be received because the gateway is in transmission
     * state.
     */
    TracedCallback<Ptr<const Packet>, uint32_t> m_noReceptionBecauseTransmitting;

    bool m_isTransmitting; //!< Flag indicating whether a transmission is going on

    std::list<double> m_frequencies; //!< List of frequencies the GatewayLoraPhy is listening to.
};

} // namespace lorawan

} // namespace ns3
#endif /* GATEWAY_LORA_PHY_H */
