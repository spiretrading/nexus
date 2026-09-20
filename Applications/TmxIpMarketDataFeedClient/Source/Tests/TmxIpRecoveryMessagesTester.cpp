#include <format>
#include <Beam/IO/SharedBuffer.hpp>
#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpMessageBuilder.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpRecoveryMessages.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpSequencer.hpp"

using namespace Beam;
using namespace Nexus;

namespace {
  std::string encode_control(std::string_view payload) {
    return std::format("{}{:04}         CDF 0  T {}{}", TmxIpPacket::START,
      TmxIpHeader::LENGTH + payload.size(), payload, TmxIpPacket::END);
  }
}

TEST_SUITE("TmxIpRecoveryMessages") {
  TEST_CASE("request") {
    auto buffer = from<SharedBuffer>("prefix");
    TmxIpRecoveryRequest(1, 999999999).encode(out(buffer));
    auto source = std::string_view(buffer.get_data(), buffer.get_size());
    REQUIRE(source == "prefixSEQN000000001999999999");
    REQUIRE(
      source.size() == sizeof("prefix") - 1 + TmxIpRecoveryRequest::LENGTH);
    reset(buffer);
    TmxIpRecoveryRequest(123, 123).encode(out(buffer));
    REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) ==
      "SEQN000000123000000123");
  }

  TEST_CASE("invalid_request") {
    for(auto request : {TmxIpRecoveryRequest(0, 1),
        TmxIpRecoveryRequest(2, 1), TmxIpRecoveryRequest(1, 1000000000),
        TmxIpRecoveryRequest(1000000000, 1000000000)}) {
      auto buffer = from<SharedBuffer>("prefix");
      REQUIRE_THROWS_AS(request.encode(out(buffer)), TmxIpParserException);
      REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) ==
        "prefix");
    }
  }

  TEST_CASE("response") {
    auto source = std::format("ACK 000000001000000100ACCEPTED{:99}{}",
      "", "SEQN000000001000000100");
    auto response = TmxIpRecoveryResponse::parse(source);
    REQUIRE(response.m_is_acknowledged);
    REQUIRE(response.m_start_sequence == 1);
    REQUIRE(response.m_end_sequence == 100);
    REQUIRE(response.m_status == TmxIpRecoveryResponse::Status::ACCEPTED);
    REQUIRE(response.m_description.empty());
    REQUIRE(response.m_request == "SEQN000000001000000100");
    REQUIRE(response.m_request.data() ==
      source.data() + source.size() - TmxIpRecoveryRequest::LENGTH);
    SUBCASE("empty") {
      source = std::format("ACK 000000000000000000ACCEPTED{:99}{:22}", "", "");
      response = TmxIpRecoveryResponse::parse(source);
      REQUIRE(response.m_is_acknowledged);
      REQUIRE(response.m_start_sequence == 0);
      REQUIRE(response.m_end_sequence == 0);
      REQUIRE(response.m_status == TmxIpRecoveryResponse::Status::ACCEPTED);
      REQUIRE(response.m_request.empty());
    }
    SUBCASE("negative") {
      for(auto status : {"INVALID ", "DENIED  ", "REJECTED"}) {
        source = std::format("NACK000000000000000000{}{:99}{:22}",
          status, "ERR006: Retransmissions are disabled.", "");
        response = TmxIpRecoveryResponse::parse(source);
        REQUIRE(!response.m_is_acknowledged);
        REQUIRE(response.m_start_sequence == 0);
        REQUIRE(response.m_end_sequence == 0);
        REQUIRE(
          response.m_description == "ERR006: Retransmissions are disabled.");
        REQUIRE(response.m_request.empty());
        if(std::string_view(status) == "INVALID ") {
          REQUIRE(response.m_status == TmxIpRecoveryResponse::Status::INVALID);
        } else if(std::string_view(status) == "DENIED  ") {
          REQUIRE(response.m_status == TmxIpRecoveryResponse::Status::DENIED);
        } else {
          REQUIRE(response.m_status == TmxIpRecoveryResponse::Status::REJECTED);
        }
      }
    }
  }

  TEST_CASE("malformed_response") {
    auto source = std::format("ACK 000000001000000100ACCEPTED{:99}{:22}",
      "", "SEQN000000001000000100");
    for(auto size = std::size_t(0); size < source.size(); ++size) {
      REQUIRE_THROWS_AS(TmxIpRecoveryResponse::parse(
        std::string_view(source).substr(0, size)), TmxIpParserException);
    }
    REQUIRE_THROWS_AS(
      TmxIpRecoveryResponse::parse(source + "X"), TmxIpParserException);
    for(auto prefix : {"BAD 000000001000000100ACCEPTED",
        "ACK 000000001000000100UNKNOWN ",
        "ACK +00000001000000100ACCEPTED",
        "ACK 00000010X000000100ACCEPTED",
        "ACK 000000101000000100ACCEPTED",
        "ACK 000000000000000100ACCEPTED",
        "ACK 000000001000000000ACCEPTED"}) {
      source = std::format("{}{:99}{:22}", prefix, "", "");
      REQUIRE_THROWS_AS(
        TmxIpRecoveryResponse::parse(source), TmxIpParserException);
    }
    source = std::format("ACK 000000001000000100ACCEPTED{:99}{:22}",
      "bad\ntext", "SEQN000000001000000100");
    REQUIRE_THROWS_AS(
      TmxIpRecoveryResponse::parse(source), TmxIpParserException);
  }

  TEST_CASE("start") {
    auto source = encode_control("HDR  000000001999999999");
    auto packet = TmxIpPacket::parse(source);
    REQUIRE(is_recovery_control(packet.m_header));
    auto start = TmxIpRecoveryStart::parse(packet);
    REQUIRE(start.m_start_sequence == 1);
    REQUIRE(start.m_end_sequence == 999999999);
    REQUIRE_THROWS_AS(parse_message(packet), TmxIpParserException);
    for(auto payload : {"HDR  000000000000000001", "HDR  000000002000000001",
        "HDR  00000000X000000001", "HDR  000000001+00000001"}) {
      source = encode_control(payload);
      REQUIRE_THROWS_AS(TmxIpRecoveryStart::parse(TmxIpPacket::parse(source)),
        TmxIpParserException);
    }
  }

  TEST_CASE("end") {
    auto source = encode_control(std::format(
      "TLR  000000100000000090{:100}", "Maximum request size exceeded."));
    auto packet = TmxIpPacket::parse(source);
    auto end = TmxIpRecoveryEnd::parse(packet);
    REQUIRE(end.m_requested_count == 100);
    REQUIRE(end.m_sent_count == 90);
    REQUIRE(end.m_status == "Maximum request size exceeded.");
    REQUIRE(end.m_status.data() == packet.m_payload.data() +
      sizeof("TLR  000000100000000090") - 1);
    source = encode_control(std::format("TLR  000000000000000000{:100}", ""));
    end = TmxIpRecoveryEnd::parse(TmxIpPacket::parse(source));
    REQUIRE(end.m_requested_count == 0);
    REQUIRE(end.m_sent_count == 0);
    REQUIRE(end.m_status.empty());
    for(auto prefix : {"TLR  000000001000000002", "TLR  00000000X000000000",
        "TLR  000000001-00000001"}) {
      source = encode_control(std::format("{}{:100}", prefix, ""));
      REQUIRE_THROWS_AS(TmxIpRecoveryEnd::parse(TmxIpPacket::parse(source)),
        TmxIpParserException);
    }
  }

  TEST_CASE("error") {
    auto source = encode_control(
      std::format("ERRORCANCELED{:100}", "Retransmission canceled."));
    auto packet = TmxIpPacket::parse(source);
    auto error = TmxIpRecoveryError::parse(packet);
    REQUIRE(error.m_code == TmxIpRecoveryError::Code::CANCELED);
    REQUIRE(error.m_description == "Retransmission canceled.");
    REQUIRE(error.m_description.data() ==
      packet.m_payload.data() + sizeof("ERRORCANCELED") - 1);
    source = encode_control(std::format("ERRORFAILED  {:100}", ""));
    error = TmxIpRecoveryError::parse(TmxIpPacket::parse(source));
    REQUIRE(error.m_code == TmxIpRecoveryError::Code::FAILED);
    REQUIRE(error.m_description.empty());
    source = encode_control(std::format("ERRORUNKNOWN {:100}", ""));
    REQUIRE_THROWS_AS(TmxIpRecoveryError::parse(TmxIpPacket::parse(source)),
      TmxIpParserException);
  }

  TEST_CASE_TEMPLATE("malformed_control", T, TmxIpRecoveryStart,
      TmxIpRecoveryEnd, TmxIpRecoveryError) {
    auto payload = [] {
      if constexpr(std::same_as<T, TmxIpRecoveryStart>) {
        return std::string("HDR  000000001000000100");
      } else if constexpr(std::same_as<T, TmxIpRecoveryEnd>) {
        return std::format("TLR  000000100000000100{:100}", "");
      } else {
        return std::format("ERRORFAILED  {:100}", "");
      }
    }();
    auto source = encode_control(payload);
    auto packet = TmxIpPacket::parse(source);
    REQUIRE_NOTHROW(T::parse(packet));
    for(auto size = std::size_t(0); size < payload.size(); ++size) {
      packet.m_payload = std::string_view(payload).substr(0, size);
      REQUIRE_THROWS_AS(T::parse(packet), TmxIpParserException);
    }
    packet.m_payload = payload;
    SUBCASE("type") {
      payload.front() = 'X';
    }
    SUBCASE("length") {
      payload += 'X';
      packet.m_payload = payload;
    }
    SUBCASE("fragment") {
      packet.m_header.m_continuation = TmxIpHeader::Continuation::FIRST;
    }
    SUBCASE("business_packet") {
      packet.m_header.m_sequence = 1;
    }
    SUBCASE("heartbeat") {
      packet.m_header.m_type = 'V';
    }
    REQUIRE_THROWS_AS(T::parse(packet), TmxIpParserException);
  }

  TEST_CASE("control_interleaving") {
    auto sequencer = TmxIpSequencer();
    auto builder = TmxIpMessageBuilder();
    auto source = encode_control("HDR  000000001000000003");
    auto control = TmxIpPacket::parse(source);
    sequencer.add(control);
    REQUIRE(!sequencer.get_sequence().has_value());
    REQUIRE(!sequencer.read().has_value());
    REQUIRE(!builder.add(control).has_value());
    sequencer.add(
      TmxIpPacket::parse("\x02" "0027000000001CDF01  T \x01\x1e" "1=H\x03"));
    auto packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(!builder.add(*packet).has_value());
    sequencer.add(TmxIpPacket::parse("\x02" "0024000000003CDF02  T X\x1d\x03"));
    sequencer.add(control);
    REQUIRE(!builder.add(control).has_value());
    auto gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 2);
    REQUIRE(gap->m_count == 1);
    REQUIRE(!sequencer.read().has_value());
    sequencer.add(
      TmxIpPacket::parse("\x02" "0029000000002CDF03  T \x1c\x1e" "55=AB\x03"));
    packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(!builder.add(*packet).has_value());
    packet = sequencer.read();
    REQUIRE(packet.has_value());
    auto message = builder.add(*packet);
    REQUIRE(message.has_value());
    REQUIRE(std::string_view(message->get_data(), message->get_size()) ==
      "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d");
  }
  TEST_CASE("recovery_heartbeat") {
    auto source = encode_control(
      "HBEAT[HEARTBEAT 2012-10-10 03:25:02-001349853902.844623]"
      "TDOTDR  00.1000010000");
    auto heartbeat = TmxIpRecoveryHeartbeat::parse(TmxIpPacket::parse(source));
    REQUIRE(heartbeat.m_timestamp ==
      boost::posix_time::time_from_string("2012-10-10 07:25:02.844623"));
    REQUIRE(heartbeat.m_host == "TDOTDR");
    REQUIRE(heartbeat.m_version == "00.1");
    REQUIRE(heartbeat.m_maximum_count == 10000);
  }

  TEST_CASE("malformed_recovery_heartbeat") {
    auto payload = std::string(
      "HBEAT[HEARTBEAT 2012-10-10 03:25:02-001349853902.844623]"
      "TDOTDR  00.1000010000");
    constexpr auto PREFIX_SIZE = TmxIpRecoveryHeartbeat::TYPE.size();
    SUBCASE("truncated") {
      payload.pop_back();
    }
    SUBCASE("delimiter") {
      payload[PREFIX_SIZE + 30] = ' ';
    }
    SUBCASE("date") {
      payload.replace(PREFIX_SIZE + 16, 2, "00");
    }
    SUBCASE("time") {
      payload.replace(PREFIX_SIZE + 22, 2, "99");
    }
    SUBCASE("epoch") {
      payload.replace(PREFIX_SIZE + 31, 12, "999999999999");
    }
    SUBCASE("limit") {
      payload.back() = 'x';
    }
    auto source = encode_control(payload);
    REQUIRE_THROWS_AS(TmxIpRecoveryHeartbeat::parse(TmxIpPacket::parse(source)),
      TmxIpParserException);
  }

}
