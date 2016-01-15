#ifndef NEXUS_STAMPHEADER_HPP
#define NEXUS_STAMPHEADER_HPP
#include <cctype>
#include <cstdint>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/FixedString.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/StampProtocol/StampParserException.hpp"
#include "Nexus/StampProtocol/StampProtocol.hpp"

namespace Nexus {
namespace StampProtocol {

  /*! \enum RetransmissionIdentifier
      \brief Lists the types of STAMP packet transmissions.
   */
  enum class RetransmissionIdentifier : std::uint8_t {

    //! No transmission type specified.
    NONE = 0,

    //! Normal transmission.
    NORMAL,

    //! Message being sent out of order from their generation by the trading
    //! system.
    OUT_OF_ORDER
  };

  /*! \enum ContinuationIndicator
      \brief Lists the types of STAMP packet continuations.
   */
  enum class ContinuationIndicator : std::uint8_t {

    //! This is stand alone packet (the message fits in one packet).
    STAND_ALONE = 0,

    //! This packet continues in the next packet
    //! (the message spans at least 2 packets).
    SPANNING = 1,

    //! This packet is the continuation of the previous packet.
    CONTINUATION = 2,

    //! This packet is both the continuation of the previous packet and
    //! continues in the next packet.
    SPANNING_CONTINUATION = 3
  };

  /*! \struct StampHeader
      \brief Stores a STAMP header.
   */
  struct StampHeader {

    //! The size of a STAMP header.
    static const std::size_t HEADER_SIZE = 22;

    //! Total length of header and message business content
    //! (excludes STX and ETX), padded with zeros to the left.
    std::uint16_t m_length;

    //! Sequence number assigned at service broadcast.
    std::uint32_t m_sequenceNumber;

    //! Code identifying the service.
    Beam::FixedString<3> m_serviceId;

    //! Specifies whether this header is part of a retransmission.
    RetransmissionIdentifier m_retransmissionIdentifier;

    //! Specifies whether this header is part of a continuation.
    ContinuationIndicator m_continuationIndicator;

    //! Specifies the type of message.
    Beam::FixedString<2> m_messageType;

    //! Specifies the exchange where this header originated from.
    Beam::FixedString<2> m_exchangeIdentifier;

    //! Parses a StampHeader.
    /*!
      \param token The beginning of the buffer to parse.
      \param size The size of the buffer being parsed.
      \return The StampHeader that was parsed.
    */
    static StampHeader Parse(Beam::Out<const char*> token, std::size_t size);
  };

  inline StampHeader StampHeader::Parse(Beam::Out<const char*> token,
      std::size_t size) {
    static const int LENGTH_LENGTH = 4;
    static const int SEQUENCE_LENGTH = 9;
    static const int SERVICE_ID_LENGTH = 3;
    static const int MESSAGE_TYPE_LENGTH = 2;
    static const int EXCHANGE_IDENTIFIER_LENGTH = 2;
    if(size < HEADER_SIZE) {
      BOOST_THROW_EXCEPTION(StampParserException("STAMP header too short."));
    }
    StampHeader header;
    header.m_length = 0;
    for(int i = 0; i < LENGTH_LENGTH; ++i) {
      if(!std::isdigit(**token)) {
        BOOST_THROW_EXCEPTION(StampParserException("Invalid length field."));
      }
      header.m_length = 10 * header.m_length + **token - '0';
      ++*token;
    }
    header.m_sequenceNumber = 0;
    if(std::isspace(**token)) {
      *token += SEQUENCE_LENGTH;
    } else {
      for(int i = 0; i < SEQUENCE_LENGTH; ++i) {
        if(!std::isdigit(**token)) {
          BOOST_THROW_EXCEPTION(
            StampParserException("Invalid sequence field."));
        }
        header.m_sequenceNumber = 10 * header.m_sequenceNumber + **token - '0';
        ++*token;
      }
    }
    header.m_serviceId = *token;
    *token += SERVICE_ID_LENGTH;
    if(**token == '0') {
      header.m_retransmissionIdentifier = RetransmissionIdentifier::NORMAL;
    } else if(**token == '1') {
      header.m_retransmissionIdentifier =
        RetransmissionIdentifier::OUT_OF_ORDER;
    } else if(std::isspace(**token)) {
      header.m_retransmissionIdentifier = RetransmissionIdentifier::NONE;
    } else {
      BOOST_THROW_EXCEPTION(StampParserException(
        "Invalid retransmission field."));
    }
    ++*token;
    if(**token == '0') {
      header.m_continuationIndicator = ContinuationIndicator::STAND_ALONE;
    } else if(**token == '1') {
      header.m_continuationIndicator = ContinuationIndicator::SPANNING;
    } else if(**token == '2') {
      header.m_continuationIndicator = ContinuationIndicator::CONTINUATION;
    } else if(**token == '3') {
      header.m_continuationIndicator =
        ContinuationIndicator::SPANNING_CONTINUATION;
    } else {
      BOOST_THROW_EXCEPTION(StampParserException(
        "Invalid continuation field."));
    }
    ++*token;
    header.m_messageType = *token;
    *token += MESSAGE_TYPE_LENGTH;
    header.m_exchangeIdentifier = *token;
    *token += EXCHANGE_IDENTIFIER_LENGTH;
    return header;
  }
}
}

#endif
