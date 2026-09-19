#ifndef NEXUS_TO_PYTHON_SOUP_BIN_TCP_PACKET_HPP
#define NEXUS_TO_PYTHON_SOUP_BIN_TCP_PACKET_HPP
#include <Beam/IO/BufferReader.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Utilities/Expect.hpp>
#include "Nexus/SoupBinTcp/SoupBinTcpPacket.hpp"

namespace Nexus {

  /** Owns a SoupBinTCP packet for use with Python. */
  class ToPythonSoupBinTcpPacket {
    public:

      /**
       * Parses an owned SoupBinTCP packet.
       * @param source One complete frame, including its length field.
       */
      explicit ToPythonSoupBinTcpPacket(std::string_view source);

      /**
       * Copies a SoupBinTCP packet into owned storage.
       * @param packet The packet to copy.
       */
      explicit ToPythonSoupBinTcpPacket(const SoupBinTcpPacket& packet);

      /** Returns the underlying packet. */
      const SoupBinTcpPacket& get() const;

    private:
      Beam::SharedBuffer m_source;
      SoupBinTcpPacket m_packet;
  };

  inline ToPythonSoupBinTcpPacket::ToPythonSoupBinTcpPacket(
      std::string_view source) {
    auto reader =
      Beam::BufferReader(Beam::SharedBuffer(source.data(), source.size()));
    try {
      m_packet = read_packet(reader, Beam::out(m_source));
    } catch(const Beam::IOException&) {
      Beam::throw_nested_with_location(
        SoupBinTcpParserException("Incomplete SoupBinTCP frame."));
    }
    if(reader.poll()) {
      boost::throw_with_location(
        SoupBinTcpParserException("Trailing SoupBinTCP frame data."));
    }
  }

  inline ToPythonSoupBinTcpPacket::ToPythonSoupBinTcpPacket(
      const SoupBinTcpPacket& packet)
      : m_source(packet.m_payload, packet.get_payload().size()),
        m_packet(packet) {
    m_packet.m_payload = m_source.get_data();
  }

  inline const SoupBinTcpPacket& ToPythonSoupBinTcpPacket::get() const {
    return m_packet;
  }
}

#endif
