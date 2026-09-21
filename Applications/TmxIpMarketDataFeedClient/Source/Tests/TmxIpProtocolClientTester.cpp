#include <future>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "TmxIpMarketDataFeedClient/TmxIpProtocolClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct Fixture {
    LocalServerConnection m_server;
    optional<LocalClientChannel> m_channel;
    FixedTimeClient m_time_client;
    optional<TmxIpProtocolClient<LocalClientChannel*, FixedTimeClient*>>
      m_client;
    std::unique_ptr<LocalServerChannel> m_server_channel;

    Fixture()
        : m_time_client(time_from_string("2026-09-20 23:59:59.900")) {
      auto server_channel = std::async(std::launch::async, [&] {
        return m_server.accept();
      });
      m_channel.emplace("tmx_ip", m_server);
      m_client.emplace(&*m_channel, &m_time_client);
      m_server_channel = server_channel.get();
    }
  };
}

TEST_SUITE("TmxIpProtocolClient") {
  TEST_CASE("ingress_timestamp") {
    auto fixture = Fixture();
    flush_pending_routines();
    auto ingress = fixture.m_time_client.get_time();
    fixture.m_server_channel->get_writer().write(
      from<SharedBuffer>("\x02" "0029000000001CDF00  T payload\x03"));
    flush_pending_routines();
    auto next_ingress = ingress + milliseconds(200);
    fixture.m_time_client.set(next_ingress);
    fixture.m_server_channel->get_writer().write(
      from<SharedBuffer>("\x02" "0026000000002CDF00  T next\x03"));
    flush_pending_routines();
    fixture.m_time_client.set(next_ingress + seconds(5));
    auto timestamp = ptime();
    auto first = fixture.m_client->read(out(timestamp));
    REQUIRE(timestamp == ingress);
    REQUIRE(first.m_header.m_sequence == std::uint32_t(1));
    REQUIRE(first.m_payload == "payload");
    auto second = fixture.m_client->read(out(timestamp));
    REQUIRE(timestamp == next_ingress);
    REQUIRE(second.m_header.m_sequence == std::uint32_t(2));
    REQUIRE(second.m_payload == "next");
  }

  TEST_CASE("read_packet") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(from<SharedBuffer>(
      "\x02" "0036000000001CDF00  T "
      "\x01\x1e" "1=H\x1c\x1e" "55=ABX\x1d\x03"));
    auto first = fixture.m_client->read();
    fixture.m_server_channel->get_writer().write(
      from<SharedBuffer>("\x02" "0029000000002CDF00  T payload\x03"));
    flush_pending_routines();
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
    auto ingress = fixture.m_time_client.get_time();
    fixture.m_server_channel->get_writer().write(malformed);
    flush_pending_routines();
    auto next_ingress = ingress + milliseconds(200);
    fixture.m_time_client.set(next_ingress);
    fixture.m_server_channel->get_writer().write(
      from<SharedBuffer>("\x02" "0026000000002CDF00  T next\x03"));
    flush_pending_routines();
    fixture.m_time_client.set(next_ingress + seconds(5));
    auto timestamp = ptime();
    REQUIRE_THROWS_AS(
      fixture.m_client->read(out(timestamp)), TmxIpParserException);
    REQUIRE(timestamp == ingress);
    auto packet = fixture.m_client->read(out(timestamp));
    REQUIRE(timestamp == next_ingress);
    REQUIRE(packet.m_header.m_sequence == std::uint32_t(2));
    REQUIRE(packet.m_payload == "next");
  }

  TEST_CASE("read_failure") {
    auto fixture = Fixture();
    auto ingress = fixture.m_time_client.get_time();
    fixture.m_server_channel->get_writer().write(
      from<SharedBuffer>("\x02" "0029000000001CDF00  T payload\x03"));
    flush_pending_routines();
    auto next_ingress = ingress + milliseconds(200);
    fixture.m_time_client.set(next_ingress);
    fixture.m_server_channel->get_writer().write(
      from<SharedBuffer>("\x02" "0026000000002CDF00  T next\x03"));
    flush_pending_routines();
    SUBCASE("channel") {
      fixture.m_server_channel->get_writer().close(EndOfFileException());
    }
    SUBCASE("time_client") {
      fixture.m_time_client.close();
      fixture.m_server_channel->get_writer().write(
        from<SharedBuffer>("\x02" "0026000000001CDF00  T next\x03"));
    }
    flush_pending_routines();
    auto timestamp = ptime();
    auto packet = fixture.m_client->read(out(timestamp));
    REQUIRE(packet.m_header.m_sequence == std::uint32_t(1));
    REQUIRE(packet.m_payload == "payload");
    REQUIRE(timestamp == ingress);
    packet = fixture.m_client->read(out(timestamp));
    REQUIRE(packet.m_header.m_sequence == std::uint32_t(2));
    REQUIRE(packet.m_payload == "next");
    REQUIRE(timestamp == next_ingress);
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
    REQUIRE_THROWS_AS(fixture.m_client->read(out(timestamp)), IOException);
  }

  TEST_CASE("close_pending_read") {
    auto fixture = Fixture();
    auto read_timestamp = false;
    SUBCASE("plain") {}
    SUBCASE("timestamp") {
      read_timestamp = true;
    }
    auto results = Queue<std::exception_ptr>();
    auto reader = RoutineHandler(spawn([&] {
      auto error = std::exception_ptr();
      try {
        if(read_timestamp) {
          auto timestamp = ptime();
          fixture.m_client->read(out(timestamp));
        } else {
          fixture.m_client->read();
        }
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
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
    auto timestamp = ptime();
    REQUIRE_THROWS_AS(fixture.m_client->read(out(timestamp)), IOException);
    REQUIRE_NOTHROW(fixture.m_time_client.get_time());
  }
}
