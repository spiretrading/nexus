#include <atomic>
#include <Beam/Queues/StatePublisher.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchClient.hpp"
#include "CxaPitchMarketDataFeedClient/CxaPitchEncoder.hpp"
#include "CxaPitchMarketDataFeedClientTests/TestCxaPitchGapClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace std::string_view_literals;

namespace {
  struct StubProtocolClient {
    Queue<SharedBuffer> m_blocks;
    SharedBuffer m_payload;

    CxaPitchBlock read() {
      m_payload = m_blocks.pop();
      return CxaPitchBlock::parse(
        std::string_view(m_payload.get_data(), m_payload.get_size()));
    }

    void close() {
      m_blocks.close();
    }
  };

  struct StubSpinClient {
    StatePublisher<std::uint32_t> m_sequences;
    Queue<std::uint32_t> m_requests;
    Queue<CxaPitchSnapshot> m_snapshots;
    std::atomic_bool m_is_closed;

    void monitor_snapshot_sequences(
        ScopedQueueWriter<std::uint32_t> queue) const {
      m_sequences.monitor(std::move(queue));
    }

    CxaPitchSnapshot load_snapshot(std::uint32_t sequence) {
      m_requests.push(sequence);
      return m_snapshots.pop();
    }

    void close() {
      m_is_closed = true;
      m_sequences.close();
      m_requests.close();
      m_snapshots.close();
    }
  };

  using Client = CxaPitchClient<StubProtocolClient*, TestCxaPitchGapClient*,
    StubSpinClient*, FixedTimeClient*, TriggerTimer*>;

  const auto TIMESTAMP = time_from_string("2026-09-09 10:00:00");
  const auto ADD_ORDER = from<SharedBuffer>(
    "\x2a\x37"
    "\xf0\x77\xbb\xce\x2a\x6a\x62\x16"
    "\x05\x40\x5b\x77\x8f\x56\x1d\x0b"
    "B"
    "\xbc\x02\x00\x00"
    "ZVZT  "
    "\x15\xcd\x5b\x07\x00\x00\x00\x00"
    "1234"
    "\x00"sv);

  struct Fixture {
    struct WithSpin {};
    struct WithoutGap {};

    inline static const auto FEED_TIMEOUT = duration_from_string("00:00:03");
    inline static const auto GAP_TIMEOUT = duration_from_string("00:00:05");
    std::vector<std::unique_ptr<StubProtocolClient>> m_feed_clients;
    std::vector<std::unique_ptr<StubProtocolClient>> m_recovery_clients;
    std::shared_ptr<TestCxaPitchGapClient::Queue> m_gap_operations;
    TestCxaPitchGapClient m_gap_client;
    optional<StubSpinClient> m_spin_client;
    FixedTimeClient m_time_client;
    TriggerTimer m_timer;
    optional<Client> m_client;

    explicit Fixture(int feeds)
      : Fixture(1, feeds, 0) {}

    Fixture(std::uint8_t unit, int feeds, int recovery)
      : Fixture(unit, feeds, recovery, none) {}

    template<typename S> requires
      std::same_as<S, none_t> || std::same_as<S, WithSpin> ||
        std::same_as<S, WithoutGap>
    Fixture(std::uint8_t unit, int feeds, int recovery, S)
        : m_gap_operations(std::make_shared<TestCxaPitchGapClient::Queue>()),
          m_gap_client(m_gap_operations),
          m_time_client(TIMESTAMP) {
      auto feed_clients = std::vector<StubProtocolClient*>();
      for(auto i = 0; i != feeds; ++i) {
        m_feed_clients.push_back(std::make_unique<StubProtocolClient>());
        feed_clients.push_back(m_feed_clients.back().get());
      }
      auto recovery_clients = std::vector<StubProtocolClient*>();
      for(auto i = 0; i != recovery; ++i) {
        m_recovery_clients.push_back(std::make_unique<StubProtocolClient>());
        recovery_clients.push_back(m_recovery_clients.back().get());
      }
      auto spin_client = optional<StubSpinClient*>();
      if constexpr(std::same_as<S, WithSpin>) {
        m_spin_client.emplace();
        spin_client = &*m_spin_client;
      }
      auto gap_client = optional<TestCxaPitchGapClient*>();
      if constexpr(!std::same_as<S, WithoutGap>) {
        gap_client = &m_gap_client;
      }
      m_client.emplace(unit, FEED_TIMEOUT, GAP_TIMEOUT, feed_clients,
        recovery_clients, gap_client, spin_client, &m_time_client, &m_timer);
    }

    void publish(const SharedBuffer& block) {
      for(auto& client : m_feed_clients) {
        client->m_blocks.push(block);
      }
    }

    template<typename O>
    std::shared_ptr<O> require_operation(const auto&... args) {
      return check_operation<O>(m_gap_operations->pop(), args...);
    }

    template<typename O>
    std::shared_ptr<O> try_require_operation(const auto&... args) {
      auto operation = m_gap_operations->try_pop();
      REQUIRE(operation.has_value());
      return check_operation<O>(*operation, args...);
    }

    template<typename O>
    std::shared_ptr<O> check_operation(
        const std::shared_ptr<TestCxaPitchGapClient::Operation>& operation,
        const CxaPitchGap& gap, std::uint32_t live) {
      auto specific = std::get_if<O>(&*operation);
      REQUIRE(specific);
      REQUIRE(specific->m_gap.m_sequence == gap.m_sequence);
      REQUIRE(specific->m_gap.m_count == gap.m_count);
      REQUIRE(specific->m_live == live);
      return std::shared_ptr<O>(operation, specific);
    }

    template<std::same_as<TestCxaPitchGapClient::RequestOperation> O>
    std::shared_ptr<O> check_operation(
        const std::shared_ptr<TestCxaPitchGapClient::Operation>& operation,
        std::uint8_t unit, const CxaPitchGap& gap, std::uint32_t live) {
      auto request = check_operation<O>(operation, gap, live);
      REQUIRE(request->m_unit == unit);
      return request;
    }

    std::shared_ptr<TestCxaPitchGapClient::RequestOperation>
        require_recovery_request(const CxaPitchGap& gap, std::uint32_t live) {
      return require_recovery_request(gap, gap, live);
    }

    std::shared_ptr<TestCxaPitchGapClient::RequestOperation>
        require_recovery_request(const CxaPitchGap& gap,
          const CxaPitchGap& requested_gap, std::uint32_t live) {
      auto recoverable = require_operation<
        TestCxaPitchGapClient::IsRecoverableOperation>(gap, live);
      recoverable->m_result.set(true);
      return require_operation<TestCxaPitchGapClient::RequestOperation>(
        1, requested_gap, live);
    }
  };

  struct MessageReader {
    Client* m_client;
    Queue<int> m_types;
    RoutineHandler m_routine;

    explicit MessageReader(Client& client)
        : m_client(&client) {
      m_routine = spawn([=, this] {
        try {
          while(true) {
            m_types.push(m_client->read().m_type);
          }
        } catch(const std::exception&) {}
      });
    }

    ~MessageReader() {
      m_client->close();
    }
  };

  SharedBuffer encode_block(
      std::uint32_t sequence, const std::vector<std::uint8_t>& types) {
    auto payload = SharedBuffer();
    auto encoder = CxaPitchEncoder(Ref(payload));
    for(auto type : types) {
      encoder.write_uint8(CxaPitchMessage::HEADER_LENGTH);
      encoder.write_uint8(type);
    }
    auto header = CxaPitchHeader(
      static_cast<std::uint16_t>(CxaPitchHeader::LENGTH + payload.get_size()),
      static_cast<std::uint8_t>(types.size()), 1, sequence);
    auto block = SharedBuffer();
    header.encode(out(block));
    append(block, payload);
    return block;
  }

  SharedBuffer encode_malformed_block(
      std::uint32_t sequence, std::vector<std::uint8_t> types) {
    types.push_back(CxaPitchAddOrder::TYPE);
    return encode_block(sequence, types);
  }
}

TEST_SUITE("CxaPitchClient") {
  TEST_CASE("message_sequence") {
    auto fixture = Fixture(1);
    fixture.publish(encode_block(1, {0x11, 0x12}));
    fixture.publish(encode_block(3, {0x13}));
    REQUIRE(fixture.m_client->read().m_type == 0x11);
    REQUIRE(fixture.m_client->read().m_type == 0x12);
    REQUIRE(fixture.m_client->read().m_type == 0x13);
  }

  TEST_CASE("feed_arbitration") {
    auto fixture = Fixture(2);
    fixture.publish(encode_block(1, {0x11}));
    fixture.m_feed_clients[1]->m_blocks.push(encode_block(2, {0x12}));
    REQUIRE(fixture.m_client->read().m_type == 0x11);
    REQUIRE(fixture.m_client->read().m_type == 0x12);
  }

  TEST_CASE("unit_mismatch") {
    auto fixture = Fixture(2, 1, 1);
    SUBCASE("feed") {
      fixture.m_feed_clients[0]->m_blocks.push(encode_block(1, {0x11}));
    }
    SUBCASE("recovery") {
      fixture.m_recovery_clients[0]->m_blocks.push(encode_block(1, {0x11}));
    }
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
  }

  TEST_CASE("unit_zero_heartbeat") {
    auto fixture = Fixture(1, 1, 1);
    auto heartbeat = from<SharedBuffer>("\x08\x00\x00\x00\x00\x00\x00\x00"sv);
    SUBCASE("feed") {
      fixture.m_feed_clients[0]->m_blocks.push(heartbeat);
    }
    SUBCASE("recovery") {
      fixture.m_recovery_clients[0]->m_blocks.push(heartbeat);
    }
    flush_pending_routines();
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(fixture.m_client->read().m_type == 0x11);
  }

  TEST_CASE("gap_recovery") {
    auto fixture = Fixture(1, 2, 1);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(fixture.m_client->read().m_type == 0x11);
    flush_pending_routines();
    fixture.publish(encode_block(4, {0x14}));
    auto request = fixture.require_recovery_request(CxaPitchGap(2, 2), 5);
    request->m_result.set(2);
    fixture.m_recovery_clients[0]->m_blocks.push(
      encode_block(2, {0x12, 0x13}));
    REQUIRE(fixture.m_client->read().m_type == 0x12);
    REQUIRE(fixture.m_client->read().m_type == 0x13);
    REQUIRE(fixture.m_client->read().m_type == 0x14);
  }

  TEST_CASE("request_throttling") {
    auto fixture = Fixture(2);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(fixture.m_client->read().m_type == 0x11);
    flush_pending_routines();
    fixture.publish(encode_block(4, {}));
    auto request = fixture.require_recovery_request(CxaPitchGap(2, 2), 4);
    auto count = std::uint32_t(1);
    auto position = std::uint32_t(4);
    SUBCASE("partial_request") {}
    SUBCASE("empty_request") {
      count = 0;
    }
    SUBCASE("growing_gap") {
      fixture.m_time_client.set(TIMESTAMP + Fixture::GAP_TIMEOUT);
      fixture.publish(encode_block(6, {}));
      for(auto i = std::size_t(0); i != fixture.m_feed_clients.size(); ++i) {
        auto recoverable = fixture.require_operation<
          TestCxaPitchGapClient::IsRecoverableOperation>(CxaPitchGap(2, 4), 6);
        recoverable->m_result.set(true);
      }
      flush_pending_routines();
      position = 6;
    }
    request->m_result.set(count);
    flush_pending_routines();
    REQUIRE(!fixture.m_gap_operations->try_pop());
    auto gap = CxaPitchGap(2, position - 2);
    for(auto i = 0; i != 2; ++i) {
      fixture.publish(encode_block(position, {}));
      for(auto j = std::size_t(0); j != fixture.m_feed_clients.size(); ++j) {
        auto recoverable = fixture.require_operation<
          TestCxaPitchGapClient::IsRecoverableOperation>(gap, position);
        recoverable->m_result.set(true);
      }
      flush_pending_routines();
      REQUIRE(!fixture.m_gap_operations->try_pop());
    }
    fixture.m_timer.trigger();
    auto retry_gap = CxaPitchGap(gap.m_sequence + count, gap.m_count - count);
    auto retry = fixture.require_recovery_request(gap, retry_gap, position);
    retry->m_result.set(retry_gap.m_count);
    flush_pending_routines();
    fixture.m_timer.trigger();
    auto recoverable = fixture.require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(gap, position);
    recoverable->m_result.set(true);
    flush_pending_routines();
    REQUIRE(!fixture.m_gap_operations->try_pop());
  }

  TEST_CASE("silent_feed") {
    auto fixture = Fixture(2);
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(reader.m_types.pop() == 0x11);
    flush_pending_routines();
    fixture.publish(encode_block(3, {0x13}));
    auto request = fixture.require_recovery_request(CxaPitchGap(2, 1), 4);
    request->m_result.set(1);
    flush_pending_routines();
    REQUIRE(!reader.m_types.try_pop());
    fixture.m_time_client.set(
      TIMESTAMP + Fixture::FEED_TIMEOUT + time_duration::unit());
    fixture.m_timer.trigger();
    auto recoverable = fixture.require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(CxaPitchGap(2, 1), 4);
    recoverable->m_result.set(true);
    flush_pending_routines();
    REQUIRE(!reader.m_types.try_pop());
    fixture.m_time_client.set(
      TIMESTAMP + Fixture::GAP_TIMEOUT + time_duration::unit());
    fixture.m_timer.trigger();
    recoverable = fixture.require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(CxaPitchGap(2, 1), 4);
    recoverable->m_result.set(true);
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == 0x13);
    fixture.m_feed_clients[0]->m_blocks.push(encode_block(5, {0x15}));
    request = fixture.require_recovery_request(CxaPitchGap(4, 1), 6);
    request->m_result.set(1);
    fixture.m_feed_clients[1]->m_blocks.push(encode_block(5, {0x15}));
    recoverable = fixture.require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(CxaPitchGap(4, 1), 6);
    recoverable->m_result.set(true);
    flush_pending_routines();
    REQUIRE(!reader.m_types.try_pop());
    fixture.m_time_client.set(fixture.m_time_client.get_time() +
      Fixture::GAP_TIMEOUT + time_duration::unit());
    fixture.m_timer.trigger();
    recoverable = fixture.require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(CxaPitchGap(4, 1), 6);
    recoverable->m_result.set(true);
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == 0x15);
  }

  TEST_CASE("timer_gap_recovery") {
    auto fixture = Fixture(1, 2, 1);
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(reader.m_types.pop() == 0x11);
    flush_pending_routines();
    SUBCASE("active_feed") {
      fixture.m_time_client.set(TIMESTAMP + Fixture::FEED_TIMEOUT);
    }
    SUBCASE("silent_feeds") {}
    fixture.m_feed_clients[0]->m_blocks.push(encode_block(3, {0x13}));
    flush_pending_routines();
    REQUIRE(!fixture.m_gap_operations->try_pop());
    REQUIRE(!reader.m_types.try_pop());
    fixture.m_time_client.set(
      TIMESTAMP + Fixture::FEED_TIMEOUT + time_duration::unit());
    fixture.m_timer.trigger();
    flush_pending_routines();
    auto recoverable = fixture.try_require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(CxaPitchGap(2, 1), 4);
    recoverable->m_result.set(true);
    flush_pending_routines();
    auto request = fixture.try_require_operation<
      TestCxaPitchGapClient::RequestOperation>(1, CxaPitchGap(2, 1), 4);
    request->m_result.set(1);
    fixture.m_recovery_clients[0]->m_blocks.push(encode_block(2, {0x12}));
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == 0x12);
    REQUIRE(reader.m_types.try_pop() == 0x13);
  }

  TEST_CASE("malformed_recovery") {
    auto fixture = Fixture(1, 1, 1);
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(reader.m_types.pop() == 0x11);
    flush_pending_routines();
    fixture.publish(encode_block(4, {0x14}));
    auto request = fixture.require_recovery_request(CxaPitchGap(2, 2), 5);
    request->m_result.set(2);
    SUBCASE("framing") {
      fixture.m_recovery_clients[0]->m_blocks.push(from<SharedBuffer>(
        "\x10\x00\x02\x01\x02\x00\x00\x00"
        "\x06\x42\x00\x00\x00\x00\xff\x43"sv));
    }
    SUBCASE("message") {
      fixture.m_recovery_clients[0]->m_blocks.push(
        encode_malformed_block(2, {0x12}));
    }
    flush_pending_routines();
    REQUIRE(!reader.m_types.try_pop());
    fixture.m_recovery_clients[0]->m_blocks.push(
      encode_block(2, {0x12, 0x13}));
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == 0x12);
    REQUIRE(reader.m_types.try_pop() == 0x13);
    REQUIRE(reader.m_types.try_pop() == 0x14);
    REQUIRE(!reader.m_types.try_pop());
  }

  TEST_CASE("recovery_arbitration") {
    auto fixture = Fixture(1, 2, 2);
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(reader.m_types.pop() == 0x11);
    flush_pending_routines();
    fixture.publish(encode_block(4, {0x14}));
    auto request = fixture.require_recovery_request(CxaPitchGap(2, 2), 5);
    request->m_result.set(2);
    auto malformed = encode_malformed_block(2, {0x12});
    auto valid = encode_block(2, {0x12, 0x13});
    SUBCASE("malformed_first") {
      fixture.m_recovery_clients[0]->m_blocks.push(malformed);
      flush_pending_routines();
      fixture.m_recovery_clients[1]->m_blocks.push(valid);
    }
    SUBCASE("valid_first") {
      fixture.m_recovery_clients[1]->m_blocks.push(valid);
      flush_pending_routines();
      fixture.m_recovery_clients[0]->m_blocks.push(malformed);
    }
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == 0x12);
    REQUIRE(reader.m_types.try_pop() == 0x13);
    REQUIRE(reader.m_types.try_pop() == 0x14);
    REQUIRE(!reader.m_types.try_pop());
  }

  TEST_CASE("throttled_gap_response") {
    auto fixture = Fixture(2);
    auto reader = MessageReader(*fixture.m_client);
    auto gap = CxaPitchGap(2, 2);
    auto response =
      CxaPitchGapResponse(1, 2, 2, CxaPitchGapResponse::SECOND_EXHAUSTED);
    auto is_complete = false;
    SUBCASE("pending_second") {}
    SUBCASE("pending_minute") {
      response.m_status = CxaPitchGapResponse::MINUTE_EXHAUSTED;
    }
    SUBCASE("pending_second_prefix") {
      gap.m_count = 200;
      response.m_count = 100;
    }
    SUBCASE("pending_minute_prefix") {
      gap.m_count = 200;
      response.m_count = 100;
      response.m_status = CxaPitchGapResponse::MINUTE_EXHAUSTED;
    }
    SUBCASE("pending_second_suffix") {
      response =
        CxaPitchGapResponse(1, 3, 1, CxaPitchGapResponse::SECOND_EXHAUSTED);
    }
    SUBCASE("pending_minute_suffix") {
      response =
        CxaPitchGapResponse(1, 3, 1, CxaPitchGapResponse::MINUTE_EXHAUSTED);
    }
    SUBCASE("completed_second") {
      is_complete = true;
    }
    SUBCASE("completed_minute") {
      is_complete = true;
      response.m_status = CxaPitchGapResponse::MINUTE_EXHAUSTED;
    }
    SUBCASE("completed_second_prefix") {
      is_complete = true;
      gap.m_count = 200;
      response.m_count = 100;
    }
    SUBCASE("completed_minute_prefix") {
      is_complete = true;
      gap.m_count = 200;
      response.m_count = 100;
      response.m_status = CxaPitchGapResponse::MINUTE_EXHAUSTED;
    }
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(reader.m_types.pop() == 0x11);
    flush_pending_routines();
    auto end = gap.m_sequence + gap.m_count;
    auto position = end + 1;
    fixture.publish(encode_block(end, {0x14}));
    auto request = fixture.require_recovery_request(gap, position);
    if(is_complete) {
      request->m_result.set(gap.m_count);
      flush_pending_routines();
    }
    fixture.m_gap_client.get_responses()->push(response);
    auto accepted_sequence = response.m_sequence + response.m_count;
    if(accepted_sequence < end) {
      fixture.m_gap_client.get_responses()->push(
        CxaPitchGapResponse(1, accepted_sequence, static_cast<std::uint16_t>(
          end - accepted_sequence), CxaPitchGapResponse::ACCEPTED));
    }
    flush_pending_routines();
    REQUIRE(!reader.m_types.try_pop());
    if(!is_complete) {
      request->m_result.set(gap.m_count);
      flush_pending_routines();
    }
    fixture.m_feed_clients[0]->m_blocks.push(encode_block(position, {0x15}));
    auto recoverable = fixture.require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(gap, position);
    recoverable->m_result.set(true);
    flush_pending_routines();
    auto retry = fixture.try_require_operation<
      TestCxaPitchGapClient::RequestOperation>(
        1, CxaPitchGap(response.m_sequence, response.m_count), position);
    retry->m_result.set(response.m_count);
    flush_pending_routines();
    fixture.m_feed_clients[0]->m_blocks.push(encode_block(position + 1, {}));
    recoverable = fixture.require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(gap, position);
    recoverable->m_result.set(true);
    flush_pending_routines();
    REQUIRE(!fixture.m_gap_operations->try_pop());
  }

  TEST_CASE("throttled_gap_ranges") {
    auto fixture = Fixture(1);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(fixture.m_client->read().m_type == 0x11);
    fixture.publish(encode_block(302, {0x14}));
    auto gap = CxaPitchGap(2, 300);
    auto request = fixture.require_recovery_request(gap, 303);
    request->m_result.set(gap.m_count);
    auto responses = fixture.m_gap_client.get_responses();
    responses->push(
      CxaPitchGapResponse(1, 2, 100, CxaPitchGapResponse::SECOND_EXHAUSTED));
    responses->push(
      CxaPitchGapResponse(1, 102, 100, CxaPitchGapResponse::ACCEPTED));
    responses->push(
      CxaPitchGapResponse(1, 202, 100, CxaPitchGapResponse::MINUTE_EXHAUSTED));
    flush_pending_routines();
    fixture.publish(encode_block(303, {}));
    auto retry =
      fixture.require_recovery_request(gap, CxaPitchGap(2, 100), 303);
    retry->m_result.set(0);
    flush_pending_routines();
    fixture.m_timer.trigger();
    retry = fixture.require_recovery_request(gap, CxaPitchGap(2, 100), 303);
    retry->m_result.set(100);
    flush_pending_routines();
    fixture.publish(encode_block(303, {}));
    retry = fixture.require_recovery_request(gap, CxaPitchGap(202, 100), 303);
    retry->m_result.set(100);
    flush_pending_routines();
    fixture.publish(encode_block(303, {}));
    auto recoverable = fixture.require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(gap, 303);
    recoverable->m_result.set(true);
    flush_pending_routines();
    REQUIRE(!fixture.m_gap_operations->try_pop());
  }

  TEST_CASE("stale_snapshot") {
    auto fixture = Fixture(1, 1, 0, Fixture::WithSpin());
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(5, {}));
    flush_pending_routines();
    fixture.m_spin_client->m_sequences.push(6);
    REQUIRE(fixture.m_spin_client->m_requests.pop() == 6);
    fixture.publish(encode_block(10, {0x1a}));
    auto request = fixture.require_recovery_request(CxaPitchGap(5, 5), 11);
    request->m_result.set(5);
    flush_pending_routines();
    fixture.m_time_client.set(
      TIMESTAMP + Fixture::GAP_TIMEOUT + time_duration::unit());
    fixture.publish(encode_block(11, {0x1b}));
    auto recoverable = fixture.require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(CxaPitchGap(5, 5), 12);
    recoverable->m_result.set(true);
    flush_pending_routines();
    auto snapshot = CxaPitchSnapshot(
      6, CxaPitchSpinResponse::ACCEPTED, std::vector{ADD_ORDER});
    fixture.m_spin_client->m_snapshots.push(snapshot);
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == int(CxaPitchAddOrder::TYPE));
    REQUIRE(reader.m_types.try_pop() == 0x1a);
    REQUIRE(reader.m_types.try_pop() == 0x1b);
  }

  TEST_CASE("malformed_feed_message") {
    auto fixture = Fixture(2);
    auto reader = MessageReader(*fixture.m_client);
    fixture.m_feed_clients[0]->m_blocks.push(encode_malformed_block(1, {}));
    fixture.m_feed_clients[1]->m_blocks.push(encode_block(1, {0x11}));
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == 0x11);
    REQUIRE(!reader.m_types.try_pop());
  }

  TEST_CASE("malformed_block") {
    auto fixture = Fixture(1);
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(reader.m_types.pop() == 0x11);
    SUBCASE("header") {
      fixture.publish(from<SharedBuffer>("\xff\x00\x01\x01\x02\x00\x00\x00"sv));
    }
    SUBCASE("message_tail") {
      fixture.publish(from<SharedBuffer>(
        "\x10\x00\x02\x01\x02\x00\x00\x00"
        "\x06\x42\x00\x00\x00\x00\xff\x43"sv));
    }
    fixture.publish(encode_block(2, {0x12, 0x13}));
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == 0x12);
    REQUIRE(reader.m_types.try_pop() == 0x13);
    REQUIRE(!reader.m_types.try_pop());
  }

  TEST_CASE("missing_snapshot_offer") {
    auto fixture = Fixture(1, 1, 0, Fixture::WithSpin());
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(1, {0x11}));
    flush_pending_routines();
    REQUIRE(!reader.m_types.try_pop());
    fixture.m_time_client.set(TIMESTAMP + Fixture::GAP_TIMEOUT);
    fixture.m_timer.trigger();
    flush_pending_routines();
    REQUIRE(!reader.m_types.try_pop());
    REQUIRE(!fixture.m_spin_client->m_is_closed);
    fixture.m_time_client.set(
      TIMESTAMP + Fixture::GAP_TIMEOUT + time_duration::unit());
    auto types = std::vector{0x11};
    SUBCASE("feed") {
      fixture.publish(encode_block(2, {0x12}));
      types.push_back(0x12);
    }
    SUBCASE("timer") {
      fixture.m_timer.trigger();
    }
    flush_pending_routines();
    for(auto type : types) {
      REQUIRE(reader.m_types.try_pop() == type);
    }
    REQUIRE(!reader.m_types.try_pop());
    REQUIRE(fixture.m_spin_client->m_is_closed);
  }

  TEST_CASE("slow_snapshot") {
    auto fixture = Fixture(1, 1, 0, Fixture::WithSpin());
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(5, {0x11, 0x12, 0x13}));
    flush_pending_routines();
    fixture.m_spin_client->m_sequences.push(6);
    REQUIRE(fixture.m_spin_client->m_requests.pop() == 6);
    fixture.m_time_client.set(
      TIMESTAMP + Fixture::GAP_TIMEOUT + time_duration::unit());
    fixture.publish(encode_block(8, {0x14}));
    flush_pending_routines();
    fixture.m_timer.trigger();
    flush_pending_routines();
    REQUIRE(!fixture.m_spin_client->m_is_closed);
    REQUIRE(!reader.m_types.try_pop());
    auto status = from<SharedBuffer>(
      "\x16\x3b\xf0\x77\xbb\xce\x2a\x6a\x62\x16ZVZT  TAUS \x00"sv);
    auto snapshot = CxaPitchSnapshot(
      6, CxaPitchSpinResponse::ACCEPTED, std::vector{ADD_ORDER, status});
    fixture.m_spin_client->m_snapshots.push(snapshot);
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == int(CxaPitchAddOrder::TYPE));
    REQUIRE(reader.m_types.try_pop() == int(CxaPitchTradingStatus::TYPE));
    REQUIRE(reader.m_types.try_pop() == 0x13);
    REQUIRE(reader.m_types.try_pop() == 0x14);
    REQUIRE(!reader.m_types.try_pop());
  }

  TEST_CASE("snapshot_interruption") {
    auto fixture = Fixture(1, 1, 0, Fixture::WithSpin());
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(5, {0x11, 0x12}));
    flush_pending_routines();
    fixture.m_spin_client->m_sequences.push(6);
    REQUIRE(fixture.m_spin_client->m_requests.pop() == 6);
    REQUIRE(!reader.m_types.try_pop());
    SUBCASE("client_close") {
      fixture.m_client->close();
    }
    SUBCASE("session_failure") {
      fixture.m_spin_client->m_snapshots.close(
        std::make_exception_ptr(IOException("Snapshot session failed.")));
      flush_pending_routines();
      REQUIRE(reader.m_types.try_pop() == 0x11);
      REQUIRE(reader.m_types.try_pop() == 0x12);
      fixture.publish(encode_block(7, {0x13}));
      flush_pending_routines();
      REQUIRE(reader.m_types.try_pop() == 0x13);
      REQUIRE(!reader.m_types.try_pop());
    }
    REQUIRE(fixture.m_spin_client->m_is_closed);
  }

  TEST_CASE("rejected_gap_range") {
    auto fixture = Fixture(1, 2, 1);
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(reader.m_types.pop() == 0x11);
    flush_pending_routines();
    fixture.publish(encode_block(6, {0x16}));
    auto request = fixture.require_recovery_request(CxaPitchGap(2, 4), 7);
    request->m_result.set(4);
    auto sequence = std::uint32_t();
    auto count = std::uint16_t(2);
    auto payload = SharedBuffer();
    auto types = std::vector<std::uint8_t>();
    SUBCASE("suffix") {
      sequence = 4;
      types = {0x12, 0x13};
      payload = encode_block(2, types);
    }
    SUBCASE("prefix") {
      sequence = 2;
      types = {0x14, 0x15};
      payload = encode_block(4, types);
    }
    SUBCASE("whole_range") {
      sequence = 2;
      count = 4;
    }
    auto response = CxaPitchGapResponse(1, sequence, count, 'O');
    fixture.m_gap_client.get_responses()->push(response);
    flush_pending_routines();
    if(!types.empty()) {
      REQUIRE(!reader.m_types.try_pop());
      fixture.m_recovery_clients[0]->m_blocks.push(payload);
      flush_pending_routines();
    }
    for(auto type : types) {
      REQUIRE(reader.m_types.try_pop() == int(type));
    }
    REQUIRE(reader.m_types.try_pop() == 0x16);
  }

  TEST_CASE("partial_gap_recovery") {
    auto fixture = Fixture(1, 2, 1);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(fixture.m_client->read().m_type == 0x11);
    flush_pending_routines();
    fixture.publish(encode_block(5, {0x15}));
    auto request = fixture.require_recovery_request(CxaPitchGap(2, 3), 6);
    request->m_result.set(3);
    flush_pending_routines();
    fixture.m_recovery_clients[0]->m_blocks.push(encode_block(2, {0x12}));
    REQUIRE(fixture.m_client->read().m_type == 0x12);
    flush_pending_routines();
    fixture.publish(encode_block(6, {0x16}));
    for(auto live : {6, 7}) {
      auto recoverable = fixture.require_operation<
        TestCxaPitchGapClient::IsRecoverableOperation>(CxaPitchGap(3, 2), live);
      recoverable->m_result.set(true);
    }
    flush_pending_routines();
    REQUIRE(!fixture.m_gap_operations->try_pop());
  }

  TEST_CASE("gap_without_proxy") {
    auto fixture = Fixture(1, 2, 0, Fixture::WithoutGap());
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(reader.m_types.pop() == 0x11);
    flush_pending_routines();
    fixture.publish(encode_block(4, {0x14}));
    flush_pending_routines();
    REQUIRE(!reader.m_types.try_pop());
    fixture.m_time_client.set(
      TIMESTAMP + Fixture::GAP_TIMEOUT + time_duration::unit());
    fixture.m_feed_clients[0]->m_blocks.push(encode_block(5, {}));
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == 0x14);
  }

  TEST_CASE("unrecoverable_gap") {
    auto fixture = Fixture(2);
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(reader.m_types.pop() == 0x11);
    flush_pending_routines();
    fixture.publish(encode_block(4, {0x14}));
    auto recoverable = fixture.require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(CxaPitchGap(2, 2), 5);
    recoverable->m_result.set(false);
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == 0x14);
    REQUIRE(!fixture.m_gap_operations->try_pop());
  }

  TEST_CASE("gap_timeout") {
    auto fixture = Fixture(2);
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(1, {0x11}));
    REQUIRE(reader.m_types.pop() == 0x11);
    flush_pending_routines();
    fixture.publish(encode_block(4, {}));
    auto request = fixture.require_recovery_request(CxaPitchGap(2, 2), 4);
    auto is_complete = false;
    SUBCASE("pending_request") {}
    SUBCASE("completed_request") {
      is_complete = true;
      request->m_result.set(2);
    }
    flush_pending_routines();
    REQUIRE(!reader.m_types.try_pop());
    fixture.m_time_client.set(
      TIMESTAMP + Fixture::GAP_TIMEOUT + time_duration::unit());
    fixture.m_feed_clients[0]->m_blocks.push(encode_block(4, {0x14, 0x15}));
    auto recoverable = fixture.require_operation<
      TestCxaPitchGapClient::IsRecoverableOperation>(CxaPitchGap(2, 2), 6);
    recoverable->m_result.set(true);
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == 0x14);
    REQUIRE(reader.m_types.try_pop() == 0x15);
    if(!is_complete) {
      request->m_result.set(2);
    }
  }

  TEST_CASE("latest_snapshot_request") {
    auto fixture = Fixture(1, 1, 0, Fixture::WithSpin());
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(5, {0x11, 0x12, 0x13, 0x14, 0x15, 0x16}));
    flush_pending_routines();
    fixture.m_spin_client->m_sequences.push(6);
    flush_pending_routines();
    REQUIRE(fixture.m_spin_client->m_requests.try_pop() == std::uint32_t(6));
    fixture.m_spin_client->m_sequences.push(7);
    fixture.m_spin_client->m_sequences.push(8);
    fixture.m_spin_client->m_sequences.push(9);
    fixture.m_spin_client->m_snapshots.push(CxaPitchSnapshot(6, 'O'));
    flush_pending_routines();
    REQUIRE(fixture.m_spin_client->m_requests.try_pop() == std::uint32_t(9));
    REQUIRE(!fixture.m_spin_client->m_requests.try_pop());
    auto snapshot = CxaPitchSnapshot(
      9, CxaPitchSpinResponse::ACCEPTED, std::vector{ADD_ORDER});
    fixture.m_spin_client->m_snapshots.push(snapshot);
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == int(CxaPitchAddOrder::TYPE));
    REQUIRE(reader.m_types.try_pop() == 0x16);
    REQUIRE(!reader.m_types.try_pop());
  }

  TEST_CASE("snapshot_attempt_limit") {
    auto fixture = Fixture(1, 1, 0, Fixture::WithSpin());
    auto reader = MessageReader(*fixture.m_client);
    fixture.publish(encode_block(5, {0x11}));
    fixture.publish(encode_block(8, {}));
    flush_pending_routines();
    for(auto i = 0; i != Client::SPIN_ATTEMPTS; ++i) {
      fixture.m_spin_client->m_sequences.push(6);
      REQUIRE(fixture.m_spin_client->m_requests.pop() == 6);
      auto snapshot = CxaPitchSnapshot(6, 'O');
      fixture.m_spin_client->m_snapshots.push(snapshot);
    }
    flush_pending_routines();
    REQUIRE(reader.m_types.try_pop() == 0x11);
    REQUIRE(fixture.m_spin_client->m_is_closed);
  }
}
