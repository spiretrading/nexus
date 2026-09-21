#ifndef TMX_IP_RECOVERY_MESSAGES_HPP
#define TMX_IP_RECOVERY_MESSAGES_HPP
#include <algorithm>
#include <charconv>
#include <format>
#include <Beam/IO/Buffer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "TmxIpMarketDataFeedClient/TmxIpPacket.hpp"

namespace Nexus {

  /** Requests retransmission of an inclusive sequence range over TCP. */
  struct TmxIpRecoveryRequest {

    /** The encoded request length, without transport framing. */
    static constexpr auto LENGTH = std::size_t(22);

    /** The first sequence to request. */
    std::uint32_t m_start_sequence;

    /** The last sequence to request. */
    std::uint32_t m_end_sequence;

    /** Appends the request to a buffer. */
    template<Beam::IsBuffer B>
    void encode(Beam::Out<B> buffer) const;
  };

  /** A TCP acknowledgement whose text fields reference the source buffer. */
  struct TmxIpRecoveryResponse {

    /** The response length, without transport framing. */
    static constexpr auto LENGTH = std::size_t(151);

    /** The disposition of a retransmission request. */
    enum class Status {

      /** The request was accepted. */
      ACCEPTED,

      /** The request failed validation. */
      INVALID,

      /** Retransmission is disabled or access was denied. */
      DENIED,

      /** The requested data cannot be supplied. */
      REJECTED
    };

    /** Whether the response code is ACK rather than NACK. */
    bool m_is_acknowledged;

    /** The first sequence to be sent, or zero when no data is sent. */
    std::uint32_t m_start_sequence;

    /** The last sequence to be sent, or zero when no data is sent. */
    std::uint32_t m_end_sequence;

    /** The request's disposition. */
    Status m_status;

    /** Additional error information, without trailing padding. */
    std::string_view m_description;

    /** The echoed request, without trailing padding; may be empty. */
    std::string_view m_request;

    /** Parses an unframed TCP acknowledgement. */
    static TmxIpRecoveryResponse parse(std::string_view source);
  };

  /** Announces the inclusive sequence range of a UDP retransmission. */
  struct TmxIpRecoveryStart {

    /** The control message's type code. */
    static constexpr auto TYPE = std::string_view("HDR  ");

    /** The control payload length. */
    static constexpr auto LENGTH = std::size_t(23);

    /** The first sequence to be sent. */
    std::uint32_t m_start_sequence;

    /** The last sequence to be sent. */
    std::uint32_t m_end_sequence;

    /** Parses a complete retransmission header control packet. */
    static TmxIpRecoveryStart parse(const TmxIpPacket& packet);
  };

  /** Completes a retransmission; status text references the packet buffer. */
  struct TmxIpRecoveryEnd {

    /** The control message's type code. */
    static constexpr auto TYPE = std::string_view("TLR  ");

    /** The control payload length. */
    static constexpr auto LENGTH = std::size_t(123);

    /** The number of packets requested. */
    std::uint32_t m_requested_count;

    /** The number of packets sent. */
    std::uint32_t m_sent_count;

    /** Optional status information, without trailing padding. */
    std::string_view m_status;

    /** Parses a complete retransmission trailer control packet. */
    static TmxIpRecoveryEnd parse(const TmxIpPacket& packet);
  };

  /** Reports interrupted recovery; its description references the packet. */
  struct TmxIpRecoveryError {

    /** The control message's type code. */
    static constexpr auto TYPE = std::string_view("ERROR");

    /** The control payload length. */
    static constexpr auto LENGTH = std::size_t(113);

    /** The reason retransmission was interrupted. */
    enum class Code {

      /** The server canceled the retransmission. */
      CANCELED,

      /** An internal failure interrupted retransmission. */
      FAILED
    };

    /** The reason for the interruption. */
    Code m_code;

    /** Additional error information, without trailing padding. */
    std::string_view m_description;

    /** Parses a complete retransmission error control packet. */
    static TmxIpRecoveryError parse(const TmxIpPacket& packet);
  };

  /** Advertises the recovery server's availability and request limit. */
  struct TmxIpRecoveryHeartbeat {

    /** The control message's type code. */
    static constexpr auto TYPE = std::string_view("HBEAT");

    /** The control payload length. */
    static constexpr auto LENGTH = std::size_t(77);

    /** The UTC time the heartbeat was sent. */
    boost::posix_time::ptime m_timestamp;

    /** The hostname, referencing the packet's payload. */
    std::string_view m_host;

    /** The service version, referencing the packet's payload. */
    std::string_view m_version;

    /** The maximum number of packets permitted in a request. */
    std::uint32_t m_maximum_count;

    /** Parses a recovery server heartbeat. */
    static TmxIpRecoveryHeartbeat parse(const TmxIpPacket& packet);
  };

  namespace Details {
    inline constexpr auto TMX_IP_RECOVERY_SEQUENCE_WIDTH = std::size_t(9);

    inline std::uint32_t parse_tmx_ip_recovery_number(std::string_view source) {
      auto value = std::uint32_t();
      auto [end, error] =
        std::from_chars(source.data(), source.data() + source.size(), value);
      auto is_invalid =
        error != std::errc() || end != source.data() + source.size();
      if(is_invalid) {
        boost::throw_with_location(
          TmxIpParserException("Invalid TMX IP recovery number."));
      }
      return value;
    }

    inline std::string_view parse_tmx_ip_recovery_text(
        std::string_view source) {
    auto is_invalid = std::ranges::any_of(source, [] (auto character) {
        return character < ' ' || character > '~';
      });
      if(is_invalid) {
        boost::throw_with_location(
          TmxIpParserException("Invalid TMX IP recovery text."));
      }
      return source.substr(0, source.find_last_not_of(' ') + 1);
    }

    inline std::string_view parse_tmx_ip_control_payload(
        const TmxIpPacket& packet, std::string_view type, std::size_t length) {
      if(!is_recovery_control(packet.m_header) ||
          packet.m_header.m_continuation != TmxIpHeader::Continuation::NONE ||
          packet.m_payload.size() != length ||
          !packet.m_payload.starts_with(type)) {
        boost::throw_with_location(
          TmxIpParserException("Invalid TMX IP recovery control message."));
      }
      return packet.m_payload.substr(type.size());
    }
  }

  template<Beam::IsBuffer B>
  void TmxIpRecoveryRequest::encode(Beam::Out<B> buffer) const {
    constexpr auto MAXIMUM_SEQUENCE = std::uint32_t(999999999);
    if(m_start_sequence == 0 || m_end_sequence < m_start_sequence ||
        m_end_sequence > MAXIMUM_SEQUENCE) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP recovery range."));
    }
    auto source =
      std::format("SEQN{:09}{:09}", m_start_sequence, m_end_sequence);
    Beam::append(*buffer, source.data(), source.size());
  }

  inline TmxIpRecoveryResponse TmxIpRecoveryResponse::parse(
      std::string_view source) {
    if(source.size() != LENGTH) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP recovery response length."));
    }
    auto read_text = [&] (std::size_t size) {
      auto value = source.substr(0, size);
      source.remove_prefix(size);
      return value;
    };
    auto response = TmxIpRecoveryResponse();
    constexpr auto RESPONSE_WIDTH = 4;
    auto code = read_text(RESPONSE_WIDTH);
    if(code != "ACK " && code != "NACK") {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP acknowledgement code."));
    }
    response.m_is_acknowledged = code == "ACK ";
    response.m_start_sequence = Details::parse_tmx_ip_recovery_number(
      read_text(Details::TMX_IP_RECOVERY_SEQUENCE_WIDTH));
    response.m_end_sequence = Details::parse_tmx_ip_recovery_number(
      read_text(Details::TMX_IP_RECOVERY_SEQUENCE_WIDTH));
    auto is_invalid_range =
      (response.m_start_sequence == 0) != (response.m_end_sequence == 0) ||
      response.m_start_sequence > response.m_end_sequence;
    if(is_invalid_range) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP recovery response range."));
    }
    constexpr auto STATUS_WIDTH = 8;
    auto status = read_text(STATUS_WIDTH);
    if(status == "ACCEPTED") {
      response.m_status = Status::ACCEPTED;
    } else if(status == "INVALID ") {
      response.m_status = Status::INVALID;
    } else if(status == "DENIED  ") {
      response.m_status = Status::DENIED;
    } else if(status == "REJECTED") {
      response.m_status = Status::REJECTED;
    } else {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP recovery response status."));
    }
    constexpr auto DESCRIPTION_WIDTH = 99;
    response.m_description =
      Details::parse_tmx_ip_recovery_text(read_text(DESCRIPTION_WIDTH));
    response.m_request = Details::parse_tmx_ip_recovery_text(source);
    return response;
  }

  inline TmxIpRecoveryStart TmxIpRecoveryStart::parse(
      const TmxIpPacket& packet) {
    auto source = Details::parse_tmx_ip_control_payload(packet, TYPE, LENGTH);
    auto start = Details::parse_tmx_ip_recovery_number(
      source.substr(0, Details::TMX_IP_RECOVERY_SEQUENCE_WIDTH));
    auto end = Details::parse_tmx_ip_recovery_number(
      source.substr(Details::TMX_IP_RECOVERY_SEQUENCE_WIDTH));
    if(start == 0 || end < start) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP recovery start range."));
    }
    return TmxIpRecoveryStart(start, end);
  }

  inline TmxIpRecoveryEnd TmxIpRecoveryEnd::parse(const TmxIpPacket& packet) {
    auto source = Details::parse_tmx_ip_control_payload(packet, TYPE, LENGTH);
    auto requested_count = Details::parse_tmx_ip_recovery_number(
      source.substr(0, Details::TMX_IP_RECOVERY_SEQUENCE_WIDTH));
    source.remove_prefix(Details::TMX_IP_RECOVERY_SEQUENCE_WIDTH);
    auto sent_count = Details::parse_tmx_ip_recovery_number(
      source.substr(0, Details::TMX_IP_RECOVERY_SEQUENCE_WIDTH));
    source.remove_prefix(Details::TMX_IP_RECOVERY_SEQUENCE_WIDTH);
    if(sent_count > requested_count) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP recovery packet counts."));
    }
    return TmxIpRecoveryEnd(
      requested_count, sent_count, Details::parse_tmx_ip_recovery_text(source));
  }

  inline TmxIpRecoveryError TmxIpRecoveryError::parse(
      const TmxIpPacket& packet) {
    auto source = Details::parse_tmx_ip_control_payload(packet, TYPE, LENGTH);
    constexpr auto CODE_WIDTH = 8;
    auto code = source.substr(0, CODE_WIDTH);
    source.remove_prefix(CODE_WIDTH);
    auto error = TmxIpRecoveryError();
    if(code == "CANCELED") {
      error.m_code = Code::CANCELED;
    } else if(code == "FAILED  ") {
      error.m_code = Code::FAILED;
    } else {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP recovery error code."));
    }
    error.m_description = Details::parse_tmx_ip_recovery_text(source);
    return error;
  }
  inline TmxIpRecoveryHeartbeat TmxIpRecoveryHeartbeat::parse(
      const TmxIpPacket& packet) {
    auto source = Details::parse_tmx_ip_control_payload(packet, TYPE, LENGTH);
    auto is_invalid = !source.starts_with("[HEARTBEAT ") ||
      source[15] != '-' || source[18] != '-' || source[21] != ' ' ||
      source[24] != ':' || source[27] != ':' || source[30] != '-' ||
      source[43] != '.' || source[50] != ']';
    if(is_invalid) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP recovery heartbeat."));
    }
    auto year = Details::parse_tmx_ip_recovery_number(source.substr(11, 4));
    auto month = Details::parse_tmx_ip_recovery_number(source.substr(16, 2));
    auto day = Details::parse_tmx_ip_recovery_number(source.substr(19, 2));
    auto hour = Details::parse_tmx_ip_recovery_number(source.substr(22, 2));
    auto minute = Details::parse_tmx_ip_recovery_number(source.substr(25, 2));
    auto second = Details::parse_tmx_ip_recovery_number(source.substr(28, 2));
    try {
      boost::gregorian::date(year, month, day);
    } catch(const std::exception&) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP recovery heartbeat date."));
    }
    if(hour > 23 || minute > 59 || second > 59) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP recovery heartbeat time."));
    }
    auto seconds = std::uint64_t();
    auto text = source.substr(31, 12);
    auto [end, error] =
      std::from_chars(text.data(), text.data() + text.size(), seconds);
    constexpr auto MAXIMUM_SECONDS = std::uint64_t(253402300799);
    if(error != std::errc() || end != text.data() + text.size() ||
        seconds > MAXIMUM_SECONDS) {
      boost::throw_with_location(
        TmxIpParserException("Invalid TMX IP recovery heartbeat timestamp."));
    }
    auto microseconds =
      Details::parse_tmx_ip_recovery_number(source.substr(44, 6));
    static const auto EPOCH =
      boost::posix_time::ptime(boost::gregorian::date(1970, 1, 1));
    auto heartbeat = TmxIpRecoveryHeartbeat();
    heartbeat.m_timestamp = EPOCH + boost::posix_time::seconds(seconds) +
      boost::posix_time::microseconds(microseconds);
    heartbeat.m_host =
      Details::parse_tmx_ip_recovery_text(source.substr(51, 8));
    heartbeat.m_version =
      Details::parse_tmx_ip_recovery_text(source.substr(59, 4));
    heartbeat.m_maximum_count =
      Details::parse_tmx_ip_recovery_number(source.substr(63, 9));
    return heartbeat;
  }

}

#endif
