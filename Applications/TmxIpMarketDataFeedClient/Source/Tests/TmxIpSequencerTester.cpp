#include <format>
#include <string>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpMessageBuilder.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpSequencer.hpp"

using namespace Nexus;

namespace {
  std::string encode_packet(std::uint32_t sequence, std::string_view payload) {
    return std::format("{}{:04}{:09}CDF00  T {}{}", TmxIpPacket::START,
      TmxIpHeader::LENGTH + payload.size(), sequence, payload,
      TmxIpPacket::END);
  }
}

TEST_SUITE("TmxIpSequencer") {
  TEST_CASE("packet_sequence") {
    auto sequencer = TmxIpSequencer();
    REQUIRE(!sequencer.get_sequence().has_value());
    REQUIRE(!sequencer.get_gap().has_value());
    REQUIRE(!sequencer.read().has_value());
    sequencer.add(TmxIpPacket::parse(encode_packet(100, "first")));
    sequencer.add(TmxIpPacket::parse(encode_packet(101, "second")));
    REQUIRE(sequencer.get_sequence() == std::uint32_t(100));
    REQUIRE(!sequencer.get_gap().has_value());
    auto packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(packet->m_header.m_sequence == std::uint32_t(100));
    REQUIRE(packet->m_payload == "first");
    packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(packet->m_header.m_sequence == std::uint32_t(101));
    REQUIRE(packet->m_payload == "second");
    REQUIRE(sequencer.get_sequence() == std::uint32_t(102));
    REQUIRE(!sequencer.get_gap().has_value());
    REQUIRE(!sequencer.read().has_value());
  }

  TEST_CASE("gap_recovery") {
    auto sequencer = TmxIpSequencer();
    sequencer.reset(100);
    sequencer.add(TmxIpPacket::parse(encode_packet(105, "last")));
    sequencer.add(TmxIpPacket::parse(encode_packet(103, "third")));
    auto gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 100);
    REQUIRE(gap->m_count == 3);
    REQUIRE(!sequencer.read().has_value());
    sequencer.add(TmxIpPacket::parse(encode_packet(101, "second")));
    gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 100);
    REQUIRE(gap->m_count == 1);
    sequencer.add(TmxIpPacket::parse(encode_packet(100, "first")));
    REQUIRE(!sequencer.get_gap().has_value());
    for(auto sequence : {100, 101}) {
      auto packet = sequencer.read();
      REQUIRE(packet.has_value());
      REQUIRE(packet->m_header.m_sequence == std::uint32_t(sequence));
    }
    gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 102);
    REQUIRE(gap->m_count == 1);
    sequencer.add(TmxIpPacket::parse(encode_packet(104, "fourth")));
    sequencer.add(TmxIpPacket::parse(encode_packet(102, "recovered")));
    for(auto sequence : {102, 103, 104, 105}) {
      auto packet = sequencer.read();
      REQUIRE(packet.has_value());
      REQUIRE(packet->m_header.m_sequence == std::uint32_t(sequence));
    }
    REQUIRE(!sequencer.get_gap().has_value());
    REQUIRE(!sequencer.read().has_value());
  }

  TEST_CASE("duplicates") {
    auto sequencer = TmxIpSequencer();
    sequencer.add(TmxIpPacket::parse(encode_packet(100, "original")));
    sequencer.add(TmxIpPacket::parse(encode_packet(100, "duplicate")));
    sequencer.add(TmxIpPacket::parse(encode_packet(99, "old")));
    auto packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(packet->m_payload == "original");
    sequencer.add(TmxIpPacket::parse(encode_packet(100, "late")));
    REQUIRE(!sequencer.read().has_value());
    REQUIRE(!sequencer.get_gap().has_value());
    REQUIRE(sequencer.get_sequence() == std::uint32_t(101));
    sequencer.add(TmxIpPacket::parse(encode_packet(102, "next")));
    sequencer.add(TmxIpPacket::parse(encode_packet(102, "duplicate")));
    sequencer.add(TmxIpPacket::parse(encode_packet(101, "recovered")));
    packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(packet->m_payload == "recovered");
    packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(packet->m_payload == "next");
    REQUIRE(!sequencer.read().has_value());
  }

  TEST_CASE("packet_ownership") {
    auto sequencer = TmxIpSequencer();
    auto source = std::string("\x02" "0027000000100CDF13  T first\x03");
    sequencer.add(TmxIpPacket::parse(source));
    source.assign(source.size(), 'X');
    auto packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(packet->m_header.m_sequence == std::uint32_t(100));
    REQUIRE(packet->m_header.m_service == "CDF");
    REQUIRE(packet->m_header.m_exchange == 'T');
    REQUIRE(packet->m_header.m_retransmission == '1');
    REQUIRE(
      packet->m_header.m_continuation == TmxIpHeader::Continuation::MIDDLE);
    REQUIRE(packet->m_payload == "first");
    sequencer.add(TmxIpPacket::parse(encode_packet(101, "second")));
    sequencer.reset();
    REQUIRE(packet->m_header.m_service == "CDF");
    REQUIRE(packet->m_payload == "first");
  }

  TEST_CASE("sequence_wrap") {
    constexpr auto MAXIMUM_SEQUENCE = std::uint32_t(999999999);
    auto sequencer = TmxIpSequencer();
    sequencer.reset(MAXIMUM_SEQUENCE - 1);
    sequencer.add(TmxIpPacket::parse(encode_packet(2, "after")));
    auto gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == MAXIMUM_SEQUENCE - 1);
    REQUIRE(gap->m_count == 2);
    sequencer.add(TmxIpPacket::parse(encode_packet(MAXIMUM_SEQUENCE, "last")));
    gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == MAXIMUM_SEQUENCE - 1);
    REQUIRE(gap->m_count == 1);
    sequencer.add(
      TmxIpPacket::parse(encode_packet(MAXIMUM_SEQUENCE - 1, "before")));
    auto packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(packet->m_header.m_sequence == MAXIMUM_SEQUENCE - 1);
    packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(packet->m_header.m_sequence == MAXIMUM_SEQUENCE);
    REQUIRE(sequencer.get_sequence() == std::uint32_t(1));
    gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 1);
    REQUIRE(gap->m_count == 1);
    sequencer.add(TmxIpPacket::parse(encode_packet(1, "first")));
    for(auto sequence : {1, 2}) {
      packet = sequencer.read();
      REQUIRE(packet.has_value());
      REQUIRE(packet->m_header.m_sequence == std::uint32_t(sequence));
    }
    sequencer.add(TmxIpPacket::parse(encode_packet(MAXIMUM_SEQUENCE, "old")));
    REQUIRE(!sequencer.read().has_value());
    REQUIRE(!sequencer.get_gap().has_value());
    REQUIRE(sequencer.get_sequence() == std::uint32_t(3));
  }

  TEST_CASE("heartbeat") {
    auto sequencer = TmxIpSequencer();
    auto heartbeat =
      TmxIpPacket::parse("\x02" "0031         CDF00V T HEARTBEAT\x03");
    sequencer.add(heartbeat);
    REQUIRE(!sequencer.get_sequence().has_value());
    REQUIRE(!sequencer.get_gap().has_value());
    REQUIRE(!sequencer.read().has_value());
    sequencer.reset(100);
    sequencer.add(TmxIpPacket::parse(encode_packet(102, "last")));
    sequencer.add(heartbeat);
    auto gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 100);
    REQUIRE(gap->m_count == 2);
    REQUIRE(sequencer.get_sequence() == std::uint32_t(100));
    REQUIRE(!sequencer.read().has_value());
  }

  TEST_CASE("reset") {
    auto sequencer = TmxIpSequencer();
    sequencer.reset(100);
    sequencer.add(TmxIpPacket::parse(encode_packet(103, "old")));
    SUBCASE("initial_position") {
      sequencer.reset();
      REQUIRE(!sequencer.get_sequence().has_value());
    }
    SUBCASE("daily_reset") {
      sequencer.reset(1);
      REQUIRE(sequencer.get_sequence() == std::uint32_t(1));
    }
    REQUIRE(!sequencer.get_gap().has_value());
    REQUIRE(!sequencer.read().has_value());
    sequencer.add(TmxIpPacket::parse(encode_packet(1, "new")));
    auto packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(packet->m_header.m_sequence == std::uint32_t(1));
    REQUIRE(packet->m_payload == "new");
    REQUIRE(!sequencer.get_gap().has_value());
    REQUIRE(!sequencer.read().has_value());
    sequencer.reset();
    sequencer.reset();
    REQUIRE(!sequencer.get_sequence().has_value());
  }

  TEST_CASE("invalid_reset") {
    auto sequencer = TmxIpSequencer();
    sequencer.add(TmxIpPacket::parse(encode_packet(100, "pending")));
    for(auto sequence : {std::uint32_t(0), std::uint32_t(1000000000)}) {
      REQUIRE_THROWS_AS(sequencer.reset(sequence), TmxIpParserException);
      REQUIRE(sequencer.get_sequence() == std::uint32_t(100));
    }
    auto packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(packet->m_payload == "pending");
  }

  TEST_CASE("fragment_recovery") {
    auto sequencer = TmxIpSequencer();
    auto builder = TmxIpMessageBuilder();
    sequencer.add(
      TmxIpPacket::parse("\x02" "0027000000001CDF01  T \x01\x1e" "1=H\x03"));
    sequencer.add(TmxIpPacket::parse("\x02" "0024000000003CDF02  T X\x1d\x03"));
    auto packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(!builder.add(*packet).has_value());
    REQUIRE(!sequencer.read().has_value());
    auto gap = sequencer.get_gap();
    REQUIRE(gap.has_value());
    REQUIRE(gap->m_sequence == 2);
    REQUIRE(gap->m_count == 1);
    sequencer.add(
      TmxIpPacket::parse("\x02" "0029000000002CDF03  T \x1c\x1e" "55=AB\x03"));
    packet = sequencer.read();
    REQUIRE(packet.has_value());
    REQUIRE(!builder.add(*packet).has_value());
    packet = sequencer.read();
    REQUIRE(packet.has_value());
    auto payload = builder.add(*packet);
    REQUIRE(payload.has_value());
    auto message = StampMessage::parse(
      std::string_view(payload->get_data(), payload->get_size()));
    auto field = message.m_business_content.find(55);
    REQUIRE(field.has_value());
    REQUIRE(field->m_value == "ABX");
    REQUIRE(!sequencer.read().has_value());
    REQUIRE(!sequencer.get_gap().has_value());
  }
}
