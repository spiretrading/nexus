#include <vector>
#include <Beam/IO/IOException.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchGapClient.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct StubSession {
    std::vector<std::string> m_requests;
    std::shared_ptr<Queue<SharedBuffer>> m_messages;
    std::shared_ptr<Queue<int>> m_gate;
    std::function<void ()> m_on_write;
    SharedBuffer m_payload;
    bool m_is_closed;

    StubSession()
      : m_messages(std::make_shared<Queue<SharedBuffer>>()),
        m_is_closed(false) {}

    CxaPitchMessage read() {
      m_payload = m_messages->pop();
      return CxaPitchMessage::parse(
        std::string_view(m_payload.get_data(), m_payload.get_size()));
    }

    void write(const auto& message) {
      if(m_gate) {
        m_gate->pop();
      }
      if(m_on_write) {
        m_on_write();
      }
      auto buffer = SharedBuffer();
      message.encode(out(buffer));
      m_requests.emplace_back(buffer.get_data(), buffer.get_size());
    }

    void close() {
      m_is_closed = true;
      m_messages->close();
      if(m_gate) {
        m_gate->push(0);
      }
    }
  };

  struct GatedTimer : TriggerTimer {
    Queue<int> m_start;
    Queue<int> m_canceled;

    void start() {
      m_start.pop();
      TriggerTimer::start();
    }

    void cancel() {
      TriggerTimer::cancel();
      m_canceled.push(0);
    }
  };

  const auto TIMESTAMP = time_from_string("2026-09-09 10:00:00");
  const auto SECOND = duration_from_string("00:00:01");
  const auto MINUTE = duration_from_string("00:01:00");
  const auto DAY = duration_from_string("24:00:00");
  constexpr auto REQUESTS_PER_SECOND = 100;

  using GapClient =
    CxaPitchGapClient<StubSession, TriggerTimer*, FixedTimeClient*>;

  struct Fixture {
    std::shared_ptr<StubSession> m_session;
    TriggerTimer m_timer;
    FixedTimeClient m_time_client;
    GapClient m_client;

    Fixture()
      : m_session(std::make_shared<StubSession>()),
        m_time_client(TIMESTAMP),
        m_client([&] (std::stop_token) { return m_session; },
          &m_timer, &m_time_client) {}
  };

  CxaPitchGapRequest parse_request(std::string_view source) {
    auto cursor = CxaPitchMessage::parse(source).get_cursor();
    auto request = CxaPitchGapRequest();
    request.m_unit = cursor.read_uint8();
    request.m_sequence = cursor.read_uint32();
    request.m_count = cursor.read_uint16();
    return request;
  }

  SharedBuffer encode_response(std::uint8_t unit, std::uint32_t sequence,
      std::uint16_t count, char status) {
    auto buffer = SharedBuffer();
    CxaPitchGapResponse(unit, sequence, count, status).encode(out(buffer));
    return buffer;
  }
}

TEST_SUITE("CxaPitchGapClient") {
  TEST_CASE("request_message_limit") {
    auto fixture = Fixture();
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(1000, 250), 1000) == 250);
    REQUIRE(fixture.m_session->m_requests.size() == 3);
    auto first = parse_request(fixture.m_session->m_requests[0]);
    REQUIRE(first.m_unit == 1);
    REQUIRE(first.m_sequence == 1000);
    REQUIRE(first.m_count == 100);
    REQUIRE(parse_request(fixture.m_session->m_requests[1]).m_sequence == 1100);
    REQUIRE(parse_request(fixture.m_session->m_requests[1]).m_count == 100);
    REQUIRE(parse_request(fixture.m_session->m_requests[2]).m_sequence == 1200);
    REQUIRE(parse_request(fixture.m_session->m_requests[2]).m_count == 50);
  }

  TEST_CASE("request_partial_write_failure") {
    auto first = std::make_shared<StubSession>();
    auto second = std::make_shared<StubSession>();
    auto sessions = std::vector{first, second};
    auto index = std::size_t(0);
    auto attempts = 0;
    first->m_on_write = [&] {
      ++attempts;
      if(attempts == 2) {
        throw IOException("Write failed.");
      }
    };
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient([&] (std::stop_token) {
      return sessions[index++];
    }, &timer, &time_client);
    auto gap = CxaPitchGap(1000, 250);
    auto count = client.request(1, gap, gap.m_sequence);
    REQUIRE(count == GapClient::MAXIMUM_COUNT);
    REQUIRE(attempts == 2);
    REQUIRE(first->m_requests.size() == 1);
    REQUIRE(parse_request(first->m_requests[0]).m_sequence == gap.m_sequence);
    REQUIRE(parse_request(first->m_requests[0]).m_count == count);
    flush_pending_routines();
    timer.trigger();
    flush_pending_routines();
    auto remainder = CxaPitchGap(gap.m_sequence + count, gap.m_count - count);
    REQUIRE(client.request(1, remainder, remainder.m_sequence) ==
      remainder.m_count);
    REQUIRE(second->m_requests.size() == 2);
    REQUIRE(parse_request(second->m_requests[0]).m_sequence ==
      remainder.m_sequence);
    REQUIRE(parse_request(second->m_requests[0]).m_count ==
      GapClient::MAXIMUM_COUNT);
    REQUIRE(parse_request(second->m_requests[1]).m_sequence ==
      remainder.m_sequence + GapClient::MAXIMUM_COUNT);
    REQUIRE(parse_request(second->m_requests[1]).m_count ==
      remainder.m_count - GapClient::MAXIMUM_COUNT);
  }

  TEST_CASE("request_second_limit") {
    auto fixture = Fixture();
    auto total = 0;
    for(auto i = 0; i != GapClient::SECOND_LIMIT; ++i) {
      total += fixture.m_client.request(1, CxaPitchGap(1, 1), 1);
    }
    REQUIRE(total == GapClient::SECOND_LIMIT);
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(1, 1), 1) == 0);
    fixture.m_time_client.set(TIMESTAMP + SECOND);
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(1, 1), 1) == 1);
  }

  TEST_CASE("concurrent_request_limit") {
    auto fixture = Fixture();
    constexpr auto REMAINING_REQUESTS = 3;
    for(auto i = 0; i != GapClient::SECOND_LIMIT - REMAINING_REQUESTS; ++i) {
      fixture.m_client.request(1, CxaPitchGap(1, 1), 1);
    }
    REQUIRE(fixture.m_session->m_requests.size() ==
      GapClient::SECOND_LIMIT - REMAINING_REQUESTS);
    fixture.m_session->m_requests.clear();
    fixture.m_session->m_gate = std::make_shared<Queue<int>>();
    auto count = 2 * GapClient::MAXIMUM_COUNT;
    auto first_count = std::uint32_t(0);
    auto second_count = std::uint32_t(0);
    auto first = RoutineHandler(spawn([&] {
      first_count = fixture.m_client.request(1, CxaPitchGap(1000, count), 1000);
    }));
    flush_pending_routines();
    auto second = RoutineHandler(spawn([&] {
      second_count = fixture.m_client.request(2, CxaPitchGap(2000, count),
        2000);
    }));
    flush_pending_routines();
    for(auto i = 0; i != 2 * count / GapClient::MAXIMUM_COUNT; ++i) {
      fixture.m_session->m_gate->push(0);
    }
    first.wait();
    second.wait();
    REQUIRE(first_count + second_count ==
      REMAINING_REQUESTS * GapClient::MAXIMUM_COUNT);
    REQUIRE(fixture.m_session->m_requests.size() == REMAINING_REQUESTS);
    auto sequences = std::vector{std::uint32_t(1000), std::uint32_t(2000)};
    for(const auto& source : fixture.m_session->m_requests) {
      auto request = parse_request(source);
      REQUIRE((request.m_unit == 1 || request.m_unit == 2));
      REQUIRE(request.m_count == GapClient::MAXIMUM_COUNT);
      REQUIRE(request.m_sequence == sequences[request.m_unit - 1]);
      sequences[request.m_unit - 1] += request.m_count;
    }
    REQUIRE(first_count == sequences[0] - 1000);
    REQUIRE(second_count == sequences[1] - 2000);
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(3000, 1), 3000) == 0);
  }

  TEST_CASE("request_minute_limit") {
    auto fixture = Fixture();
    auto total = 0;
    for(auto i = 0; i != GapClient::MINUTE_LIMIT; ++i) {
      fixture.m_time_client.set(TIMESTAMP + SECOND * (i / REQUESTS_PER_SECOND));
      total += fixture.m_client.request(1, CxaPitchGap(1, 1), 1);
    }
    REQUIRE(total == GapClient::MINUTE_LIMIT);
    fixture.m_time_client.set(TIMESTAMP + duration_from_string("00:00:30"));
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(1, 1), 1) == 0);
    fixture.m_time_client.set(TIMESTAMP + MINUTE);
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(1, 1), 1) == 1);
  }

  TEST_CASE("request_clock_boundary") {
    for(auto duration : {SECOND, MINUTE, DAY}) {
      auto fixture = Fixture();
      fixture.m_session->m_on_write = [&] {
        fixture.m_time_client.set(TIMESTAMP + duration);
      };
      auto count = GapClient::SECOND_LIMIT * GapClient::MAXIMUM_COUNT;
      REQUIRE(fixture.m_client.request(
        1, CxaPitchGap(1, count + GapClient::MAXIMUM_COUNT), 1) == count);
      REQUIRE(fixture.m_session->m_requests.size() == GapClient::SECOND_LIMIT);
      REQUIRE(fixture.m_client.request(1, CxaPitchGap(count + 1, 1), 1) == 0);
    }
  }

  TEST_CASE("request_clock_rollback") {
    auto fixture = Fixture();
    auto count = GapClient::SECOND_LIMIT * GapClient::MAXIMUM_COUNT;
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(1, count), 1) == count);
    fixture.m_time_client.set(TIMESTAMP - DAY);
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(count + 1, 1), 1) == 0);
    fixture.m_time_client.set(TIMESTAMP);
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(count + 1, 1), 1) == 0);
    fixture.m_time_client.set(TIMESTAMP + SECOND);
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(count + 1, 1), 1) == 1);
  }

  TEST_CASE("request_daily_limit") {
    auto fixture = Fixture();
    constexpr auto REQUESTS_PER_MINUTE = 1000;
    auto total = 0;
    for(auto i = 0; i != GapClient::DAY_LIMIT; ++i) {
      fixture.m_time_client.set(TIMESTAMP + MINUTE * (i / REQUESTS_PER_MINUTE) +
        SECOND * ((i % REQUESTS_PER_MINUTE) / REQUESTS_PER_SECOND));
      total += fixture.m_client.request(1, CxaPitchGap(1, 1), 1);
    }
    REQUIRE(total == GapClient::DAY_LIMIT);
    fixture.m_time_client.set(TIMESTAMP + duration_from_string("02:00:00"));
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(1, 1), 1) == 0);
    fixture.m_time_client.set(TIMESTAMP + DAY);
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(1, 1), 1) == 1);
  }

  TEST_CASE("request_unrecoverable_gap") {
    auto fixture = Fixture();
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(500000, 1), 2000000) == 0);
    REQUIRE(fixture.m_session->m_requests.empty());
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(500000, 1), 1000000) == 1);
  }

  TEST_CASE("read_responses") {
    auto fixture = Fixture();
    fixture.m_session->m_messages->push(encode_response(1, 4155, 50, 'A'));
    fixture.m_session->m_messages->push(encode_response(2, 900, 10, 'D'));
    flush_pending_routines();
    auto accepted = fixture.m_client.get_responses()->try_pop();
    REQUIRE(accepted.has_value());
    REQUIRE(accepted->m_unit == 1);
    REQUIRE(accepted->m_sequence == 4155);
    REQUIRE(accepted->m_count == 50);
    REQUIRE(accepted->m_status == CxaPitchGapResponse::ACCEPTED);
    auto rejected = fixture.m_client.get_responses()->try_pop();
    REQUIRE(rejected.has_value());
    REQUIRE(rejected->m_unit == 2);
    REQUIRE(rejected->m_sequence == 900);
    REQUIRE(rejected->m_count == 10);
    REQUIRE(rejected->m_status == 'D');
  }

  TEST_CASE("close_during_connection") {
    auto first = std::make_shared<StubSession>();
    auto second = std::make_shared<StubSession>();
    auto sessions = std::vector{first, second};
    auto index = std::size_t(0);
    auto gate = std::make_shared<Queue<int>>();
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient([&] (std::stop_token stop_token) {
      if(index != 0) {
        auto cancellation = std::stop_callback(stop_token, [&] {
          gate->close();
        });
        gate->pop();
      }
      return sessions[index++];
    }, &timer, &time_client);
    timer.trigger();
    first->close();
    flush_pending_routines();
    auto completion = Queue<bool>();
    auto closer = RoutineHandler(spawn([&] {
      client.close();
      completion.push(true);
    }));
    flush_pending_routines();
    auto is_closed = completion.try_pop().has_value();
    gate->close();
    second->close();
    closer.wait();
    REQUIRE(is_closed);
  }

  TEST_CASE("connection_completed_during_close") {
    auto first = std::make_shared<StubSession>();
    auto second = std::make_shared<StubSession>();
    auto sessions = std::vector{first, second};
    auto index = std::size_t(0);
    auto gate = Queue<int>();
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient([&] (std::stop_token stop_token) {
      if(index != 0) {
        auto cancellation = std::stop_callback(stop_token, [&] {
          gate.push(0);
        });
        gate.pop();
      }
      return sessions[index++];
    }, &timer, &time_client);
    timer.trigger();
    first->close();
    flush_pending_routines();
    auto completion = Queue<bool>();
    auto closer = RoutineHandler(spawn([&] {
      client.close();
      completion.push(true);
    }));
    flush_pending_routines();
    auto is_closed = completion.try_pop().has_value();
    auto is_session_closed = second->m_is_closed;
    gate.push(0);
    second->close();
    closer.wait();
    REQUIRE(is_closed);
    REQUIRE(is_session_closed);
    REQUIRE(index == sessions.size());
    REQUIRE(client.request(1, CxaPitchGap(1000, 50), 1000) == 0);
    REQUIRE(second->m_requests.empty());
  }

  TEST_CASE("close_during_reconnect_delay") {
    auto session = std::make_shared<StubSession>();
    auto timer = GatedTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto connections = 0;
    auto client = CxaPitchGapClient<StubSession, GatedTimer*, FixedTimeClient*>(
      [&] (std::stop_token stop_token) {
        if(stop_token.stop_requested()) {
          throw IOException("Connection canceled.");
        }
        ++connections;
        return session;
      }, &timer, &time_client);
    SUBCASE("timer_start") {}
    SUBCASE("timer_wait") {
      timer.m_start.push(0);
    }
    session->close();
    flush_pending_routines();
    auto completion = Queue<bool>();
    auto closer = RoutineHandler(spawn([&] {
      client.close();
      completion.push(true);
    }));
    flush_pending_routines();
    auto canceled = timer.m_canceled.try_pop();
    timer.m_start.push(0);
    flush_pending_routines();
    auto is_closed = completion.try_pop().has_value();
    timer.trigger();
    closer.wait();
    REQUIRE(canceled.has_value());
    REQUIRE(is_closed);
    REQUIRE(connections == 1);
  }

  TEST_CASE("close_during_request") {
    auto fixture = Fixture();
    fixture.m_session->m_gate = std::make_shared<Queue<int>>();
    auto requester = RoutineHandler(spawn([&] {
      fixture.m_client.request(1, CxaPitchGap(1000, 50), 1000);
    }));
    flush_pending_routines();
    auto completion = Queue<bool>();
    auto closer = RoutineHandler(spawn([&] {
      fixture.m_client.close();
      completion.push(true);
    }));
    flush_pending_routines();
    auto is_closed = completion.try_pop().has_value();
    fixture.m_session->m_gate->push(0);
    requester.wait();
    closer.wait();
    REQUIRE(is_closed);
  }

  TEST_CASE("disconnected_session") {
    auto fixture = Fixture();
    fixture.m_session->m_messages->close();
    flush_pending_routines();
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(1000, 50), 1000) == 0);
    REQUIRE(fixture.m_session->m_requests.empty());
    fixture.m_client.close();
    REQUIRE(fixture.m_session->m_is_closed);
    REQUIRE(fixture.m_client.request(1, CxaPitchGap(1000, 50), 1000) == 0);
  }

  TEST_CASE("session_reconnection") {
    auto first = std::make_shared<StubSession>();
    auto second = std::make_shared<StubSession>();
    auto sessions = std::vector{first, second};
    auto index = std::size_t(0);
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient([&] (std::stop_token) {
      return sessions[index++];
    }, &timer, &time_client);
    first->m_messages->push(encode_response(1, 4155, 50, 'A'));
    flush_pending_routines();
    auto response = client.get_responses()->try_pop();
    REQUIRE(response.has_value());
    REQUIRE(response->m_sequence == 4155);
    timer.trigger();
    first->close();
    second->m_messages->push(encode_response(1, 900, 10, 'A'));
    flush_pending_routines();
    response = client.get_responses()->try_pop();
    REQUIRE(response.has_value());
    REQUIRE(response->m_sequence == 900);
    REQUIRE(client.request(1, CxaPitchGap(1000, 50), 1000) == 50);
    REQUIRE(second->m_requests.size() == 1);
    REQUIRE(parse_request(second->m_requests[0]).m_sequence == 1000);
  }

  TEST_CASE("reconnect_timer_failure") {
    auto first = std::make_shared<StubSession>();
    auto second = std::make_shared<StubSession>();
    auto sessions = std::vector{first, second};
    auto index = std::size_t(0);
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient([&] (std::stop_token) {
      return sessions[index++];
    }, &timer, &time_client);
    first->close();
    flush_pending_routines();
    second->m_messages->push(encode_response(1, 900, 10, 'A'));
    timer.fail();
    flush_pending_routines();
    auto response = client.get_responses()->try_pop();
    client.close();
    REQUIRE(response.has_value());
    REQUIRE(response->m_sequence == 900);
  }

  TEST_CASE("close_failed_session") {
    auto first = std::make_shared<StubSession>();
    auto second = std::make_shared<StubSession>();
    auto sessions = std::vector{first, second};
    auto index = std::size_t(0);
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient([&] (std::stop_token) {
      return sessions[index++];
    }, &timer, &time_client);
    first->m_messages->close();
    timer.trigger();
    second->m_messages->push(encode_response(1, 900, 10, 'A'));
    flush_pending_routines();
    auto response = client.get_responses()->try_pop();
    REQUIRE(response.has_value());
    REQUIRE(response->m_sequence == 900);
    REQUIRE(first->m_is_closed);
  }

  TEST_CASE("old_session_write_failure") {
    auto first = std::make_shared<StubSession>();
    auto second = std::make_shared<StubSession>();
    auto sessions = std::vector{first, second};
    auto index = std::size_t(0);
    auto gate = Queue<int>();
    first->m_on_write = [&] {
      gate.pop();
      throw IOException("Old session write failed.");
    };
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient([&] (std::stop_token) {
      return sessions[index++];
    }, &timer, &time_client);
    auto requested = std::uint32_t(0);
    auto requester = RoutineHandler(spawn([&] {
      requested = client.request(1, CxaPitchGap(1000, 50), 1000);
    }));
    flush_pending_routines();
    timer.trigger();
    first->m_messages->close();
    second->m_messages->push(encode_response(1, 900, 10, 'A'));
    flush_pending_routines();
    auto response = client.get_responses()->try_pop();
    gate.push(0);
    requester.wait();
    REQUIRE(response.has_value());
    REQUIRE(response->m_sequence == 900);
    REQUIRE(requested == 0);
    REQUIRE(client.request(1, CxaPitchGap(1000, 50), 1000) == 50);
    REQUIRE(second->m_requests.size() == 1);
  }
}
