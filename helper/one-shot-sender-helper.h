/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef ONE_SHOT_SENDER_HELPER_H
#define ONE_SHOT_SENDER_HELPER_H

#include "ns3/address.h"
#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/one-shot-sender.h"

#include <stdint.h>
#include <string>

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * This class can be used to install OneShotSender applications on multiple nodes at once.
 */
class OneShotSenderHelper
{
  public:
    OneShotSenderHelper();  //!< Default constructor
    ~OneShotSenderHelper(); //!< Destructor

    /**
     * Helper function used to set the underlying application attributes.
     *
     * @param name The name of the application attribute to set.
     * @param value The value of the application attribute to set.
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * Install a OneShotSender application on each node of the input container
     * configured with all the attributes set with SetAttribute or other functions of this class.
     *
     * @param c NodeContainer of the set of nodes on which an OneShotSender will be installed.
     * @return Container of Ptr to the applications installed.
     */
    ApplicationContainer Install(NodeContainer c) const;

    /**
     * Install a OneShotSender application on the input Node configured with all the attributes set
     * with SetAttribute or other functions of this class.
     *
     * @param node The node on which a OneShotSender will be installed.
     * @return Container of the Ptr to the application installed.
     */
    ApplicationContainer Install(Ptr<Node> node) const;

    /**
     * Set the send time of the applications.
     *
     * @param sendTime The Time to set.
     *
     * @todo It does not make sense that all applications send at the exact same time.
     */
    void SetSendTime(Time sendTime);

  private:
    /**
     * Install a OneShotSender application on the input Node configured with all the attributes set
     * with SetAttribute or other functions of this class.
     *
     * @param node The node on which a OneShotSender will be installed.
     * @return A pointer to the applications installed.
     */
    Ptr<Application> InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory; //!< The object factory
    Time m_sendTime; //!< Time at which the OneShotSender applications will be configured to send
                     //!< the packet
};

} // namespace lorawan

} // namespace ns3
#endif /* ONE_SHOT_SENDER_HELPER_H */
