#ifndef CXA_PITCH_CURSOR_HPP
#define CXA_PITCH_CURSOR_HPP
#include <cstdint>
#include <cstring>
#include <string>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/endian/conversion.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/Definitions/Side.hpp"

namespace Nexus {

  /** Decodes the fields of a CXA PITCH message. */
  class CxaPitchCursor {
    public:

      /**
       * Constructs a CxaPitchCursor.
       * @param position The first byte to decode.
       */
      explicit CxaPitchCursor(const char* position) noexcept;

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

      /** Decodes a UTC timestamp. */
      boost::posix_time::ptime read_timestamp();

      /** Decodes a price. */
      Money read_price();

      /** Decodes a side. */
      Side read_side();

      /**
       * Decodes a space padded text field.
       * @param size The width of the field.
       * @return The field with its padding removed.
       */
      std::string read_text(int size);

      /**
       * Advances past a number of bytes.
       * @param size The number of bytes to advance past.
       */
      void skip(int size);

    private:
      const char* m_position;
  };

  inline CxaPitchCursor::CxaPitchCursor(const char* position) noexcept
    : m_position(position) {}

  inline std::uint8_t CxaPitchCursor::read_uint8() {
    auto value = std::uint8_t();
    std::memcpy(&value, m_position, sizeof(value));
    m_position += sizeof(value);
    return value;
  }

  inline std::uint16_t CxaPitchCursor::read_uint16() {
    auto value = std::uint16_t();
    std::memcpy(&value, m_position, sizeof(value));
    m_position += sizeof(value);
    return boost::endian::little_to_native(value);
  }

  inline std::uint32_t CxaPitchCursor::read_uint32() {
    auto value = std::uint32_t();
    std::memcpy(&value, m_position, sizeof(value));
    m_position += sizeof(value);
    return boost::endian::little_to_native(value);
  }

  inline std::uint64_t CxaPitchCursor::read_uint64() {
    auto value = std::uint64_t();
    std::memcpy(&value, m_position, sizeof(value));
    m_position += sizeof(value);
    return boost::endian::little_to_native(value);
  }

  inline char CxaPitchCursor::read_char() {
    auto value = *m_position;
    ++m_position;
    return value;
  }

  inline boost::posix_time::ptime CxaPitchCursor::read_timestamp() {
    static const auto EPOCH =
      boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));
    return EPOCH + boost::posix_time::microseconds(read_uint64() / 1000);
  }

  inline Money CxaPitchCursor::read_price() {
    static constexpr auto DENOMINATOR = 10000000 / Quantity::MULTIPLIER;
    auto price = read_uint64();
    return Money(Quantity::from_representation(price / DENOMINATOR +
      static_cast<double>(price % DENOMINATOR) / DENOMINATOR));
  }

  inline Side CxaPitchCursor::read_side() {
    auto value = read_char();
    if(value == 'B') {
      return Side::BID;
    } else if(value == 'S') {
      return Side::ASK;
    }
    return Side::NONE;
  }

  inline std::string CxaPitchCursor::read_text(int size) {
    auto end = size;
    while(end > 0 && m_position[end - 1] == ' ') {
      --end;
    }
    auto value = std::string(m_position, end);
    m_position += size;
    return value;
  }

  inline void CxaPitchCursor::skip(int size) {
    m_position += size;
  }
}

#endif
