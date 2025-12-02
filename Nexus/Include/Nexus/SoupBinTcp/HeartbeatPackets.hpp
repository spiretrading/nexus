#ifndef NEXUS_SOUP_BIN_TCP_HEARTBEAT_PACKETS_HPP
#define NEXUS_SOUP_BIN_TCP_HEARTBEAT_PACKETS_HPP
#include <cstdint>
#include <Beam/IO/Buffer.hpp>
#include <Beam/Pointers/Out.hpp>
#include <boost/endian.hpp>

namespace Nexus {

  /**
   * Returns a Client Heartbeat Packet.
   * @param buffer The Buffer to store the packet in.
   */
  template<Beam::IsBuffer B>
  void make_client_heartbeat_packet(Beam::Out<B> buffer) {
    append(*buffer, boost::endian::native_to_big(std::uint16_t(1)));
    append(*buffer, 'R');
  }
}

#endif
