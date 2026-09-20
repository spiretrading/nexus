#include <future>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Routines/RoutineHandler.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpProtocolClient.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

namespace {
  struct Fixture {
    LocalServerConnection m_server;
    optional<LocalClientChannel> m_channel;
    optional<TmxIpProtocolClient<LocalClientChannel*>> m_client;
    std::unique_ptr<LocalServerChannel> m_server_channel;

    Fixture() {
      auto server_channel = std::async(std::launch::async, [&] {
        return m_server.accept();
      });
      m_channel.emplace("tmx_ip", m_server);
      m_client.emplace(&*m_channel);
      m_server_channel = server_channel.get();
    }
  };
}

TEST_SUITE("TmxIpProtocolClient") {
  TEST_CASE("read_packet") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(from<SharedBuffer>(
      "\x02" "0036000000001CDF00  T "
      "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d\x03"));
    fixture.m_server_channel->get_writer().write(
      from<SharedBuffer>("\x02" "0029000000002CDF00  T payload\x03"));
    auto first = fixture.m_client->read();
    REQUIRE(first.m_header.m_sequence == std::uint32_t(1));
    REQUIRE(first.m_header.m_service == "CDF");
    REQUIRE(first.m_header.m_exchange == 'T');
    REQUIRE(first.m_payload == "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d");
    auto second = fixture.m_client->read();
    REQUIRE(second.m_header.m_sequence == std::uint32_t(2));
    REQUIRE(second.m_payload == "payload");
  }

  TEST_CASE("heartbeat_and_fragments") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(
      from<SharedBuffer>("\x02" "0031         CDF00V T HEARTBEAT\x03"));
    fixture.m_server_channel->get_writer().write(
      from<SharedBuffer>("\x02" "0027000000001CDF01  T \x01\x1e" "1=H\x03"));
    fixture.m_server_channel->get_writer().write(
      from<SharedBuffer>("\x02" "0024000000002CDF02  T X\x1d\x03"));
    auto heartbeat = fixture.m_client->read();
    REQUIRE(is_heartbeat(heartbeat.m_header));
    REQUIRE(heartbeat.m_payload == "HEARTBEAT");
    auto first = fixture.m_client->read();
    REQUIRE(first.m_header.m_sequence == std::uint32_t(1));
    REQUIRE(first.m_header.m_continuation == TmxIpHeader::Continuation::FIRST);
    REQUIRE(first.m_payload == "\x01\x1e" "1=H");
    auto last = fixture.m_client->read();
    REQUIRE(last.m_header.m_sequence == std::uint32_t(2));
    REQUIRE(last.m_header.m_continuation == TmxIpHeader::Continuation::LAST);
    REQUIRE(last.m_payload == "X\x1d");
  }

  TEST_CASE("malformed_packet") {
    auto fixture = Fixture();
    auto malformed =
      from<SharedBuffer>("\x02" "0029000000001CDF00  T payload\x03");
    SUBCASE("truncated") {
      malformed.shrink(sizeof(TmxIpPacket::END));
    }
    SUBCASE("length") {
      malformed.write(sizeof(TmxIpPacket::START), "0028", sizeof("0028") - 1);
    }
    SUBCASE("multiple_packets") {
      auto duplicate = malformed;
      append(malformed, duplicate);
    }
    fixture.m_server_channel->get_writer().write(malformed);
    fixture.m_server_channel->get_writer().write(
      from<SharedBuffer>("\x02" "0026000000002CDF00  T next\x03"));
    REQUIRE_THROWS_AS(fixture.m_client->read(), TmxIpParserException);
    auto packet = fixture.m_client->read();
    REQUIRE(packet.m_header.m_sequence == std::uint32_t(2));
    REQUIRE(packet.m_payload == "next");
  }

  TEST_CASE("read_failure") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().close(EndOfFileException());
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
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
