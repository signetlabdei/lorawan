/*
 * Copyright (c) 2017 University of Padova
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "lora-interference-helper.h"

#include "ns3/enum.h"
#include "ns3/log.h"

#include <limits>

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraInterferenceHelper");

/***************************************
 *    LoraInterferenceHelper::Event    *
 ***************************************/

// Event Constructor
LoraInterferenceHelper::Event::Event(Time duration,
                                     double rxPowerdBm,
                                     uint8_t spreadingFactor,
                                     Ptr<Packet> packet,
                                     uint32_t frequencyHz)
    : m_startTime(Now()),
      m_endTime(m_startTime + duration),
      m_sf(spreadingFactor),
      m_rxPowerdBm(rxPowerdBm),
      m_packet(packet),
      m_frequencyHz(frequencyHz)
{
    // NS_LOG_FUNCTION_NOARGS ();
}

// Event Destructor
LoraInterferenceHelper::Event::~Event()
{
    // NS_LOG_FUNCTION_NOARGS ();
}

// Getters
Time
LoraInterferenceHelper::Event::GetStartTime() const
{
    return m_startTime;
}

Time
LoraInterferenceHelper::Event::GetEndTime() const
{
    return m_endTime;
}

Time
LoraInterferenceHelper::Event::GetDuration() const
{
    return m_endTime - m_startTime;
}

double
LoraInterferenceHelper::Event::GetRxPowerdBm() const
{
    return m_rxPowerdBm;
}

uint8_t
LoraInterferenceHelper::Event::GetSpreadingFactor() const
{
    return m_sf;
}

Ptr<Packet>
LoraInterferenceHelper::Event::GetPacket() const
{
    return m_packet;
}

uint32_t
LoraInterferenceHelper::Event::GetFrequency() const
{
    return m_frequencyHz;
}

void
LoraInterferenceHelper::Event::Print(std::ostream& stream) const
{
    stream << "(" << m_startTime.As(Time::S) << " - " << m_endTime.As(Time::S) << "), SF"
           << unsigned(m_sf) << ", " << m_rxPowerdBm << " dBm, " << m_frequencyHz << " Hz";
}

std::ostream&
operator<<(std::ostream& os, const LoraInterferenceHelper::Event& event)
{
    event.Print(os);

    return os;
}

/****************************
 *  LoraInterferenceHelper  *
 ****************************/
// This collision matrix can be used for comparisons with the performance of Aloha
// systems, where collisions imply the loss of both packets.
double inf = std::numeric_limits<double>::max();
std::vector<std::vector<double>> LoraInterferenceHelper::collisionSnirAloha = {
    //   7   8   9  10  11  12
    {inf, -inf, -inf, -inf, -inf, -inf}, // SF7
    {-inf, inf, -inf, -inf, -inf, -inf}, // SF8
    {-inf, -inf, inf, -inf, -inf, -inf}, // SF9
    {-inf, -inf, -inf, inf, -inf, -inf}, // SF10
    {-inf, -inf, -inf, -inf, inf, -inf}, // SF11
    {-inf, -inf, -inf, -inf, -inf, inf}  // SF12
};

// LoRa Collision Matrix (Goursaud)
// Values are inverted w.r.t. the paper since here we interpret this as an
// _isolation_ matrix instead of a cochannel _rejection_ matrix like in
// Goursaud's paper.
std::vector<std::vector<double>> LoraInterferenceHelper::collisionSnirGoursaud = {
    // SF7  SF8  SF9  SF10 SF11 SF12
    {6, -16, -18, -19, -19, -20}, // SF7
    {-24, 6, -20, -22, -22, -22}, // SF8
    {-27, -27, 6, -23, -25, -25}, // SF9
    {-30, -30, -30, 6, -26, -28}, // SF10
    {-33, -33, -33, -33, 6, -29}, // SF11
    {-36, -36, -36, -36, -36, 6}  // SF12
};

LoraInterferenceHelper::CollisionMatrix LoraInterferenceHelper::collisionMatrix =
    LoraInterferenceHelper::GOURSAUD;

NS_OBJECT_ENSURE_REGISTERED(LoraInterferenceHelper);

void
LoraInterferenceHelper::SetCollisionMatrix(
    enum LoraInterferenceHelper::CollisionMatrix collisionMatrix)
{
    switch (collisionMatrix)
    {
    case LoraInterferenceHelper::ALOHA:
        NS_LOG_DEBUG("Setting the ALOHA collision matrix");
        m_collisionSnir = LoraInterferenceHelper::collisionSnirAloha;
        break;
    case LoraInterferenceHelper::GOURSAUD:
        NS_LOG_DEBUG("Setting the GOURSAUD collision matrix");
        m_collisionSnir = LoraInterferenceHelper::collisionSnirGoursaud;
        break;
    }
}

TypeId
LoraInterferenceHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LoraInterferenceHelper").SetParent<Object>().SetGroupName("lorawan");

    return tid;
}

LoraInterferenceHelper::LoraInterferenceHelper()
    : m_collisionSnir(LoraInterferenceHelper::collisionSnirGoursaud)
{
    NS_LOG_FUNCTION(this);

    SetCollisionMatrix(collisionMatrix);
}

LoraInterferenceHelper::~LoraInterferenceHelper()
{
    NS_LOG_FUNCTION(this);
}

Time LoraInterferenceHelper::oldEventThreshold = Seconds(2);

Ptr<LoraInterferenceHelper::Event>
LoraInterferenceHelper::Add(Time duration,
                            double rxPower,
                            uint8_t spreadingFactor,
                            Ptr<Packet> packet,
                            uint32_t frequencyHz)
{
    NS_LOG_FUNCTION(this << duration.As(Time::MS) << rxPower << unsigned(spreadingFactor) << packet
                         << frequencyHz);

    // Create an event based on the parameters
    Ptr<LoraInterferenceHelper::Event> event =
        Create<LoraInterferenceHelper::Event>(duration,
                                              rxPower,
                                              spreadingFactor,
                                              packet,
                                              frequencyHz);

    // Add the event to the list
    m_events.push_back(event);

    // Clean the event list
    if (m_events.size() > 100)
    {
        CleanOldEvents();
    }

    return event;
}

void
LoraInterferenceHelper::CleanOldEvents()
{
    NS_LOG_FUNCTION(this);

    // Cycle the events, and clean up if an event is old.
    for (auto it = m_events.begin(); it != m_events.end();)
    {
        if ((*it)->GetEndTime() + oldEventThreshold < Now())
        {
            it = m_events.erase(it);
        }
        else
        {
            it++;
        }
    }
}

std::list<Ptr<LoraInterferenceHelper::Event>>
LoraInterferenceHelper::GetInterferers()
{
    return m_events;
}

void
LoraInterferenceHelper::PrintEvents(std::ostream& stream)
{
    NS_LOG_FUNCTION_NOARGS();

    stream << "Currently registered events:" << std::endl;

    for (auto it = m_events.begin(); it != m_events.end(); it++)
    {
        (*it)->Print(stream);
        stream << std::endl;
    }
}

uint8_t
LoraInterferenceHelper::IsDestroyedByInterference(Ptr<LoraInterferenceHelper::Event> event)
{
    NS_LOG_FUNCTION(this << event);

    NS_LOG_INFO("Current number of events in LoraInterferenceHelper: " << m_events.size());

    // We want to see the interference affecting this event: cycle through events
    // that overlap with this one and see whether it survives the interference or
    // not.

    // Gather information about the event
    double rxPowerDbm = event->GetRxPowerdBm();
    uint8_t sf = event->GetSpreadingFactor();
    uint32_t frequencyHz = event->GetFrequency();

    // Handy information about the time frame when the packet was received
    Time now = Now();
    Time duration = event->GetDuration();
    Time packetStartTime = now - duration;

    // Get the list of interfering events
    std::list<Ptr<LoraInterferenceHelper::Event>>::iterator it;

    // Energy for interferers of various SFs
    std::vector<double> cumulativeInterferenceEnergy(6, 0);

    // Cycle over the events
    for (it = m_events.begin(); it != m_events.end();)
    {
        // Pointer to the current interferer
        Ptr<LoraInterferenceHelper::Event> interferer = *it;

        // Only consider the current event if the channel is the same: we
        // assume there's no interchannel interference. Also skip the current
        // event if it's the same that we want to analyze.
        if (!(interferer->GetFrequency() == frequencyHz) || interferer == event)
        {
            NS_LOG_DEBUG("Different channel or same event");
            it++;
            continue; // Continues from the first line inside the for cycle
        }

        NS_LOG_DEBUG("Interferer on same channel");

        // Gather information about this interferer
        uint8_t interfererSf = interferer->GetSpreadingFactor();
        double interfererPower = interferer->GetRxPowerdBm();
        Time interfererStartTime = interferer->GetStartTime();
        Time interfererEndTime = interferer->GetEndTime();

        NS_LOG_INFO("Found an interferer: sf = " << unsigned(interfererSf)
                                                 << ", power = " << interfererPower
                                                 << ", start time = " << interfererStartTime
                                                 << ", end time = " << interfererEndTime);

        // Compute the fraction of time the two events are overlapping
        Time overlap = GetOverlapTime(event, interferer);

        NS_LOG_DEBUG("The two events overlap for " << overlap.As(Time::S));

        // Compute the equivalent energy of the interference
        // Power [mW] = 10^(Power[dBm]/10)
        // Power [W] = Power [mW] / 1000
        double interfererPowerW = pow(10, interfererPower / 10) / 1000;
        // Energy [J] = Time [s] * Power [W]
        double interferenceEnergy = overlap.GetSeconds() * interfererPowerW;
        cumulativeInterferenceEnergy.at(unsigned(interfererSf) - 7) += interferenceEnergy;
        NS_LOG_DEBUG("Interferer power in W: " << interfererPowerW);
        NS_LOG_DEBUG("Interference energy: " << interferenceEnergy);
        it++;
    }

    // For each spreading factor, check if there was destructive interference
    for (auto currentSf = uint8_t(7); currentSf <= uint8_t(12); currentSf++)
    {
        NS_LOG_DEBUG("Cumulative Interference Energy: "
                     << cumulativeInterferenceEnergy.at(unsigned(currentSf) - 7));

        // Use the computed cumulativeInterferenceEnergy to determine whether the
        // interference with this spreading factor destroys the packet
        double signalPowerW = pow(10, rxPowerDbm / 10) / 1000;
        double signalEnergy = duration.GetSeconds() * signalPowerW;
        NS_LOG_DEBUG("Signal power in W: " << signalPowerW);
        NS_LOG_DEBUG("Signal energy: " << signalEnergy);

        // Check whether the packet survives the interference of this spreading factor
        double snirIsolation = m_collisionSnir[unsigned(sf) - 7][unsigned(currentSf) - 7];
        NS_LOG_DEBUG("The needed isolation to survive is " << snirIsolation << " dB");
        double snir =
            10 * log10(signalEnergy / cumulativeInterferenceEnergy.at(unsigned(currentSf) - 7));
        NS_LOG_DEBUG("The current SNIR is " << snir << " dB");

        if (snir >= snirIsolation)
        {
            // Move on and check the rest of the interferers
            NS_LOG_DEBUG("Packet survived interference with SF " << currentSf);
        }
        else
        {
            NS_LOG_DEBUG("Packet destroyed by interference with SF" << unsigned(currentSf));

            return currentSf;
        }
    }
    // If we get to here, it means that the packet survived all interference
    NS_LOG_DEBUG("Packet survived all interference");

    // Since the packet was not destroyed, we return 0.
    return uint8_t(0);
}

void
LoraInterferenceHelper::ClearAllEvents()
{
    NS_LOG_FUNCTION_NOARGS();

    m_events.clear();
}

Time
LoraInterferenceHelper::GetOverlapTime(Ptr<LoraInterferenceHelper::Event> event1,
                                       Ptr<LoraInterferenceHelper::Event> event2)
{
    NS_LOG_FUNCTION_NOARGS();

    // Create the value we will return later
    Time overlap;

    // Get handy values
    Time s1 = event1->GetStartTime(); // Start times
    Time s2 = event2->GetStartTime();
    Time e1 = event1->GetEndTime(); // End times
    Time e2 = event2->GetEndTime();

    // Non-overlapping events
    if (e1 <= s2 || e2 <= s1)
    {
        overlap = Time(0);
    }
    // event1 before event2
    else if (s1 < s2)
    {
        if (e2 < e1)
        {
            overlap = e2 - s2;
        }
        else
        {
            overlap = e1 - s2;
        }
    }
    // event2 before event1 or they start at the same time (s1 = s2)
    else
    {
        if (e1 < e2)
        {
            overlap = e1 - s1;
        }
        else
        {
            overlap = e2 - s1;
        }
    }

    return overlap;
}
} // namespace lorawan
} // namespace ns3

/*
  ----------------------------------------------------------------------------

  // Event1 starts before Event2
  if (s1 < s2)
  {
  // Non-overlapping events
  if (e1 < s2)
  {
  overlap = Seconds (0);
  }
  // event1 contains event2
  else if (e1 >= e2)
  {
  overlap = e2 - s2;
  }
  // Partially overlapping events
  else
  {
  overlap = e1 - s2;
  }
  }
  // Event2 starts before Event1
  else
  {
  // Non-overlapping events
  if (e2 < s1)
  {
  overlap = Seconds (0);
  }
  // event2 contains event1
  else if (e2 >= e1)
  {
  overlap = e1 - s1;
  }
  // Partially overlapping events
  else
  {
  overlap = e2 - s1;
  }
  }
  return overlap;
  }
  }
  }
*/
