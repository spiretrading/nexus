#ifndef CXA_PITCH_ENCODER_HPP
#define CXA_PITCH_ENCODER_HPP
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <Beam/IO/Buffer.hpp>
#include <Beam/Pointers/Ref.hpp>
#include <boost/endian/conversion.hpp>

namespace Nexus {

  /**
   * Encodes the fields of a CXA PITCH message.
   * @param <B> The type of buffer to encode to.
   */
  template<Beam::IsBuffer B>
  class CxaPitchEncoder {
    public:

      /** The type of buffer to encode to. */
      using Buffer = B;

      /**
       * Constructs a CxaPitchEncoder.
       * @param buffer The buffer to append the encoded fields to.
       */
      explicit CxaPitchEncoder(Beam::Ref<Buffer> buffer) noexcept;

      /** Encodes an 8 bit unsigned integer. */
      void write_uint8(std::uint8_t value);

      /** Encodes a 16 bit unsigned integer. */
      void write_uint16(std::uint16_t value);

      /** Encodes a 32 bit unsigned integer. */
      void write_uint32(std::uint32_t value);

      /**
       * Encodes a space padded text field.
       * @param value The value to encode, truncated to the field's width.
       * @param size The width of the field.
       */
      void write_text(std::string_view value, int size);

      /**
       * Encodes a run of spaces.
       * @param size The number of spaces to encode.
       */
      void pad(int size);

    private:
      Buffer* m_buffer;
  };

  template<Beam::IsBuffer B>
  CxaPitchEncoder<B>::CxaPitchEncoder(Beam::Ref<Buffer> buffer) noexcept
    : m_buffer(buffer.get()) {}

  template<Beam::IsBuffer B>
  void CxaPitchEncoder<B>::write_uint8(std::uint8_t value) {
    Beam::append(*m_buffer, &value, sizeof(value));
  }

  template<Beam::IsBuffer B>
  void CxaPitchEncoder<B>::write_uint16(std::uint16_t value) {
    auto encoding = boost::endian::native_to_little(value);
    Beam::append(*m_buffer, &encoding, sizeof(encoding));
  }

  template<Beam::IsBuffer B>
  void CxaPitchEncoder<B>::write_uint32(std::uint32_t value) {
    auto encoding = boost::endian::native_to_little(value);
    Beam::append(*m_buffer, &encoding, sizeof(encoding));
  }

  template<Beam::IsBuffer B>
  void CxaPitchEncoder<B>::write_text(std::string_view value, int size) {
    auto length = std::min(value.size(), static_cast<std::size_t>(size));
    Beam::append(*m_buffer, value.data(), length);
    pad(size - static_cast<int>(length));
  }

  template<Beam::IsBuffer B>
  void CxaPitchEncoder<B>::pad(int size) {
    for(auto i = 0; i != size; ++i) {
      Beam::append(*m_buffer, " ", 1);
    }
  }
}

#endif
