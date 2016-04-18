#ifndef NEXUS_UTPMESSAGE_HPP
#define NEXUS_UTPMESSAGE_HPP
#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <Beam/Pointers/Out.hpp>
#include <boost/throw_exception.hpp>

namespace Nexus {
namespace MarketDataService {

  /*! \struct UtpMessage
      \brief Stores a single message from a UTP service.
   */
  struct UtpMessage {

    //! The message category.
    std::uint8_t m_category;

    //! The message type.
    std::uint8_t m_type;

    //! The session identifier.
    std::uint8_t m_identifier;

    //! The retransmission requestor.
    std::array<char, 2> m_retransmissionRequestor;

    //! The message sequence number.
    std::uint32_t m_sequenceNumber;

    //! The market center originator ID.
    std::uint8_t m_marketCenterOriginatorId;

    //! The SIP timestamp.
    std::array<char, 6> m_sipTimestamp;

    //! The sub market center ID.
    std::uint8_t m_subMarketCenterId;

    //! The timestamp from the market participant.
    std::array<char, 6> m_marketTimestamp;

    //! The timestamp from the FINRA ADF.
    std::array<char, 6> m_finraTimestamp;

    //! The length of data in the payload.
    std::uint16_t m_dataLength;

    //! The payload data.
    const char* m_data;

    //! Parses a UtpMessage from a Buffer.
    /*!
      \param data A pointer to the first byte in the packet to parse, this
                  pointer will be modified to point to the end of the message.
      \param size The number of bytes remaining in the packet.
      \return The UtpMessage represented by the <i>buffer</i>.
    */
    static UtpMessage Parse(Beam::Out<const char*> data, std::uint16_t size);
  };

  inline UtpMessage UtpMessage::Parse(Beam::Out<const char*> data,
      std::uint16_t size) {
    static const auto HEADER_LENGTH = 43;
    static const auto SEQUENCE_NUMBER_LENGTH = 8;
    static const auto RETRANSMISSION_REQUESTOR_LENGTH = 2;
    static const auto TIMESTAMP_LENGTH = 6;
    static const auto RESERVED_GAP_LENGTH = 3;
    static const auto TRANSACTION_ID_LENGTH = 7;
    if(size < HEADER_LENGTH) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Buffer too short."));
    }
    UtpMessage message;
    const char* token = *data;
    message.m_category = *token;
    ++token;
    message.m_type = *token;
    ++token;
    message.m_identifier = *token;
    ++token;
    std::memcpy(message.m_retransmissionRequestor.data(), token,
      RETRANSMISSION_REQUESTOR_LENGTH);
    token += RETRANSMISSION_REQUESTOR_LENGTH;
    message.m_sequenceNumber = 0;
    for(auto i = 0; i < SEQUENCE_NUMBER_LENGTH; ++i) {
      message.m_sequenceNumber = message.m_sequenceNumber * 10 +
        (*token - '0');
      ++token;
    }
    message.m_marketCenterOriginatorId = *token;
    ++token;
    std::memcpy(message.m_sipTimestamp.data(), token, TIMESTAMP_LENGTH);
    token += TIMESTAMP_LENGTH;
    token += RESERVED_GAP_LENGTH;
    message.m_subMarketCenterId = *token;
    ++token;
    std::memcpy(message.m_marketTimestamp.data(), token, TIMESTAMP_LENGTH);
    token += TIMESTAMP_LENGTH;
    std::memcpy(message.m_finraTimestamp.data(), token, TIMESTAMP_LENGTH);
    token += TIMESTAMP_LENGTH;
    token += TRANSACTION_ID_LENGTH;
    message.m_dataLength = static_cast<std::uint16_t>(size - HEADER_LENGTH);
    message.m_data = token;
    const char* endToken = std::strpbrk(token, "\x03\x1F");
    if(endToken == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error("End of message not found."));
    }
    *data = endToken + 1;
    return message;
  }
}
}

#endif
