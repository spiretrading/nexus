#ifndef TMX_IP_HEADER_HPP
#define TMX_IP_HEADER_HPP
#include <algorithm>
#include <charconv>
#include <cstdint>
#include <string_view>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpParserException.hpp"

namespace Nexus {

  /** A TMX IP transport header view into its source buffer. */
  struct TmxIpHeader {

    /** The transport header length, excluding framing characters. */
    static constexpr auto LENGTH = std::size_t(22);

    /** Identifies a packet's position within a fragmented message. */
    enum class Continuation : std::uint8_t {

      /** Contains the complete message in one packet. */
      NONE = 0,

      /** Begins a message continued in the next sequential packet. */
      FIRST = 1,

      /** Completes a message begun in preceding sequential packets. */
      LAST = 2,

      /** Continues a message from the previous packet into the next. */
      MIDDLE = 3
    };

    /** The header and payload length, excluding STX and ETX. */
    std::uint16_t m_length;

    /** The packet sequence number, absent for heartbeats. */
    boost::optional<std::uint32_t> m_sequence;

    /** The three-character service identifier. */
    std::string_view m_service;

    /** The originating-market retransmission flag, not customer recovery. */
    char m_retransmission;

    /** The packet's fragmentation indicator. */
    Continuation m_continuation;

    /** The message type: V for a heartbeat, otherwise a space. */
    char m_type;

    /** The originating exchange or consolidated service code. */
    char m_exchange;

    /**
     * Parses the transport header at the start of a buffer.
     * @param source The buffer, excluding the leading STX.
     */
    static TmxIpHeader parse(std::string_view source);

  };

  /** Returns whether a header identifies a heartbeat. */
  inline bool is_heartbeat(const TmxIpHeader& header);

  inline TmxIpHeader TmxIpHeader::parse(std::string_view source) {
    if(source.size() < LENGTH) {
      boost::throw_with_location(
        TmxIpParserException("TMX IP header too short."));
    }
    auto read_text = [&] (std::size_t size) {
      auto value = source.substr(0, size);
      source.remove_prefix(size);
      return value;
    };
    auto parse_number = [] (std::string_view source) {
      auto value = std::uint32_t();
      auto [end, error] =
        std::from_chars(source.data(), source.data() + source.size(), value);
      auto is_invalid =
        error != std::errc() || end != source.data() + source.size();
      if(is_invalid) {
        boost::throw_with_location(
          TmxIpParserException("Invalid TMX IP numeric field."));
      }
      return value;
    };
    constexpr auto LENGTH_WIDTH = 4;
    constexpr auto SEQUENCE_WIDTH = 9;
    constexpr auto SERVICE_WIDTH = 3;
    constexpr auto CODE_WIDTH = 2;
    auto header = TmxIpHeader();
    header.m_length =
      static_cast<std::uint16_t>(parse_number(read_text(LENGTH_WIDTH)));
    if(header.m_length < LENGTH) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP packet length."));
    }
    auto sequence = read_text(SEQUENCE_WIDTH);
    if(sequence.find_first_not_of(' ') != std::string_view::npos) {
      header.m_sequence = parse_number(sequence);
      if(*header.m_sequence == 0) {
        boost::throw_with_location(
          TmxIpParserException("Invalid TMX IP sequence number."));
      }
    }
    header.m_service = read_text(SERVICE_WIDTH);
    auto has_invalid_service =
      std::ranges::any_of(header.m_service, [] (auto character) {
        return character <= ' ' || character > '~';
      });
    if(has_invalid_service) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP service identifier."));
    }
    header.m_retransmission = read_text(1).front();
    auto continuation = read_text(1).front();
    if(continuation < '0' || continuation > '3') {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP continuation indicator."));
    }
    header.m_continuation = static_cast<Continuation>(continuation - '0');
    auto type = read_text(CODE_WIDTH);
    if(type != "  " && type != "V ") {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP message type."));
    }
    header.m_type = type.front();
    if(is_heartbeat(header) == header.m_sequence.has_value()) {
      boost::throw_with_location(
        TmxIpParserException("TMX IP sequence does not match message type."));
    }
    auto is_valid_retransmission = header.m_retransmission == '0' ||
      header.m_retransmission == '1' ||
      (is_heartbeat(header) && header.m_retransmission == ' ');
    if(!is_valid_retransmission) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP retransmission identifier."));
    }
    auto exchange = read_text(CODE_WIDTH);
    auto is_invalid_exchange = exchange.front() <= ' ' ||
      exchange.front() > '~' || exchange.back() != ' ';
    if(is_invalid_exchange) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP exchange identifier."));
    }
    header.m_exchange = exchange.front();
    return header;
  }

  inline bool is_heartbeat(const TmxIpHeader& header) {
    return header.m_type == 'V';
  }
}

#endif
