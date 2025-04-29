/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef PERIODIC_SENDER_HELPER_H
#define PERIODIC_SENDER_HELPER_H

#include "ns3/address.h"
#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/periodic-sender.h"

#include <stdint.h>
#include <string>

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * This class can be used to install PeriodicSender applications on a wide
 * range of nodes.
 */
class PeriodicSenderHelper
{
  public:
    PeriodicSenderHelper();  //!< Default constructor
    ~PeriodicSenderHelper(); //!< Destructor

    /**
     * Helper function used to set the underlying application attributes.
     *
     * @param name The name of the application attribute to set.
     * @param value The value of the application attribute to set.
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * Install a PeriodicSender application on each node of the input container
     * configured with all the attributes set with SetAttribute or other functions of this class.
     *
     * @param c NodeContainer of the set of nodes on which an PeriodicSender
     * will be installed.
     * @return Container of Ptr to the applications installed.
     */
    ApplicationContainer Install(NodeContainer c) const;

    /**
     * Install a PeriodicSender application on the input Node configured with all the attributes set
     * with SetAttribute or other functions of this class.
     *
     * @param node The node on which a PeriodicSender will be installed.
     * @return Container of the Ptr to the application installed.
     */
    ApplicationContainer Install(Ptr<Node> node) const;

    /**
     * Set the period to be used by the applications created by this helper.
     *
     * A value of Seconds (0) results in randomly generated periods according to
     * the model contained in the TR 45.820 document.
     *
     * @param period The period to set.
     */
    void SetPeriod(Time period);

    /**
     * Set a random variable to enable a random size to be added to the base packet size for
     * each new transmission of PacketSender applications.
     *
     * @param rv The random variable.
     */
    void SetPacketSizeRandomVariable(Ptr<RandomVariableStream> rv);

    /**
     * Set the base value for applications packet size in bytes.
     *
     * @param size The packet size in bytes.
     */
    void SetPacketSize(uint8_t size);

  private:
    /**
     * Install a PeriodicSender application on the input Node configured with all the attributes set
     * with SetAttribute or other functions of this class.
     *
     * @param node The node on which a PeriodicSender will be installed.
     * @return A pointer to the application installed.
     */
    Ptr<Application> InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory; //!< The factory to create PeriodicSender applications
    Ptr<UniformRandomVariable> m_initialDelay; //!< The random variable used to extract a start
                                               //!< off delay for each PeriodicSender application
    Ptr<UniformRandomVariable>
        m_intervalProb; //!< The random variable used to pick inter-transmission intervals of
                        //!< different applications from a discrete probability distribution
    Time m_period;      //!< The base period with which the application will be set to send messages
    Ptr<RandomVariableStream>
        m_pktSizeRV;   //!< Whether or not a random component is added to the packet size.
    uint8_t m_pktSize; //!< The base packet size.
};

} // namespace lorawan

} // namespace ns3
#endif /* PERIODIC_SENDER_HELPER_H */
