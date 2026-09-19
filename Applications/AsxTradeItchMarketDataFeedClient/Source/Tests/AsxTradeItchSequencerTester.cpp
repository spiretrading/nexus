#include <limits>
#include <vector>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchSequencer.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  std::string encode_packet(const AsxTradeItchSequencer::Session& session,
      std::uint64_t sequence, const std::vector<std::string_view>& messages) {
    auto source = std::string(
      session.get_data(), MoldUdp64Packet::SESSION_FIELD_LENGTH);
    auto encoded_sequence = boost::endian::native_to_big(sequence);
    source.append(reinterpret_cast<const char*>(&encoded_sequence),
      sizeof(encoded_sequence));
    auto count = boost::endian::native_to_big(
      static_cast<std::uint16_t>(messages.size()));
    source.append(reinterpret_cast<const char*>(&count), sizeof(count));
    for(auto message : messages) {
      auto length = boost::endian::native_to_big(
        static_cast<std::uint16_t>(message.size()));
      source.append(reinterpret_cast<const char*>(&length), sizeof(length));
      source.append(message);
    }
    return source;
  }

  struct Fixture {
    AsxTradeItchSequencer::Session m_session;
    ptime m_timestamp;
    AsxTradeItchSequencer m_sequencer;

    Fixture()
      : m_session("SESSION123"),
        m_timestamp(time_from_string("2026-09-18 10:00:00")),
        m_sequencer(3, duration_from_string("00:00:03")) {}

    void add(int feed, std::uint64_t sequence,
        const std::vector<std::string_view>& messages) {
      auto source = encode_packet(m_session, sequence, messages);
      m_sequencer.add(feed, MoldUdp64Packet::parse(source), m_timestamp);
    }

    void recover(std::uint64_t sequence,
        const std::vector<std::string_view>& messages) {
      auto source = encode_packet(m_session, sequence, messages);
      m_sequencer.recover(MoldUdp64Packet::parse(source));
    }

    void require_message(std::string_view expected) {
      auto message = m_sequencer.read();
      REQUIRE(message.has_value());
      REQUIRE(*message == expected);
    }

    void require_gap(std::uint64_t sequence, std::uint64_t count) {
      auto gap = m_sequencer.get_gap();
      REQUIRE(gap.has_value());
      REQUIRE(gap->m_sequence == sequence);
      REQUIRE(gap->m_count == count);
    }
  };
}

TEST_SUITE("AsxTradeItchSequencer") {
  TEST_CASE_FIXTURE(Fixture, "message_order") {
    REQUIRE(!m_sequencer.get_session());
    REQUIRE(!m_sequencer.get_sequence());
    REQUIRE(m_sequencer.get_position() == 0);
    REQUIRE(!m_sequencer.get_gap());
    REQUIRE(!m_sequencer.read());
    REQUIRE(!m_sequencer.is_end_of_session());
    recover(1, {"old"});
    REQUIRE(!m_sequencer.get_session());
    add(0, 100, {"first", "", "third"});
    REQUIRE(m_sequencer.get_session() == m_session);
    REQUIRE(m_sequencer.get_sequence() == std::uint64_t(100));
    REQUIRE(m_sequencer.get_position() == 103);
    require_message("first");
    require_message("");
    require_message("third");
    REQUIRE(m_sequencer.get_sequence() == std::uint64_t(103));
    REQUIRE(!m_sequencer.read());
    REQUIRE(!m_sequencer.get_gap());
  }

  TEST_CASE_FIXTURE(Fixture, "overlapping_packets") {
    auto start = std::uint64_t(1) << 40;
    add(0, start, {"one", "two"});
    require_message("one");
    add(0, start + 4, {"five", "six"});
    SUBCASE("redundant_feed") {
      add(1, start, {"one", "two", "three", "four", "five", "six", "seven"});
    }
    SUBCASE("recovery") {
      recover(start,
        {"one", "two", "three", "four", "five", "six", "seven"});
    }
    for(auto message : {"two", "three", "four", "five", "six", "seven"}) {
      require_message(message);
    }
    add(2, start, {"one", "two", "three"});
    REQUIRE(!m_sequencer.read());
    REQUIRE(m_sequencer.get_sequence() == start + 7);
    REQUIRE(!m_sequencer.get_gap());
  }

  TEST_CASE_FIXTURE(Fixture, "gap_quorum") {
    for(auto feed = 0; feed != 3; ++feed) {
      add(feed, 1, {"one", "two"});
    }
    require_message("one");
    require_message("two");
    add(0, 6, {"six"});
    add(1, 4, {});
    REQUIRE(!m_sequencer.get_gap());
    add(2, 5, {});
    REQUIRE(m_sequencer.get_position() == 4);
    require_gap(3, 1);
    REQUIRE(!m_sequencer.read());
    recover(3, {"three"});
    require_message("three");
    REQUIRE(!m_sequencer.get_gap());
    add(1, 6, {});
    require_gap(4, 1);
    add(2, 6, {});
    require_gap(4, 2);
    recover(4, {"four", "five"});
    require_message("four");
    require_message("five");
    require_message("six");
    REQUIRE(!m_sequencer.read());
    REQUIRE(!m_sequencer.get_gap());
  }

  TEST_CASE_FIXTURE(Fixture, "recovery_quorum") {
    add(0, 1, {"one"});
    add(1, 1, {"one"});
    require_message("one");
    recover(3, {"three"});
    REQUIRE(!m_sequencer.read());
    REQUIRE(!m_sequencer.get_gap());
    REQUIRE(m_sequencer.get_position() == 2);
    auto foreign = encode_packet("SESSION456", 2, {"wrong"});
    m_sequencer.recover(MoldUdp64Packet::parse(foreign));
    REQUIRE(!m_sequencer.read());
    add(0, 4, {});
    REQUIRE(!m_sequencer.get_gap());
    add(1, 4, {});
    require_gap(2, 1);
    recover(2, {"two", "three"});
    require_message("two");
    require_message("three");
    REQUIRE(!m_sequencer.read());
    REQUIRE(!m_sequencer.get_gap());
  }

  TEST_CASE_FIXTURE(Fixture, "feed_timeout") {
    add(0, 1, {"one"});
    add(1, 1, {"one"});
    require_message("one");
    add(0, 3, {"three"});
    REQUIRE(!m_sequencer.get_gap());
    auto replay = std::vector<std::string_view>();
    auto sequence = std::uint64_t(2);
    SUBCASE("heartbeat") {}
    SUBCASE("duplicate_data") {
      replay = {"one"};
      sequence = 1;
    }
    for(auto i = 0; i != 3; ++i) {
      m_timestamp += duration_from_string("00:00:01");
      add(0, 4, {});
      add(1, sequence, replay);
      REQUIRE(!m_sequencer.get_gap());
    }
    m_timestamp += duration_from_string("00:00:01");
    add(0, 4, {});
    add(1, sequence, replay);
    require_gap(2, 1);
    REQUIRE(m_sequencer.get_position() == 4);
    m_timestamp += duration_from_string("00:00:04");
    m_sequencer.update(m_timestamp);
    require_gap(2, 1);
    REQUIRE(m_sequencer.get_position() == 4);
    add(1, 2, {"two"});
    require_message("two");
    require_message("three");
    add(0, 5, {"five"});
    REQUIRE(m_sequencer.get_position() == 3);
    REQUIRE(!m_sequencer.get_gap());
  }

  TEST_CASE_FIXTURE(Fixture, "idle_heartbeat") {
    add(0, 1, {"one"});
    add(1, 1, {"one"});
    require_message("one");
    for(auto i = 0; i != 5; ++i) {
      m_timestamp += duration_from_string("00:00:01");
      add(0, 2, {});
      add(1, 2, {});
      REQUIRE(!m_sequencer.get_gap());
    }
    add(0, 4, {"four"});
    REQUIRE(m_sequencer.get_position() == 2);
    REQUIRE(!m_sequencer.get_gap());
    add(1, 4, {});
    require_gap(2, 2);
    REQUIRE(!m_sequencer.is_end_of_session());
  }

  TEST_CASE_FIXTURE(Fixture, "end_of_session") {
    auto source = encode_packet(m_session, 4, {});
    auto packet = MoldUdp64Packet::parse(source);
    packet.m_count = MoldUdp64Packet::END_OF_SESSION;
    SUBCASE("initial") {
      m_sequencer.add(0, packet, m_timestamp);
      REQUIRE(m_sequencer.get_sequence() == std::uint64_t(4));
      REQUIRE(m_sequencer.get_position() == 4);
      REQUIRE(m_sequencer.is_end_of_session());
      REQUIRE(!m_sequencer.read());
    }
    SUBCASE("trailing_gap") {
      add(0, 1, {"one"});
      add(1, 1, {"one"});
      require_message("one");
      m_sequencer.recover(packet);
      REQUIRE(!m_sequencer.is_end_of_session());
      REQUIRE(m_sequencer.get_position() == 2);
      m_sequencer.add(0, packet, m_timestamp);
      REQUIRE(!m_sequencer.get_gap());
      m_sequencer.add(1, packet, m_timestamp);
      REQUIRE(m_sequencer.get_position() == 4);
      require_gap(2, 2);
      REQUIRE(!m_sequencer.is_end_of_session());
      recover(2, {"two", "three"});
      require_message("two");
      REQUIRE(!m_sequencer.is_end_of_session());
      require_message("three");
      REQUIRE(m_sequencer.is_end_of_session());
      m_timestamp += duration_from_string("00:00:04");
      m_sequencer.add(0, packet, m_timestamp);
      REQUIRE(!m_sequencer.read());
    }
    REQUIRE(!m_sequencer.get_gap());
  }

  TEST_CASE_FIXTURE(Fixture, "session_reset") {
    add(0, 100, {"old"});
    add(1, 100, {"old"});
    require_message("old");
    add(0, 102, {"buffered"});
    auto source = encode_packet(m_session, 103, {});
    auto packet = MoldUdp64Packet::parse(source);
    packet.m_count = MoldUdp64Packet::END_OF_SESSION;
    m_sequencer.add(1, packet, m_timestamp);
    require_gap(101, 1);
    auto foreign = encode_packet("ANOTHER123", 1, {"wrong"});
    m_sequencer.add(1, MoldUdp64Packet::parse(foreign), m_timestamp);
    m_sequencer.recover(MoldUdp64Packet::parse(foreign));
    REQUIRE(m_sequencer.get_session() == m_session);
    REQUIRE(m_sequencer.get_sequence() == std::uint64_t(101));
    REQUIRE(!m_sequencer.read());
    m_session = "ANOTHER123";
    m_sequencer.reset(m_session, 1);
    REQUIRE(m_sequencer.get_session() == m_session);
    REQUIRE(m_sequencer.get_position() == 0);
    REQUIRE(!m_sequencer.is_end_of_session());
    REQUIRE(!m_sequencer.get_gap());
    REQUIRE(!m_sequencer.read());
    m_sequencer.add(1, packet, m_timestamp);
    m_sequencer.recover(packet);
    auto stale = encode_packet("SESSION123", 1, {"stale"});
    m_sequencer.add(0, MoldUdp64Packet::parse(stale), m_timestamp);
    m_sequencer.recover(MoldUdp64Packet::parse(stale));
    REQUIRE(m_sequencer.get_position() == 0);
    REQUIRE(!m_sequencer.read());
    REQUIRE(!m_sequencer.is_end_of_session());
    add(0, 1, {"new"});
    require_message("new");
    add(0, 1, {"new"});
    REQUIRE(!m_sequencer.read());
    add(0, 102, {"current"});
    m_sequencer.reset(102);
    require_message("current");
    REQUIRE(m_sequencer.get_sequence() == std::uint64_t(103));
    REQUIRE(!m_sequencer.is_end_of_session());
    REQUIRE(!m_sequencer.read());
  }

  TEST_CASE_FIXTURE(Fixture, "sequence_reset") {
    SUBCASE("initial") {
      m_sequencer.reset(100);
      add(0, 101, {"next"});
      REQUIRE(m_sequencer.get_session() == m_session);
      require_gap(100, 1);
      recover(100, {"first"});
      require_message("first");
      require_message("next");
    }
    SUBCASE("buffered") {
      add(0, 1, {"one"});
      add(0, 5, {"five"});
      add(0, 7, {"seven"});
      m_sequencer.reset(5);
      require_message("five");
      require_gap(6, 1);
      recover(6, {"six"});
      require_message("six");
      require_message("seven");
      REQUIRE(m_sequencer.get_sequence() == std::uint64_t(8));
    }
    REQUIRE(!m_sequencer.read());
    REQUIRE(!m_sequencer.get_gap());
  }

  TEST_CASE_FIXTURE(Fixture, "initial_heartbeat") {
    auto sequence = std::uint64_t(10);
    SUBCASE("mid_session") {}
    SUBCASE("zero_position") {
      sequence = 0;
    }
    add(0, sequence, {});
    add(1, sequence, {});
    REQUIRE(m_sequencer.get_sequence() == sequence);
    REQUIRE(m_sequencer.get_session() == m_session);
    REQUIRE(!m_sequencer.read());
    REQUIRE(!m_sequencer.get_gap());
    add(0, sequence + 2, {"later"});
    REQUIRE(m_sequencer.get_position() == sequence);
    REQUIRE(!m_sequencer.get_gap());
    add(1, sequence + 2, {});
    require_gap(sequence, 2);
    recover(sequence, {"first", "second"});
    require_message("first");
    require_message("second");
    require_message("later");
    REQUIRE(!m_sequencer.get_gap());
  }

  TEST_CASE_FIXTURE(Fixture, "shared_packet_storage") {
    auto source = encode_packet(m_session, 1, {"first", "second", "third"});
    m_sequencer.add(0, MoldUdp64Packet::parse(source), m_timestamp);
    auto first = m_sequencer.read();
    REQUIRE(first.has_value());
    auto second = m_sequencer.read();
    REQUIRE(second.has_value());
    REQUIRE(second->get_data() == first->get_data() + first->get_size() +
      MoldUdp64Message::HEADER_LENGTH);
    source.assign(source.size(), char(0));
    m_sequencer.reset("SESSION456", 1);
    REQUIRE(*first == "first");
    REQUIRE(*second == "second");
    REQUIRE(!m_sequencer.read());
  }

  TEST_CASE_FIXTURE(Fixture, "sequence_limits") {
    SUBCASE("large_gap") {
      m_sequencer.reset(m_session, 0);
      auto position = std::uint64_t(1) << 40;
      add(0, position, {});
      require_gap(0, position);
      REQUIRE(m_sequencer.get_position() == position);
    }
    SUBCASE("last_position") {
      auto last = std::numeric_limits<std::uint64_t>::max();
      add(0, last - 1, {"last"});
      require_message("last");
      add(0, last, {});
      REQUIRE(m_sequencer.get_sequence() == last);
      REQUIRE(m_sequencer.get_position() == last);
      REQUIRE_THROWS_AS(add(0, last, {"overflow"}),
        AsxTradeItchParserException);
      REQUIRE_THROWS_AS(recover(last, {"overflow"}),
        AsxTradeItchParserException);
      REQUIRE(m_sequencer.get_sequence() == last);
      REQUIRE(m_sequencer.get_position() == last);
      REQUIRE(!m_sequencer.read());
      REQUIRE(!m_sequencer.get_gap());
      auto source = encode_packet(m_session, last, {});
      auto packet = MoldUdp64Packet::parse(source);
      packet.m_count = MoldUdp64Packet::END_OF_SESSION;
      m_sequencer.add(0, packet, m_timestamp);
      REQUIRE(m_sequencer.is_end_of_session());
    }
  }
}
