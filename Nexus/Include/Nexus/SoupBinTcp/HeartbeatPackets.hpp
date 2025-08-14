#ifndef NEXUS_SOUP_BIN_TCP_HEARTBEAT_PACKETS_HPP
#define NEXUS_SOUP_BIN_TCP_HEARTBEAT_PACKETS_HPP
#include <cstdint>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Endian.hpp>

namespace Nexus::SoupBinTcp {

  /**
   * Returns a Client Heartbeat Packet.
   * @param buffer The Buffer to store the packet in.
   */
  template<typename Buffer>
  void make_client_heartbeat_packet(Beam::Out<Buffer> buffer) {
    buffer->Append(Beam::ToBigEndian(std::uint16_t(1)));
    buffer->Append('R');
  }
}

#endif
