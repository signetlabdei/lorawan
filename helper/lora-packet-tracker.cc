#include "lora-packet-tracker.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/lora-mac-header.h"
#include <iostream>
#include <fstream>

namespace ns3
{
  NS_LOG_COMPONENT_DEFINE ("LoraPacketTracker");

  LoraPacketTracker::LoraPacketTracker (std::string filename) :
    m_outputFilename (filename)
  {
    NS_LOG_FUNCTION (this);

    // Empty file
    std::ofstream myfile;
    myfile.open(m_outputFilename, std::ofstream::out | std::ofstream::trunc);
    myfile.close();
  }

  LoraPacketTracker::~LoraPacketTracker ()
  {
    NS_LOG_FUNCTION (this);
  }

  /////////////////
  // MAC metrics //
  /////////////////

  void
  LoraPacketTracker::MacTransmissionCallback (Ptr<Packet const> packet)
  {
    NS_LOG_INFO ("A new packet was sent by the MAC layer");

    MacPacketStatus status;
    status.sendTime = Simulator::Now ();
    status.receivedTime = Time::Max ();
    status.systemId = Simulator::GetContext ();

    m_macPacketTracker.insert (std::pair<Ptr<Packet const>, MacPacketStatus> (packet, status));
  }

  void
  LoraPacketTracker::RequiredTransmissionsCallback (uint8_t reqTx, bool success,
                                                    Time firstAttempt,
                                                    Ptr<Packet> packet)
  {
    NS_LOG_INFO ("Finished retransmission attempts for a packet");
    NS_LOG_DEBUG ("ReqTx " << unsigned(reqTx) << ", succ: " << success <<
                  ", firstAttempt: " << firstAttempt.GetSeconds ());

    RetransmissionStatus entry;
    entry.firstAttempt = firstAttempt;
    entry.finishTime = Simulator::Now ();
    entry.reTxAttempts = reqTx;
    entry.successful = success;

    m_reTransmissionTracker.insert (std::pair<Ptr<Packet>, RetransmissionStatus>
                                    (packet, entry));
  }

  void
  LoraPacketTracker::MacGwReceptionCallback (Ptr<Packet const> packet)
  {
    NS_LOG_INFO ("A packet was successfully received at MAC layer of a gateway");

    // Find the received packet in the m_macPacketTracker
    auto it = m_macPacketTracker.find(packet);
    if (it != m_macPacketTracker.end())
      {
        (*it).second.receivedTime = Simulator::Now ();

        // NS_LOG_INFO ("Delay for device " << (*it).second.systemId << ": " <<
        //                            ((*it).second.receivedTime -
        //                            (*it).second.sendTime).GetSeconds ());
      }
    else
      {
        NS_ABORT_MSG ("Packet not found in tracker");
      }
  }

  /////////////////
  // PHY metrics //
  /////////////////

  void
  LoraPacketTracker::TransmissionCallback (Ptr<Packet const> packet, uint32_t systemId)
  {
    NS_LOG_INFO ("Transmitted a packet from device " << systemId);

    // Create a packetStatus
    PacketStatus status;
    status.packet = packet;
    status.senderId = systemId;
    status.outcomeNumber = 0;
    status.outcomes = std::vector<enum PacketOutcome> (1, UNSET);

    m_packetTracker.insert (std::pair<Ptr<Packet const>, PacketStatus> (packet, status));
  }

  void
  LoraPacketTracker::PacketReceptionCallback (Ptr<Packet const> packet, uint32_t systemId)
  {
    // Remove the successfully received packet from the list of sent ones
    NS_LOG_INFO ("A packet was successfully received at gateway " << systemId);

    std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
    (*it).second.outcomes.at (0) = RECEIVED;
    (*it).second.outcomeNumber += 1;

    m_phyPacketOutcomes.push_back (std::pair<Time, PacketOutcome> (Simulator::Now (), RECEIVED));
  }

  void
  LoraPacketTracker::InterferenceCallback (Ptr<Packet const> packet, uint32_t systemId)
  {
    NS_LOG_INFO ("A packet was lost because of interference at gateway " << systemId);

    std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
    (*it).second.outcomes.at (0) = INTERFERED;
    (*it).second.outcomeNumber += 1;

    m_phyPacketOutcomes.push_back (std::pair<Time, PacketOutcome> (Simulator::Now (), INTERFERED));
  }

  void
  LoraPacketTracker::NoMoreReceiversCallback (Ptr<Packet const> packet, uint32_t systemId)
  {
    NS_LOG_INFO ("A packet was lost because there were no more receivers at gateway " << systemId);
    std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
    (*it).second.outcomes.at (0) = NO_MORE_RECEIVERS;
    (*it).second.outcomeNumber += 1;

    m_phyPacketOutcomes.push_back (std::pair<Time, PacketOutcome> (Simulator::Now (), NO_MORE_RECEIVERS));
  }

  void
  LoraPacketTracker::UnderSensitivityCallback (Ptr<Packet const> packet, uint32_t systemId)
  {
    NS_LOG_INFO ("A packet arrived at the gateway under sensitivity at gateway " << systemId);

    std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
    (*it).second.outcomes.at (0) = UNDER_SENSITIVITY;
    (*it).second.outcomeNumber += 1;

    m_phyPacketOutcomes.push_back (std::pair<Time, PacketOutcome> (Simulator::Now (), UNDER_SENSITIVITY));
  }

  void
  LoraPacketTracker::LostBecauseTxCallback (Ptr<Packet const> packet, uint32_t systemId)
  {
    NS_LOG_INFO ("A packet arrived at the gateway under sensitivity at gateway " << systemId);

    std::map<Ptr<Packet const>, PacketStatus>::iterator it = m_packetTracker.find (packet);
    (*it).second.outcomes.at (0) = LOST_BECAUSE_TX;
    (*it).second.outcomeNumber += 1;

    m_phyPacketOutcomes.push_back (std::pair<Time, PacketOutcome> (Simulator::Now (), LOST_BECAUSE_TX));
  }

  void
  LoraPacketTracker::PrintPerformance (Time start, Time stop)
  {
    NS_LOG_FUNCTION (this);

    CountRetransmissions (start, stop, m_macPacketTracker,
                          m_reTransmissionTracker, m_packetTracker);
  }

  void
  LoraPacketTracker::CountPhyPackets (Time start, Time stop)
  {
    NS_LOG_FUNCTION (this);

    DoCountPhyPackets (start, stop, m_packetTracker);
  }

  void
  LoraPacketTracker::PrintVector (std::vector<int> vector)
  {
    // NS_LOG_INFO ("PrintRetransmissions");

    for (int i = 0; i < int(vector.size ()); i++)
      {
        // NS_LOG_INFO ("i: " << i);
        std::cout << vector.at (i) << " ";
      }
    //
    // std::cout << std::endl;
  }


  void
  LoraPacketTracker::PrintSumRetransmissions (std::vector<int> reTxVector)
  {
    // NS_LOG_INFO ("PrintSumRetransmissions");

    int total = 0;
    for (int i = 0; i < int(reTxVector.size ()); i++)
      {
        // NS_LOG_INFO ("i: " << i);
        total += reTxVector[i] * (i + 1);
      }
    std::cout << total;
  }

  void
  LoraPacketTracker::CountRetransmissions (Time transient, Time simulationTime, MacPacketData
                                           macPacketTracker, RetransmissionData reTransmissionTracker,
                                           PhyPacketData packetTracker)
  {
    std::vector<int> totalReTxAmounts (8, 0);
    std::vector<int> successfulReTxAmounts (8, 0);
    std::vector<int> failedReTxAmounts (8, 0);
    Time delaySum = Seconds (0);
    Time ackDelaySum = Seconds(0);

    int packetsOutsideTransient = 0;
    int MACpacketsOutsideTransient = 0;

    for (auto itMac = macPacketTracker.begin (); itMac != macPacketTracker.end(); ++itMac)
      {
        // NS_LOG_DEBUG ("Dealing with packet " << (*itMac).first);

        if ((*itMac).second.sendTime > transient && (*itMac).second.sendTime < simulationTime - transient)
          {
            // Count retransmissions
            ////////////////////////
            auto itRetx = reTransmissionTracker.find ((*itMac).first);

            if (itRetx == reTransmissionTracker.end())
              {
                // This means that the device did not finish retransmitting
                NS_ABORT_MSG ("Searched packet was not found" << "Packet " <<
                              (*itMac).first << " not found. Sent at " <<
                              (*itMac).second.sendTime.GetSeconds());
              }

            packetsOutsideTransient++;
            MACpacketsOutsideTransient++;

            totalReTxAmounts.at ((*itRetx).second.reTxAttempts - 1)++;

            if ((*itRetx).second.successful)
              {
                successfulReTxAmounts.at ((*itRetx).second.reTxAttempts - 1)++;
              }
            else
              {
                failedReTxAmounts.at ((*itRetx).second.reTxAttempts - 1)++;
              }

            // Compute delays
            /////////////////
            if ((*itMac).second.receivedTime == Time::Max())
              {
                // NS_LOG_DEBUG ("Packet never received, ignoring it");
                packetsOutsideTransient--;
              }
            else
              {
                delaySum += (*itMac).second.receivedTime - (*itMac).second.sendTime;
                ackDelaySum += (*itRetx).second.finishTime - (*itRetx).second.firstAttempt;
              }

          }
      }

    // Sum PHY outcomes
    //////////////////////////////////
    // vector performanceAmounts will contain - for the interval given in the
    // input of the function, the following fields:
    // totPacketsSent receivedPackets interferedPackets noMoreGwPackets underSensitivityPackets
    std::vector<int> performancesAmounts (6, 0);
    for (auto itPhy = m_phyPacketOutcomes.begin(); itPhy != m_phyPacketOutcomes.end(); ++itPhy)
      {
        if ((*itPhy).first >= transient && (*itPhy).first <= simulationTime - transient)
          {
            performancesAmounts.at(0)++;

            switch ((*itPhy).second)
              {
              case RECEIVED:
                {
                  performancesAmounts.at(1)++;
                  break;
                }
              case INTERFERED:
                {
                  performancesAmounts.at(2)++;
                  break;
                }
              case NO_MORE_RECEIVERS:
                {
                  performancesAmounts.at(3)++;
                  break;
                }
              case UNDER_SENSITIVITY:
                {
                  performancesAmounts.at(4)++;
                  break;
                }
              case LOST_BECAUSE_TX:
                {
                  performancesAmounts.at(5)++;
                  break;
                }
              case UNSET:
                {
                  break;
                }
              }   //end switch
          }
      }

    double avgDelay = 0;
    double avgAckDelay = 0;
    if (packetsOutsideTransient != 0)
      {
        avgDelay = (delaySum / packetsOutsideTransient).GetSeconds ();
        avgAckDelay = ((ackDelaySum) / packetsOutsideTransient).GetSeconds ();
      }

    // Print legend
    std::cout <<
      "Successful with 1 | Successful with 2 | Successful with 3 | Successful with 4 | Successful with 5 | Successful with 6 | Successful with 7 | Successful with 8 | Failed after 1 | Failed after 2 | Failed after 3 | Failed after 4 | Failed after 5 | Failed after 6 | Failed after 7 | Failed after 8 | Average Delay | Average ACK Delay | Total Retransmission amounts || PHY Total | PHY Successful | PHY Interfered | PHY No More Receivers | PHY Under Sensitivity | PHY Lost Because TX" <<
      std::endl;
    PrintVector (successfulReTxAmounts);
    std::cout << " | ";
    PrintVector (failedReTxAmounts);
    std::cout << " | ";
    std::cout << avgDelay << " ";
    std::cout << avgAckDelay << " ";
    std::cout << " | ";
    PrintSumRetransmissions (totalReTxAmounts);
    std::cout << " || ";
    PrintVector (performancesAmounts);
    std::cout << std::endl;
  }

  void
  LoraPacketTracker::DoCountPhyPackets (Time startTime, Time stopTime,
                                        PhyPacketData packetTracker)
  {
    // Sum PHY outcomes
    //////////////////////////////////
    // vector performanceAmounts will contain - for the interval given in the
    // input of the function, the following fields:
    // totPacketsSent receivedPackets interferedPackets noMoreGwPackets underSensitivityPackets
    std::vector<int> performancesAmounts (6, 0);
    for (auto itPhy = m_phyPacketOutcomes.begin(); itPhy != m_phyPacketOutcomes.end(); ++itPhy)
      {
        if ((*itPhy).first >= startTime && (*itPhy).first <= stopTime)
          {
            performancesAmounts.at(0)++;

            switch ((*itPhy).second)
              {
              case RECEIVED:
                {
                  performancesAmounts.at(1)++;
                  break;
                }
              case INTERFERED:
                {
                  performancesAmounts.at(2)++;
                  break;
                }
              case NO_MORE_RECEIVERS:
                {
                  performancesAmounts.at(3)++;
                  break;
                }
              case UNDER_SENSITIVITY:
                {
                  performancesAmounts.at(4)++;
                  break;
                }
              case LOST_BECAUSE_TX:
                {
                  performancesAmounts.at(5)++;
                  break;
                }
              case UNSET:
                {
                  break;
                }
              }   //end switch
          }
      }

    PrintVector (performancesAmounts);
    std::cout << std::endl;
  }
}
