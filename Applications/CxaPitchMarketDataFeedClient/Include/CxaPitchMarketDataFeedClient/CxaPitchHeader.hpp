#ifndef CXA_PITCH_HEADER_HPP
#define CXA_PITCH_HEADER_HPP
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <Beam/IO/Buffer.hpp>
#include <Beam/Pointers/Out.hpp>
#include <boost/throw_exception.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchCursor.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchEncoder.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchParserException.hpp"

namespace Nexus {

  /** Stores a CXA PITCH sequenced unit header. */
  struct CxaPitchHeader {

    /** The length of a sequenced unit header. */
    static constexpr auto LENGTH = std::size_t(8);

    /** The length of this header and the messages that follow it. */
    std::uint16_t m_length;

    /** The number of messages that follow this header. */
    std::uint8_t m_count;

    /** The unit that the messages following this header belong to. */
    std::uint8_t m_unit;

    /** The sequence number of the first message to follow this header. */
    std::uint32_t m_sequence;

    /**
     * Parses a CxaPitchHeader.
     * @param source The buffer to parse.
     * @return The CxaPitchHeader represented by the <i>source</i>.
     */
    static CxaPitchHeader parse(std::string_view source);

    /**
     * Encodes this header, appending it to a buffer.
     * @param buffer The buffer to append this header to.
     */
    template<Beam::IsBuffer B>
    void encode(Beam::Out<B> buffer) const;
  };

  inline CxaPitchHeader CxaPitchHeader::parse(std::string_view source) {
    if(source.size() < LENGTH) {
      boost::throw_with_location(
        CxaPitchParserException("Sequenced unit header too short."));
    }
    auto cursor = CxaPitchCursor(source.data());
    auto header = CxaPitchHeader();
    header.m_length = cursor.read_uint16();
    header.m_count = cursor.read_uint8();
    header.m_unit = cursor.read_uint8();
    header.m_sequence = cursor.read_uint32();
    return header;
  }

  template<Beam::IsBuffer B>
  void CxaPitchHeader::encode(Beam::Out<B> buffer) const {
    auto encoder = CxaPitchEncoder(Beam::Ref(*buffer));
    encoder.write_uint16(m_length);
    encoder.write_uint8(m_count);
    encoder.write_uint8(m_unit);
    encoder.write_uint32(m_sequence);
  }
}

#endif
