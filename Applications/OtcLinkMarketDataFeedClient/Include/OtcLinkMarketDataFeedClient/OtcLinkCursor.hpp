#ifndef OTC_LINK_CURSOR_HPP
#define OTC_LINK_CURSOR_HPP
#include <concepts>
#include <cstdint>
#include <cstring>
#include <string_view>
#include <boost/endian/conversion.hpp>
#include <boost/throw_exception.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkParserException.hpp"

namespace Nexus {

  /** Decodes the fields of an OTC Link message. */
  class OtcLinkCursor {
    public:

      /**
       * Constructs an OtcLinkCursor.
       * @param source The fields to decode.
       */
      explicit OtcLinkCursor(std::string_view source) noexcept;

      /** Decodes an 8 bit unsigned integer. */
      std::uint8_t read_uint8();

      /** Decodes a 16 bit unsigned integer. */
      std::uint16_t read_uint16();

      /** Decodes a 32 bit unsigned integer. */
      std::uint32_t read_uint32();

      /** Decodes a 64 bit unsigned integer. */
      std::uint64_t read_uint64();

      /**
       * Reads a view of a field's bytes.
       * @param size The width of the field.
       */
      std::string_view read_bytes(std::size_t size);

      /**
       * Advances past a number of bytes.
       * @param size The number of bytes to advance past.
       */
      void skip(std::size_t size);

    private:
      std::string_view m_source;

      template<std::unsigned_integral T>
      T read_integer();
  };

  inline OtcLinkCursor::OtcLinkCursor(std::string_view source) noexcept
    : m_source(source) {}

  inline std::uint8_t OtcLinkCursor::read_uint8() {
    return read_integer<std::uint8_t>();
  }

  inline std::uint16_t OtcLinkCursor::read_uint16() {
    return read_integer<std::uint16_t>();
  }

  inline std::uint32_t OtcLinkCursor::read_uint32() {
    return read_integer<std::uint32_t>();
  }

  inline std::uint64_t OtcLinkCursor::read_uint64() {
    return read_integer<std::uint64_t>();
  }

  inline std::string_view OtcLinkCursor::read_bytes(std::size_t size) {
    if(size > m_source.size()) {
      boost::throw_with_location(
        OtcLinkParserException("OTC Link field too short."));
    }
    auto value = m_source.substr(0, size);
    m_source.remove_prefix(size);
    return value;
  }

  inline void OtcLinkCursor::skip(std::size_t size) {
    read_bytes(size);
  }

  template<std::unsigned_integral T>
  T OtcLinkCursor::read_integer() {
    auto source = read_bytes(sizeof(T));
    auto value = T();
    std::memcpy(&value, source.data(), sizeof(value));
    return boost::endian::big_to_native(value);
  }
}

#endif
