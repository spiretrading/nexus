#ifndef NEXUS_MOLD_UDP_64_REQUEST_HPP
#define NEXUS_MOLD_UDP_64_REQUEST_HPP
#include <Beam/IO/Buffer.hpp>
#include <boost/endian/conversion.hpp>
#include "Nexus/MoldUdp64/MoldUdp64Packet.hpp"

namespace Nexus {

  /** Requests retransmission of a range of messages in a MoldUDP64 session. */
  struct MoldUdp64Request {

    /** The size of an encoded request in bytes. */
    static constexpr auto LENGTH = MoldUdp64Packet::PACKET_LENGTH;

    /** The session containing the requested messages. */
    Beam::FixedString<MoldUdp64Packet::SESSION_FIELD_LENGTH> m_session;

    /** The first sequence to retransmit. */
    std::uint64_t m_sequence_number;

    /** The number of messages to retransmit. */
    std::uint16_t m_count;
  };

  /**
   * Appends a MoldUDP64 request to a buffer.
   * @param request The range to request.
   * @param destination The buffer receiving the encoded request.
   */
  template<Beam::IsBuffer B>
  void encode(const MoldUdp64Request& request, Beam::Out<B> destination) {
    Beam::append(*destination, request.m_session.get_data(),
      MoldUdp64Packet::SESSION_FIELD_LENGTH);
    Beam::append(
      *destination, boost::endian::native_to_big(request.m_sequence_number));
    Beam::append(*destination, boost::endian::native_to_big(request.m_count));
  }
}

#endif
