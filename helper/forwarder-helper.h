/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef FORWARDER_HELPER_H
#define FORWARDER_HELPER_H

#include "ns3/address.h"
#include "ns3/application-container.h"
#include "ns3/attribute.h"
#include "ns3/forwarder.h"
#include "ns3/net-device.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"

#include <stdint.h>
#include <string>

namespace ns3
{
namespace lorawan
{

/**
 * @ingroup lorawan
 *
 * This class can be used to install Forwarder applications on a set of gateways.
 */
class ForwarderHelper
{
  public:
    ForwarderHelper();  //!< Default constructor
    ~ForwarderHelper(); //!< Destructor

    /**
     * Helper function used to set the underlying application attributes.
     *
     * @param name The name of the application attribute to set.
     * @param value The value of the application attribute to set.
     */
    void SetAttribute(std::string name, const AttributeValue& value);

    /**
     * Install a Forwarder application on each node of the input container configured with
     * all the attributes set with SetAttribute or other functions of this class.
     *
     * @param c NodeContainer of the set of nodes on which an Forwarder will be installed.
     * @return Container of Ptr to the applications installed.
     */
    ApplicationContainer Install(NodeContainer c) const;

    /**
     * Install a Forwarder application on the input Node configured with all the attributes
     * set with SetAttribute or other functions of this class.
     *
     * @param node The node on which a Forwarder will be installed.
     * @return Container of the Ptr to the application installed.
     */
    ApplicationContainer Install(Ptr<Node> node) const;

  private:
    /**
     * Install a Forwarder application on the input Node configured with all the attributes
     * set with SetAttribute or other functions of this class.
     *
     * @param node The node on which a Forwarder will be installed.
     * @return A pointer to the applications installed.
     */
    Ptr<Application> InstallPriv(Ptr<Node> node) const;

    ObjectFactory m_factory; //!< The object factory
};

} // namespace lorawan

} // namespace ns3
#endif /* FORWARDER_HELPER_H */
