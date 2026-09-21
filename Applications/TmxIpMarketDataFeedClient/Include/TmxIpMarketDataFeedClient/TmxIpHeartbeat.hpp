#ifndef TMX_IP_HEARTBEAT_HPP
#define TMX_IP_HEARTBEAT_HPP
#include <charconv>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpPacket.hpp"

namespace Nexus {

  /** A circuit assurance message from a TMX IP stream. */
  struct TmxIpHeartbeat {

    /** The heartbeat payload length. */
    static constexpr auto LENGTH = std::size_t(185);

    /** The UTC time the heartbeat was sent. */
    boost::posix_time::ptime m_timestamp;

    /** The last broadcast sequence, or zero before the first packet. */
    std::uint32_t m_last_sequence;

    /** The UTC timestamp reported for the last broadcast packet. */
    boost::posix_time::ptime m_last_timestamp;

    /** The last broadcast sequence reported by the previous heartbeat. */
    std::uint32_t m_previous_sequence;

    /** The UTC timestamp reported in the previous heartbeat. */
    boost::posix_time::ptime m_previous_timestamp;

    /** The originating hostname, referencing the packet's payload. */
    std::string_view m_host;

    /** The service version, referencing the packet's payload. */
    std::string_view m_version;

    /** Parses a complete heartbeat packet. */
    static TmxIpHeartbeat parse(const TmxIpPacket& packet);
  };

  inline TmxIpHeartbeat TmxIpHeartbeat::parse(const TmxIpPacket& packet) {
    if(!is_heartbeat(packet.m_header) ||
        packet.m_header.m_continuation != TmxIpHeader::Continuation::NONE ||
        packet.m_payload.size() != LENGTH) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP heartbeat."));
    }
    auto source = packet.m_payload;
    auto read_text = [&] (std::size_t size) {
      auto value = source.substr(0, size);
      source.remove_prefix(size);
      return value;
    };
    auto read_literal = [&] (std::string_view value) {
      if(read_text(value.size()) != value) {
        boost::throw_with_location(
          TmxIpParserException("Invalid TMX IP heartbeat delimiter."));
      }
    };
    auto read_number = [&] (std::size_t size) {
      auto text = read_text(size);
      auto value = std::uint64_t();
      auto [end, error] =
        std::from_chars(text.data(), text.data() + text.size(), value);
      auto is_invalid =
        error != std::errc() || end != text.data() + text.size();
      if(is_invalid) {
        boost::throw_with_location(
          TmxIpParserException("Invalid TMX IP heartbeat number."));
      }
      return value;
    };
    auto read_time = [&] {
      auto hour = read_number(2);
      read_literal(":");
      auto minute = read_number(2);
      read_literal(":");
      auto second = read_number(2);
      if(hour > 23 || minute > 59 || second > 59) {
        boost::throw_with_location(
          TmxIpParserException("Invalid TMX IP heartbeat time."));
      }
    };
    auto read_timestamp = [&] {
      constexpr auto SECONDS_WIDTH = 12;
      constexpr auto MICROSECONDS_WIDTH = 6;
      constexpr auto MAXIMUM_SECONDS = std::uint64_t(253402300799);
      static const auto EPOCH =
        boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));
      auto seconds = read_number(SECONDS_WIDTH);
      read_literal(".");
      auto microseconds = read_number(MICROSECONDS_WIDTH);
      if(seconds > MAXIMUM_SECONDS) {
        boost::throw_with_location(
          TmxIpParserException("TMX IP heartbeat timestamp out of range."));
      }
      return EPOCH + boost::posix_time::seconds(seconds) +
        boost::posix_time::microseconds(microseconds);
    };
    auto read_trimmed = [&] (std::size_t size) {
      auto text = read_text(size);
      return text.substr(0, text.find_last_not_of(' ') + 1);
    };
    read_literal("[HEARTBEAT ");
    auto year = read_number(4);
    read_literal("-");
    auto month = read_number(2);
    read_literal("-");
    auto day = read_number(2);
    try {
      boost::gregorian::date(year, month, day);
    } catch(const std::exception&) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP heartbeat date."));
    }
    read_literal(" ");
    read_time();
    read_literal("-");
    auto heartbeat = TmxIpHeartbeat();
    heartbeat.m_timestamp = read_timestamp();
    constexpr auto SEQUENCE_WIDTH = 9;
    read_literal("][LAST SENT ");
    heartbeat.m_last_sequence =
      static_cast<std::uint32_t>(read_number(SEQUENCE_WIDTH));
    read_literal("-");
    read_time();
    read_literal("-");
    heartbeat.m_last_timestamp = read_timestamp();
    read_literal("][LAST HB   ");
    heartbeat.m_previous_sequence =
      static_cast<std::uint32_t>(read_number(SEQUENCE_WIDTH));
    read_literal("-");
    read_time();
    read_literal("-");
    heartbeat.m_previous_timestamp = read_timestamp();
    read_literal("]");
    constexpr auto RESERVED_WIDTH = 22;
    constexpr auto HOST_WIDTH = 8;
    constexpr auto VERSION_WIDTH = 4;
    read_text(RESERVED_WIDTH);
    heartbeat.m_host = read_trimmed(HOST_WIDTH);
    heartbeat.m_version = read_trimmed(VERSION_WIDTH);
    return heartbeat;
  }
}

#endif
