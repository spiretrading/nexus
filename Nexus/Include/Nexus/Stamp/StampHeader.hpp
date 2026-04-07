#ifndef NEXUS_STAMP_HEADER_HPP
#define NEXUS_STAMP_HEADER_HPP
#include <cctype>
#include <cstdint>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Stamp/StampParserException.hpp"

namespace Nexus {

  /** Lists the types of STAMP packet transmissions. */
  enum class RetransmissionIdentifier : std::uint8_t {

    /** No transmission type specified. */
    NONE = 0,

    /** Normal transmission. */
    NORMAL,

    /**
     * Message being sent out of order from their generation by the trading
     * system.
     */
    OUT_OF_ORDER
  };

  /** Lists the types of STAMP packet continuations. */
  enum class ContinuationIndicator : std::uint8_t {

    /** This is stand alone packet (the message fits in one packet). */
    STAND_ALONE = 0,

    /**
     * This packet continues in the next packet
     * (the message spans at least 2 packets).
     */
    SPANNING = 1,

    /** This packet is the continuation of the previous packet. */
    CONTINUATION = 2,

    /**
     * This packet is both the continuation of the previous packet and
     * continues in the next packet.
     */
    SPANNING_CONTINUATION = 3
  };

  /** Stores a STAMP header. */
  struct StampHeader {

    /** The size of a STAMP header. */
    static const auto HEADER_SIZE = std::size_t(22);

    /**
     * Total length of header and message business content
     * (excludes STX and ETX), padded with zeros to the left.
     */
    std::uint16_t m_length;

    /** Sequence number assigned at service broadcast. */
    std::uint32_t m_sequence_number;

    /** Code identifying the service. */
    Beam::FixedString<3> m_service_id;

    /** Specifies whether this header is part of a retransmission. */
    RetransmissionIdentifier m_retransmission_identifier;

    /** Specifies whether this header is part of a continuation. */
    ContinuationIndicator m_continuation_indicator;

    /** Specifies the type of message. */
    Beam::FixedString<2> m_message_type;

    /** Specifies the exchange where this header originated from. */
    Beam::FixedString<2> m_exchange_identifier;

    /**
     * Parses a StampHeader.
     * @param token The beginning of the buffer to parse.
     * @param size The size of the buffer being parsed.
     * @return The StampHeader that was parsed.
     */
    static StampHeader parse(Beam::Out<const char*> token, std::size_t size);
  };

  inline StampHeader StampHeader::parse(
      Beam::Out<const char*> token, std::size_t size) {
    static const auto LENGTH_LENGTH = 4;
    static const auto SEQUENCE_LENGTH = 9;
    static const auto SERVICE_ID_LENGTH = 3;
    static const auto MESSAGE_TYPE_LENGTH = 2;
    static const auto EXCHANGE_IDENTIFIER_LENGTH = 2;
    if(size < HEADER_SIZE) {
      boost::throw_with_location(
        StampParserException("STAMP header too short."));
    }
    auto header = StampHeader();
    header.m_length = 0;
    for(int i = 0; i < LENGTH_LENGTH; ++i) {
      if(!std::isdigit(**token)) {
        boost::throw_with_location(
          StampParserException("Invalid length field."));
      }
      header.m_length = 10 * header.m_length + **token - '0';
      ++*token;
    }
    header.m_sequence_number = 0;
    if(std::isspace(**token)) {
      *token += SEQUENCE_LENGTH;
    } else {
      for(int i = 0; i < SEQUENCE_LENGTH; ++i) {
        if(!std::isdigit(**token)) {
          boost::throw_with_location(
            StampParserException("Invalid sequence field."));
        }
        header.m_sequence_number =
          10 * header.m_sequence_number + **token - '0';
        ++*token;
      }
    }
    header.m_service_id = *token;
    *token += SERVICE_ID_LENGTH;
    if(**token == '0') {
      header.m_retransmission_identifier = RetransmissionIdentifier::NORMAL;
    } else if(**token == '1') {
      header.m_retransmission_identifier =
        RetransmissionIdentifier::OUT_OF_ORDER;
    } else if(std::isspace(**token)) {
      header.m_retransmission_identifier = RetransmissionIdentifier::NONE;
    } else {
      boost::throw_with_location(
        StampParserException("Invalid retransmission field."));
    }
    ++*token;
    if(**token == '0') {
      header.m_continuation_indicator = ContinuationIndicator::STAND_ALONE;
    } else if(**token == '1') {
      header.m_continuation_indicator = ContinuationIndicator::SPANNING;
    } else if(**token == '2') {
      header.m_continuation_indicator = ContinuationIndicator::CONTINUATION;
    } else if(**token == '3') {
      header.m_continuation_indicator =
        ContinuationIndicator::SPANNING_CONTINUATION;
    } else {
      boost::throw_with_location(
        StampParserException("Invalid continuation field."));
    }
    ++*token;
    header.m_message_type = *token;
    *token += MESSAGE_TYPE_LENGTH;
    header.m_exchange_identifier = *token;
    *token += EXCHANGE_IDENTIFIER_LENGTH;
    return header;
  }
}

#endif
