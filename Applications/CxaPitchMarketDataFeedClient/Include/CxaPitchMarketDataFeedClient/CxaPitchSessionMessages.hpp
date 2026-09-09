#ifndef CXA_PITCH_SESSION_MESSAGES_HPP
#define CXA_PITCH_SESSION_MESSAGES_HPP
#include <cstddef>
#include <cstdint>
#include <string>
#include <Beam/IO/Buffer.hpp>
#include <Beam/Pointers/Out.hpp>
#include <boost/throw_exception.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchBlock.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchEncoder.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"

namespace Nexus {
namespace Details {
  constexpr auto SESSION_SUB_ID_LENGTH = 4;
  constexpr auto USERNAME_LENGTH = 4;
  constexpr auto FILLER_LENGTH = 2;
  constexpr auto PASSWORD_LENGTH = 10;
}

  /** Stores a login message. */
  struct CxaPitchLogin {

    /** The type of a login message. */
    static constexpr auto TYPE = std::uint8_t(0x01);

    /** The length of a login message. */
    static constexpr auto LENGTH = std::size_t(22);

    /** The session sub id to log in with. */
    std::string m_session_sub_id;

    /** The username to log in with. */
    std::string m_username;

    /** The password to log in with. */
    std::string m_password;

    /**
     * Encodes this message, appending it to a buffer.
     * @param buffer The buffer to append this message to.
     */
    template<Beam::IsBuffer B>
    void encode(Beam::Out<B> buffer) const;
  };

  /** Stores a login response message. */
  struct CxaPitchLoginResponse {

    /** The type of a login response message. */
    static constexpr auto TYPE = std::uint8_t(0x02);

    /** The smallest valid length of a login response message. */
    static constexpr auto LENGTH = std::size_t(3);

    /** The status indicating that the login was accepted. */
    static constexpr auto ACCEPTED = 'A';

    /** Whether the login was accepted, or the reason it was rejected. */
    char m_status;

    /**
     * Parses a CxaPitchLoginResponse.
     * @param message The message to parse.
     * @return The CxaPitchLoginResponse represented by the <i>message</i>.
     */
    static CxaPitchLoginResponse parse(const CxaPitchMessage& message);
  };

  /** Stores a gap request message. */
  struct CxaPitchGapRequest {

    /** The type of a gap request message. */
    static constexpr auto TYPE = std::uint8_t(0x03);

    /** The length of a gap request message. */
    static constexpr auto LENGTH = std::size_t(9);

    /** The unit that the missing messages belong to. */
    std::uint8_t m_unit;

    /** The sequence number of the first message being requested. */
    std::uint32_t m_sequence;

    /** The number of messages being requested. */
    std::uint16_t m_count;

    /**
     * Encodes this message, appending it to a buffer.
     * @param buffer The buffer to append this message to.
     */
    template<Beam::IsBuffer B>
    void encode(Beam::Out<B> buffer) const;
  };

  /** Stores a gap response message. */
  struct CxaPitchGapResponse {

    /** The type of a gap response message. */
    static constexpr auto TYPE = std::uint8_t(0x04);

    /** The smallest valid length of a gap response message. */
    static constexpr auto LENGTH = std::size_t(10);

    /** The status indicating that the request was accepted. */
    static constexpr auto ACCEPTED = 'A';

    /** The unit that the request was made for. */
    std::uint8_t m_unit;

    /** The sequence number of the first message requested. */
    std::uint32_t m_sequence;

    /** The number of messages requested. */
    std::uint16_t m_count;

    /** Whether the request was accepted, or the reason it was rejected. */
    char m_status;

    /**
     * Parses a CxaPitchGapResponse.
     * @param message The message to parse.
     * @return The CxaPitchGapResponse represented by the <i>message</i>.
     */
    static CxaPitchGapResponse parse(const CxaPitchMessage& message);
  };

  /** Stores a spin image available message. */
  struct CxaPitchSpinImageAvailable {

    /** The type of a spin image available message. */
    static constexpr auto TYPE = std::uint8_t(0x80);

    /** The smallest valid length of a spin image available message. */
    static constexpr auto LENGTH = std::size_t(6);

    /** The sequence that the available snapshot is current through. */
    std::uint32_t m_sequence;

    /**
     * Parses a CxaPitchSpinImageAvailable.
     * @param message The message to parse.
     * @return The CxaPitchSpinImageAvailable represented by the
     *         <i>message</i>.
     */
    static CxaPitchSpinImageAvailable parse(const CxaPitchMessage& message);
  };

  /** Stores a spin request message. */
  struct CxaPitchSpinRequest {

    /** The type of a spin request message. */
    static constexpr auto TYPE = std::uint8_t(0x81);

    /** The length of a spin request message. */
    static constexpr auto LENGTH = std::size_t(6);

    /** The sequence to request the snapshot at. */
    std::uint32_t m_sequence;

    /**
     * Encodes this message, appending it to a buffer.
     * @param buffer The buffer to append this message to.
     */
    template<Beam::IsBuffer B>
    void encode(Beam::Out<B> buffer) const;
  };

  /** Stores a spin response message. */
  struct CxaPitchSpinResponse {

    /** The type of a spin response message. */
    static constexpr auto TYPE = std::uint8_t(0x82);

    /** The smallest valid length of a spin response message. */
    static constexpr auto LENGTH = std::size_t(11);

    /** The status indicating that the request was accepted. */
    static constexpr auto ACCEPTED = 'A';

    /** The sequence that the snapshot is current through. */
    std::uint32_t m_sequence;

    /** The number of add order messages that the snapshot contains. */
    std::uint32_t m_order_count;

    /** Whether the request was accepted, or the reason it was rejected. */
    char m_status;

    /**
     * Parses a CxaPitchSpinResponse.
     * @param message The message to parse.
     * @return The CxaPitchSpinResponse represented by the <i>message</i>.
     */
    static CxaPitchSpinResponse parse(const CxaPitchMessage& message);
  };

  /** Stores a spin finished message. */
  struct CxaPitchSpinFinished {

    /** The type of a spin finished message. */
    static constexpr auto TYPE = std::uint8_t(0x83);

    /** The smallest valid length of a spin finished message. */
    static constexpr auto LENGTH = std::size_t(6);

    /** The sequence that the completed snapshot is current through. */
    std::uint32_t m_sequence;

    /**
     * Parses a CxaPitchSpinFinished.
     * @param message The message to parse.
     * @return The CxaPitchSpinFinished represented by the <i>message</i>.
     */
    static CxaPitchSpinFinished parse(const CxaPitchMessage& message);
  };

  template<Beam::IsBuffer B>
  void CxaPitchLogin::encode(Beam::Out<B> buffer) const {
    auto encoder = CxaPitchEncoder(Beam::Ref(*buffer));
    encoder.write_uint8(static_cast<std::uint8_t>(LENGTH));
    encoder.write_uint8(TYPE);
    encoder.write_text(m_session_sub_id, Details::SESSION_SUB_ID_LENGTH);
    encoder.write_text(m_username, Details::USERNAME_LENGTH);
    encoder.pad(Details::FILLER_LENGTH);
    encoder.write_text(m_password, Details::PASSWORD_LENGTH);
  }

  inline CxaPitchLoginResponse CxaPitchLoginResponse::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Login response message too short."));
    }
    auto cursor = message.get_cursor();
    auto response = CxaPitchLoginResponse();
    response.m_status = cursor.read_char();
    return response;
  }

  template<Beam::IsBuffer B>
  void CxaPitchGapRequest::encode(Beam::Out<B> buffer) const {
    auto encoder = CxaPitchEncoder(Beam::Ref(*buffer));
    encoder.write_uint8(static_cast<std::uint8_t>(LENGTH));
    encoder.write_uint8(TYPE);
    encoder.write_uint8(m_unit);
    encoder.write_uint32(m_sequence);
    encoder.write_uint16(m_count);
  }

  inline CxaPitchGapResponse CxaPitchGapResponse::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Gap response message too short."));
    }
    auto cursor = message.get_cursor();
    auto response = CxaPitchGapResponse();
    response.m_unit = cursor.read_uint8();
    response.m_sequence = cursor.read_uint32();
    response.m_count = cursor.read_uint16();
    response.m_status = cursor.read_char();
    return response;
  }

  inline CxaPitchSpinImageAvailable CxaPitchSpinImageAvailable::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Spin image available message too short."));
    }
    auto cursor = message.get_cursor();
    auto available = CxaPitchSpinImageAvailable();
    available.m_sequence = cursor.read_uint32();
    return available;
  }

  template<Beam::IsBuffer B>
  void CxaPitchSpinRequest::encode(Beam::Out<B> buffer) const {
    auto encoder = CxaPitchEncoder(Beam::Ref(*buffer));
    encoder.write_uint8(static_cast<std::uint8_t>(LENGTH));
    encoder.write_uint8(TYPE);
    encoder.write_uint32(m_sequence);
  }

  inline CxaPitchSpinResponse CxaPitchSpinResponse::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Spin response message too short."));
    }
    auto cursor = message.get_cursor();
    auto response = CxaPitchSpinResponse();
    response.m_sequence = cursor.read_uint32();
    response.m_order_count = cursor.read_uint32();
    response.m_status = cursor.read_char();
    return response;
  }

  inline CxaPitchSpinFinished CxaPitchSpinFinished::parse(
      const CxaPitchMessage& message) {
    if(message.m_length < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Spin finished message too short."));
    }
    auto cursor = message.get_cursor();
    auto finished = CxaPitchSpinFinished();
    finished.m_sequence = cursor.read_uint32();
    return finished;
  }
}

#endif
