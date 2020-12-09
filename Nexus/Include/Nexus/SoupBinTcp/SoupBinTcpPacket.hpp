#ifndef NEXUS_SOUP_BIN_TCP_PACKET_HPP
#define NEXUS_SOUP_BIN_TCP_PACKET_HPP
#include <cstdint>
#include <Beam/IO/Reader.hpp>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Endian.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/SoupBinTcp/SoupBinTcp.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpParserException.hpp"

namespace Nexus::SoupBinTcp {

  /** Stores a logical SoupBinTCP packet. */
  struct SoupBinTcpPacket {

    /** The number of bytes in the packet, excluding the length field. */
    std::uint16_t m_length;

    /** The type of packet. */
    std::uint8_t m_type;

    /** The payload. */
    const char* m_payload;
  };

  /**
   * Appends an alphanumeric value to a SoupBinTcpPacket.
   * @param value The value to append.
   * @param length The length of the field.
   * @param buffer The Buffer to append the <i>value</i> to.
   */
  template<typename Buffer>
  void Append(const std::string& value, std::size_t length,
      Beam::Out<Buffer> buffer) {
    if(value.size() > length) {
      BOOST_THROW_EXCEPTION(
        SoupBinTcpParserException("Alphanumeric value too long."));
    }
    buffer->Append(value.c_str(), value.size());
    for(auto i = value.size(); i < length; ++i) {
      buffer->Append(' ');
    }
  }

  /**
   * Reads a logical packet from a Reader.
   * @param reader The Reader to read the logical packet from.
   * @param payloadBuffer The Buffer used to store the payload.
   * @return The logical packet read from the <i>reader</i>.
   */
  template<typename Reader, typename Buffer>
  SoupBinTcpPacket ReadPacket(Reader& reader, Beam::Out<Buffer> buffer) {
    auto packet = SoupBinTcpPacket();
    Beam::IO::ReadExactSize(reader, reinterpret_cast<char*>(&packet.m_length),
      sizeof(packet.m_length));
    packet.m_length = Beam::FromBigEndian(packet.m_length);
    Beam::IO::ReadExactSize(reader, reinterpret_cast<char*>(&packet.m_type),
      sizeof(packet.m_type));
    packet.m_type = Beam::FromBigEndian(packet.m_type);
    Beam::IO::ReadExactSize(reader, Beam::Store(buffer), packet.m_length - 1);
    packet.m_payload = buffer->GetData();
    return packet;
  }
}

#endif
