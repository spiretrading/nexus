#ifndef NEXUS_SOUP_BIN_TCP_LOGIN_PACKETS_HPP
#define NEXUS_SOUP_BIN_TCP_LOGIN_PACKETS_HPP
#include <cstdint>
#include <string>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Endian.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/SoupBinTcp/DataTypes.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpPacket.hpp"

namespace Nexus::SoupBinTcp {

  /** Stores a Login Accepted Packet. */
  struct LoginAcceptedPacket {

    /** The session ID of the session that is now logged into. */
    std::string m_session;

    /** The sequence number to be sent. */
    std::uint64_t m_sequence_number;
  };

  /** Stores a Login Rejected Packet. */
  struct LoginRejectedPacket {

    /** The code for why the login was rejected. */
    std::string m_reason;
  };

  /**
   * Parses a LoginAcceptedPacket.
   * @param packet The underlying packet to parse from.
   * @return The LoginAcceptedPacket.
   */
  inline LoginAcceptedPacket parse_login_accepted_packet(
      const SoupBinTcpPacket& packet) {
    auto login_packet = LoginAcceptedPacket();
    auto cursor = packet.m_payload;
    login_packet.m_session =
      parse_left_padded_alpha_numeric(10, Beam::Store(cursor));
    login_packet.m_sequence_number =
      parse_left_padded_numeric<std::uint64_t>(20, Beam::Store(cursor));
    return login_packet;
  }

  /**
   * Parses a LoginRejectedPacket.
   * @param packet The underlying packet to parse from.
   * @return The LoginRejectedPacket.
   */
  inline LoginRejectedPacket parse_login_rejected_packet(
      const SoupBinTcpPacket& packet) {
    auto login_packet = LoginRejectedPacket();
    auto cursor = packet.m_payload;
    login_packet.m_reason =
      parse_left_padded_alpha_numeric(1, Beam::Store(cursor));
    return login_packet;
  }

  /**
   * Returns a Login Request Packet.
   * @param username The username.
   * @param password The password.
   * @param session Specifies the session to login to, or all blanks to login to
   *        the currently active session.
   * @param sequence_number Specifies the next sequence number to receive, or 0
   *        to start receiving the most recently generated message.
   * @param buffer The Buffer to store the packet in.
   */
  template<typename Buffer>
  void make_login_request_packet(const std::string& username,
      const std::string& password, const std::string& session,
      std::uint64_t sequence_number, Beam::Out<Buffer> buffer) {
    buffer->Append(Beam::ToBigEndian(std::uint16_t(47)));
    buffer->Append('L');
    append(username, 6, Beam::Store(buffer));
    append(password, 10, Beam::Store(buffer));
    append(session, 10, Beam::Store(buffer));
    append(boost::lexical_cast<std::string>(sequence_number), 20,
      Beam::Store(buffer));
  }
}

#endif
