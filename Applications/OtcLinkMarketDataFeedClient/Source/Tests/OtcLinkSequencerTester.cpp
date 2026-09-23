#include <initializer_list>
#include <limits>
#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkSequencer.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  void append(std::string& target, auto value) {
    value = endian::native_to_big(value);
    target.append(reinterpret_cast<const char*>(&value), sizeof(value));
  }

  std::string make_packet(std::string_view messages, std::uint8_t count,
      std::uint32_t sequence, std::uint8_t flags) {
    auto source = std::string();
    append(source, static_cast<std::uint16_t>(
      OtcLinkHeader::LENGTH + messages.size()));
    append(source, sequence);
    append(source, flags);
    append(source, count);
    append(source, std::uint32_t(0));
    source += messages;
    return source;
  }

  std::string make_packet(std::initializer_list<std::uint32_t> sequences,
      std::uint32_t sequence, std::uint8_t flags) {
    auto messages = std::string();
    for(auto value : sequences) {
      append(messages, static_cast<std::uint16_t>(
        OtcLinkMessage::HEADER_LENGTH + sizeof(value)));
      append(messages, std::uint8_t(0xFF));
      append(messages, value);
    }
    return make_packet(
      messages, static_cast<std::uint8_t>(sequences.size()), sequence, flags);
  }

  struct Fixture {
    ptime m_timestamp;
    OtcLinkSequencer m_sequencer;

    explicit Fixture(int feeds)
      : m_timestamp(time_from_string("2026-09-23 14:00:00")),
        m_sequencer(feeds, seconds(3)) {}

    void add(int feed, std::initializer_list<std::uint32_t> sequences) {
      auto source = make_packet(sequences, 100, 0);
      m_sequencer.add(feed, OtcLinkPacket::parse(source), m_timestamp);
    }

    void heartbeat(int feed) {
      auto source = make_packet({}, 1000,
        static_cast<std::uint8_t>(OtcLinkHeader::Flag::HEARTBEAT));
      m_sequencer.add(feed, OtcLinkPacket::parse(source), m_timestamp);
    }

    void recover(std::initializer_list<std::uint32_t> sequences) {
      auto source = make_packet(sequences, 200,
        static_cast<std::uint8_t>(OtcLinkHeader::Flag::REPLAY));
      m_sequencer.recover(OtcLinkPacket::parse(source));
    }

    void require_message(std::uint32_t sequence) {
      auto payload = m_sequencer.read();
      REQUIRE(payload.has_value());
      auto message = OtcLinkMessage::parse(
        std::string_view(payload->get_data(), payload->get_size()));
      REQUIRE(message.m_type == 0xFF);
      REQUIRE(message.get_cursor().read_uint32() == sequence);
    }

    void require_gap(std::uint64_t sequence, std::uint64_t count) {
      auto gap = m_sequencer.get_gap();
      REQUIRE(gap.has_value());
      REQUIRE(gap->m_sequence == sequence);
      REQUIRE(gap->m_count == count);
    }
  };
}

TEST_SUITE("OtcLinkSequencer") {
  TEST_CASE("sequence_limits") {
    auto fixture = Fixture(1);
    auto maximum = std::numeric_limits<std::uint32_t>::max();
    auto end = static_cast<std::uint64_t>(maximum) + 1;
    fixture.add(0, {maximum - 2, maximum});
    fixture.require_message(maximum - 2);
    fixture.require_gap(maximum - 1, 1);
    fixture.recover({maximum - 1});
    fixture.require_message(maximum - 1);
    fixture.require_message(maximum);
    REQUIRE(fixture.m_sequencer.get_sequence() == end);
    REQUIRE(fixture.m_sequencer.get_position() == end);
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    fixture.add(0, {1, maximum});
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
    REQUIRE(fixture.m_sequencer.get_sequence() == end);
    fixture.m_sequencer.reset(1);
    fixture.add(0, {1});
    fixture.require_message(1);
  }

  TEST_CASE("test_packets") {
    auto fixture = Fixture(1);
    auto source = make_packet({50, 51}, 100,
      static_cast<std::uint8_t>(OtcLinkHeader::Flag::TEST));
    fixture.m_sequencer.add(
      0, OtcLinkPacket::parse(source), fixture.m_timestamp);
    REQUIRE_FALSE(fixture.m_sequencer.get_sequence().has_value());
    fixture.add(0, {1});
    fixture.require_message(1);
    fixture.m_sequencer.add(
      0, OtcLinkPacket::parse(source), fixture.m_timestamp);
    fixture.m_sequencer.recover(OtcLinkPacket::parse(source));
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    REQUIRE(fixture.m_sequencer.get_position() == 2);
  }

  TEST_CASE("channel_reset") {
    auto fixture = Fixture(2);
    fixture.add(0, {40, 42});
    fixture.add(1, {40});
    fixture.require_message(40);
    auto source = make_packet(
      {}, 1, static_cast<std::uint8_t>(OtcLinkHeader::Flag::SEQUENCE_RESET));
    fixture.m_sequencer.add(
      0, OtcLinkPacket::parse(source), fixture.m_timestamp);
    REQUIRE(fixture.m_sequencer.get_sequence() == std::uint64_t(41));
    fixture.m_sequencer.reset(1);
    REQUIRE(fixture.m_sequencer.get_position() == 0);
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
    fixture.add(0, {1, 2});
    fixture.require_message(1);
    fixture.m_sequencer.add(
      1, OtcLinkPacket::parse(source), fixture.m_timestamp);
    fixture.m_sequencer.recover(OtcLinkPacket::parse(source));
    fixture.require_message(2);
    fixture.add(1, {4});
    fixture.m_sequencer.reset(42);
    REQUIRE(fixture.m_sequencer.get_sequence() == std::uint64_t(42));
    REQUIRE(fixture.m_sequencer.get_position() == 0);
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
    fixture.add(0, {41, 42});
    fixture.require_message(42);
    fixture.m_sequencer.reset();
    REQUIRE_FALSE(fixture.m_sequencer.get_sequence().has_value());
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    REQUIRE(fixture.m_sequencer.get_position() == 0);
    fixture.add(1, {100});
    fixture.require_message(100);
  }

  TEST_CASE("gap_skip") {
    auto fixture = Fixture(1);
    REQUIRE_THROWS_AS(fixture.m_sequencer.skip(1), std::invalid_argument);
    fixture.add(0, {1, 5});
    REQUIRE_THROWS_AS(fixture.m_sequencer.skip(1), std::invalid_argument);
    fixture.require_message(1);
    fixture.require_gap(2, 3);
    REQUIRE_THROWS_AS(fixture.m_sequencer.skip(0), std::invalid_argument);
    REQUIRE_THROWS_AS(fixture.m_sequencer.skip(4), std::invalid_argument);
    fixture.require_gap(2, 3);
    fixture.m_sequencer.skip(1);
    fixture.require_gap(3, 2);
    fixture.recover({1, 2});
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
    fixture.m_sequencer.skip(2);
    fixture.require_message(5);
    fixture.recover({2, 3, 4});
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    REQUIRE_THROWS_AS(fixture.m_sequencer.skip(1), std::invalid_argument);
  }

  TEST_CASE("packet_ownership") {
    auto fixture = Fixture(1);
    auto source = make_packet({1, 2, 3}, 100, 0);
    auto expected = std::string(OtcLinkPacket::parse(source).get_payload());
    fixture.m_sequencer.add(
      0, OtcLinkPacket::parse(source), fixture.m_timestamp);
    std::ranges::fill(source, '\0');
    auto first = fixture.m_sequencer.read();
    auto second = fixture.m_sequencer.read();
    auto third = fixture.m_sequencer.read();
    REQUIRE(first.has_value());
    REQUIRE(second.has_value());
    REQUIRE(third.has_value());
    REQUIRE(second->get_data() == first->get_data() + first->get_size());
    REQUIRE(third->get_data() == second->get_data() + second->get_size());
    fixture.m_sequencer.reset();
    fixture.add(0, {10});
    fixture.require_message(10);
    auto actual = std::string(first->get_data(), first->get_size());
    actual.append(second->get_data(), second->get_size());
    actual.append(third->get_data(), third->get_size());
    REQUIRE(actual == expected);
  }

  TEST_CASE("recovery_quorum") {
    auto fixture = Fixture(2);
    fixture.add(0, {1});
    fixture.add(1, {1});
    fixture.require_message(1);
    fixture.add(0, {5});
    SUBCASE("recovery_feed") {
      fixture.recover({2, 3});
    }
    SUBCASE("live_replay") {
      auto source = make_packet(
        {2, 3}, 100, static_cast<std::uint8_t>(OtcLinkHeader::Flag::REPLAY));
      fixture.m_sequencer.add(
        1, OtcLinkPacket::parse(source), fixture.m_timestamp);
    }
    REQUIRE(fixture.m_sequencer.get_position() == 2);
    fixture.require_message(2);
    fixture.require_message(3);
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    fixture.add(1, {6});
    fixture.require_gap(4, 1);
    fixture.recover({3, 4, 5, 6});
    REQUIRE(fixture.m_sequencer.get_position() == 6);
    for(auto sequence : {4, 5, 6}) {
      fixture.require_message(sequence);
    }
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
  }

  TEST_CASE("malformed_packet") {
    auto fixture = Fixture(2);
    auto valid = make_packet({2, 4}, 100, 0);
    auto messages = std::string(OtcLinkPacket::parse(valid).get_payload());
    auto short_length = sizeof(std::uint32_t) - 1;
    append(messages,
      static_cast<std::uint16_t>(OtcLinkMessage::HEADER_LENGTH + short_length));
    append(messages, std::uint8_t(0xFF));
    messages.append(short_length, '\0');
    auto source = make_packet(messages, 3, 101, 0);
    auto packet = OtcLinkPacket::parse(source);
    SUBCASE("initial") {
      REQUIRE_THROWS_AS(fixture.m_sequencer.add(
        0, packet, fixture.m_timestamp), OtcLinkParserException);
      REQUIRE_FALSE(fixture.m_sequencer.get_sequence().has_value());
      REQUIRE(fixture.m_sequencer.get_position() == 0);
    }
    SUBCASE("live") {
      fixture.add(0, {1});
      fixture.require_message(1);
      REQUIRE_THROWS_AS(fixture.m_sequencer.add(
        0, packet, fixture.m_timestamp), OtcLinkParserException);
      REQUIRE(fixture.m_sequencer.get_position() == 2);
    }
    SUBCASE("recovery") {
      fixture.add(0, {1});
      fixture.require_message(1);
      REQUIRE_THROWS_AS(
        fixture.m_sequencer.recover(packet), OtcLinkParserException);
      REQUIRE(fixture.m_sequencer.get_position() == 2);
    }
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    fixture.add(1, {2, 3});
    fixture.require_message(2);
    fixture.require_message(3);
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
  }

  TEST_CASE("clock_rollback") {
    auto fixture = Fixture(2);
    fixture.add(0, {1});
    fixture.add(1, {1});
    fixture.require_message(1);
    fixture.add(0, {4});
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    fixture.m_timestamp -= time_duration::unit();
    fixture.m_sequencer.update(fixture.m_timestamp);
    fixture.require_gap(2, 2);
    fixture.add(1, {3});
    REQUIRE(fixture.m_sequencer.get_position() == 4);
    fixture.require_gap(2, 1);
  }

  TEST_CASE("idle_heartbeat") {
    auto fixture = Fixture(2);
    fixture.add(0, {1});
    fixture.add(1, {1});
    fixture.require_message(1);
    fixture.m_timestamp += seconds(2);
    fixture.heartbeat(0);
    fixture.m_timestamp += seconds(2);
    fixture.add(1, {4});
    REQUIRE(fixture.m_sequencer.get_position() == 2);
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    fixture.m_timestamp += seconds(1);
    fixture.heartbeat(0);
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    fixture.m_timestamp += time_duration::unit();
    fixture.m_sequencer.update(fixture.m_timestamp);
    fixture.require_gap(2, 2);
  }

  TEST_CASE("stalled_feed") {
    auto fixture = Fixture(2);
    fixture.add(0, {1});
    fixture.add(1, {1});
    fixture.require_message(1);
    fixture.m_timestamp += seconds(2);
    fixture.add(0, {4});
    fixture.m_timestamp += seconds(1);
    SUBCASE("silence") {
      fixture.m_sequencer.update(fixture.m_timestamp);
    }
    SUBCASE("duplicate_messages") {
      fixture.add(1, {1});
    }
    SUBCASE("heartbeat") {
      fixture.heartbeat(1);
    }
    SUBCASE("replay") {
      auto source = make_packet(
        {1, 4}, 100, static_cast<std::uint8_t>(OtcLinkHeader::Flag::REPLAY));
      fixture.m_sequencer.add(
        1, OtcLinkPacket::parse(source), fixture.m_timestamp);
    }
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    fixture.m_timestamp += time_duration::unit();
    fixture.m_sequencer.update(fixture.m_timestamp);
    fixture.require_gap(2, 2);
    fixture.add(1, {3});
    REQUIRE(fixture.m_sequencer.get_position() == 4);
    fixture.require_gap(2, 1);
    fixture.m_timestamp += seconds(4);
    fixture.m_sequencer.update(fixture.m_timestamp);
    REQUIRE(fixture.m_sequencer.get_position() == 5);
    fixture.require_gap(2, 1);
  }

  TEST_CASE("feed_quorum") {
    for(auto lagging = 0; lagging < 3; ++lagging) {
      auto fixture = Fixture(3);
      for(auto feed = 0; feed < 3; ++feed) {
        fixture.add(feed, {1});
      }
      fixture.require_message(1);
      for(auto feed = 0; feed < 3; ++feed) {
        if(feed != lagging) {
          fixture.add(feed, {4});
        }
      }
      REQUIRE(fixture.m_sequencer.get_position() == 2);
      REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
      fixture.add(lagging, {3});
      fixture.require_gap(2, 1);
      fixture.recover({1, 2, 3});
      for(auto sequence : {2, 3, 4}) {
        fixture.require_message(sequence);
      }
      REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
      REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    }
  }

  TEST_CASE("initialization") {
    auto fixture = Fixture(2);
    fixture.heartbeat(0);
    fixture.recover({1, 2});
    REQUIRE_FALSE(fixture.m_sequencer.get_sequence().has_value());
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
    REQUIRE(fixture.m_sequencer.get_position() == 0);
    fixture.add(1, {102, 100, 101});
    for(auto sequence : {100, 101, 102}) {
      fixture.require_message(sequence);
    }
    REQUIRE(fixture.m_sequencer.get_sequence() == std::uint64_t(103));
    REQUIRE(fixture.m_sequencer.get_position() == 103);
    fixture.heartbeat(0);
    REQUIRE(fixture.m_sequencer.get_position() == 103);
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    REQUIRE_THROWS_AS(OtcLinkSequencer(0, seconds(1)), std::invalid_argument);
    REQUIRE_THROWS_AS(OtcLinkSequencer(-1, seconds(1)), std::invalid_argument);
    REQUIRE_THROWS_AS(OtcLinkSequencer(1, seconds(0)), std::invalid_argument);
    REQUIRE_THROWS_AS(OtcLinkSequencer(1, seconds(-1)), std::invalid_argument);
    REQUIRE_THROWS_AS(OtcLinkSequencer(1, time_duration(not_a_date_time)),
      std::invalid_argument);
  }

  TEST_CASE("message_arbitration") {
    auto fixture = Fixture(3);
    fixture.add(0, {42, 44});
    fixture.require_message(42);
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
    fixture.require_gap(43, 1);
    auto source = make_packet({42, 43}, 900, 0);
    fixture.m_sequencer.add(
      1, OtcLinkPacket::parse(source), fixture.m_timestamp);
    fixture.add(2, {45, 44, 43});
    for(auto sequence : {43, 44, 45}) {
      fixture.require_message(sequence);
    }
    fixture.add(0, {42, 43, 44});
    fixture.add(1, {43, 44, 45});
    REQUIRE_FALSE(fixture.m_sequencer.read().has_value());
    REQUIRE_FALSE(fixture.m_sequencer.get_gap().has_value());
    REQUIRE(fixture.m_sequencer.get_sequence() == std::uint64_t(46));
  }
}
