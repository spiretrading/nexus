#ifndef NEXUS_SOUPBINTCPHEARTBEATPACKETS_HPP
#define NEXUS_SOUPBINTCPHEARTBEATPACKETS_HPP
#include <cstdint>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Endian.hpp>
#include "Nexus/SoupBinTcp/SoupBinTcp.hpp"

namespace Nexus {
namespace SoupBinTcp {

  //! Builds a Client Heartbeat Packet.
  /*!
    \param buffer The Buffer to store the packet in.
  */
  template<typename Buffer>
  void BuildClientHeartbeatPacket(Beam::Out<Buffer> buffer) {
    buffer->Append(Beam::ToBigEndian(std::uint16_t{1}));
    buffer->Append('R');
  }
}
}

#endif
