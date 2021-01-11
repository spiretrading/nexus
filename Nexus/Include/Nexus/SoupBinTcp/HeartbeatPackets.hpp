#ifndef NEXUS_SOUP_BIN_TCP_HEART_BEAT_PACKETS_HPP
#define NEXUS_SOUP_BIN_TCP_HEART_BEAT_PACKETS_HPP
#include <cstdint>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Endian.hpp>
#include "Nexus/SoupBinTcp/SoupBinTcp.hpp"

namespace Nexus::SoupBinTcp {

  /**
   * Returns a Client Heartbeat Packet.
   * @param buffer The Buffer to store the packet in.
   */
  template<typename Buffer>
  void MakeClientHeartbeatPacket(Beam::Out<Buffer> buffer) {
    buffer->Append(Beam::ToBigEndian(std::uint16_t{1}));
    buffer->Append('R');
  }
}

#endif
