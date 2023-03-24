#ifndef NEXUS_PITCH_MESSAGE_HPP
#define NEXUS_PITCH_MESSAGE_HPP
#include <cstdint>
#include <Beam/Pointers/Out.hpp>
#include <Beam/Utilities/Endian.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"
#include "ChiaMarketDataFeedClient/PitchParserException.hpp"

namespace Nexus::MarketDataService {

  /** Stores a single PITCH message. */
  struct PitchMessage {

    /** The length of this message including this field. */
    std::uint8_t m_length;

    /** The message type. */
    char m_type;

    /** The contents of the message. */
    const char* m_payload;

    /**
     * Parses a PitchMessage.
     * @param cursor The beginning of the buffer to parse, adjusted to the end
     *        of the message if parsed successfully.
     * @param size The size of the data to parse.
     */
    static PitchMessage Parse(Beam::Out<const char*> cursor, std::size_t size);

    /**
     * Parses a <code>std::uint32_t</code> field from a message.
     * @param cursor A pointer to the first byte of the field to parse.
     * @return The value of the field.
     */
    static std::uint32_t ParseUint32(Beam::Out<const char*> cursor);

    /**
     * Parses a <code>std::uint64_t</code> field from a message.
     * @param cursor A pointer to the first byte of the field to parse.
     * @return The value of the field.
     */
    static std::uint64_t ParseUint64(Beam::Out<const char*> cursor);

    /**
     * Parses a timestamp field from a message.
     * @param cursor A pointer to the first byte of the field to parse.
     * @return The value of the field.
     */
    static boost::posix_time::ptime ParseTimestamp(
      Beam::Out<const char*> cursor);

    /**
     * Parses a <code>char</code> field from a message.
     * @param cursor A pointer to the first byte of the field to parse.
     * @return The value of the field.
     */
    static char ParseChar(Beam::Out<const char*> cursor);

    /**
     * Parses a <code>std::string</code> field from a message.
     * @param size The maximum number of characters to parse.
     * @param cursor A pointer to the first byte of the field to parse.
     * @return The value of the field.
     */
    static std::string ParseAlphanumeric(
      int size, Beam::Out<const char*> cursor);

    /**
     * Parses a <code>Side</code> field from a message.
     * @param cursor A pointer to the first byte of the field to parse.
     * @return The value of the field.
     */
    static Side ParseSide(Beam::Out<const char*> cursor);

    /**
     * Parses a <code>Money</code> field from a message.
     * @param cursor A pointer to the first byte of the field to parse.
     * @return The value of the field.
     */
    static Money ParsePrice(Beam::Out<const char*> cursor);
  };

  inline PitchMessage PitchMessage::Parse(
      Beam::Out<const char*> cursor, std::size_t size) {
    auto message = PitchMessage();
    if(size == 0) {
      BOOST_THROW_EXCEPTION(PitchParserException("PITCH message too short."));
    }
    message.m_length =
      Beam::FromLittleEndian(*reinterpret_cast<const std::uint8_t*>(*cursor));
    if(size < message.m_length) {
      BOOST_THROW_EXCEPTION(PitchParserException("PITCH message too short."));
    }
    ++*cursor;
    message.m_type = **cursor;
    ++*cursor;
    message.m_payload = *cursor;
    return message;
  }

  inline std::uint32_t PitchMessage::ParseUint32(
      Beam::Out<const char*> cursor) {
    auto value =
      Beam::FromLittleEndian(*reinterpret_cast<const std::uint32_t*>(*cursor));
    *cursor += sizeof(std::uint32_t);
    return value;
  }

  inline std::uint64_t PitchMessage::ParseUint64(
      Beam::Out<const char*> cursor) {
    auto value =
      Beam::FromLittleEndian(*reinterpret_cast<const std::uint64_t*>(*cursor));
    *cursor += sizeof(std::uint64_t);
    return value;
  }

  inline boost::posix_time::ptime PitchMessage::ParseTimestamp(
      Beam::Out<const char*> cursor) {
    static const auto EPOCH =
      boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));
    return EPOCH +
      boost::posix_time::microseconds(ParseUint64(Beam::Store(*cursor)) / 1000);
  }

  inline char PitchMessage::ParseChar(Beam::Out<const char*> cursor) {
    auto value = **cursor;
    ++*cursor;
    return value;
  }

  inline std::string PitchMessage::ParseAlphanumeric(
      int size, Beam::Out<const char*> cursor) {
    auto value = std::string();
    for(auto i = 0; i < size; ++i) {
      if((*cursor)[i] == ' ') {
        break;
      }
      value += (*cursor)[i];
    }
    *cursor += size;
    return value;
  }

  inline Side PitchMessage::ParseSide(Beam::Out<const char*> cursor) {
    auto value = ParseChar(Beam::Store(cursor));
    auto side = [&] {
      if(value == 'B') {
        return Side::BID;
      } else if(value == 'S') {
        return Side::ASK;
      }
      return Side::NONE;
    }();
    return side;
  }

  inline Money PitchMessage::ParsePrice(Beam::Out<const char*> cursor) {
    static auto DENOMINATOR = 10000000;
    auto value = Quantity(ParseUint64(Beam::Store(*cursor)));
    return Money(value / DENOMINATOR);
  }
}

#endif
