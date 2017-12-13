#ifndef NEXUS_CHIAMESSAGE_HPP
#define NEXUS_CHIAMESSAGE_HPP
#include <cstdint>
#include <string.h>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/BinarySequenceProtocol/BinarySequenceProtocolParserException.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"

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

    //! Parses a character field from a message.
    /*!
      \param length The length of the field.
      \param cursor A pointer to the first byte of the field to parse.
      \return The value of the field.
    */
    static char ParseChar(Beam::Out<const char*> cursor);

    //! Parses a numeric field from a message.
    /*!
      \param length The length of the field.
      \param cursor A pointer to the first byte of the field to parse.
      \return The value of the field.
    */
    static std::int64_t ParseNumeric(int length, Beam::Out<const char*> cursor);

    //! Parses an alphanumeric field from a message.
    /*!
      \param length The length of the field.
      \param cursor A pointer to the first byte of the field to parse.
      \return The value of the field.
    */
    static std::string ParseAlphanumeric(int length,
      Beam::Out<const char*> cursor);

    //! Parses a Side field from a message.
    /*!
      \param length The length of the field.
      \param cursor A pointer to the first byte of the field to parse.
      \return The numeric value of the field.
    */
    static Side ParseSide(Beam::Out<const char*> cursor);

    //! Parses a price field from a message.
    /*!
      \param length The length of the field.
      \param cursor A pointer to the first byte of the field to parse.
      \return The numeric value of the field.
    */
    static Money ParsePrice(bool isLongForm, Beam::Out<const char*> cursor);
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

  inline char ChiaMessage::ParseChar(Beam::Out<const char*> cursor) {
    auto value = **cursor;
    ++*cursor;
    return value;
  }

  inline std::int64_t ChiaMessage::ParseNumeric(int length,
      Beam::Out<const char*> cursor) {
    while(**cursor == ' ' && length > 0) {
      ++*cursor;
      --length;
    }
    std::int64_t value = 0;
    while(length > 0) {
      value = value * 10 + (**cursor - '0');
      --length;
      ++*cursor;
    }
    return value;
  }

  inline std::string ChiaMessage::ParseAlphanumeric(int length,
      Beam::Out<const char*> cursor) {
    std::string value;
    for(auto i = 0; i < length; ++i) {
      if((*cursor)[i] == ' ') {
        break;
      }
      value += (*cursor)[i];
    }
    *cursor += length;
    return value;
  }

  inline Side ChiaMessage::ParseSide(Beam::Out<const char*> cursor) {
    auto value = ChiaMessage::ParseChar(Beam::Store(cursor));
    auto side =
      [&] {
        if(value == 'B') {
          return Side::BID;
        } else if(value == 'S') {
          return Side::ASK;
        }
        return Side::NONE;
      }();
    return side;
  }

  inline Money ChiaMessage::ParsePrice(bool isLongForm,
      Beam::Out<const char*> cursor) {
    auto PowerOfTen =
      [] (std::uint16_t exponent) {
        std::uint64_t result = 1;
        for(auto i = std::uint16_t{0}; i < exponent; ++i) {
          result *= 10;
        }
        return result;
      };
    auto value = 0;
    auto length =
      [&] {
        if(isLongForm) {
          return 19;
        }
        return 10;
      }();
    auto remainingLength = length;
    while(**cursor == ' ' && remainingLength > 0) {
      ++*cursor;
      --remainingLength;
    }
    while(remainingLength > 0) {
      value = 10 * value + (**cursor - '0');
      ++*cursor;
      --remainingLength;
    }
    auto decimalPlaces =
      [&] {
        if(isLongForm) {
          return 7;
        }
        return 4;
      }();
    auto multiplier = PowerOfTen(Money::DECIMAL_PLACES - decimalPlaces);
    auto price = Money::FromRepresentation(value * multiplier);
    return price;
  }
}
}

#endif
