#ifndef ASX_TRADE_ITCH_CURSOR_HPP
#define ASX_TRADE_ITCH_CURSOR_HPP
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>
#include <boost/endian/conversion.hpp>
#include <boost/throw_exception.hpp>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchParserException.hpp"
#include "Nexus/Definitions/Side.hpp"

namespace Nexus {

  /** Decodes the fields of an ASX Trade ITCH message. */
  class AsxTradeItchCursor {
    public:

      /**
       * Constructs an AsxTradeItchCursor.
       * @param source The fields to decode.
       */
      explicit AsxTradeItchCursor(std::string_view source) noexcept;

      /** Decodes an 8 bit unsigned integer. */
      std::uint8_t read_uint8();

      /** Decodes a 16 bit unsigned integer. */
      std::uint16_t read_uint16();

      /** Decodes a 32 bit unsigned integer. */
      std::uint32_t read_uint32();

      /** Decodes a 64 bit unsigned integer. */
      std::uint64_t read_uint64();

      /** Decodes a single character. */
      char read_char();

      /** Decodes a signed price in the order book's wire units. */
      std::int32_t read_price();

      /** Decodes an order side, or NONE for a blank trade side. */
      Side read_side();

      /**
       * Decodes a space padded text field.
       * @param size The width of the field.
       * @return The field with its trailing padding removed.
       */
      std::string read_text(std::size_t size);

      /**
       * Advances past a number of bytes.
       * @param size The number of bytes to advance past.
       */
      void skip(std::size_t size);

    private:
      std::string_view m_source;

      std::string_view take(std::size_t size);
      template<typename T>
      T read_integer();
  };

  inline AsxTradeItchCursor::AsxTradeItchCursor(
    std::string_view source) noexcept
    : m_source(source) {}

  inline std::uint8_t AsxTradeItchCursor::read_uint8() {
    return read_integer<std::uint8_t>();
  }

  inline std::uint16_t AsxTradeItchCursor::read_uint16() {
    return read_integer<std::uint16_t>();
  }

  inline std::uint32_t AsxTradeItchCursor::read_uint32() {
    return read_integer<std::uint32_t>();
  }

  inline std::uint64_t AsxTradeItchCursor::read_uint64() {
    return read_integer<std::uint64_t>();
  }

  inline char AsxTradeItchCursor::read_char() {
    return take(sizeof(char)).front();
  }

  inline std::int32_t AsxTradeItchCursor::read_price() {
    return read_integer<std::int32_t>();
  }

  inline Side AsxTradeItchCursor::read_side() {
    auto value = read_char();
    if(value == 'B') {
      return Side::BID;
    } else if(value == 'S') {
      return Side::ASK;
    } else if(value == ' ') {
      return Side::NONE;
    }
    boost::throw_with_location(
      AsxTradeItchParserException("Invalid order side."));
  }

  inline std::string AsxTradeItchCursor::read_text(std::size_t size) {
    auto value = take(size);
    while(!value.empty() && value.back() == ' ') {
      value.remove_suffix(1);
    }
    return std::string(value);
  }

  inline void AsxTradeItchCursor::skip(std::size_t size) {
    take(size);
  }

  inline std::string_view AsxTradeItchCursor::take(std::size_t size) {
    if(size > m_source.size()) {
      boost::throw_with_location(
        AsxTradeItchParserException("ITCH field too short."));
    }
    auto value = m_source.substr(0, size);
    m_source.remove_prefix(size);
    return value;
  }

  template<typename T>
  T AsxTradeItchCursor::read_integer() {
    auto source = take(sizeof(T));
    auto value = T();
    std::memcpy(&value, source.data(), sizeof(value));
    return boost::endian::big_to_native(value);
  }
}

#endif
