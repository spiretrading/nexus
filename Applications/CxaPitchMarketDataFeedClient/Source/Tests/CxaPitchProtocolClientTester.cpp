#include <future>
#include <vector>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchProtocolClient.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

namespace {
  constexpr auto UNKNOWN_TYPE = std::uint8_t(0xFF);
  constexpr auto OTHER_UNKNOWN_TYPE = std::uint8_t(0xFE);

  struct Fixture {
    LocalServerConnection m_server;
    optional<LocalClientChannel> m_channel;
    optional<CxaPitchProtocolClient<LocalClientChannel*>> m_client;
    std::unique_ptr<LocalServerChannel> m_server_channel;

    Fixture() {
      auto server_channel = std::async(std::launch::async, [&] {
        return m_server.accept();
      });
      m_channel.emplace("cxa", m_server);
      m_client.emplace(&*m_channel);
      m_server_channel = server_channel.get();
    }
  };

  SharedBuffer encode_block(
      std::uint32_t sequence, const std::vector<std::uint8_t>& types) {
    constexpr auto MESSAGE_LENGTH =
      CxaPitchMessage::HEADER_LENGTH + sizeof(std::uint32_t);
    auto block = SharedBuffer();
    CxaPitchHeader(static_cast<std::uint16_t>(
      CxaPitchHeader::LENGTH + MESSAGE_LENGTH * types.size()),
      static_cast<std::uint8_t>(types.size()), 1, sequence).encode(out(block));
    auto encoder = CxaPitchEncoder(Ref(block));
    for(auto type : types) {
      encoder.write_uint8(static_cast<std::uint8_t>(MESSAGE_LENGTH));
      encoder.write_uint8(type);
      encoder.write_uint32(sequence);
      ++sequence;
    }
    return block;
  }
}

TEST_SUITE("CxaPitchProtocolClient") {
  TEST_CASE("read_block") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(
      encode_block(4155, {UNKNOWN_TYPE, OTHER_UNKNOWN_TYPE}));
    auto block = fixture.m_client->read();
    REQUIRE(block.get_header().m_count == 2);
    REQUIRE(block.get_header().m_unit == 1);
    REQUIRE(block.get_header().m_sequence == 4155);
    auto types = std::vector<std::uint8_t>();
    auto sequence = block.get_header().m_sequence;
    for(auto& message : block) {
      types.push_back(message.m_type);
      REQUIRE(message.get_cursor().read_uint32() == sequence);
      ++sequence;
    }
    REQUIRE(
      types == std::vector<std::uint8_t>{UNKNOWN_TYPE, OTHER_UNKNOWN_TYPE});
  }

  TEST_CASE("read_heartbeat") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(encode_block(4155, {}));
    auto block = fixture.m_client->read();
    REQUIRE(block.get_header().m_length == CxaPitchHeader::LENGTH);
    REQUIRE(block.get_header().m_count == 0);
    REQUIRE(block.get_header().m_sequence == 4155);
    REQUIRE(block.begin() == block.end());
  }

  TEST_CASE("read_consecutive_blocks") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(
      encode_block(1, {UNKNOWN_TYPE, OTHER_UNKNOWN_TYPE}));
    fixture.m_server_channel->get_writer().write(
      encode_block(3, {OTHER_UNKNOWN_TYPE}));
    fixture.m_server_channel->get_writer().write(encode_block(4, {}));
    fixture.m_server_channel->get_writer().write(
      encode_block(4, {OTHER_UNKNOWN_TYPE, UNKNOWN_TYPE, OTHER_UNKNOWN_TYPE}));
    auto first = fixture.m_client->read();
    REQUIRE(first.get_header().m_sequence == 1);
    REQUIRE(first.get_header().m_count == 2);
    auto message = first.begin();
    REQUIRE(message->m_type == UNKNOWN_TYPE);
    REQUIRE(message->get_cursor().read_uint32() == 1);
    ++message;
    REQUIRE(message->m_type == OTHER_UNKNOWN_TYPE);
    REQUIRE(message->get_cursor().read_uint32() == 2);
    ++message;
    REQUIRE(message == first.end());
    auto second = fixture.m_client->read();
    REQUIRE(second.get_header().m_sequence == 3);
    REQUIRE(second.get_header().m_count == 1);
    REQUIRE(second.begin()->m_type == OTHER_UNKNOWN_TYPE);
    REQUIRE(second.begin()->get_cursor().read_uint32() == 3);
    auto heartbeat = fixture.m_client->read();
    REQUIRE(heartbeat.get_header().m_sequence == 4);
    REQUIRE(heartbeat.get_header().m_count == 0);
    REQUIRE(heartbeat.get_payload().empty());
    auto last = fixture.m_client->read();
    REQUIRE(last.get_header().m_sequence == 4);
    REQUIRE(last.get_header().m_count == 3);
    auto types = std::vector<std::uint8_t>();
    auto sequence = last.get_header().m_sequence;
    for(auto& message : last) {
      types.push_back(message.m_type);
      REQUIRE(message.get_cursor().read_uint32() == sequence);
      ++sequence;
    }
    REQUIRE(types == std::vector<std::uint8_t>{
      OTHER_UNKNOWN_TYPE, UNKNOWN_TYPE, OTHER_UNKNOWN_TYPE});
  }

  TEST_CASE("read_truncated_block") {
    auto fixture = Fixture();
    auto data = encode_block(1, {});
    data.shrink(sizeof(std::uint32_t));
    fixture.m_server_channel->get_writer().write(data);
    fixture.m_server_channel->get_writer().write(
      encode_block(2, {UNKNOWN_TYPE}));
    REQUIRE_THROWS_AS(fixture.m_client->read(), CxaPitchParserException);
    auto block = fixture.m_client->read();
    REQUIRE(block.get_header().m_sequence == 2);
    REQUIRE(block.get_header().m_count == 1);
    REQUIRE(block.begin()->m_type == UNKNOWN_TYPE);
    REQUIRE(block.begin()->get_cursor().read_uint32() == 2);
  }

  TEST_CASE("read_failure") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().close(
      EndOfFileException("Feed disconnected."));
    auto error = std::exception_ptr();
    try {
      fixture.m_client->read();
    } catch(const std::exception&) {
      error = std::current_exception();
    }
    REQUIRE(error);
    try {
      std::rethrow_exception(error);
    } catch(const IOException& e) {
      REQUIRE(std::string_view(e.what()) == "Failed to read CXA PITCH block.");
      REQUIRE_THROWS_WITH_AS(
        std::rethrow_if_nested(e), "Feed disconnected.", EndOfFileException);
    }
  }

  TEST_CASE("close_pending_read") {
    auto fixture = Fixture();
    auto results = Queue<std::exception_ptr>();
    auto reader = RoutineHandler(spawn([&] {
      auto error = std::exception_ptr();
      try {
        fixture.m_client->read();
      } catch(const std::exception&) {
        error = std::current_exception();
      }
      results.push(error);
    }));
    flush_pending_routines();
    auto before = results.try_pop();
    fixture.m_client->close();
    flush_pending_routines();
    auto after = results.try_pop();
    fixture.m_channel->get_connection().close();
    reader.wait();
    REQUIRE(!before);
    REQUIRE(after.has_value());
    REQUIRE(*after);
    REQUIRE_THROWS_AS(std::rethrow_exception(*after), IOException);
    REQUIRE_NOTHROW(fixture.m_client->close());
  }
}
