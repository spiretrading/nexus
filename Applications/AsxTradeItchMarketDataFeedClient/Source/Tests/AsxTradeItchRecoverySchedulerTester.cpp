#include <future>
#include <Beam/IO/LocalServerConnection.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchRecoveryScheduler.hpp"
#include "Nexus/MoldUdp64/MoldUdp64Client.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct Fixture {
    LocalServerConnection m_server;
    std::unique_ptr<LocalServerChannel> m_server_channel;
    optional<MoldUdp64Client<LocalClientChannel>> m_client;
    AsxTradeItchSequencer::Session m_session;
    ptime m_timestamp;
    time_duration m_timeout;
    AsxTradeItchSequencer m_sequencer;
    AsxTradeItchRecoveryScheduler m_recovery;

    Fixture()
        : m_session("SESSION123"),
          m_timestamp(time_from_string("2026-09-18 10:00:00")),
          m_timeout(duration_from_string("00:00:01")),
          m_sequencer(1, duration_from_string("00:00:03")),
          m_recovery(m_timeout) {
      auto connection = std::async(std::launch::async, [&] {
        return m_server.accept();
      });
      m_client.emplace(init("rewind", m_server));
      m_server_channel = connection.get();
    }

    SharedBuffer encode_packet(
        std::uint64_t sequence, const std::vector<std::string_view>& messages) {
      auto source = SharedBuffer();
      encode(MoldUdp64Request(m_session, sequence,
        static_cast<std::uint16_t>(messages.size())), out(source));
      for(auto message : messages) {
        append(source,
          endian::native_to_big(static_cast<std::uint16_t>(message.size())));
        append(source, message);
      }
      return source;
    }

    void add(
        std::uint64_t sequence, const std::vector<std::string_view>& messages) {
      auto source = encode_packet(sequence, messages);
      m_sequencer.add(0, MoldUdp64Packet::parse(
        std::string_view(source.get_data(), source.get_size())), m_timestamp);
    }

    void recover(
        std::uint64_t sequence, const std::vector<std::string_view>& messages) {
      m_server_channel->get_writer().write(encode_packet(sequence, messages));
      m_sequencer.recover(m_client->read());
    }

    void require_message(std::string_view expected) {
      auto message = m_sequencer.read();
      REQUIRE(message.has_value());
      REQUIRE(*message == expected);
    }

    void require_request(std::uint64_t sequence, std::uint16_t count) {
      auto request = m_recovery.request(m_sequencer, m_timestamp);
      REQUIRE(request.has_value());
      REQUIRE(request->m_session == m_session);
      REQUIRE(request->m_sequence_number == sequence);
      REQUIRE(request->m_count == count);
      m_client->request(*request);
      auto source = SharedBuffer();
      m_server_channel->get_reader().read(out(source));
      auto expected = SharedBuffer();
      encode(MoldUdp64Request(m_session, sequence, count), out(expected));
      REQUIRE(source == expected);
    }
  };
}

TEST_SUITE("AsxTradeItchRecoveryScheduler") {
  TEST_CASE("timeout") {
    REQUIRE_THROWS_AS(
      AsxTradeItchRecoveryScheduler(duration_from_string("00:00:00")),
      std::invalid_argument);
    REQUIRE_THROWS_AS(
      AsxTradeItchRecoveryScheduler(duration_from_string("-00:00:01")),
      std::invalid_argument);
    REQUIRE_THROWS_AS(
      AsxTradeItchRecoveryScheduler(time_duration(not_a_date_time)),
      std::invalid_argument);
  }

  TEST_CASE_FIXTURE(Fixture, "reset") {
    add(1, {"one"});
    require_message("one");
    add(4, {"four"});
    require_request(2, 2);
    SUBCASE("request") {
      m_recovery.reset();
    }
    SUBCASE("session") {
      m_session = "SESSION456";
      m_sequencer.reset(m_session, 2);
      add(4, {"four"});
    }
    SUBCASE("sequence") {
      m_sequencer.reset(3);
      require_request(3, 1);
      m_sequencer.reset(2);
    }
    require_request(2, 2);
    recover(2, {"two", "three"});
    require_message("two");
    require_message("three");
    require_message("four");
    REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
  }

  TEST_CASE_FIXTURE(Fixture, "request_range") {
    auto start = std::uint64_t(1) << 40;
    m_sequencer.reset(start);
    SUBCASE("count_limit") {
      auto maximum = std::numeric_limits<std::uint16_t>::max();
      add(start + maximum + 1, {});
      require_request(start, maximum);
      recover(start, {"first", "second"});
      require_message("first");
      require_message("second");
      require_request(start + 2, maximum - 1);
    }
    SUBCASE("end_of_session") {
      auto source = SharedBuffer();
      encode(MoldUdp64Request(m_session, start + 2,
        MoldUdp64Packet::END_OF_SESSION), out(source));
      m_sequencer.add(0, MoldUdp64Packet::parse(
        std::string_view(source.get_data(), source.get_size())), m_timestamp);
      REQUIRE(!m_sequencer.is_end_of_session());
      require_request(start, 2);
      recover(start, {"first", "last"});
      require_message("first");
      require_message("last");
      REQUIRE(m_sequencer.is_end_of_session());
      REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
    }
  }

  TEST_CASE_FIXTURE(Fixture, "live_recovery") {
    add(1, {"one"});
    require_message("one");
    add(5, {"five"});
    require_request(2, 3);
    SUBCASE("complete") {
      add(2, {"two", "three", "four"});
      require_message("two");
      require_message("three");
      require_message("four");
      require_message("five");
      REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
      recover(2, {"two", "three", "four"});
      REQUIRE(!m_sequencer.read());
      m_timestamp += m_timeout;
      REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
    }
    SUBCASE("prefix") {
      add(2, {"two"});
      require_message("two");
      require_request(3, 2);
      recover(2, {"two", "three", "four"});
      require_message("three");
      require_message("four");
      require_message("five");
      REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
    }
    SUBCASE("suffix") {
      add(4, {"four"});
      REQUIRE(!m_sequencer.read());
      REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
      m_timestamp += m_timeout;
      require_request(2, 2);
    }
  }

  TEST_CASE_FIXTURE(Fixture, "request_timeout") {
    add(1, {"one"});
    require_message("one");
    add(4, {"four"});
    require_request(2, 2);
    m_timestamp += m_timeout / 2;
    SUBCASE("heartbeat") {
      recover(2, {});
    }
    SUBCASE("duplicate_reply") {
      recover(1, {"one"});
    }
    SUBCASE("different_session") {
      m_session = "SESSION456";
      recover(2, {"two", "three"});
      m_session = "SESSION123";
    }
    SUBCASE("malformed_reply") {
      m_server_channel->get_writer().write(SharedBuffer("SHORT", 5));
      REQUIRE_THROWS_AS(m_client->read(), MoldUdp64ParserException);
    }
    REQUIRE(!m_sequencer.read());
    REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
    m_timestamp += m_timeout / 2;
    require_request(2, 2);
    REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
    recover(2, {"two", "three"});
    require_message("two");
    require_message("three");
    require_message("four");
    REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
  }

  TEST_CASE_FIXTURE(Fixture, "partial_reply") {
    REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
    add(1, {"one"});
    require_message("one");
    add(6, {"six"});
    require_request(2, 4);
    REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
    recover(2, {"two", "three"});
    require_message("two");
    require_message("three");
    require_request(4, 2);
    recover(4, {"four", "five"});
    require_message("four");
    require_message("five");
    require_message("six");
    REQUIRE(!m_sequencer.read());
    REQUIRE(!m_recovery.request(m_sequencer, m_timestamp));
  }
}
