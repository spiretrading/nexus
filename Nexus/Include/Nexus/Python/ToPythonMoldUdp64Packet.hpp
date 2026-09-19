#ifndef NEXUS_TO_PYTHON_MOLD_UDP_64_PACKET_HPP
#define NEXUS_TO_PYTHON_MOLD_UDP_64_PACKET_HPP
#include <Beam/IO/SharedBuffer.hpp>
#include "Nexus/MoldUdp64/MoldUdp64Packet.hpp"

namespace Nexus {

  /** Owns a MoldUDP64 packet for use with Python. */
  class ToPythonMoldUdp64Packet {
    public:

      /**
       * Parses an owned MoldUDP64 packet.
       * @param source The complete downstream datagram.
       */
      ToPythonMoldUdp64Packet(std::string_view source);

      /**
       * Copies a MoldUDP64 packet into owned storage.
       * @param packet The packet to copy.
       */
      ToPythonMoldUdp64Packet(const MoldUdp64Packet& packet);

      /** Returns the underlying packet. */
      const MoldUdp64Packet& get() const;

    private:
      Beam::SharedBuffer m_source;
      MoldUdp64Packet m_packet;
  };

  inline ToPythonMoldUdp64Packet::ToPythonMoldUdp64Packet(
      std::string_view source)
    : m_source(source.data(), source.size()),
      m_packet(MoldUdp64Packet::parse(
        std::string_view(m_source.get_data(), m_source.get_size()))) {}

  inline ToPythonMoldUdp64Packet::ToPythonMoldUdp64Packet(
      const MoldUdp64Packet& packet)
      : m_source(packet.m_payload.data(), packet.m_payload.size()),
        m_packet(packet) {
    m_packet.m_payload =
      std::string_view(m_source.get_data(), m_source.get_size());
  }

  inline const MoldUdp64Packet& ToPythonMoldUdp64Packet::get() const {
    return m_packet;
  }
}

#endif
