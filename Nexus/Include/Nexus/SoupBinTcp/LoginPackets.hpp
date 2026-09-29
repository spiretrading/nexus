#ifndef NEXUS_SOUP_BIN_TCP_LOGIN_PACKETS_HPP
#define NEXUS_SOUP_BIN_TCP_LOGIN_PACKETS_HPP
#include <boost/endian.hpp>
#include "Nexus/SoupBinTcp/DataTypes.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpPacket.hpp"

namespace Nexus {

  /** Stores a Login Accepted Packet. */
  struct LoginAcceptedPacket {
    static constexpr auto TYPE = std::uint8_t('A');
    static constexpr auto SESSION_LENGTH = std::size_t(10);
    static constexpr auto SEQUENCE_LENGTH = std::size_t(20);
    static constexpr auto LENGTH =
      std::uint16_t(1 + SESSION_LENGTH + SEQUENCE_LENGTH);

    /** The session ID of the session that is now logged into. */
    std::string m_session;

    /** The sequence number to be sent. */
    std::uint64_t m_sequence_number;
  };

  /** Stores a Login Rejected Packet. */
  struct LoginRejectedPacket {
    static constexpr auto TYPE = std::uint8_t('J');
    static constexpr auto LENGTH = std::uint16_t(2);

    /** The code for why the login was rejected. */
    char m_reason;
  };

  /**
   * Parses a LoginAcceptedPacket.
   * @param packet The underlying packet to parse from.
   * @return The LoginAcceptedPacket.
   */
  inline LoginAcceptedPacket parse_login_accepted_packet(
      const SoupBinTcpPacket& packet) {
    if(packet.m_type != LoginAcceptedPacket::TYPE ||
        packet.m_length != LoginAcceptedPacket::LENGTH) {
      boost::throw_with_location(
        SoupBinTcpParserException("Invalid login accepted packet."));
    }
    auto login_packet = LoginAcceptedPacket();
    auto cursor = packet.m_payload;
    login_packet.m_session =
      parse_left_padded_alpha_numeric(
        LoginAcceptedPacket::SESSION_LENGTH, Beam::out(cursor));
    login_packet.m_sequence_number =
      parse_left_padded_numeric<std::uint64_t>(
        LoginAcceptedPacket::SEQUENCE_LENGTH, Beam::out(cursor));
    return login_packet;
  }

  /**
   * Parses a LoginRejectedPacket.
   * @param packet The underlying packet to parse from.
   * @return The LoginRejectedPacket.
   */
  inline LoginRejectedPacket parse_login_rejected_packet(
      const SoupBinTcpPacket& packet) {
    if(packet.m_type != LoginRejectedPacket::TYPE ||
        packet.m_length != LoginRejectedPacket::LENGTH) {
      boost::throw_with_location(
        SoupBinTcpParserException("Invalid login rejected packet."));
    }
    return LoginRejectedPacket(packet.m_payload[0]);
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
  template<Beam::IsBuffer B>
  void make_login_request_packet(std::string_view username,
      std::string_view password, std::string_view session,
      std::uint64_t sequence_number, Beam::Out<B> buffer) {
    static constexpr auto USERNAME_LENGTH = std::size_t(6);
    static constexpr auto PASSWORD_LENGTH = std::size_t(10);
    static constexpr auto LENGTH = std::uint16_t(sizeof(std::uint8_t) +
      USERNAME_LENGTH + PASSWORD_LENGTH + LoginAcceptedPacket::SESSION_LENGTH +
      LoginAcceptedPacket::SEQUENCE_LENGTH);
    append(*buffer, boost::endian::native_to_big(LENGTH));
    append(*buffer, 'L');
    append(username, USERNAME_LENGTH, Beam::out(buffer));
    append(password, PASSWORD_LENGTH, Beam::out(buffer));
    append(session, LoginAcceptedPacket::SESSION_LENGTH, Beam::out(buffer));
    append(std::to_string(sequence_number),
      LoginAcceptedPacket::SEQUENCE_LENGTH, Beam::out(buffer));
  }
}

#endif
