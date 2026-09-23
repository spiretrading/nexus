#ifndef OTC_LINK_RECOVERY_MESSAGES_HPP
#define OTC_LINK_RECOVERY_MESSAGES_HPP
#include <charconv>
#include <concepts>
#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <boost/optional/optional.hpp>
#include "OtcLinkMarketDataFeedClient/OtcLinkParserException.hpp"

namespace Nexus {

  /** A request to replay a range of messages from a live OTC Link channel. */
  struct OtcLinkRecoveryRequest {

    /** The maximum number of messages allowed in one request. */
    static constexpr auto MAXIMUM_COUNT = std::uint32_t(2000);

    /** The subscriber's assigned SenderCompID. */
    std::string m_sender;

    /** The identifier associating the request with its acknowledgement. */
    std::uint64_t m_id;

    /** The real-time channel's RefApplID. */
    std::uint16_t m_channel;

    /** The first message sequence to replay. */
    std::uint32_t m_sequence;

    /** The number of messages to replay. */
    std::uint32_t m_count;

    /** Returns the encoded request, including its checksum. */
    std::string encode() const;
  };

  /** An acknowledgement of an OTC Link recovery request. */
  struct OtcLinkRecoveryResponse {

    /** The server's response to a recovery request. */
    enum class Status {

      /** The requested messages are available for replay. */
      ACCEPTED,

      /** The request exceeds the server's limits. */
      LIMIT_EXCEEDED,

      /** The requested messages are not available. */
      UNAVAILABLE,

      /** The subscriber is not entitled to the channel. */
      NOT_ENTITLED,

      /** The server could not interpret the request. */
      MALFORMED
    };

    /** The recipient's TargetCompID. */
    std::string m_recipient;

    /** The acknowledged request identifier. */
    std::uint64_t m_id;

    /** The requested channel's RefApplID. */
    std::uint16_t m_channel;

    /** The result of the request. */
    Status m_status;

    /** Additional detail supplied by the server. */
    std::string m_text;

    /** The first replayed sequence, present on acceptance. */
    boost::optional<std::uint32_t> m_first_sequence;

    /** The last replayed sequence, present on acceptance. */
    boost::optional<std::uint32_t> m_last_sequence;

    /** Parses a complete acknowledgement and validates its checksum. */
    static OtcLinkRecoveryResponse parse(std::string_view source);
  };

  inline std::string OtcLinkRecoveryRequest::encode() const {
    if(m_sender.empty() || m_sender.find('\x01') != std::string::npos ||
        m_channel == 0 || m_sequence == 0 || m_count == 0 ||
        m_count > MAXIMUM_COUNT ||
        m_count - 1 > std::numeric_limits<std::uint32_t>::max() - m_sequence) {
      boost::throw_with_location(
        OtcLinkParserException("Invalid OTC Link recovery request."));
    }
    auto result = std::format("35=BW\x01" "49={}\x01" "1346={}\x01"
      "1347=0\x01" "1355={}\x01" "1182={}\x01" "1183={}\x01",
      m_sender, m_id, m_channel, m_sequence, m_sequence + m_count - 1);
    auto checksum = 0U;
    for(auto byte : result) {
      checksum += static_cast<unsigned char>(byte);
    }
    result += std::format("10={:03}\x01", checksum % 256);
    return result;
  }

  inline OtcLinkRecoveryResponse OtcLinkRecoveryResponse::parse(
      std::string_view source) {
    auto fail = [] {
      boost::throw_with_location(
        OtcLinkParserException("Invalid OTC Link recovery acknowledgement."));
    };
    auto fields = std::unordered_map<unsigned int, std::string_view>();
    auto checksum = 0U;
    auto has_checksum = false;
    while(!source.empty()) {
      auto end = source.find('\x01');
      if(end == std::string_view::npos) {
        fail();
      }
      auto field = source.substr(0, end);
      auto separator = field.find('=');
      if(separator == std::string_view::npos) {
        fail();
      }
      auto tag = 0U;
      auto number = std::from_chars(
        field.data(), field.data() + separator, tag);
      if(number.ec != std::errc() || number.ptr != field.data() + separator) {
        fail();
      }
      auto value = field.substr(separator + 1);
      if(!fields.emplace(tag, value).second) {
        fail();
      }
      if(tag == 10) {
        auto actual = 0U;
        auto number =
          std::from_chars(value.data(), value.data() + value.size(), actual);
        if(value.size() != 3 || number.ec != std::errc() ||
            number.ptr != value.data() + value.size() ||
            actual != checksum % 256 || end + 1 != source.size()) {
          fail();
        }
        has_checksum = true;
      } else {
        for(auto byte : source.substr(0, end + 1)) {
          checksum += static_cast<unsigned char>(byte);
        }
      }
      source.remove_prefix(end + 1);
    }
    auto text = [&] (unsigned int tag) {
      auto i = fields.find(tag);
      if(i == fields.end() || i->second.empty()) {
        fail();
      }
      return i->second;
    };
    auto integer = [&]<std::unsigned_integral T>(unsigned int tag, T& value) {
      auto field = text(tag);
      auto result =
        std::from_chars(field.data(), field.data() + field.size(), value);
      if(result.ec != std::errc() ||
          result.ptr != field.data() + field.size()) {
        fail();
      }
    };
    if(!has_checksum || text(35) != "BX") {
      fail();
    }
    auto response = OtcLinkRecoveryResponse();
    response.m_recipient = text(59);
    integer(1346, response.m_id);
    integer(1355, response.m_channel);
    auto status = 0U;
    integer(1348, status);
    if(status > static_cast<unsigned int>(Status::MALFORMED) ||
        response.m_channel == 0) {
      fail();
    }
    response.m_status = static_cast<Status>(status);
    auto i = fields.find(58);
    if(i != fields.end()) {
      response.m_text = i->second;
    }
    auto sequence = std::uint32_t();
    if(fields.contains(1182)) {
      integer(1182, sequence);
      response.m_first_sequence = sequence;
    }
    if(fields.contains(1183)) {
      integer(1183, sequence);
      response.m_last_sequence = sequence;
    }
    if(response.m_status == Status::ACCEPTED &&
        (!response.m_first_sequence || !response.m_last_sequence ||
          *response.m_first_sequence == 0 ||
          *response.m_last_sequence < *response.m_first_sequence)) {
      fail();
    }
    return response;
  }
}

#endif
