#ifndef NEXUS_SOUP_BIN_TCP_PACKET_HPP
#define NEXUS_SOUP_BIN_TCP_PACKET_HPP
#include <cstdint>
#include <string_view>
#include <Beam/IO/Buffer.hpp>
#include <Beam/IO/Reader.hpp>
#include <Beam/Pointers/Out.hpp>
#include <boost/endian.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/SoupBinTcp/SoupBinTcpParserException.hpp"

namespace Nexus {

  /** Stores a logical SoupBinTCP packet. */
  struct SoupBinTcpPacket {

    /** The number of bytes in the packet, excluding the length field. */
    std::uint16_t m_length;

    /** The type of packet. */
    std::uint8_t m_type;

    /** The payload owned by the buffer supplied to read_packet. */
    const char* m_payload;

    /** Returns the payload bytes. */
    std::string_view get_payload() const;
  };

  inline std::string_view SoupBinTcpPacket::get_payload() const {
    return std::string_view(m_payload, m_length - sizeof(m_type));
  }

  /**
   * Appends an alphanumeric value to a SoupBinTcpPacket.
   * @param value The value to append.
   * @param length The length of the field.
   * @param buffer The Buffer to append the <i>value</i> to.
   */
  template<Beam::IsBuffer B>
  void append(std::string_view value, std::size_t length, Beam::Out<B> buffer) {
    if(value.size() > length) {
      boost::throw_with_location(
        SoupBinTcpParserException("Alphanumeric value too long."));
    }
    append(*buffer, value);
    for(auto i = value.size(); i < length; ++i) {
      append(*buffer, ' ');
    }
  }

  /**
   * Reads a logical packet from a Reader.
   * @param reader The Reader to read the logical packet from.
   * @param buffer The Buffer to append the packet type and payload to.
   * @return The logical packet read from the <i>reader</i>.
   *         The payload remains valid until the buffer is modified or destroyed.
   */
  template<Beam::IsReader R, Beam::IsBuffer B>
  SoupBinTcpPacket read_packet(R& reader, Beam::Out<B> buffer) {
    auto packet = SoupBinTcpPacket();
    read(reader, Beam::out(packet.m_length));
    packet.m_length = boost::endian::big_to_native(packet.m_length);
    if(packet.m_length == 0) {
      boost::throw_with_location(
        SoupBinTcpParserException("SoupBinTCP packet length is zero."));
    }
    auto offset = buffer->get_size();
    read_exact(reader, Beam::out(buffer), packet.m_length);
    packet.m_type = static_cast<std::uint8_t>(buffer->get_data()[offset]);
    packet.m_payload = buffer->get_data() + offset + sizeof(packet.m_type);
    return packet;
  }
}

#endif
