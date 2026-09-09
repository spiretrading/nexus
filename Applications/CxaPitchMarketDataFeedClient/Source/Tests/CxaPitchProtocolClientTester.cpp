#include <cstdint>
#include <future>
#include <memory>
#include <string>
#include <vector>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchProtocolClient.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

namespace {
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
}

TEST_SUITE("CxaPitchProtocolClient") {
  TEST_CASE("read_block") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(
      encode_block(4155, {0x37, 0x38}));
    auto block = fixture.m_client->read();
    REQUIRE(block.get_header().m_count == 2);
    REQUIRE(block.get_header().m_unit == 1);
    REQUIRE(block.get_header().m_sequence == 4155);
    auto types = std::vector<std::uint8_t>();
    for(auto& message : block) {
      types.push_back(message.m_type);
    }
    REQUIRE(types == std::vector<std::uint8_t>({0x37, 0x38}));
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
    fixture.m_server_channel->get_writer().write(encode_block(1, {0x37}));
    fixture.m_server_channel->get_writer().write(encode_block(2, {0x3C}));
    auto first = fixture.m_client->read();
    REQUIRE(first.get_header().m_sequence == 1);
    REQUIRE(first.begin()->m_type == 0x37);
    auto second = fixture.m_client->read();
    REQUIRE(second.get_header().m_sequence == 2);
    REQUIRE(second.begin()->m_type == 0x3C);
  }

  TEST_CASE("read_truncated_block") {
    auto fixture = Fixture();
    auto data = std::string("\x08\x00\x00\x01", 4);
    fixture.m_server_channel->get_writer().write(
      SharedBuffer(data.data(), data.size()));
    REQUIRE_THROWS_AS(fixture.m_client->read(), CxaPitchParserException);
  }
}
