#ifndef CXA_PITCH_HEADER_HPP
#define CXA_PITCH_HEADER_HPP
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <boost/throw_exception.hpp>
#include "CxaPitchMarketDataFeedClient/CxaPitchCursor.hpp"
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
}

#endif
