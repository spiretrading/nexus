#ifndef OTC_LINK_HEADER_HPP
#define OTC_LINK_HEADER_HPP
#include "OtcLinkMarketDataFeedClient/OtcLinkCursor.hpp"

namespace Nexus {

  /** An OTC Link multicast packet header. */
  struct OtcLinkHeader {

    /** The length of a packet header. */
    static constexpr auto LENGTH = std::size_t(12);

    /** Identifies packet attributes. */
    enum class Flag : std::uint8_t {

      /** An idle channel's next expected sequence number. */
      HEARTBEAT = 0x01,

      /** A channel sequence restart notification. */
      SEQUENCE_RESET = 0x02,

      /** Replayed messages. */
      REPLAY = 0x40,

      /** Test messages. */
      TEST = 0x80
    };

    /** The total packet length, including this header. */
    std::uint16_t m_length;

    /** The channel-specific packet sequence value. */
    std::uint32_t m_sequence;

    /** The packet's attribute bits. */
    std::uint8_t m_flags;

    /** The number of messages following this header. */
    std::uint8_t m_count;

    /** Milliseconds since midnight in New York local time. */
    std::uint32_t m_milliseconds;

    /**
     * Parses a packet header.
     * @param source The buffer beginning with the header.
     */
    static OtcLinkHeader parse(std::string_view source);

    /** Returns whether a packet attribute is set. */
    bool has_flag(Flag flag) const;
  };

  inline OtcLinkHeader OtcLinkHeader::parse(std::string_view source) {
    auto cursor = OtcLinkCursor(source);
    auto header = OtcLinkHeader();
    header.m_length = cursor.read_uint16();
    header.m_sequence = cursor.read_uint32();
    header.m_flags = cursor.read_uint8();
    header.m_count = cursor.read_uint8();
    header.m_milliseconds = cursor.read_uint32();
    if(header.m_length < LENGTH) {
      boost::throw_with_location(
        OtcLinkParserException("OTC Link packet length too short."));
    }
    return header;
  }

  inline bool OtcLinkHeader::has_flag(Flag flag) const {
    return (m_flags & static_cast<std::uint8_t>(flag)) != 0;
  }
}

#endif
