#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <stop_token>
#include <string>
#include <string_view>
#include <vector>
#include <Beam/IO/IOException.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
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

    template<typename M>
    void write(const M& message) {
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

  const auto TIMESTAMP = time_from_string("2026-09-09 10:00:00");

  using GapClient =
    CxaPitchGapClient<StubSession, TriggerTimer*, FixedTimeClient*>;

  CxaPitchGapRequest parse_request(const std::string& source) {
    auto cursor = CxaPitchMessage::parse(source).get_cursor();
    auto request = CxaPitchGapRequest();
    request.m_unit = cursor.read_uint8();
    request.m_sequence = cursor.read_uint32();
    request.m_count = cursor.read_uint16();
    return request;
  }

  SharedBuffer encode_response(std::uint8_t unit, std::uint32_t sequence,
      std::uint16_t count, char status) {
    auto message = std::string();
    message += char(CxaPitchGapResponse::LENGTH);
    message += static_cast<char>(CxaPitchGapResponse::TYPE);
    message += static_cast<char>(unit);
    message += static_cast<char>(sequence & 0xFF);
    message += static_cast<char>((sequence >> 8) & 0xFF);
    message += static_cast<char>((sequence >> 16) & 0xFF);
    message += static_cast<char>((sequence >> 24) & 0xFF);
    message += static_cast<char>(count & 0xFF);
    message += static_cast<char>((count >> 8) & 0xFF);
    message += status;
    return SharedBuffer(message.data(), message.size());
  }
}

TEST_SUITE("CxaPitchGapClient") {
  TEST_CASE("request_message_limit") {
    auto session = std::make_shared<StubSession>();
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient(
      [=] (std::stop_token) { return session; }, &timer, &time_client);
    REQUIRE(client.request(1, CxaPitchGap(1000, 250), 1000) == 250);
    REQUIRE(session->m_requests.size() == 3);
    auto first = parse_request(session->m_requests[0]);
    REQUIRE(first.m_unit == 1);
    REQUIRE(first.m_sequence == 1000);
    REQUIRE(first.m_count == 100);
    REQUIRE(parse_request(session->m_requests[1]).m_sequence == 1100);
    REQUIRE(parse_request(session->m_requests[1]).m_count == 100);
    REQUIRE(parse_request(session->m_requests[2]).m_sequence == 1200);
    REQUIRE(parse_request(session->m_requests[2]).m_count == 50);
  }

  TEST_CASE("request_second_limit") {
    auto session = std::make_shared<StubSession>();
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient(
      [=] (std::stop_token) { return session; }, &timer, &time_client);
    auto total = 0;
    for(auto i = 0; i != GapClient::SECOND_LIMIT; ++i) {
      total += client.request(1, CxaPitchGap(1, 1), 1);
    }
    REQUIRE(total == GapClient::SECOND_LIMIT);
    REQUIRE(client.request(1, CxaPitchGap(1, 1), 1) == 0);
    time_client.set(TIMESTAMP + seconds(1));
    REQUIRE(client.request(1, CxaPitchGap(1, 1), 1) == 1);
  }

  TEST_CASE("request_minute_limit") {
    auto session = std::make_shared<StubSession>();
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient(
      [=] (std::stop_token) { return session; }, &timer, &time_client);
    auto total = 0;
    for(auto i = 0; i != GapClient::MINUTE_LIMIT; ++i) {
      time_client.set(TIMESTAMP + seconds(i / 100));
      total += client.request(1, CxaPitchGap(1, 1), 1);
    }
    REQUIRE(total == GapClient::MINUTE_LIMIT);
    time_client.set(TIMESTAMP + seconds(30));
    REQUIRE(client.request(1, CxaPitchGap(1, 1), 1) == 0);
    time_client.set(TIMESTAMP + minutes(1));
    REQUIRE(client.request(1, CxaPitchGap(1, 1), 1) == 1);
  }

  TEST_CASE("request_clock_boundary") {
    for(auto duration : {seconds(1), seconds(60), seconds(86400)}) {
      auto session = std::make_shared<StubSession>();
      auto timer = TriggerTimer();
      auto time_client = FixedTimeClient(TIMESTAMP);
      auto client = GapClient(
        [=] (std::stop_token) { return session; }, &timer, &time_client);
      session->m_on_write = [&] {
        time_client.set(TIMESTAMP + duration);
      };
      auto count = GapClient::SECOND_LIMIT * GapClient::MAXIMUM_COUNT;
      REQUIRE(client.request(
        1, CxaPitchGap(1, count + GapClient::MAXIMUM_COUNT), 1) == count);
      REQUIRE(session->m_requests.size() == GapClient::SECOND_LIMIT);
      REQUIRE(client.request(1, CxaPitchGap(count + 1, 1), 1) == 0);
    }
  }

  TEST_CASE("request_clock_rollback") {
    auto session = std::make_shared<StubSession>();
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient(
      [=] (std::stop_token) { return session; }, &timer, &time_client);
    auto count = GapClient::SECOND_LIMIT * GapClient::MAXIMUM_COUNT;
    REQUIRE(client.request(1, CxaPitchGap(1, count), 1) == count);
    time_client.set(TIMESTAMP - hours(24));
    REQUIRE(client.request(1, CxaPitchGap(count + 1, 1), 1) == 0);
    time_client.set(TIMESTAMP);
    REQUIRE(client.request(1, CxaPitchGap(count + 1, 1), 1) == 0);
    time_client.set(TIMESTAMP + seconds(1));
    REQUIRE(client.request(1, CxaPitchGap(count + 1, 1), 1) == 1);
  }

  TEST_CASE("request_daily_limit") {
    auto session = std::make_shared<StubSession>();
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient(
      [=] (std::stop_token) { return session; }, &timer, &time_client);
    auto total = 0;
    for(auto i = 0; i != GapClient::DAY_LIMIT; ++i) {
      time_client.set(
        TIMESTAMP + minutes(i / 1000) + seconds((i % 1000) / 100));
      total += client.request(1, CxaPitchGap(1, 1), 1);
    }
    REQUIRE(total == GapClient::DAY_LIMIT);
    time_client.set(TIMESTAMP + hours(2));
    REQUIRE(client.request(1, CxaPitchGap(1, 1), 1) == 0);
    time_client.set(TIMESTAMP + hours(24));
    REQUIRE(client.request(1, CxaPitchGap(1, 1), 1) == 1);
  }

  TEST_CASE("request_unrecoverable_gap") {
    auto session = std::make_shared<StubSession>();
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient(
      [=] (std::stop_token) { return session; }, &timer, &time_client);
    REQUIRE(client.request(1, CxaPitchGap(500000, 1), 2000000) == 0);
    REQUIRE(session->m_requests.empty());
    REQUIRE(client.request(1, CxaPitchGap(500000, 1), 1000000) == 1);
  }

  TEST_CASE("read_responses") {
    auto session = std::make_shared<StubSession>();
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient(
      [=] (std::stop_token) { return session; }, &timer, &time_client);
    session->m_messages->push(encode_response(1, 4155, 50, 'A'));
    session->m_messages->push(encode_response(2, 900, 10, 'D'));
    auto accepted = client.get_responses()->pop();
    REQUIRE(accepted.m_unit == 1);
    REQUIRE(accepted.m_sequence == 4155);
    REQUIRE(accepted.m_count == 50);
    REQUIRE(accepted.m_status == CxaPitchGapResponse::ACCEPTED);
    auto rejected = client.get_responses()->pop();
    REQUIRE(rejected.m_sequence == 900);
    REQUIRE(rejected.m_status != CxaPitchGapResponse::ACCEPTED);
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
    auto is_closed = std::make_shared<Queue<bool>>();
    auto closer = RoutineHandler(spawn([&] {
      client.close();
      is_closed->push(true);
    }));
    flush_pending_routines();
    auto is_shut = static_cast<bool>(is_closed->try_pop());
    gate->close();
    second->close();
    closer.wait();
    REQUIRE(is_shut);
  }

  TEST_CASE("close_during_request") {
    auto session = std::make_shared<StubSession>();
    session->m_gate = std::make_shared<Queue<int>>();
    auto timer = TriggerTimer();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = GapClient(
      [=] (std::stop_token) { return session; }, &timer, &time_client);
    auto requester = RoutineHandler(spawn([&] {
      client.request(1, CxaPitchGap(1000, 50), 1000);
    }));
    flush_pending_routines();
    auto is_closed = std::make_shared<Queue<bool>>();
    auto closer = RoutineHandler(spawn([&] {
      client.close();
      is_closed->push(true);
    }));
    flush_pending_routines();
    auto is_shut = static_cast<bool>(is_closed->try_pop());
    session->m_gate->push(0);
    requester.wait();
    closer.wait();
    REQUIRE(is_shut);
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
    REQUIRE(client.get_responses()->pop().m_sequence == 4155);
    timer.trigger();
    first->close();
    second->m_messages->push(encode_response(1, 900, 10, 'A'));
    REQUIRE(client.get_responses()->pop().m_sequence == 900);
    REQUIRE(client.request(1, CxaPitchGap(1000, 50), 1000) == 50);
    REQUIRE(second->m_requests.size() == 1);
    REQUIRE(parse_request(second->m_requests[0]).m_sequence == 1000);
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
    REQUIRE(client.get_responses()->pop().m_sequence == 900);
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
    auto response = client.get_responses()->pop();
    gate.push(0);
    requester.wait();
    REQUIRE(response.m_sequence == 900);
    REQUIRE(requested == 0);
    REQUIRE(client.request(1, CxaPitchGap(1000, 50), 1000) == 50);
    REQUIRE(second->m_requests.size() == 1);
  }
}
