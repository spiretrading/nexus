#ifndef NEXUS_CHIAMESSAGE_HPP
#define NEXUS_CHIAMESSAGE_HPP
#include <cstdint>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolParserException.hpp"

namespace Nexus {
namespace MarketDataService {

  /*! \struct ChiaMessage
      \brief Stores a single CHIA market data message.
   */
  struct ChiaMessage {

    //! The amount of time elapsed since midnight when the message was sent,
    //! timezone local to the market sending the message.
    boost::posix_time::time_duration m_timestamp;

    //! Identifies the type of message.
    char m_type;

    //! The length of the data.
    std::uint16_t m_length;

    //! The message's data.
    const char* m_data;

    //! Parses a ChiaMessage.
    /*!
      \param source The data to parse.
      \param size The size of the data to parse.
    */
    static ChiaMessage Parse(const char* source, std::size_t size);
  };

  inline ChiaMessage ChiaMessage::Parse(const char* source, std::size_t size) {
    static const auto TIMESTAMP_LENGTH = 8;
    static const auto TYPE_LENGTH = 1;
    static const auto HEADER_LENGTH = TIMESTAMP_LENGTH + TYPE_LENGTH;
    if(size < HEADER_LENGTH) {
      BOOST_THROW_EXCEPTION(
        BinarySequenceProtocol::BinarySequenceProtocolParserException(
        "Packet too short."));
    }
    ChiaMessage message;
    auto timestampRemainder = TIMESTAMP_LENGTH;
    for(auto i = 0; i < TIMESTAMP_LENGTH; ++i) {
      if(*source != ' ') {
        break;
      }
      --timestampRemainder;
      ++source;
    }
    auto milliseconds = 0;
    for(auto i = 0; i < timestampRemainder; ++i) {
      milliseconds = 10 * milliseconds + (*source - '0');
      ++source;
    }
    message.m_timestamp = boost::posix_time::milliseconds(milliseconds);
    message.m_type = *source;
    ++source;
    message.m_length = static_cast<std::uint16_t>(size - HEADER_LENGTH);
    message.m_data = source;
    return message;
  }
}
}

#endif
