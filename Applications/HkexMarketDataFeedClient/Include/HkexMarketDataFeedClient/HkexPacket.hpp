#ifndef NEXUS_HKEX_PACKET_HPP
#define NEXUS_HKEX_PACKET_HPP
#include <cstdint>
#include <stdexcept>
#include <Beam/Utilities/Endian.hpp>
#include <boost/throw_exception.hpp>

namespace Nexus::MarketDataService {

  /** Stores a single packet received from an HKEX channel. */
  struct HkexPacket {

    /** The length of a packet header. */
    static constexpr auto HEADER_LENGTH = 16;

    /** The size of the packet. */
    std::uint16_t m_size;

    /** The number of messages included in the packet. */
    std::uint8_t m_count;

    /** The packet's sequence number. */
    std::uint32_t m_sequenceNumber;

    /** The time the packet was sent. */
    std::uint64_t m_sendTime;

    /** The packet's payload. */
    const char* m_payload;

    /**
     * Parses an HkexPacket.
     * @param source A pointer to the first byte in the packet.
     * @param size The size of a packet.
     */
    static HkexPacket Parse(const char* data, std::size_t size);
  };

  inline HkexPacket HkexPacket::Parse(const char* source, std::size_t size) {
    if(size < HEADER_LENGTH) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Packet too short."));
    }
    auto packet = HkexPacket();
    packet.m_size = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint16_t*>(source));
    source += sizeof(std::uint16_t);
    packet.m_count = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint8_t*>(source));
    source += sizeof(std::uint8_t);
    source += sizeof(std::uint8_t);
    packet.m_sequenceNumber = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint32_t*>(source));
    source += sizeof(std::uint32_t);
    packet.m_sendTime = Beam::FromLittleEndian(
      *reinterpret_cast<const std::uint64_t*>(source));
    source += sizeof(std::uint64_t);
    packet.m_payload = source;
    return packet;
  }
}

#endif
