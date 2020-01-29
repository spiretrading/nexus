#ifndef NEXUS_JPX_FLEX_PACKET_HPP
#define NEXUS_JPX_FLEX_PACKET_HPP
#include <cstdint>
#include <stdexcept>
#include <Beam/Pointers/Out.hpp>
#include <boost/throw_exception.hpp>

namespace Nexus::MarketDataService {

  /** Stores a single packet received from a JPX Flex channel. */
  struct JpxFlexPacket {

    /** The length of a packet header. */
    static constexpr auto HEADER_LENGTH = 42;

    /** The size of the packet. */
    std::uint32_t m_size;

    /** The multicast group number. */
    std::uint16_t m_group;

    /** The packet's sequence number. */
    std::uint32_t m_sequenceNumber;

    /** The packet's exchange code. */
    std::uint8_t m_exchangeCode;

    /** The packet's payload. */
    const char* m_payload;

    /**
     * Parses a JpxFlexPacket.
     * @param source A pointer to the first byte in the packet.
     * @param size The size of a packet.
     */
    static JpxFlexPacket Parse(const char* data, std::size_t size);
  };

  template<typename T>
  T ParseNumber(Beam::Out<const char*> source,
      Beam::Out<std::size_t> remainingSize, std::size_t size) {
    auto value = T{0};
    while(size-- != 0) {
      value = 10 * value + static_cast<T>(**source - '0');
      ++*source;
    }
    *remainingSize -= size;
    return value;
  }

  inline JpxFlexPacket JpxFlexPacket::Parse(const char* source,
      std::size_t size) {
    if(size < HEADER_LENGTH) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Packet too short."));
    }
    auto packet = JpxFlexPacket();
    ++source;
    --size;
    packet.m_size = ParseNumber<std::uint32_t>(Beam::Store(source),
      Beam::Store(size), 6);
    packet.m_group = ParseNumber<std::uint16_t>(Beam::Store(source),
      Beam::Store(size), 3);
    packet.m_sequenceNumber = ParseNumber<std::uint32_t>(Beam::Store(source),
      Beam::Store(size), 8);
    source += 3;
    size -= 3;
    packet.m_exchangeCode = *source;
    ++source;
    --size;
    source += 19;
    size -= 19;
    packet.m_payload = source;
    return packet;
  }
}

#endif
