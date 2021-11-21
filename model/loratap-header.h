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
 * Author: Alessandro Aimi <alleaimi95@gmail.com>
 */

#ifndef LORATAP_HEADER_H
#define LORATAP_HEADER_H

#include "ns3/header.h"
#include "ns3/lora-tag.h"

namespace ns3 {
namespace lorawan {

/**
 * This class represents the LoRaTap header that needs to be added to a packet 
 * before it is traced using Pcap.
 */
class LoratapHeader : public Header
{
public:
  LoratapHeader ();
  ~LoratapHeader ();

  // Methods inherited from Header
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  /**
   * Return the size required for serialization of this header
   *
   * \return The serialized size in bytes
   */
  virtual uint32_t GetSerializedSize (void) const;

  /**
   * Serialize the header.
   *
   * See eriknl/LoRaTap repository README.md for a representation of fields.
   *
   * \param start A pointer to the buffer that will be filled with the
   * serialization.
   */
  virtual void Serialize (Buffer::Iterator start) const;

  /**
   * Deserialize the contents of the buffer into a LoratapHeader object.
   *
   * \param start A pointer to the buffer we need to deserialize.
   * \return The number of consumed bytes.
   */
  virtual uint32_t Deserialize (Buffer::Iterator start);

  /**
   * Print the header in a human-readable format.
   *
   * \param os The std::ostream on which to print the header.
   */
  virtual void Print (std::ostream &os) const;

  /**
   * Fill the header with info from LoraTag
   *
   * \param tag The tag of the packet we want to add the header to.
   */
  void Fill (LoraTag &tag);

  private:
  
  uint8_t m_lt_version; /* LoRatap header version */
  uint8_t m_lt_padding; /* Unused, for boundary alignment */
  uint16_t m_lt_length; /* LoRatap header length, reserved for future header expansion */

  uint32_t m_frequency; /* LoRa frequency (Hz) */
  uint8_t m_bandwidth; /* Channel bandwidth (KHz) in 125 KHz steps */
  uint8_t m_sf; /* LoRa SF (sf_t) [7, 8, 9, 10, 11, 12] */

  uint8_t m_packet_rssi; /* LoRa packet RSSI, if snr >= 0 then dBm value is -139 + packet_rssi, otherwise dBm value is -139 + packet_rssi * .25 */
  uint8_t m_max_rssi; /* LoRa receiver max RSSI (dBm value is -139 + rssi) */
  uint8_t m_current_rssi; /* LoRa receiver current RSSI (dBm value is -139 + rssi) */
  uint8_t m_snr; /* LoRa SNR (dB value is (snr[two's complement])/4) */

  uint8_t m_sync_word; /* LoRa radio sync word [0x34 = LoRaWAN] */

};

}
}
#endif
