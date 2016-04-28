#ifndef NEXUS_CTAMESSAGE_HPP
#define NEXUS_CTAMESSAGE_HPP
#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <Beam/Pointers/Out.hpp>
#include <boost/throw_exception.hpp>

namespace Nexus {
namespace MarketDataService {

  /*! \struct CtaMessage
      \brief Stores a single message from a CTA service.
   */
  struct CtaMessage {

    //! The message category.
    std::uint8_t m_category;

    //! The message type.
    std::uint8_t m_type;

    //! The network.
    std::uint8_t m_network;

    //! The retransmission requestor.
    std::array<char, 2> m_retransmissionRequestor;

    //! The message header identifier.
    std::uint8_t m_identifier;

    //! The transaction ID Part A.
    std::array<char, 2> m_transactionIdA;

    //! The message sequence number.
    std::uint32_t m_sequenceNumber;

    //! The participant ID.
    std::uint8_t m_participantId;

    //! The CQS timestamp.
    std::array<char, 6> m_cqsTimestamp;

    //! Timestamp 1.
    std::array<char, 6> m_timestamp1;

    //! Timestamp 2.
    std::array<char, 6> m_timestamp2;

    //! The Transaction ID Part B.
    std::array<char, 9> m_transactionIdB;

    //! The length of data in the payload.
    std::uint16_t m_dataLength;

    //! The payload data.
    const char* m_data;

    //! Parses a CtaMessage from a Buffer.
    /*!
      \param data A pointer to the first byte in the packet to parse, this
                  pointer will be modified to point to the end of the message.
      \param size The number of bytes remaining in the packet.
      \return The CtaMessage represented by the <i>buffer</i>.
    */
    static CtaMessage Parse(Beam::Out<const char*> data, std::uint16_t size);
  };

  inline CtaMessage CtaMessage::Parse(Beam::Out<const char*> data,
      std::uint16_t size) {
    static const auto HEADER_LENGTH = 45;
    static const auto SEQUENCE_NUMBER_LENGTH = 9;
    static const auto RETRANSMISSION_REQUESTOR_LENGTH = 2;
    static const auto TRANSACTION_ID_A_LENGTH = 2;
    static const auto TIMESTAMP_LENGTH = 6;
    static const auto TRANSACTION_ID_B_LENGTH = 9;
    if(size < HEADER_LENGTH) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Buffer too short."));
    }
    CtaMessage message;
    const char* token = *data;
    message.m_category = *token;
    ++token;
    message.m_type = *token;
    ++token;
    message.m_network = *token;
    ++token;
    std::memcpy(message.m_retransmissionRequestor.data(), token,
      RETRANSMISSION_REQUESTOR_LENGTH);
    token += RETRANSMISSION_REQUESTOR_LENGTH;
    message.m_identifier = *token;
    ++token;
    std::memcpy(message.m_transactionIdA.data(), token,
      TRANSACTION_ID_A_LENGTH);
    token += TRANSACTION_ID_A_LENGTH;
    message.m_sequenceNumber = 0;
    for(auto i = 0; i < SEQUENCE_NUMBER_LENGTH; ++i) {
      message.m_sequenceNumber = message.m_sequenceNumber * 10 +
        (*token - '0');
      ++token;
    }
    message.m_participantId = *token;
    ++token;
    std::memcpy(message.m_cqsTimestamp.data(), token, TIMESTAMP_LENGTH);
    token += TIMESTAMP_LENGTH;
    std::memcpy(message.m_timestamp1.data(), token, TIMESTAMP_LENGTH);
    token += TIMESTAMP_LENGTH;
    std::memcpy(message.m_timestamp2.data(), token, TIMESTAMP_LENGTH);
    token += TIMESTAMP_LENGTH;
    std::memcpy(message.m_transactionIdB.data(), token,
      TRANSACTION_ID_B_LENGTH);
    token += TRANSACTION_ID_B_LENGTH;
    message.m_data = token;
    const char* endToken = std::strpbrk(token, "\x03\x1F");
    if(endToken == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error("End of message not found."));
    }
    message.m_dataLength = static_cast<std::uint16_t>(
      (endToken - *data) - HEADER_LENGTH);
    *data = endToken + 1;
    return message;
  }
}
}

#endif
