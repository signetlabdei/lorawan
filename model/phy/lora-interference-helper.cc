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

#include "lora-interference-helper.h"

#include "ns3/simulator.h"

namespace ns3
{
namespace lorawan
{

NS_LOG_COMPONENT_DEFINE("LoraInterferenceHelper");

NS_OBJECT_ENSURE_REGISTERED(LoraInterferenceHelper);

/***************************************
 *    Event    *
 ***************************************/

// Event Constructor
LoraInterferenceHelper::Event::Event(Time duration,
                                     double rxPowerdBm,
                                     uint8_t spreadingFactor,
                                     Ptr<Packet> packet,
                                     double frequency)
    : m_startTime(Simulator::Now()),
      m_endTime(m_startTime + duration),
      m_sf(spreadingFactor),
      m_rxPowerdBm(rxPowerdBm),
      m_packet(packet),
      m_frequencyHz(frequency)
{
}

// Event Destructor
LoraInterferenceHelper::Event::~Event()
{
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

double
LoraInterferenceHelper::Event::GetFrequency() const
{
    return m_frequencyHz;
}

void
LoraInterferenceHelper::Event::Print(std::ostream& stream) const
{
    stream << "(" << m_startTime.GetSeconds() << " s - " << m_endTime.GetSeconds() << " s), SF"
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

TypeId
LoraInterferenceHelper::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::LoraInterferenceHelper")
            .SetParent<Object>()
            .SetGroupName("lorawan")
            .AddConstructor<LoraInterferenceHelper>()
            .AddAttribute("IsolationMatrix",
                          "Signal to Interference Ratio (SIR) matrix used to determine "
                          "if a packet is destroyed by interference on collision event",
                          EnumValue(IsolationMatrix::CROCE),
                          MakeEnumAccessor(&LoraInterferenceHelper::SetIsolationMatrixAttribute),
                          MakeEnumChecker(IsolationMatrix::CROCE,
                                          "CROCE",
                                          IsolationMatrix::GOURSAUD,
                                          "GOURSAUD",
                                          IsolationMatrix::ALOHA,
                                          "ALOHA"));

    return tid;
}

LoraInterferenceHelper::LoraInterferenceHelper()
    : m_isolationMatrix(CROCE)
{
    NS_LOG_FUNCTION(this);
}

LoraInterferenceHelper::~LoraInterferenceHelper()
{
    NS_LOG_FUNCTION(this);
}

Ptr<LoraInterferenceHelper::Event>
LoraInterferenceHelper::Add(Time duration,
                            double rxPower,
                            uint8_t spreadingFactor,
                            Ptr<Packet> packet,
                            double frequency)
{
    NS_LOG_FUNCTION(this << duration.GetSeconds() << rxPower << unsigned(spreadingFactor) << packet
                         << frequency);
    // Create an event based on the parameters
    auto event = Create<Event>(duration, rxPower, spreadingFactor, packet, frequency);
    // Add the event to the list
    m_events.push_back(event);
    // Clean the event list
    if (m_events.size() > 100)
    {
        CleanOldEvents();
    }
    return event;
}

uint8_t
LoraInterferenceHelper::IsDestroyedByInterference(Ptr<Event> event)
{
    NS_LOG_FUNCTION(this << event);
    // We want to see the interference affecting this event: cycle through events
    // that overlap with this one and see whether it survives the interference or
    // not.
    NS_LOG_INFO("Current number of events in LoraInterferenceHelper: " << m_events.size());
    // Gather information about the event
    double rxPowerDbm = event->GetRxPowerdBm();
    uint8_t sf = event->GetSpreadingFactor();
    double frequency = event->GetFrequency();
    // Handy information about the time frame when the packet was received
    Time now = Simulator::Now();
    Time duration = event->GetDuration();
    Time packetStartTime = now - duration;
    // Energy for interferers of various SFs
    std::vector<double> cumulativeInterferenceEnergy(6, 0);
    // Cycle over the events
    for (auto& interferer : m_events)
    {
        // Only consider the current event if the channel is the same: we
        // assume there's no interchannel interference. Also skip the current
        // event if it's the same that we want to analyze.
        if (!(interferer->GetFrequency() == frequency) || interferer == event)
        {
            NS_LOG_DEBUG("Different channel or same event");
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
        NS_LOG_DEBUG("The two events overlap for " << overlap.GetSeconds() << " s.");
        // Compute the equivalent energy of the interference
        // Power [mW] = 10^(Power[dBm]/10)
        // Power [W] = Power [mW] / 1000
        double interfererPowerW = pow(10, interfererPower / 10) / 1000;
        // Energy [J] = Time [s] * Power [W]
        double interferenceEnergy = overlap.GetSeconds() * interfererPowerW;
        cumulativeInterferenceEnergy.at(unsigned(interfererSf) - 7) += interferenceEnergy;
        NS_LOG_DEBUG("Interferer power in W: " << interfererPowerW);
        NS_LOG_DEBUG("Interference energy: " << interferenceEnergy);
    }
    // For each SF, check if there was destructive interference
    for (uint8_t currentSf = 7; currentSf <= 12; ++currentSf)
    {
        NS_LOG_DEBUG("Cumulative Interference Energy: "
                     << cumulativeInterferenceEnergy.at(unsigned(currentSf) - 7));
        // Use the computed cumulativeInterferenceEnergy to determine whether the
        // interference with this SF destroys the packet
        double signalPowerW = pow(10, rxPowerDbm / 10) / 1000;
        double signalEnergy = duration.GetSeconds() * signalPowerW;
        NS_LOG_DEBUG("Signal power in W: " << signalPowerW);
        NS_LOG_DEBUG("Signal energy: " << signalEnergy);
        // Check whether the packet survives the interference of this SF
        double sirIsolation = m_isolationMatrix[unsigned(sf) - 7][unsigned(currentSf) - 7];
        NS_LOG_DEBUG("The needed isolation to survive is " << sirIsolation << " dB");
        double sir =
            10 * log10(signalEnergy / cumulativeInterferenceEnergy.at(unsigned(currentSf) - 7));
        NS_LOG_DEBUG("The current SIR is " << sir << " dB");
        if (sir >= sirIsolation)
        {
            // Move on and check the rest of the interferers
            NS_LOG_DEBUG("Packet survived interference with SF " << unsigned(currentSf));
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
    return 0;
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
    for (const auto& e : m_events)
    {
        stream << e << std::endl;
    }
}

Time
LoraInterferenceHelper::GetOverlapTime(Ptr<Event> event1, Ptr<Event> event2)
{
    NS_LOG_FUNCTION_NOARGS();
    Time s1 = event1->GetStartTime(); // Start times
    Time s2 = event2->GetStartTime();
    Time e1 = event1->GetEndTime(); // End times
    Time e2 = event2->GetEndTime();
    if (e1 <= s2 || e2 <= s1)
    { // Non-overlapping events
        return Seconds(0);
    }
    else if (s1 < s2) // Overlapping: event1 starts before event2
    {
        if (e2 < e1)
        {                   // event2 is temporally contained in event1
            return e2 - s2; // return duration of event2
        }
        else
        {
            return e1 - s2; // return time between start of event2 and end of event1
        }
    }
    else // Overlapping: event2 starts before event1 (or s1 = s2)
    {
        if (e1 < e2)
        {                   // event1 is temporally contained in event2
            return e1 - s1; // return duration of event1
        }
        else
        {
            return e2 - s1; // return time between start of event1 and end of event2
        }
    }
}

void
LoraInterferenceHelper::ClearAllEvents()
{
    NS_LOG_FUNCTION_NOARGS();
    m_events.clear();
}

void
LoraInterferenceHelper::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_events.clear();
    Object::DoDispose();
}

void
LoraInterferenceHelper::CleanOldEvents()
{
    NS_LOG_FUNCTION(this);
    // Cycle the events, and clean up if an event is old.
    Time now = Simulator::Now();
    Time threshold = m_oldEventThreshold;
    auto isOld = [&now, &threshold](Ptr<Event> e) { return e->GetEndTime() + threshold < now; };
    m_events.remove_if(isOld);
}

void
LoraInterferenceHelper::SetIsolationMatrix(IsolationMatrix matrix)
{
    switch (matrix)
    {
    case ALOHA:
        NS_LOG_DEBUG("Setting the ALOHA collision matrix");
        m_isolationMatrix = LoraInterferenceHelper::m_ALOHA;
        break;
    case GOURSAUD:
        NS_LOG_DEBUG("Setting the GOURSAUD collision matrix");
        m_isolationMatrix = LoraInterferenceHelper::m_GOURSAUD;
        break;
    case CROCE:
        NS_LOG_DEBUG("Setting the CROCE collision matrix");
        m_isolationMatrix = LoraInterferenceHelper::m_CROCE;
        break;
    }
}

void
LoraInterferenceHelper::SetIsolationMatrixAttribute(EnumValue matrix)
{
    SetIsolationMatrix((IsolationMatrix)matrix.Get());
}

const Time LoraInterferenceHelper::m_oldEventThreshold = Seconds(2);

using sirMatrix_t = std::vector<std::vector<double>>;

// This collision matrix can be used for comparisons with the performance of Aloha
// systems, where collisions imply the loss of both packets.
double inf = std::numeric_limits<double>::max();
const sirMatrix_t LoraInterferenceHelper::m_ALOHA = {
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
const sirMatrix_t LoraInterferenceHelper::m_GOURSAUD = {
    // SF7  SF8  SF9  SF10 SF11 SF12
    {6, -16, -18, -19, -19, -20}, // SF7
    {-24, 6, -20, -22, -22, -22}, // SF8
    {-27, -27, 6, -23, -25, -25}, // SF9
    {-30, -30, -30, 6, -26, -28}, // SF10
    {-33, -33, -33, -33, 6, -29}, // SF11
    {-36, -36, -36, -36, -36, 6}  // SF12
};

// LoRa Collision Matrix (Croce)
const sirMatrix_t LoraInterferenceHelper::m_CROCE = {
    // SF7  SF8  SF9  SF10 SF11 SF12
    {1, -8, -9, -9, -9, -9},      // SF7
    {-11, 1, -11, -12, -13, -13}, // SF8
    {-15, -13, 1, -13, -14, -15}, // SF9
    {-19, -18, -17, 1, -17, -18}, // SF10
    {-22, -22, -21, -20, 1, -20}, // SF11
    {-25, -25, -25, -24, -23, 1}  // SF12
};

} // namespace lorawan
} // namespace ns3
