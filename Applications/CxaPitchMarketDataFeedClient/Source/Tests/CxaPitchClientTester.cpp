#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Queues/Queue.hpp>
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
    std::uint32_t m_limit;

    StubGapClient()
      : m_requests(std::make_shared<Queue<CxaPitchGap>>()),
        m_responses(std::make_shared<Queue<CxaPitchGapResponse>>()),
        m_limit(std::numeric_limits<std::uint32_t>::max()) {}

    std::uint32_t request(std::uint8_t unit, const CxaPitchGap& gap,
        std::uint32_t live, ptime timestamp) {
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
    }
  };

  struct StubSpinClient {
    std::shared_ptr<Queue<std::uint32_t>> m_offers;
    std::shared_ptr<Queue<std::uint32_t>> m_requests;
    std::shared_ptr<Queue<CxaPitchSpin>> m_spins;

    StubSpinClient()
      : m_offers(std::make_shared<Queue<std::uint32_t>>()),
        m_requests(std::make_shared<Queue<std::uint32_t>>()),
        m_spins(std::make_shared<Queue<CxaPitchSpin>>()) {}

    const std::shared_ptr<Queue<std::uint32_t>>& get_offers() const {
      return m_offers;
    }

    CxaPitchSpin request(std::uint32_t sequence) {
      m_requests->push(sequence);
      return m_spins->pop();
    }

    void close() {
      m_offers->close();
      m_requests->close();
      m_spins->close();
    }
  };

  using Client = CxaPitchClient<StubProtocolClient*, StubGapClient*,
    StubSpinClient*, FixedTimeClient*>;

  const auto TIMESTAMP = time_from_string("2026-09-09 10:00:00");

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
      std::vector<StubProtocolClient*>({&feed}),
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
      std::vector<StubProtocolClient*>({&first, &second}),
      std::vector<StubProtocolClient*>(), none, none, &time_client);
    first.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(1, {0x11}));
    second.m_blocks->push(encode_block(2, {0x12}));
    REQUIRE(client.read().m_type == 0x11);
    REQUIRE(client.read().m_type == 0x12);
  }

  TEST_CASE("ignore_another_unit") {
    auto feed = StubProtocolClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(2, seconds(3), seconds(5),
      std::vector<StubProtocolClient*>({&feed}),
      std::vector<StubProtocolClient*>(), none, none, &time_client);
    feed.m_blocks->push(encode_block(1, {0x11}));
    feed.m_reads->pop();
    REQUIRE(!feed.m_blocks->try_pop());
  }

  TEST_CASE("request_and_recover_a_gap") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto recovery = StubProtocolClient();
    auto gap_client = StubGapClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector<StubProtocolClient*>({&first, &second}),
      std::vector<StubProtocolClient*>({&recovery}), &gap_client, none,
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

  TEST_CASE("retry_a_throttled_gap_request") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto gap_client = StubGapClient();
    gap_client.m_limit = 1;
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector<StubProtocolClient*>({&first, &second}),
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

  TEST_CASE("skip_a_gap_that_stays_unfilled") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto gap_client = StubGapClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector<StubProtocolClient*>({&first, &second}),
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

  TEST_CASE("skip_a_rejected_gap") {
    auto first = StubProtocolClient();
    auto second = StubProtocolClient();
    auto gap_client = StubGapClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector<StubProtocolClient*>({&first, &second}),
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

  TEST_CASE("apply_a_snapshot_before_the_live_messages") {
    auto feed = StubProtocolClient();
    auto spin_client = StubSpinClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector<StubProtocolClient*>({&feed}),
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

  TEST_CASE("give_up_on_a_rejected_snapshot") {
    auto feed = StubProtocolClient();
    auto spin_client = StubSpinClient();
    auto time_client = FixedTimeClient(TIMESTAMP);
    auto client = Client(1, seconds(3), seconds(5),
      std::vector<StubProtocolClient*>({&feed}),
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
