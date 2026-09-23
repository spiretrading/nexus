#ifndef OTC_LINK_SNAPSHOT_HPP
#define OTC_LINK_SNAPSHOT_HPP
#include <vector>
#include <Beam/IO/SharedBuffer.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkMessage.hpp"

namespace Nexus {

  /** Identifies the contents of an OTC Link spin. */
  enum class OtcLinkSpinType : std::uint8_t {

    /** Security reference data. */
    REFERENCE = 1,

    /** Current market data. */
    MARKET_DATA = 2,

    /** Opening data. */
    OPENING = 3
  };

  /** Marks the beginning of an OTC Link spin. */
  struct OtcLinkSpinStart {
    static constexpr auto TYPE = std::uint8_t(11);

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The contents of the spin. */
    OtcLinkSpinType m_type;

    /** The start time in milliseconds since the UTC epoch. */
    std::uint64_t m_timestamp;

    /** The last live message sequence applied to the spin. */
    std::uint32_t m_last_sequence;

    /** Parses a Start of Spin message. */
    static OtcLinkSpinStart parse(const OtcLinkMessage& message);
  };

  /** Marks the end of an OTC Link spin. */
  struct OtcLinkSpinEnd {
    static constexpr auto TYPE = std::uint8_t(12);

    /** The channel message sequence. */
    std::uint32_t m_sequence;

    /** The contents of the spin. */
    OtcLinkSpinType m_type;

    /** The message count reported by the server. */
    std::uint32_t m_count;

    /** The end time in milliseconds since the UTC epoch. */
    std::uint64_t m_timestamp;

    /** The last live message sequence applied to the spin. */
    std::uint32_t m_last_sequence;

    /** Parses an End of Spin message. */
    static OtcLinkSpinEnd parse(const OtcLinkMessage& message);
  };

  /** A complete snapshot and its live-feed resume position. */
  struct OtcLinkSnapshot {

    /** The first live message sequence to apply after the snapshot. */
    std::uint64_t m_sequence;

    /** The complete encoded messages, excluding spin markers. */
    std::vector<Beam::SharedBuffer> m_messages;
  };

  inline OtcLinkSpinStart OtcLinkSpinStart::parse(
      const OtcLinkMessage& message) {
    if(message.m_type != TYPE) {
      boost::throw_with_location(
        OtcLinkParserException("Expected OTC Link Start of Spin."));
    }
    auto cursor = message.get_cursor();
    auto result = OtcLinkSpinStart();
    result.m_sequence = cursor.read_uint32();
    auto type = cursor.read_uint8();
    if(type < 1 || type > 3) {
      boost::throw_with_location(
        OtcLinkParserException("Invalid OTC Link spin type."));
    }
    result.m_type = static_cast<OtcLinkSpinType>(type);
    result.m_timestamp = cursor.read_uint64();
    result.m_last_sequence = cursor.read_uint32();
    return result;
  }

  inline OtcLinkSpinEnd OtcLinkSpinEnd::parse(const OtcLinkMessage& message) {
    if(message.m_type != TYPE) {
      boost::throw_with_location(
        OtcLinkParserException("Expected OTC Link End of Spin."));
    }
    auto cursor = message.get_cursor();
    auto result = OtcLinkSpinEnd();
    result.m_sequence = cursor.read_uint32();
    auto type = cursor.read_uint8();
    if(type < 1 || type > 3) {
      boost::throw_with_location(
        OtcLinkParserException("Invalid OTC Link spin type."));
    }
    result.m_type = static_cast<OtcLinkSpinType>(type);
    result.m_count = cursor.read_uint32();
    result.m_timestamp = cursor.read_uint64();
    result.m_last_sequence = cursor.read_uint32();
    return result;
  }
}

#endif
