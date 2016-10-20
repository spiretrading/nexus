#ifndef NEXUS_UTPMESSAGE_HPP
#define NEXUS_UTPMESSAGE_HPP
#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Endian.hpp>
#include <boost/throw_exception.hpp>

namespace Nexus {
namespace MarketDataService {

  /*! \struct UtpMessage
      \brief Stores a single message from a UTP service.
   */
  struct UtpMessage {

    //! Protocol Version.
    std::uint8_t m_version;

    //! The message category.
    std::uint8_t m_category;

    //! The message type.
    std::uint8_t m_type;

    //! The market center originator ID.
    std::uint8_t m_marketCenterOriginatorId;

    //! The sub market center ID.
    std::uint8_t m_subMarketCenterId;

    //! The SIP timestamp.
    std::uint64_t m_sipTimestamp;

    //! The participant timestamp.
    std::uint64_t m_participantTimestamp;

    //! The participant token.
    std::uint64_t m_participantToken;

    //! The size of the payload;
    int m_dataLength;

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
    static const auto HEADER_LENGTH = 29;
    if(size < HEADER_LENGTH) {
      BOOST_THROW_EXCEPTION(std::runtime_error("Buffer too short."));
    }
    UtpMessage message;
    const char* token = *data;
    message.m_version = Beam::FromBigEndian(*token);
    ++token;
    message.m_category = Beam::FromBigEndian(*token);
    ++token;
    message.m_type = Beam::FromBigEndian(*token);
    ++token;
    message.m_marketCenterOriginatorId = Beam::FromBigEndian(*token);
    ++token;
    message.m_subMarketCenterId = Beam::FromBigEndian(*token);
    ++token;
    message.m_sipTimestamp = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint64_t*>(token));
    token += sizeof(std::uint64_t);
    message.m_participantTimestamp = Beam::FromBigEndian(
      *reinterpret_cast<const std::uint64_t*>(token));
    token += sizeof(std::uint64_t);
    message.m_data = token;
    const char* endToken = std::strpbrk(token, "\x03\x1F");
    if(endToken == nullptr) {
      BOOST_THROW_EXCEPTION(std::runtime_error("End of message not found."));
    }
    message.m_dataLength = static_cast<int>(size - HEADER_LENGTH);
    *data += size;
    return message;
  }
}
}

#endif
