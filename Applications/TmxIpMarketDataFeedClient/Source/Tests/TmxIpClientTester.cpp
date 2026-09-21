#include <format>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/StandardTimeZones.hpp"
#include "TmxIpMarketDataFeedClient/TmxIpClient.hpp"

using namespace Beam;
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

  struct ProtocolClient {
    Queue<std::string> m_packets;
    std::string m_packet;

    TmxIpPacket read() {
      m_packet = m_packets.pop();
      return TmxIpPacket::parse(m_packet);
    }

    void close() {
      m_packets.close(std::make_exception_ptr(EndOfFileException()));
    }
  };

  struct RecoveryClient {
    ProtocolClient m_protocol_client;
    Queue<TmxIpRecoveryRequest> m_requests;
    Queue<TmxIpRecoveryResult> m_results;
    Queue<std::uint64_t> m_sessions;
    std::atomic_uint32_t m_maximum_count = 10000;
    std::atomic_uint64_t m_session = 0;

    std::uint32_t get_maximum_count() const {
      return m_maximum_count;
    }

    TmxIpRecoveryResult request(const TmxIpRecoveryRequest& request) {
      m_requests.push(request);
      return m_results.pop();
    }

    TmxIpPacket read() {
      return m_protocol_client.read();
    }

    TmxIpRecoveryResult request(
        const TmxIpRecoveryRequest& request, std::uint64_t session) {
      if(session != m_session) {
        throw IOException("Expired session.");
      }
      return this->request(request);
    }

    TmxIpPacket read(Out<std::uint64_t> session) {
      auto packet = read();
      *session = m_sessions.pop();
      return packet;
    }

    void reset(std::uint64_t session) {
      m_session = session;
    }

    void close() {
      m_protocol_client.close();
      m_requests.close();
      m_results.close();
      m_sessions.close();
    }
  };

  struct WithoutRecovery {};

  struct Fixture {
    inline static const auto GAP_TIMEOUT = seconds(1);
    std::vector<std::unique_ptr<ProtocolClient>> m_feed_clients;
    RecoveryClient m_recovery_client;
    FixedTimeClient m_time_client;
    TriggerTimer m_timer;
    TmxIpClient<ProtocolClient*, RecoveryClient*, FixedTimeClient*,
      TriggerTimer*> m_client;

    Fixture()
      : Fixture(1) {}

    explicit Fixture(std::size_t feeds)
      : Fixture(feeds, &m_recovery_client) {}

    explicit Fixture(ptime timestamp)
      : Fixture(1, &m_recovery_client, timestamp) {}

    Fixture(std::size_t feeds, WithoutRecovery)
      : Fixture(feeds, boost::none) {}

    Fixture(std::size_t feeds, boost::optional<RecoveryClient*> recovery)
      : Fixture(feeds, recovery, time_from_string("2026-09-20 12:00:00")) {}

    Fixture(std::size_t feeds, boost::optional<RecoveryClient*> recovery,
        ptime timestamp)
      : Fixture(feeds, recovery, timestamp,
          TIME_ZONES.time_zone_from_region("America/Toronto"), minutes(30)) {}

    Fixture(std::size_t feeds, boost::optional<RecoveryClient*> recovery,
        ptime timestamp, boost::local_time::time_zone_ptr time_zone,
        time_duration rollover_time)
        : m_time_client(timestamp),
          m_client(time_zone, rollover_time, seconds(3), GAP_TIMEOUT, [&] {
            auto clients = std::vector<ProtocolClient*>();
            for(auto i = std::size_t(0); i != feeds; ++i) {
              m_feed_clients.push_back(std::make_unique<ProtocolClient>());
              clients.push_back(m_feed_clients.back().get());
            }
            return clients;
          }(), recovery, &m_time_client, &m_timer) {}

    void publish(std::uint32_t sequence, std::string_view payload,
        TmxIpHeader::Continuation continuation, ProtocolClient& client) {
      publish(sequence, payload, continuation, client,
        m_recovery_client.m_session);
    }

    void publish(std::uint32_t sequence, std::string_view payload,
        TmxIpHeader::Continuation continuation, ProtocolClient& client,
        std::uint64_t session) {
      if(&client == &m_recovery_client.m_protocol_client) {
        m_recovery_client.m_sessions.push(session);
      }
      client.m_packets.push(std::format("{}{:04}{:09}CDF0{}  T {}{}",
        TmxIpPacket::START, TmxIpHeader::LENGTH + payload.size(), sequence,
        static_cast<int>(continuation), payload, TmxIpPacket::END));
    }

    void publish(std::uint32_t sequence, std::string_view symbol,
        ProtocolClient& client) {
      publish(sequence, std::format("\x01\x1e" "1=H\x1c\x1e" "55={}\x1d",
        symbol), TmxIpHeader::Continuation::NONE, client);
    }

    void publish(std::uint32_t sequence, std::string_view symbol) {
      publish(sequence, symbol, *m_feed_clients.front());
    }

    void heartbeat(std::uint32_t sequence) {
      heartbeat(sequence, *m_feed_clients.front());
    }

    void heartbeat(std::uint32_t sequence, ProtocolClient& client) {
      auto payload = std::format(
        "[HEARTBEAT 2012-10-10 03:25:02-001349853902.844623]"
        "[LAST SENT {:09}-03:05:03-001349852703.441869]"
        "[LAST HB   {:09}-03:24:02-001349853842.845443]"
        "OCSA-CDF-1           ATDOTDR  00.1", sequence, sequence);
      client.m_packets.push(std::format(
        "{}{:04}         CDF00V T {}{}", TmxIpPacket::START,
        TmxIpHeader::LENGTH + payload.size(), payload, TmxIpPacket::END));
    }

    void recover(std::uint32_t sequence, std::string_view symbol) {
      publish(sequence, symbol, m_recovery_client.m_protocol_client);
    }

    void recover(std::uint32_t sequence, std::string_view symbol,
        std::uint64_t session) {
      publish(sequence, std::format("\x01\x1e" "1=H\x1c\x1e" "55={}\x1d",
        symbol), TmxIpHeader::Continuation::NONE,
        m_recovery_client.m_protocol_client, session);
    }

    void complete(const TmxIpRecoveryRequest& request) {
      auto count = request.m_end_sequence - request.m_start_sequence + 1;
      m_recovery_client.m_results.push(TmxIpRecoveryResult(true,
        TmxIpRecoveryResponse::Status::ACCEPTED, request.m_start_sequence,
        request.m_end_sequence, count, count, ""));
      flush_pending_routines();
    }

    TmxIpRecoveryRequest require_request(
        std::uint32_t start, std::uint32_t end) {
      auto request = m_recovery_client.m_requests.pop();
      REQUIRE(request.m_start_sequence == start);
      REQUIRE(request.m_end_sequence == end);
      return request;
    }

    void require_symbol(std::string_view expected) {
      auto message = m_client.read();
      auto symbol = message.m_business_content.find(55);
      REQUIRE(symbol.has_value());
      REQUIRE(symbol->m_value == expected);
    }
  };
}

TEST_SUITE("TmxIpClient") {
  TEST_CASE("configured_rollover") {
    auto rollover = time_from_string("2026-09-20 20:00:00");
    auto fixture = Fixture(1, boost::none, rollover - minutes(1),
      TIME_ZONES.time_zone_from_region("Australia/Sydney"), hours(6));
    fixture.publish(100, "OLD");
    fixture.require_symbol("OLD");
    fixture.m_time_client.set(rollover - time_duration::unit());
    fixture.m_timer.trigger();
    flush_pending_routines();
    fixture.publish(101, "BEFORE");
    fixture.require_symbol("BEFORE");
    fixture.m_time_client.set(rollover);
    fixture.publish(1, "NEW");
    flush_pending_routines();
    fixture.m_time_client.set(rollover + hours(24));
    fixture.m_timer.trigger();
    flush_pending_routines();
    fixture.publish(1, "NEXT");
    flush_pending_routines();
    fixture.m_client.close();
    fixture.require_symbol("NEW");
    fixture.require_symbol("NEXT");
    REQUIRE_THROWS_AS(fixture.m_client.read(), EndOfFileException);
  }

  TEST_CASE("session_initial_gap") {
    auto rollover = time_from_string("2026-09-21 04:30:00");
    auto fixture = Fixture(rollover - minutes(1));
    fixture.publish(100, "OLD");
    fixture.require_symbol("OLD");
    fixture.m_time_client.set(rollover);
    fixture.publish(2, "TWO");
    auto request = fixture.require_request(1, 1);
    fixture.recover(1, "ONE");
    fixture.require_symbol("ONE");
    fixture.require_symbol("TWO");
    fixture.complete(request);
  }

  TEST_CASE("session_fragments") {
    auto rollover = time_from_string("2026-09-21 04:30:00");
    auto fixture = Fixture(2, boost::none, rollover - minutes(1));
    fixture.publish(100, "OLD");
    fixture.require_symbol("OLD");
    fixture.publish(101, "\x01\x1e" "1=H\x1c\x1e" "55=OLD",
      TmxIpHeader::Continuation::FIRST, *fixture.m_feed_clients.front());
    flush_pending_routines();
    fixture.m_time_client.set(rollover);
    fixture.publish(1, "NEW", *fixture.m_feed_clients.back());
    fixture.publish(1, "NEW");
    fixture.publish(2, "NEXT");
    fixture.require_symbol("NEW");
    fixture.require_symbol("NEXT");
    flush_pending_routines();
    fixture.m_client.close();
    REQUIRE_THROWS_AS(fixture.m_client.read(), EndOfFileException);
  }

  TEST_CASE("session_rollover") {
    auto rollover = time_from_string("2026-09-21 04:30:00");
    SUBCASE("summer") {}
    SUBCASE("winter") {
      rollover = time_from_string("2026-12-22 05:30:00");
    }
    auto fixture = Fixture(rollover - hours(1));
    fixture.publish(100, "OLD");
    fixture.require_symbol("OLD");
    fixture.m_time_client.set(rollover - time_duration::unit());
    fixture.m_timer.trigger();
    flush_pending_routines();
    fixture.publish(101, "BEFORE");
    fixture.require_symbol("BEFORE");
    fixture.m_time_client.set(rollover);
    SUBCASE("timer") {
      fixture.m_timer.trigger();
      flush_pending_routines();
    }
    SUBCASE("packet") {}
    fixture.publish(1, "NEW");
    fixture.publish(2, "AFTER");
    flush_pending_routines();
    fixture.require_symbol("NEW");
    fixture.require_symbol("AFTER");
    fixture.m_client.close();
    REQUIRE_THROWS_AS(fixture.m_client.read(), EndOfFileException);
  }

  TEST_CASE("session_recovery") {
    auto rollover = time_from_string("2026-09-21 04:30:00");
    auto fixture = Fixture(rollover - minutes(1));
    fixture.publish(1, "OLD");
    fixture.require_symbol("OLD");
    fixture.publish(3, "BUFFERED");
    fixture.require_request(2, 2);
    fixture.m_time_client.set(rollover);
    fixture.m_timer.trigger();
    flush_pending_routines();
    fixture.publish(1, "NEW");
    fixture.require_symbol("NEW");
    fixture.publish(3, "THREE");
    fixture.recover(2, "STALE", 0);
    fixture.m_recovery_client.m_results.push(TmxIpRecoveryResult(false,
      TmxIpRecoveryResponse::Status::REJECTED, 2, 2, 0, 0, "old"));
    auto request = fixture.require_request(2, 2);
    fixture.recover(2, "TWO");
    fixture.require_symbol("TWO");
    fixture.require_symbol("THREE");
    fixture.complete(request);
  }

  TEST_CASE("message_sequence") {
    auto fixture = Fixture();
    fixture.publish(1, "ABX");
    auto message = fixture.m_client.read();
    fixture.publish(1, "DUPLICATE");
    fixture.publish(2, "XYZ");
    flush_pending_routines();
    auto symbol = message.m_business_content.find(55);
    REQUIRE(symbol.has_value());
    REQUIRE(symbol->m_value == "ABX");
    fixture.require_symbol("XYZ");
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
  }

  TEST_CASE("gap_recovery") {
    auto fixture = Fixture();
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    fixture.publish(4, "FOUR");
    auto request = fixture.require_request(2, 3);
    fixture.publish(5, "FIVE");
    fixture.recover(3, "THREE");
    fixture.recover(2, "TWO");
    fixture.require_symbol("TWO");
    fixture.require_symbol("THREE");
    fixture.require_symbol("FOUR");
    fixture.require_symbol("FIVE");
    fixture.publish(6, "SIX");
    fixture.require_symbol("SIX");
    fixture.complete(request);
    fixture.m_timer.trigger();
    flush_pending_routines();
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
  }


  TEST_CASE("partial_recovery") {
    auto fixture = Fixture();
    fixture.m_recovery_client.m_maximum_count = 2;
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    fixture.publish(6, "SIX");
    auto request = fixture.require_request(2, 3);
    fixture.m_timer.trigger();
    flush_pending_routines();
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
    fixture.recover(2, "TWO");
    fixture.require_symbol("TWO");
    fixture.complete(request);
    fixture.publish(7, "SEVEN");
    flush_pending_routines();
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
    fixture.m_timer.trigger();
    request = fixture.require_request(3, 4);
    fixture.recover(4, "FOUR");
    fixture.recover(3, "THREE");
    fixture.require_symbol("THREE");
    fixture.require_symbol("FOUR");
    fixture.complete(request);
    fixture.m_timer.trigger();
    request = fixture.require_request(5, 5);
    fixture.recover(5, "FIVE");
    fixture.require_symbol("FIVE");
    fixture.require_symbol("SIX");
    fixture.require_symbol("SEVEN");
    fixture.complete(request);
  }


  TEST_CASE("fragment_recovery") {
    auto fixture = Fixture();
    auto first = std::uint32_t(1);
    auto middle = std::uint32_t(2);
    auto last = std::uint32_t(3);
    SUBCASE("consecutive") {}
    SUBCASE("sequence_wrap") {
      first = 999999998;
      middle = 999999999;
      last = 1;
    }
    fixture.publish(first, "\x01\x1e" "1=H",
      TmxIpHeader::Continuation::FIRST, *fixture.m_feed_clients.front());
    fixture.publish(last, "X\x1d", TmxIpHeader::Continuation::LAST,
      *fixture.m_feed_clients.front());
    auto request = fixture.require_request(middle, middle);
    fixture.publish(middle, "\x1c\x1e" "55=AB",
      TmxIpHeader::Continuation::MIDDLE,
      fixture.m_recovery_client.m_protocol_client);
    fixture.require_symbol("ABX");
    fixture.complete(request);
    fixture.publish(last + 1, "XYZ");
    fixture.require_symbol("XYZ");
  }


  TEST_CASE("heartbeat_gap") {
    auto fixture = Fixture();
    fixture.heartbeat(0);
    flush_pending_routines();
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
    fixture.heartbeat(2);
    auto request = fixture.require_request(1, 2);
    fixture.recover(2, "TWO");
    fixture.recover(1, "ONE");
    fixture.require_symbol("ONE");
    fixture.require_symbol("TWO");
    fixture.complete(request);
    fixture.m_timer.trigger();
    flush_pending_routines();
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
  }


  TEST_CASE("gap_timeout") {
    auto log = Log();
    auto fixture = Fixture();
    auto is_limited = false;
    auto is_complete = false;
    SUBCASE("pending_request") {}
    SUBCASE("completed_request") {
      is_complete = true;
    }
    SUBCASE("zero_request_limit") {
      is_limited = true;
      fixture.m_recovery_client.m_maximum_count = 0;
    }
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    fixture.publish(4, "FOUR");
    if(!is_limited) {
      auto request = fixture.require_request(2, 3);
      if(is_complete) {
        fixture.complete(request);
      }
    }
    flush_pending_routines();
    auto timestamp = fixture.m_time_client.get_time();
    fixture.m_time_client.set(timestamp + Fixture::GAP_TIMEOUT);
    fixture.m_timer.trigger();
    flush_pending_routines();
    REQUIRE(log.m_output.str().empty());
    fixture.m_time_client.set(
      fixture.m_time_client.get_time() + time_duration::unit());
    fixture.m_timer.trigger();
    flush_pending_routines();
    fixture.publish(5, "FIVE");
    flush_pending_routines();
    fixture.m_client.close();
    fixture.require_symbol("FOUR");
    fixture.require_symbol("FIVE");
    REQUIRE(log.m_output.str() ==
      "(dropped 2026-Sep-20 12:00:01.000001 2 2 timeout)\n");
  }

  TEST_CASE("recovery_failure") {
    auto log = Log();
    auto fixture = Fixture();
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    fixture.publish(3, "THREE");
    fixture.require_request(2, 2);
    auto reason = std::string("rejected Rejected");
    SUBCASE("rejection") {
      fixture.m_recovery_client.m_results.push(TmxIpRecoveryResult(false,
        TmxIpRecoveryResponse::Status::REJECTED, 2, 2, 0, 0, "Rejected"));
    }
    SUBCASE("no_history") {
      reason = "rejected No history";
      fixture.m_recovery_client.m_results.push(TmxIpRecoveryResult(true,
        TmxIpRecoveryResponse::Status::ACCEPTED, 0, 0, 0, 0, "No history"));
    }
    SUBCASE("request_error") {
      reason = "recovery_failed Request failed.";
      fixture.m_recovery_client.m_results.close(
        std::make_exception_ptr(IOException("Request failed.")));
    }
    SUBCASE("recovery_feed") {
      reason = "recovery_failed Recovery feed failed.";
      fixture.m_recovery_client.m_protocol_client.m_packets.close(
        std::make_exception_ptr(IOException("Recovery feed failed.")));
    }
    flush_pending_routines();
    fixture.publish(4, "FOUR");
    flush_pending_routines();
    fixture.m_client.close();
    fixture.require_symbol("THREE");
    fixture.require_symbol("FOUR");
    REQUIRE(log.m_output.str() ==
      "(dropped 2026-Sep-20 12:00:00 2 1 " + reason + ")\n");
  }

  TEST_CASE("failed_recovery_range") {
    auto log = Log();
    auto fixture = Fixture();
    fixture.m_recovery_client.m_maximum_count = 2;
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    fixture.publish(6, "SIX");
    fixture.require_request(2, 3);
    fixture.recover(2, "TWO");
    fixture.require_symbol("TWO");
    fixture.publish(4, "FOUR");
    flush_pending_routines();
    fixture.m_recovery_client.m_results.push(TmxIpRecoveryResult(false,
      TmxIpRecoveryResponse::Status::REJECTED, 2, 3, 0, 0, "Rejected"));
    fixture.require_symbol("FOUR");
    flush_pending_routines();
    REQUIRE(log.m_output.str() ==
      "(dropped 2026-Sep-20 12:00:00 3 1 rejected Rejected)\n");
    fixture.m_timer.trigger();
    auto request = fixture.require_request(5, 5);
    fixture.recover(5, "FIVE");
    fixture.require_symbol("FIVE");
    fixture.require_symbol("SIX");
    fixture.complete(request);
  }

  TEST_CASE("transport_failure") {
    auto fixture = Fixture();
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    auto error = std::make_exception_ptr(IOException("Connection failed."));
    SUBCASE("live_feed") {
      fixture.m_feed_clients.front()->m_packets.close(error);
    }

    SUBCASE("timer") {
      fixture.m_timer.fail();
    }
    REQUIRE_THROWS_AS(fixture.m_client.read(), IOException);
  }


  TEST_CASE("malformed_message") {
    auto fixture = Fixture();
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    SUBCASE("live") {
      fixture.publish(2, "not STAMP", TmxIpHeader::Continuation::NONE,
        *fixture.m_feed_clients.front());
    }
    SUBCASE("recovery") {
      fixture.publish(3, "THREE");
      fixture.require_request(2, 2);
      fixture.publish(2, "not STAMP", TmxIpHeader::Continuation::NONE,
        fixture.m_recovery_client.m_protocol_client);
    }
    SUBCASE("assembly") {
      fixture.publish(2, "\x01\x1e" "1=H", TmxIpHeader::Continuation::FIRST,
        *fixture.m_feed_clients.front());
      fixture.publish(3, "missing delimiter", TmxIpHeader::Continuation::LAST,
        *fixture.m_feed_clients.front());
    }
    REQUIRE_THROWS_AS(fixture.m_client.read(), StampParserException);
    fixture.publish(4, "FOUR");
    flush_pending_routines();
    REQUIRE_THROWS_AS(fixture.m_client.read(), StampParserException);
  }


  TEST_CASE("malformed_packet") {
    auto fixture = Fixture();
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    SUBCASE("framing") {
      fixture.m_feed_clients.front()->m_packets.push("not a packet");
    }
    SUBCASE("heartbeat") {
      fixture.m_feed_clients.front()->m_packets.push(
        "\x02" "0022         CDF00V T \x03");
    }
    fixture.publish(3, "THREE");
    auto request = fixture.require_request(2, 2);
    fixture.recover(2, "TWO");
    fixture.require_symbol("TWO");
    fixture.require_symbol("THREE");
    fixture.complete(request);
  }


  TEST_CASE("close") {
    auto fixture = Fixture();
    SUBCASE("idle") {}
    SUBCASE("request") {
      fixture.publish(1, "ONE");
      fixture.require_symbol("ONE");
      fixture.publish(3, "THREE");
      fixture.require_request(2, 2);
    }
    SUBCASE("retry") {
      fixture.publish(1, "ONE");
      fixture.require_symbol("ONE");
      fixture.publish(3, "THREE");
      fixture.complete(fixture.require_request(2, 2));
    }
    fixture.m_client.close();
    REQUIRE_THROWS_AS(fixture.m_client.read(), EndOfFileException);
  }


  TEST_CASE("zero_request_limit") {
    auto fixture = Fixture();
    fixture.m_recovery_client.m_maximum_count = 0;
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    fixture.publish(3, "THREE");
    flush_pending_routines();
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
    fixture.m_recovery_client.m_maximum_count = 1;
    fixture.m_timer.trigger();
    auto request = fixture.require_request(2, 2);
    fixture.recover(2, "TWO");
    fixture.require_symbol("TWO");
    fixture.require_symbol("THREE");
    fixture.complete(request);
  }

  TEST_CASE("feed_arbitration") {
    auto fixture = Fixture(2);
    auto first = std::uint32_t(1);
    SUBCASE("consecutive") {}
    SUBCASE("wrap") {
      first = 999999999;
    }
    auto second = first % 999999999 + 1;
    auto third = second + 1;
    fixture.publish(first, "ONE");
    fixture.require_symbol("ONE");
    auto& other = *fixture.m_feed_clients[1];
    fixture.publish(first, "ONE", other);
    fixture.publish(third, "THREE");
    flush_pending_routines();
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
    fixture.publish(second, "TWO", other);
    fixture.require_symbol("TWO");
    fixture.require_symbol("THREE");
    fixture.publish(third, "THREE", other);
    fixture.publish(third + 1, "FOUR", other);
    fixture.require_symbol("FOUR");
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
  }

  TEST_CASE("gap_confirmation") {
    auto fixture = Fixture(2);
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    auto& other = *fixture.m_feed_clients[1];
    fixture.publish(1, "ONE", other);
    fixture.publish(4, "FOUR");
    flush_pending_routines();
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
    auto end = std::uint32_t(3);
    SUBCASE("data") {
      fixture.publish(4, "FOUR", other);
    }
    SUBCASE("heartbeat") {
      fixture.heartbeat(3, other);
    }
    SUBCASE("confirmed_prefix") {
      fixture.heartbeat(2, other);
      end = 2;
    }
    auto request = fixture.require_request(2, end);
    fixture.recover(2, "TWO");
    if(end == 2) {
      fixture.publish(3, "THREE", other);
    } else {
      fixture.recover(3, "THREE");
    }
    fixture.require_symbol("TWO");
    fixture.require_symbol("THREE");
    fixture.require_symbol("FOUR");
    fixture.complete(request);
  }

  TEST_CASE("clock_rollback") {
    auto log = Log();
    auto fixture = Fixture(2);
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    fixture.publish(1, "ONE", *fixture.m_feed_clients[1]);
    fixture.publish(3, "THREE");
    flush_pending_routines();
    auto timestamp = fixture.m_time_client.get_time();
    fixture.m_time_client.set(timestamp + seconds(4));
    fixture.m_timer.trigger();
    auto request = fixture.require_request(2, 2);
    fixture.m_time_client.set(timestamp + seconds(1));
    fixture.m_timer.trigger();
    flush_pending_routines();
    fixture.complete(request);
    fixture.m_client.close();
    fixture.require_symbol("THREE");
    REQUIRE(log.m_output.str() ==
      "(dropped 2026-Sep-20 12:00:01 2 1 clock_rollback)\n");
  }

  TEST_CASE("stalled_feed") {
    auto fixture = Fixture(2);
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    auto& other = *fixture.m_feed_clients[1];
    auto timestamp = fixture.m_time_client.get_time();
    SUBCASE("unseen") {}
    SUBCASE("replayed_data") {
      fixture.publish(1, "ONE", other);
      flush_pending_routines();
      fixture.m_time_client.set(timestamp + seconds(2));
      fixture.publish(1, "ONE", other);
    }
    SUBCASE("stale_heartbeat") {
      fixture.heartbeat(1, other);
      flush_pending_routines();
      fixture.publish(4, "FOUR");
      flush_pending_routines();
      fixture.m_time_client.set(timestamp + seconds(2));
      fixture.heartbeat(1, other);
    }
    fixture.publish(4, "FOUR");
    flush_pending_routines();
    fixture.m_time_client.set(timestamp + seconds(3));
    fixture.m_timer.trigger();
    flush_pending_routines();
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
    fixture.m_time_client.set(
      fixture.m_time_client.get_time() + time_duration::unit());
    fixture.m_timer.trigger();
    auto request = fixture.require_request(2, 3);
    fixture.recover(2, "TWO");
    fixture.recover(3, "THREE");
    fixture.require_symbol("TWO");
    fixture.require_symbol("THREE");
    fixture.require_symbol("FOUR");
    fixture.complete(request);
    fixture.publish(5, "FIVE", other);
    fixture.require_symbol("FIVE");
  }

  TEST_CASE("optional_recovery") {
    auto fixture = Fixture(2, WithoutRecovery());
    auto& other = *fixture.m_feed_clients[1];
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    fixture.publish(1, "ONE", other);
    fixture.publish(3, "THREE");
    flush_pending_routines();
    SUBCASE("redundant_copy") {
      fixture.publish(2, "TWO", other);
      fixture.require_symbol("TWO");
      fixture.require_symbol("THREE");
    }
    SUBCASE("loss") {
      fixture.publish(3, "THREE", other);
      fixture.require_symbol("THREE");
      fixture.publish(2, "STALE", other);
      fixture.publish(4, "FOUR");
      fixture.require_symbol("FOUR");
    }
    SUBCASE("stalled_feed") {
      fixture.m_time_client.set(
        fixture.m_time_client.get_time() + seconds(4));
      fixture.m_timer.trigger();
      fixture.require_symbol("THREE");
    }
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
    fixture.m_client.close();
    REQUIRE_THROWS_AS(fixture.m_client.read(), EndOfFileException);
  }

  TEST_CASE("fragment_loss") {
    auto fixture = Fixture(1, WithoutRecovery());
    auto first = std::uint32_t(1);
    auto last = std::uint32_t(3);
    SUBCASE("consecutive") {}
    SUBCASE("wrap") {
      first = 999999998;
      last = 1;
    }
    fixture.publish(first, "\x01\x1e" "1=H",
      TmxIpHeader::Continuation::FIRST, *fixture.m_feed_clients.front());
    fixture.publish(last, "X\x1d", TmxIpHeader::Continuation::LAST,
      *fixture.m_feed_clients.front());
    fixture.publish(last + 1, "NEXT");
    fixture.require_symbol("NEXT");
    fixture.publish(last + 2, "\x01\x1e" "1=H\x1c\x1e" "55=AB",
      TmxIpHeader::Continuation::FIRST, *fixture.m_feed_clients.front());
    fixture.publish(last + 3, "X\x1d", TmxIpHeader::Continuation::LAST,
      *fixture.m_feed_clients.front());
    fixture.require_symbol("ABX");
  }

  TEST_CASE("fragment_recovery_failure") {
    auto log = Log();
    auto fixture = Fixture();
    auto first = std::uint32_t(1);
    auto middle = std::uint32_t(2);
    auto last = std::uint32_t(3);
    SUBCASE("consecutive") {}
    SUBCASE("wrap") {
      first = 999999998;
      middle = 999999999;
      last = 1;
    }
    fixture.publish(first, "\x01\x1e" "1=H",
      TmxIpHeader::Continuation::FIRST, *fixture.m_feed_clients.front());
    fixture.publish(last, "X\x1d", TmxIpHeader::Continuation::LAST,
      *fixture.m_feed_clients.front());
    fixture.require_request(middle, middle);
    fixture.m_recovery_client.m_results.push(TmxIpRecoveryResult(false,
      TmxIpRecoveryResponse::Status::REJECTED, middle, middle, 0, 0,
      "Rejected"));
    flush_pending_routines();
    fixture.publish(last + 1, "NEXT");
    flush_pending_routines();
    fixture.m_client.close();
    fixture.require_symbol("NEXT");
    REQUIRE(log.m_output.str() == std::format(
      "(dropped 2026-Sep-20 12:00:00 {} 1 rejected Rejected)\n", middle));
  }

  TEST_CASE("feed_failure") {
    auto fixture = Fixture(2);
    fixture.publish(1, "ONE");
    fixture.require_symbol("ONE");
    fixture.publish(3, "THREE");
    flush_pending_routines();
    REQUIRE(!fixture.m_recovery_client.m_requests.try_pop());
    fixture.m_feed_clients[1]->close();
    auto request = fixture.require_request(2, 2);
    fixture.recover(2, "TWO");
    fixture.require_symbol("TWO");
    fixture.require_symbol("THREE");
    fixture.complete(request);
    fixture.publish(4, "FOUR");
    fixture.require_symbol("FOUR");
    fixture.m_feed_clients.front()->close();
    REQUIRE_THROWS_AS(fixture.m_client.read(), EndOfFileException);
  }

}
