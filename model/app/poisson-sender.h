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

#ifndef POISSON_SENDER_H
#define POISSON_SENDER_H

#include "ns3/lora-application.h"

namespace ns3
{
namespace lorawan
{

class PoissonSender : public LoraApplication
{
  public:
    PoissonSender();
    ~PoissonSender() override;

    static TypeId GetTypeId();

  protected:
    void DoInitialize() override;
    void DoDispose() override;

  private:
    /**
     * Start the application by scheduling the first SendPacket event
     */
    void StartApplication() override;

    /**
     * Send a packet using the LoraNetDevice's Send method
     */
    void SendPacket() override;

    Ptr<ExponentialRandomVariable> m_interval; //!< Random variable modeling packet inter-send time
};

} // namespace lorawan

} // namespace ns3
#endif /* POISSON_SENDER_H */
