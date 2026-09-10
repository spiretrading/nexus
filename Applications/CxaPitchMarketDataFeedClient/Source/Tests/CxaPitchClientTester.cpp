#include <algorithm>
#include <atomic>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct StubProtocolClient {
    std::shared_ptr<Queue<SharedBuffer>> m_blocks;
    std::shared_ptr<Queue<int>> m_reads;
    SharedBuffer m_payload;

    StubProtocolClient()
      : m_blocks(std::make_shared<Queue<SharedBuffer>>()),
        m_reads(std::make_shared<Queue<int>>()) {}

    CxaPitchBlock read() {
      m_payload = m_blocks->pop();
      m_reads->push(0);
      return CxaPitchBlock::parse(
        std::string_view(m_payload.get_data(), m_payload.get_size()));
    }

    void close() {
      m_blocks->close();
      m_reads->close();
    }
  };

  struct StubGapClient {
    std::shared_ptr<Queue<CxaPitchGap>> m_requests;
    std::shared_ptr<Queue<CxaPitchGapResponse>> m_responses;
    std::shared_ptr<Queue<int>> m_gate;
    std::uint32_t m_limit;
    bool m_is_recoverable;

    StubGapClient()
      : m_requests(std::make_shared<Queue<CxaPitchGap>>()),
        m_responses(std::make_shared<Queue<CxaPitchGapResponse>>()),
        m_limit(std::numeric_limits<std::uint32_t>::max()),
        m_is_recoverable(true) {}

    bool is_recoverable(const CxaPitchGap& gap, std::uint32_t live) const {
      return m_is_recoverable;
    }

    std::uint32_t request(std::uint8_t unit, const CxaPitchGap& gap,
        std::uint32_t live) {
      if(m_gate) {
        m_gate->pop();
      }
      m_requests->push(gap);
      return std::min(gap.m_count, m_limit);
    }

    const std::shared_ptr<Queue<CxaPitchGapResponse>>&
        get_responses() const {
      return m_responses;
    }

    void close() {
      m_requests->close();
      m_responses->close();
      if(m_gate) {
        m_gate->push(0);
      }
    }
  };

  struct StubSpinClient {
    std::shared_ptr<Queue<std::uint32_t>> m_offers;
    std::shared_ptr<Queue<std::uint32_t>> m_requests;
    std::shared_ptr<Queue<CxaPitchSpin>> m_spins;
    std::atomic_uint64_t m_progress;
    std::atomic_bool m_is_closed;

    StubSpinClient()
      : m_offers(std::make_shared<Queue<std::uint32_t>>()),
        m_requests(std::make_shared<Queue<std::uint32_t>>()),
        m_spins(std::make_shared<Queue<CxaPitchSpin>>()),
        m_progress(0),
        m_is_closed(false) {}

    const std::shared_ptr<Queue<std::uint32_t>>& get_offers() const {
      return m_offers;
    }

    std::uint64_t get_progress() const {
      return m_progress.load();
    }

    CxaPitchSpin request(std::uint32_t sequence) {
      m_requests->push(sequence);
      return m_spins->pop();
    }

    void close() {
      m_is_closed = true;
      m_offers->close();
      m_requests->close();
      m_spins->close();
    }
  };

  using Client = CxaPitchClient<StubProtocolClient*, StubGapClient*,
    StubSpinClient*, FixedTimeClient*>;

  const auto TIMESTAMP = time_from_string("2026-09-09 10:00:00");

  struct MessageReader {
    Client* m_client;
    std::shared_ptr<Queue<int>> m_types;
    RoutineHandler m_routine;

    explicit MessageReader(Client& client)
      : m_client(&client),
        m_types(std::make_shared<Queue<int>>()) {
      m_routine = spawn([=, this] {
        try {
          while(true) {
            m_types->push(m_client->read().m_type);
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
    auto payload = std::string();
    for(auto type : types) {
      payload += char(6);
      payload += static_cast<char>(type);
      payload.append(4, char(0));
    }
    auto length =
      static_cast<std::uint16_t>(CxaPitchHeader::LENGTH + payload.size());
    auto block = std::string();
    block += static_cast<char>(length & 0xFF);
    block += static_cast<char>((length >> 8) & 0xFF);
    block += static_cast<char>(types.size());
    block += char(1);
    block += static_cast<char>(sequence & 0xFF);
    block += static_cast<char>((sequence >> 8) & 0xFF);
    block += static_cast<char>((sequence >> 16) & 0xFF);
    block += static_cast<char>((sequence >> 24) & 0xFF);
    block += payload;
    return SharedBuffer(block.data(), block.size());
  }

  SharedBuffer encode_message(std::uint8_t type) {
    auto message = std::string();
    message += char(6);
    message += static_cast<char>(type);
    message.append(4, char(0));
    return SharedBuffer(message.data(), message.size());
  }
}

TEST_SUITE("CxaPitchClient") {
  TEST_CASE("read_messages_in_sequence") {
    auto feed = StubProtocolClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&feed},
      std::vector<StubProtocolClient*>(), none, none, &time_client);
    feed.m_blocks->push(encode_block(1, {0x11, 0x12}));
    feed.m_blocks->push(encode_block(3, {0x13}));
    REQUIRE(client.read().m_type == 0x11);
    REQUIRE(client.read().m_type == 0x12);
    REQUIRE(client.read().m_type == 0x13);
  }

  TEST_CASE("arbitrate_two_feeds") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&first, &second},
      std::vector<StubProtocolClient*>(), none, none, &time_client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(2, {0x12}));
    REQUIRE(client.read().m_type == 0x11);
    REQUIRE(client.read().m_type == 0x12);
  }

  TEST_CASE("own_clients") {
    using OwnedClient = CxaPitchClient<StubProtocolClient, StubGapClient,
      StubSpinClient*, FixedTimeClient*>;
    auto feed = StubProtocolClient();
    auto gap_client = StubGapClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = OwnedClient(1, seconds(3), seconds(5), std::vector{feed},
      std::vector<StubProtocolClient>(), optional(gap_client), none,
      &time_client);
    feed.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(client.read().m_type == 0x11);
  }

  TEST_CASE("ignore_another_unit") {
    auto feed = StubProtocolClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(2, seconds(3), seconds(5), std::vector{&feed},
      std::vector<StubProtocolClient*>(), none, none, &time_client);
    feed.m_blocks->push(encode_block(1, {0x11}));
    feed.m_reads->pop();
    REQUIRE(!feed.m_blocks->try_pop());
  }

  TEST_CASE("recover_gap") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto recovery = StubProtocolClient();
    auto gap_client = StubGapClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&first, &second},
      std::vector{&recovery}, &gap_client, none,
      &time_client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(client.read().m_type == 0x11);
    first.m_blocks->push(encode_block(4, {0x14}));
    second.m_blocks->push(encode_block(4, {0x14}));
    auto gap = gap_client.m_requests->pop();
    REQUIRE(gap.m_sequence == 2);
    REQUIRE(gap.m_count == 2);
    recovery.m_blocks->push(encode_block(2, {0x12, 0x13}));
    REQUIRE(client.read().m_type == 0x12);
    REQUIRE(client.read().m_type == 0x13);
    REQUIRE(client.read().m_type == 0x14);
  }

  TEST_CASE("request_throttling") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto gap_client = StubGapClient();
    gap_client.m_limit = 1;
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&first, &second},
      std::vector<StubProtocolClient*>(), &gap_client, none, &time_client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(client.read().m_type == 0x11);
    first.m_blocks->push(encode_block(4, {0x14}));
    second.m_blocks->push(encode_block(4, {0x14}));
    auto request = gap_client.m_requests->pop();
    REQUIRE(request.m_sequence == 2);
    REQUIRE(request.m_count == 2);
    first.m_blocks->push(encode_block(5, {0x15}));
    second.m_blocks->push(encode_block(5, {0x15}));
    auto retry = gap_client.m_requests->pop();
    REQUIRE(retry.m_sequence == 3);
    REQUIRE(retry.m_count == 1);
  }

  TEST_CASE("feed_during_request") {
    auto feed = StubProtocolClient();
    auto gap_client = StubGapClient();
    gap_client.m_gate = std::make_shared<Queue<int>>();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5), std::vector{&feed},
      std::vector<StubProtocolClient*>(), &gap_client, none, &time_client);
    auto reader = MessageReader(client);
    feed.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(reader.m_types->pop() == 0x11);
    feed.m_blocks->push(encode_block(4, {}));
    flush_pending_routines();
    time_client.set(TIMESTAMP + seconds(6));
    feed.m_blocks->push(encode_block(4, {0x14}));
    flush_pending_routines();
    REQUIRE(reader.m_types->try_pop().value_or(0) == 0x14);
  }

  TEST_CASE("partial_request_retry") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto gap_client = StubGapClient();
    gap_client.m_limit = 1;
    gap_client.m_gate = std::make_shared<Queue<int>>();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client =
      Client(1, seconds(3), seconds(30), std::vector{&first, &second},
        std::vector<StubProtocolClient*>(), &gap_client, none, &time_client);
    auto reader = MessageReader(client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(reader.m_types->pop() == 0x11);
    first.m_blocks->push(encode_block(4, {}));
    second.m_blocks->push(encode_block(4, {}));
    flush_pending_routines();
    time_client.set(TIMESTAMP + seconds(4));
    first.m_blocks->push(encode_block(6, {}));
    second.m_blocks->push(encode_block(6, {}));
    flush_pending_routines();
    gap_client.m_gate->push(0);
    auto request = gap_client.m_requests->pop();
    REQUIRE(request.m_sequence == 2);
    REQUIRE(request.m_count == 2);
    gap_client.m_gate->push(0);
    auto retry = gap_client.m_requests->pop();
    REQUIRE(retry.m_sequence == 3);
    REQUIRE(retry.m_count == 3);
  }

  TEST_CASE("read_malformed_block") {
    auto feed = StubProtocolClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(
      1, seconds(3), seconds(5), std::vector{&feed},
      std::vector<StubProtocolClient*>(), none, none, &time_client);
    feed.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(client.read().m_type == 0x11);
    feed.m_blocks->push(SharedBuffer("\xff\x00\x01\x01\x02\x00\x00\x00", 8));
    feed.m_blocks->push(encode_block(2, {0x12}));
    flush_pending_routines();
    REQUIRE(!feed.m_blocks->try_pop());
  }

  TEST_CASE("missing_snapshot_offer") {
    auto feed = StubProtocolClient();
    auto spin_client = StubSpinClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(
      1, seconds(3), seconds(5), std::vector{&feed},
      std::vector<StubProtocolClient*>(), none, &spin_client, &time_client);
    auto reader = MessageReader(client);
    feed.m_blocks->push(encode_block(1, {0x11}));
    time_client.set(TIMESTAMP + seconds(6));
    feed.m_blocks->push(encode_block(2, {0x12}));
    flush_pending_routines();
    auto type = reader.m_types->try_pop().value_or(0);
    REQUIRE(type == 0x11);
  }

  TEST_CASE("recover_malformed_datagram") {
    for(auto is_recovery : {false, true}) {
      auto feed = StubProtocolClient();
      auto recovery = StubProtocolClient();
      auto gap_client = StubGapClient();
      auto time_client = FixedTimeClient(TIMESTAMP);
      auto client = Client(1, seconds(3), seconds(5),
        std::vector{&feed},
        std::vector{&recovery}, &gap_client, none,
        &time_client);
      auto reader = MessageReader(client);
      feed.m_blocks->push(encode_block(1, {0x11}));
      REQUIRE(reader.m_types->pop() == 0x11);
      auto malformed = SharedBuffer(
        "\x10\x00\x02\x01\x02\x00\x00\x00"
        "\x06\x42\x00\x00\x00\x00\xff\x43", 16);
      if(!is_recovery) {
        feed.m_blocks->push(malformed);
      }
      feed.m_blocks->push(encode_block(4, {0x14}));
      auto gap = gap_client.m_requests->pop();
      REQUIRE(gap.m_sequence == 2);
      REQUIRE(gap.m_count == 2);
      if(is_recovery) {
        recovery.m_blocks->push(malformed);
      }
      flush_pending_routines();
      REQUIRE(!reader.m_types->try_pop());
      recovery.m_blocks->push(encode_block(2, {0x12, 0x13}));
      flush_pending_routines();
      REQUIRE(reader.m_types->try_pop().value_or(0) == 0x12);
      REQUIRE(reader.m_types->try_pop().value_or(0) == 0x13);
      REQUIRE(reader.m_types->try_pop().value_or(0) == 0x14);
    }
  }

  TEST_CASE("slow_snapshot") {
    auto feed = StubProtocolClient();
    auto spin_client = StubSpinClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&feed},
      std::vector<StubProtocolClient*>(), none, &spin_client, &time_client);
    auto reader = MessageReader(client);
    feed.m_blocks->push(encode_block(5, {0x11, 0x12, 0x13}));
    flush_pending_routines();
    spin_client.m_offers->push(6);
    REQUIRE(spin_client.m_requests->pop() == 6);
    time_client.set(TIMESTAMP + duration_from_string("00:00:04"));
    ++spin_client.m_progress;
    feed.m_blocks->push(encode_block(8, {}));
    flush_pending_routines();
    time_client.set(TIMESTAMP + seconds(6));
    feed.m_blocks->push(encode_block(8, {0x14}));
    flush_pending_routines();
    REQUIRE(!reader.m_types->try_pop());
    REQUIRE(!spin_client.m_is_closed.load());
    auto spin = CxaPitchSpin();
    spin.m_sequence = 6;
    spin.m_status = CxaPitchSpinResponse::ACCEPTED;
    spin.m_messages.push_back(encode_message(0x37));
    spin_client.m_spins->push(spin);
    flush_pending_routines();
    REQUIRE(reader.m_types->try_pop().value_or(0) == 0x37);
    REQUIRE(reader.m_types->try_pop().value_or(0) == 0x13);
    REQUIRE(reader.m_types->try_pop().value_or(0) == 0x14);
  }

  TEST_CASE("snapshot_progress_timeout") {
    auto feed = StubProtocolClient();
    auto spin_client = StubSpinClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(
      1, seconds(3), seconds(5), std::vector{&feed},
      std::vector<StubProtocolClient*>(), none, &spin_client, &time_client);
    auto reader = MessageReader(client);
    feed.m_blocks->push(encode_block(5, {0x11, 0x12}));
    flush_pending_routines();
    spin_client.m_offers->push(6);
    REQUIRE(spin_client.m_requests->pop() == 6);
    time_client.set(TIMESTAMP + duration_from_string("00:00:04"));
    ++spin_client.m_progress;
    feed.m_blocks->push(encode_block(7, {}));
    flush_pending_routines();
    time_client.set(TIMESTAMP + duration_from_string("00:00:08"));
    spin_client.m_offers->push(7);
    feed.m_blocks->push(encode_block(7, {}));
    flush_pending_routines();
    REQUIRE(!reader.m_types->try_pop());
    time_client.set(TIMESTAMP + duration_from_string("00:00:10"));
    feed.m_blocks->push(encode_block(7, {0x13}));
    flush_pending_routines();
    REQUIRE(spin_client.m_is_closed.load());
    REQUIRE(reader.m_types->try_pop().value_or(0) == 0x11);
    REQUIRE(reader.m_types->try_pop().value_or(0) == 0x12);
    REQUIRE(reader.m_types->try_pop().value_or(0) == 0x13);
  }

  TEST_CASE("rejected_gap_chunk") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto recovery = StubProtocolClient();
    auto gap_client = StubGapClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&first, &second},
      std::vector{&recovery}, &gap_client, none, &time_client);
    auto reader = MessageReader(client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(reader.m_types->pop() == 0x11);
    first.m_blocks->push(encode_block(6, {0x16}));
    second.m_blocks->push(encode_block(6, {0x16}));
    auto gap = gap_client.m_requests->pop();
    REQUIRE(gap.m_sequence == 2);
    REQUIRE(gap.m_count == 4);
    auto response = CxaPitchGapResponse();
    response.m_unit = 1;
    response.m_sequence = 4;
    response.m_count = 2;
    response.m_status = 'O';
    gap_client.m_responses->push(response);
    flush_pending_routines();
    recovery.m_blocks->push(encode_block(2, {0x12, 0x13}));
    REQUIRE(reader.m_types->pop() == 0x12);
    REQUIRE(reader.m_types->pop() == 0x13);
    REQUIRE(reader.m_types->pop() == 0x16);
  }

  TEST_CASE("rejected_gap_prefix") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto recovery = StubProtocolClient();
    auto gap_client = StubGapClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&first, &second},
      std::vector{&recovery}, &gap_client, none, &time_client);
    auto reader = MessageReader(client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(reader.m_types->pop() == 0x11);
    first.m_blocks->push(encode_block(6, {0x16}));
    second.m_blocks->push(encode_block(6, {0x16}));
    REQUIRE(gap_client.m_requests->pop().m_count == 4);
    auto response = CxaPitchGapResponse();
    response.m_unit = 1;
    response.m_sequence = 2;
    response.m_count = 2;
    response.m_status = 'O';
    gap_client.m_responses->push(response);
    flush_pending_routines();
    recovery.m_blocks->push(encode_block(4, {0x14, 0x15}));
    REQUIRE(reader.m_types->pop() == 0x14);
    REQUIRE(reader.m_types->pop() == 0x15);
    REQUIRE(reader.m_types->pop() == 0x16);
  }

  TEST_CASE("partial_gap_recovery") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto recovery = StubProtocolClient();
    auto gap_client = StubGapClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&first, &second},
      std::vector{&recovery}, &gap_client, none,
      &time_client);
    auto reader = MessageReader(client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(reader.m_types->pop() == 0x11);
    first.m_blocks->push(encode_block(5, {0x15}));
    second.m_blocks->push(encode_block(5, {0x15}));
    REQUIRE(gap_client.m_requests->pop().m_count == 3);
    recovery.m_blocks->push(encode_block(2, {0x12}));
    REQUIRE(reader.m_types->pop() == 0x12);
    first.m_blocks->push(encode_block(6, {0x16}));
    second.m_blocks->push(encode_block(6, {0x16}));
    flush_pending_routines();
    auto is_requested = static_cast<bool>(gap_client.m_requests->try_pop());
    REQUIRE(!is_requested);
  }

  TEST_CASE("gap_without_proxy") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(-1),
      std::vector{&first, &second},
      std::vector<StubProtocolClient*>(), none, none, &time_client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(client.read().m_type == 0x11);
    first.m_blocks->push(encode_block(4, {0x14}));
    second.m_blocks->push(encode_block(4, {0x14}));
    REQUIRE(client.read().m_type == 0x14);
  }

  TEST_CASE("unrecoverable_gap") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto gap_client = StubGapClient();
    gap_client.m_is_recoverable = false;
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&first, &second},
      std::vector<StubProtocolClient*>(), &gap_client, none, &time_client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(client.read().m_type == 0x11);
    first.m_blocks->push(encode_block(4, {0x14}));
    second.m_blocks->push(encode_block(4, {0x14}));
    REQUIRE(client.read().m_type == 0x14);
    REQUIRE(!gap_client.m_requests->try_pop());
  }

  TEST_CASE("gap_timeout") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto gap_client = StubGapClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&first, &second},
      std::vector<StubProtocolClient*>(), &gap_client, none, &time_client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(client.read().m_type == 0x11);
    first.m_blocks->push(encode_block(4, {0x14}));
    second.m_blocks->push(encode_block(4, {0x14}));
    REQUIRE(gap_client.m_requests->pop().m_sequence == 2);
    time_client.set(TIMESTAMP + seconds(6));
    first.m_blocks->push(encode_block(5, {0x15}));
    REQUIRE(client.read().m_type == 0x14);
    REQUIRE(client.read().m_type == 0x15);
  }

  TEST_CASE("rejected_gap") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto gap_client = StubGapClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&first, &second},
      std::vector<StubProtocolClient*>(), &gap_client, none, &time_client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    REQUIRE(client.read().m_type == 0x11);
    first.m_blocks->push(encode_block(4, {0x14}));
    second.m_blocks->push(encode_block(4, {0x14}));
    REQUIRE(gap_client.m_requests->pop().m_sequence == 2);
    auto response = CxaPitchGapResponse();
    response.m_unit = 1;
    response.m_sequence = 2;
    response.m_count = 2;
    response.m_status = 'O';
    gap_client.m_responses->push(response);
    REQUIRE(client.read().m_type == 0x14);
  }

  TEST_CASE("snapshot_message_order") {
    auto feed = StubProtocolClient();
    auto spin_client = StubSpinClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&feed},
      std::vector<StubProtocolClient*>(), none, &spin_client,
      &time_client);
    feed.m_blocks->push(encode_block(5, {0x11, 0x12, 0x13}));
    feed.m_blocks->push(encode_block(8, {}));
    feed.m_reads->pop();
    feed.m_reads->pop();
    spin_client.m_offers->push(6);
    REQUIRE(spin_client.m_requests->pop() == 6);
    auto spin = CxaPitchSpin();
    spin.m_sequence = 6;
    spin.m_status = CxaPitchSpinResponse::ACCEPTED;
    spin.m_messages.push_back(encode_message(0x37));
    spin.m_messages.push_back(encode_message(0x3B));
    spin_client.m_spins->push(std::move(spin));
    REQUIRE(client.read().m_type == 0x37);
    REQUIRE(client.read().m_type == 0x3B);
    REQUIRE(client.read().m_type == 0x13);
  }

  TEST_CASE("snapshot_attempt_limit") {
    auto feed = StubProtocolClient();
    auto spin_client = StubSpinClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector{&feed},
      std::vector<StubProtocolClient*>(), none, &spin_client,
      &time_client);
    feed.m_blocks->push(encode_block(5, {0x11}));
    feed.m_blocks->push(encode_block(8, {}));
    feed.m_reads->pop();
    feed.m_reads->pop();
    for(auto i = 0; i != Client::SPIN_ATTEMPTS; ++i) {
      spin_client.m_offers->push(6);
      REQUIRE(spin_client.m_requests->pop() == 6);
      auto spin = CxaPitchSpin();
      spin.m_sequence = 6;
      spin.m_status = 'O';
      spin_client.m_spins->push(std::move(spin));
    }
    REQUIRE(client.read().m_type == 0x11);
  }
}
