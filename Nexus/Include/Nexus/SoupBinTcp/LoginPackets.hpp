#ifndef NEXUS_SOUP_BIN_TCP_LOGIN_PACKETS_HPP
#define NEXUS_SOUP_BIN_TCP_LOGIN_PACKETS_HPP
#include <cstdint>
#include <string>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Endian.hpp>
#include <boost/lexical_cast.hpp>
#include "Nexus/SoupBinTcp/DataTypes.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcp.hpp"
#include "Nexus/SoupBinTcp/SoupBinTcpPacket.hpp"

namespace Nexus::SoupBinTcp {

  /** Stores a Login Accepted Packet. */
  struct LoginAcceptedPacket {

    /** The session ID of the session that is now logged into. */
    std::string m_session;

    /** The sequence number to be sent. */
    std::uint64_t m_sequenceNumber;
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
  inline LoginAcceptedPacket ParseLoginAcceptedPacket(
      const SoupBinTcpPacket& packet) {
    auto loginPacket = LoginAcceptedPacket();
    auto cursor = packet.m_payload;
    loginPacket.m_session = ParseLeftPaddedAlphaNumeric(10,
      Beam::Store(cursor));
    loginPacket.m_sequenceNumber = ParseLeftPaddedNumeric<std::uint64_t>(20,
      Beam::Store(cursor));
    return loginPacket;
  }

  /**
   * Parses a LoginRejectedPacket.
   * @param packet The underlying packet to parse from.
   * @return The LoginRejectedPacket.
   */
  inline LoginRejectedPacket ParseLoginRejectedPacket(
      const SoupBinTcpPacket& packet) {
    auto loginPacket = LoginRejectedPacket();
    auto cursor = packet.m_payload;
    loginPacket.m_reason = ParseLeftPaddedAlphaNumeric(1, Beam::Store(cursor));
    return loginPacket;
  }

  /**
   * Returns a Login Request Packet.
   * @param username The username.
   * @param password The password.
   * @param session Specifies the session to login to, or all blanks to login to
   *        the currently active session.
   * @param sequenceNumber Specifies the next sequence number to receive, or 0
   *        to start receiving the most recently generated message.
   * @param buffer The Buffer to store the packet in.
   */
  template<typename Buffer>
  void MakeLoginRequestPacket(const std::string& username,
      const std::string& password, const std::string& session,
      std::uint64_t sequenceNumber, Beam::Out<Buffer> buffer) {
    buffer->Append(Beam::ToBigEndian(std::uint16_t{47}));
    buffer->Append('L');
    Append(username, 6, Beam::Store(buffer));
    Append(password, 10, Beam::Store(buffer));
    Append(session, 10, Beam::Store(buffer));
    Append(boost::lexical_cast<std::string>(sequenceNumber), 20,
      Beam::Store(buffer));
  }
}

#endif
