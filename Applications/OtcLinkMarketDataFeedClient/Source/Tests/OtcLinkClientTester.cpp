#include <future>
#include <limits>
#include <sstream>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/endian/conversion.hpp>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct Log {
    std::stringstream m_output;
    std::streambuf* m_buffer;

    Log()
      : m_buffer(std::cout.rdbuf(m_output.rdbuf())) {}

    ~Log() {
      std::cout.rdbuf(m_buffer);
    }
  };

  using ProtocolClient =
    OtcLinkProtocolClient<LocalClientChannel*, FixedTimeClient*>;
  using Client =
    OtcLinkClient<ProtocolClient*, FixedTimeClient*, TriggerTimer*>;

  struct Feed {
    optional<LocalClientChannel> m_channel;
    std::unique_ptr<ProtocolClient> m_client;
    std::unique_ptr<LocalServerChannel> m_server_channel;
  };

  struct Fixture {
    inline static const auto FEED_TIMEOUT = seconds(3);
    inline static const auto GAP_TIMEOUT = seconds(5);
    Log m_log;
    LocalServerConnection m_server;
    FixedTimeClient m_time_client;
    TriggerTimer m_timer;
    std::vector<std::unique_ptr<Feed>> m_feeds;
    optional<Client> m_client;

    explicit Fixture(int feeds)
        : m_time_client(time_from_string("2026-09-23 14:00:00")) {
      auto clients = std::vector<ProtocolClient*>();
      for(auto i = 0; i < feeds; ++i) {
        auto feed = std::make_unique<Feed>();
        auto server_channel = std::async(std::launch::async, [&] {
          return m_server.accept();
        });
        feed->m_channel.emplace("otc_link", m_server);
        feed->m_server_channel = server_channel.get();
        feed->m_client = std::make_unique<ProtocolClient>(
          &*feed->m_channel, &m_time_client);
        clients.push_back(feed->m_client.get());
        m_feeds.push_back(std::move(feed));
      }
      m_client.emplace(
        FEED_TIMEOUT, GAP_TIMEOUT, clients, &m_time_client, &m_timer);
      flush_pending_routines();
    }

    SharedBuffer make_packet(std::initializer_list<std::uint32_t> sequences,
        std::uint8_t flags) {
      auto buffer = SharedBuffer();
      auto write = [&] (auto value) {
        append(buffer, endian::native_to_big(value));
      };
      auto length = OtcLinkMessage::HEADER_LENGTH + sizeof(std::uint32_t);
      write(static_cast<std::uint16_t>(
        OtcLinkHeader::LENGTH + sequences.size() * length));
      write(std::uint32_t(1));
      write(flags);
      write(static_cast<std::uint8_t>(sequences.size()));
      write(static_cast<std::uint32_t>(
        m_time_client.get_time().time_of_day().total_milliseconds()));
      for(auto sequence : sequences) {
        write(static_cast<std::uint16_t>(length));
        write(std::uint8_t(0xFF));
        write(sequence);
      }
      return buffer;
    }

    void publish(int feed, std::initializer_list<std::uint32_t> sequences) {
      m_feeds[feed]->m_server_channel->get_writer().write(
        make_packet(sequences, 0));
      flush_pending_routines();
    }

    void publish(int feed, OtcLinkHeader::Flag flag) {
      m_feeds[feed]->m_server_channel->get_writer().write(
        make_packet({}, static_cast<std::uint8_t>(flag)));
      flush_pending_routines();
    }

    void advance(time_duration duration) {
      m_time_client.set(m_time_client.get_time() + duration);
      m_timer.trigger();
      flush_pending_routines();
    }

    void require_message(std::uint32_t sequence, std::uint64_t session) {
      auto actual = std::uint64_t(0);
      auto message = m_client->read(out(actual));
      REQUIRE(message.get_cursor().read_uint32() == sequence);
      REQUIRE(actual == session);
    }
  };
}

TEST_SUITE("OtcLinkClient") {
  TEST_CASE("daylight_saving_reset") {
    auto fixture = Fixture(1);
    fixture.m_time_client.set(time_from_string("2026-11-01 05:59:00"));
    auto source = fixture.make_packet({},
      static_cast<std::uint8_t>(OtcLinkHeader::Flag::SEQUENCE_RESET));
    auto offset = OtcLinkHeader::LENGTH - sizeof(std::uint32_t);
    auto milliseconds = endian::native_to_big(static_cast<std::uint32_t>(
      duration_from_string("01:59:00").total_milliseconds()));
    source.write(offset, &milliseconds, sizeof(milliseconds));
    fixture.m_feeds[0]->m_server_channel->get_writer().write(source);
    flush_pending_routines();
    fixture.publish(0, {1});
    fixture.require_message(1, 1);
    fixture.m_time_client.set(time_from_string("2026-11-01 06:00:00"));
    milliseconds = endian::native_to_big(
      static_cast<std::uint32_t>(hours(1).total_milliseconds()));
    source.write(offset, &milliseconds, sizeof(milliseconds));
    fixture.m_feeds[0]->m_server_channel->get_writer().write(source);
    flush_pending_routines();
    fixture.publish(0, {1});
    fixture.m_client->close();
    fixture.require_message(1, 2);
    REQUIRE_THROWS_AS(fixture.m_client->read(), EndOfFileException);
  }

  TEST_CASE("retired_packet") {
    auto fixture = Fixture(1);
    auto retired = fixture.make_packet({2}, 0);
    fixture.publish(0, {40});
    fixture.require_message(40, 0);
    fixture.advance(milliseconds(1));
    fixture.publish(0, OtcLinkHeader::Flag::SEQUENCE_RESET);
    fixture.publish(0, {1});
    fixture.m_feeds[0]->m_server_channel->get_writer().write(retired);
    flush_pending_routines();
    fixture.m_client->close();
    fixture.require_message(1, 1);
    REQUIRE_THROWS_AS(fixture.m_client->read(), EndOfFileException);
  }

  TEST_CASE("midnight_reset") {
    auto fixture = Fixture(1);
    auto timestamp = time_from_string("2026-09-24 03:59:00");
    SUBCASE("daylight_time") {}
    SUBCASE("standard_time") {
      timestamp = time_from_string("2026-12-24 04:59:00");
    }
    fixture.m_time_client.set(timestamp);
    auto previous = fixture.make_packet({},
      static_cast<std::uint8_t>(OtcLinkHeader::Flag::SEQUENCE_RESET));
    auto offset = OtcLinkHeader::LENGTH - sizeof(std::uint32_t);
    auto milliseconds = endian::native_to_big(static_cast<std::uint32_t>(
      duration_from_string("23:59:00").total_milliseconds()));
    previous.write(offset, &milliseconds, sizeof(milliseconds));
    fixture.m_feeds[0]->m_server_channel->get_writer().write(previous);
    flush_pending_routines();
    fixture.publish(0, {1});
    fixture.require_message(1, 1);
    fixture.m_time_client.set(timestamp + minutes(1));
    auto current = previous;
    milliseconds = 0;
    current.write(offset, &milliseconds, sizeof(milliseconds));
    fixture.m_feeds[0]->m_server_channel->get_writer().write(current);
    flush_pending_routines();
    fixture.publish(0, {1});
    fixture.m_feeds[0]->m_server_channel->get_writer().write(previous);
    flush_pending_routines();
    fixture.publish(0, {2});
    fixture.m_client->close();
    fixture.require_message(1, 2);
    fixture.require_message(2, 2);
    REQUIRE_THROWS_AS(fixture.m_client->read(), EndOfFileException);
  }

  TEST_CASE("test_packets") {
    auto fixture = Fixture(1);
    auto flags = static_cast<std::uint8_t>(OtcLinkHeader::Flag::TEST);
    fixture.m_feeds[0]->m_server_channel->get_writer().write(
      fixture.make_packet({10}, flags));
    fixture.m_feeds[0]->m_server_channel->get_writer().write(
      fixture.make_packet({}, flags |
        static_cast<std::uint8_t>(OtcLinkHeader::Flag::SEQUENCE_RESET)));
    flush_pending_routines();
    fixture.publish(0, OtcLinkHeader::Flag::HEARTBEAT);
    fixture.publish(0, {40});
    fixture.m_client->close();
    fixture.require_message(40, 0);
    REQUIRE_THROWS_AS(fixture.m_client->read(), EndOfFileException);
    REQUIRE(fixture.m_log.m_output.str().empty());
  }

  TEST_CASE("clock_rollback") {
    auto fixture = Fixture(1);
    fixture.publish(0, {1, 3});
    fixture.require_message(1, 0);
    fixture.advance(-time_duration::unit());
    fixture.require_message(3, 0);
    auto expected = std::stringstream();
    expected << "(dropped " << fixture.m_time_client.get_time() <<
      " 2 1 recovery_disabled)\n";
    REQUIRE(fixture.m_log.m_output.str() == expected.str());
    fixture.publish(0, {4});
    fixture.require_message(4, 0);
  }

  TEST_CASE("timer_failure") {
    auto fixture = Fixture(1);
    fixture.publish(0, {1});
    fixture.m_timer.fail();
    flush_pending_routines();
    fixture.require_message(1, 0);
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
    REQUIRE_NOTHROW(fixture.m_client->close());
  }

  TEST_CASE("replay") {
    auto fixture = Fixture(1);
    auto flags = static_cast<std::uint8_t>(OtcLinkHeader::Flag::REPLAY);
    fixture.m_feeds[0]->m_server_channel->get_writer().write(
      fixture.make_packet({99}, flags));
    flush_pending_routines();
    fixture.publish(0, {1, 3});
    fixture.m_feeds[0]->m_server_channel->get_writer().write(
      fixture.make_packet({}, flags |
        static_cast<std::uint8_t>(OtcLinkHeader::Flag::SEQUENCE_RESET)));
    fixture.m_feeds[0]->m_server_channel->get_writer().write(
      fixture.make_packet({2}, flags));
    flush_pending_routines();
    fixture.m_client->close();
    for(auto sequence : {1, 2, 3}) {
      fixture.require_message(sequence, 0);
    }
    REQUIRE_THROWS_AS(fixture.m_client->read(), EndOfFileException);
  }

  TEST_CASE("close_pending_read") {
    auto fixture = Fixture(2);
    auto read = std::packaged_task([&] {
      return fixture.m_client->read();
    });
    auto result = read.get_future();
    auto routine = RoutineHandler(spawn([&] {
      read();
    }));
    flush_pending_routines();
    fixture.m_client->close();
    routine.wait();
    REQUIRE_THROWS_AS(result.get(), EndOfFileException);
    REQUIRE_NOTHROW(fixture.m_client->close());
    REQUIRE_NOTHROW(fixture.m_time_client.get_time());
    for(auto& feed : fixture.m_feeds) {
      REQUIRE_THROWS_AS(feed->m_client->read(), IOException);
    }
  }

  TEST_CASE("feed_failure") {
    auto fixture = Fixture(2);
    fixture.publish(0, {1});
    fixture.require_message(1, 0);
    fixture.m_feeds[0]->m_server_channel->get_writer().close();
    flush_pending_routines();
    fixture.publish(1, {2});
    fixture.require_message(2, 0);
    REQUIRE(fixture.m_log.m_output.str().starts_with("(feed_failed 0 "));
    fixture.m_feeds[1]->m_server_channel->get_writer().close();
    flush_pending_routines();
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
  }

  TEST_CASE("malformed_packet") {
    auto fixture = Fixture(2);
    auto source = fixture.make_packet({1, 2}, 0);
    SUBCASE("header") {
      source.shrink(1);
    }
    SUBCASE("message_sequence") {
      source.shrink(1);
      auto length = endian::native_to_big(
        static_cast<std::uint16_t>(source.get_size()));
      source.write(0, &length, sizeof(length));
      auto message_length = OtcLinkMessage::HEADER_LENGTH +
        sizeof(std::uint32_t);
      auto short_length = endian::native_to_big(
        static_cast<std::uint16_t>(message_length - 1));
      source.write(OtcLinkHeader::LENGTH + message_length,
        &short_length, sizeof(short_length));
    }
    SUBCASE("reset_timestamp") {
      source = fixture.make_packet({},
        static_cast<std::uint8_t>(OtcLinkHeader::Flag::SEQUENCE_RESET));
      auto milliseconds = endian::native_to_big(
        std::numeric_limits<std::uint32_t>::max());
      source.write(OtcLinkHeader::LENGTH - sizeof(milliseconds),
        &milliseconds, sizeof(milliseconds));
    }
    fixture.m_feeds[0]->m_server_channel->get_writer().write(source);
    flush_pending_routines();
    fixture.publish(1, {1, 2});
    fixture.publish(0, {3});
    fixture.m_client->close();
    for(auto sequence : {1, 2, 3}) {
      fixture.require_message(sequence, 0);
    }
    REQUIRE(fixture.m_log.m_output.str().starts_with("(bad_datagram 0 "));
    REQUIRE_THROWS_AS(fixture.m_client->read(), EndOfFileException);
  }

  TEST_CASE("channel_reset") {
    auto fixture = Fixture(2);
    fixture.publish(0, {40, 42});
    fixture.publish(1, {40});
    auto reset = fixture.make_packet({},
      static_cast<std::uint8_t>(OtcLinkHeader::Flag::SEQUENCE_RESET));
    fixture.publish(0, OtcLinkHeader::Flag::SEQUENCE_RESET);
    fixture.publish(1, {41});
    fixture.publish(0, {1, 3});
    fixture.publish(0, OtcLinkHeader::Flag::SEQUENCE_RESET);
    fixture.publish(1, OtcLinkHeader::Flag::SEQUENCE_RESET);
    fixture.publish(1, {2, 3});
    fixture.require_message(40, 0);
    for(auto sequence : {1, 2, 3}) {
      fixture.require_message(sequence, 1);
    }
    fixture.advance(milliseconds(1));
    fixture.publish(1, OtcLinkHeader::Flag::SEQUENCE_RESET);
    fixture.publish(0, {4});
    fixture.publish(1, {1});
    fixture.publish(0, OtcLinkHeader::Flag::SEQUENCE_RESET);
    fixture.publish(0, {2});
    fixture.m_feeds[0]->m_server_channel->get_writer().write(reset);
    flush_pending_routines();
    fixture.publish(0, {3});
    fixture.m_client->close();
    for(auto sequence : {1, 2, 3}) {
      fixture.require_message(sequence, 2);
    }
    REQUIRE_THROWS_AS(fixture.m_client->read(), EndOfFileException);
  }

  TEST_CASE("delayed_feed") {
    auto fixture = Fixture(2);
    fixture.publish(0, {1, 3});
    fixture.require_message(1, 0);
    fixture.advance(Fixture::GAP_TIMEOUT);
    fixture.publish(1, {1, 2, 3});
    fixture.require_message(2, 0);
    fixture.require_message(3, 0);
    fixture.advance(Fixture::GAP_TIMEOUT + time_duration::unit());
    REQUIRE(fixture.m_log.m_output.str().empty());
  }

  TEST_CASE("stalled_feed") {
    auto fixture = Fixture(2);
    fixture.publish(0, {1});
    fixture.publish(1, {1});
    fixture.require_message(1, 0);
    fixture.publish(0, {3});
    fixture.advance(Fixture::FEED_TIMEOUT);
    SUBCASE("silence") {}
    SUBCASE("heartbeat") {
      fixture.publish(1, OtcLinkHeader::Flag::HEARTBEAT);
    }
    SUBCASE("duplicate") {
      fixture.publish(1, {1});
    }
    REQUIRE(fixture.m_log.m_output.str().empty());
    fixture.advance(time_duration::unit());
    fixture.advance(Fixture::GAP_TIMEOUT);
    REQUIRE(fixture.m_log.m_output.str().empty());
    fixture.advance(time_duration::unit());
    fixture.require_message(3, 0);
    auto expected = std::stringstream();
    expected << "(dropped " << fixture.m_time_client.get_time() <<
      " 2 1 recovery_disabled)\n";
    REQUIRE(fixture.m_log.m_output.str() == expected.str());
  }

  TEST_CASE("gap_timeout") {
    auto fixture = Fixture(1);
    fixture.publish(0, {1, 3});
    fixture.require_message(1, 0);
    fixture.advance(Fixture::GAP_TIMEOUT);
    REQUIRE(fixture.m_log.m_output.str().empty());
    SUBCASE("timer") {
      fixture.advance(time_duration::unit());
      fixture.require_message(3, 0);
    }
    SUBCASE("feed") {
      fixture.m_time_client.set(
        fixture.m_time_client.get_time() + time_duration::unit());
      fixture.publish(0, {4});
      fixture.require_message(3, 0);
      fixture.require_message(4, 0);
    }
    auto expected = std::stringstream();
    expected << "(dropped " << fixture.m_time_client.get_time() <<
      " 2 1 recovery_disabled)\n";
    REQUIRE(fixture.m_log.m_output.str() == expected.str());
    fixture.publish(0, {2});
    fixture.m_client->close();
    REQUIRE_THROWS_AS(fixture.m_client->read(), EndOfFileException);
  }

  TEST_CASE("feed_arbitration") {
    auto fixture = Fixture(3);
    fixture.publish(0, {1, 3});
    fixture.publish(1, {1, 2});
    fixture.publish(2, {2, 3, 4});
    for(auto sequence : {1, 2, 3, 4}) {
      fixture.require_message(sequence, 0);
    }
    fixture.publish(0, {1, 2, 3, 4});
    fixture.m_client->close();
    REQUIRE_THROWS_AS(fixture.m_client->read(), EndOfFileException);
    REQUIRE(fixture.m_log.m_output.str().empty());
  }
}
