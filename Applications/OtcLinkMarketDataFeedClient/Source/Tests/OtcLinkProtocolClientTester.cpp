#include <future>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkProtocolClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  SharedBuffer make_packet(std::uint32_t sequence) {
    auto buffer = SharedBuffer();
    auto write = [&] (auto value) {
      value = endian::native_to_big(value);
      append(buffer, value);
    };
    auto length = OtcLinkMessage::HEADER_LENGTH + sizeof(sequence);
    write(static_cast<std::uint16_t>(OtcLinkHeader::LENGTH + length));
    write(sequence);
    write(std::uint8_t(0));
    write(std::uint8_t(1));
    write(std::uint32_t(0));
    write(static_cast<std::uint16_t>(length));
    write(std::uint8_t(0xFF));
    write(sequence);
    return buffer;
  }

  struct Fixture {
    LocalServerConnection m_server;
    optional<LocalClientChannel> m_channel;
    FixedTimeClient m_time_client;
    optional<OtcLinkProtocolClient<LocalClientChannel*, FixedTimeClient*>>
      m_client;
    std::unique_ptr<LocalServerChannel> m_server_channel;

    Fixture()
        : m_time_client(time_from_string("2026-09-23 14:00:00")) {
      auto server_channel = std::async(std::launch::async, [&] {
        return m_server.accept();
      });
      m_channel.emplace("otc_link", m_server);
      m_client.emplace(&*m_channel, &m_time_client);
      m_server_channel = server_channel.get();
    }
  };
}

TEST_SUITE("OtcLinkProtocolClient") {
  TEST_CASE("packet_flags") {
    auto fixture = Fixture();
    for(auto flag : {OtcLinkHeader::Flag::HEARTBEAT,
        OtcLinkHeader::Flag::SEQUENCE_RESET, OtcLinkHeader::Flag::REPLAY,
        OtcLinkHeader::Flag::TEST}) {
      auto source = make_packet(42);
      auto flags = static_cast<std::uint8_t>(flag);
      auto offset = sizeof(std::uint16_t) + sizeof(std::uint32_t);
      source.write(offset, &flags, sizeof(flags));
      auto is_control = flag == OtcLinkHeader::Flag::HEARTBEAT ||
        flag == OtcLinkHeader::Flag::SEQUENCE_RESET;
      if(is_control) {
        source.shrink(source.get_size() - OtcLinkHeader::LENGTH);
        auto length = endian::native_to_big(
          static_cast<std::uint16_t>(source.get_size()));
        source.write(0, &length, sizeof(length));
        auto count = std::uint8_t(0);
        source.write(offset + sizeof(flags), &count, sizeof(count));
      }
      fixture.m_server_channel->get_writer().write(source);
      auto packet = fixture.m_client->read();
      REQUIRE(packet.get_header().has_flag(flag));
      REQUIRE(packet.get_header().m_sequence == 42);
      if(is_control) {
        REQUIRE(packet.get_payload().empty());
      } else {
        REQUIRE(packet.begin()->get_cursor().read_uint32() == 42);
      }
    }
  }

  TEST_CASE("close_pending_read") {
    auto fixture = Fixture();
    auto read_timestamp = false;
    SUBCASE("plain") {}
    SUBCASE("timestamp") {
      read_timestamp = true;
    }
    auto read = std::packaged_task([&] {
      if(read_timestamp) {
        auto timestamp = ptime();
        return fixture.m_client->read(out(timestamp));
      }
      return fixture.m_client->read();
    });
    auto result = read.get_future();
    auto routine = RoutineHandler(spawn([&] {
      read();
    }));
    flush_pending_routines();
    fixture.m_client->close();
    routine.wait();
    REQUIRE_THROWS_AS(result.get(), IOException);
    REQUIRE_NOTHROW(fixture.m_client->close());
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
    auto timestamp = ptime();
    REQUIRE_THROWS_AS(fixture.m_client->read(out(timestamp)), IOException);
    REQUIRE_NOTHROW(fixture.m_time_client.get_time());
  }

  TEST_CASE("read_failure") {
    auto fixture = Fixture();
    auto receive_time = fixture.m_time_client.get_time();
    fixture.m_server_channel->get_writer().write(make_packet(42));
    flush_pending_routines();
    SUBCASE("channel") {
      fixture.m_server_channel->get_writer().close();
    }
    SUBCASE("time_client") {
      fixture.m_time_client.close();
      fixture.m_server_channel->get_writer().write(make_packet(43));
    }
    flush_pending_routines();
    auto timestamp = ptime();
    auto packet = fixture.m_client->read(out(timestamp));
    REQUIRE(packet.get_header().m_sequence == 42);
    REQUIRE(timestamp == receive_time);
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
    REQUIRE_THROWS_AS(fixture.m_client->read(out(timestamp)), IOException);
  }

  TEST_CASE("malformed_packet") {
    auto fixture = Fixture();
    auto malformed = make_packet(42);
    SUBCASE("truncated") {
      malformed.shrink(1);
    }
    SUBCASE("length") {
      auto length = endian::native_to_big(
        static_cast<std::uint16_t>(malformed.get_size() - 1));
      malformed.write(0, &length, sizeof(length));
    }
    SUBCASE("message_count") {
      auto offset = sizeof(std::uint16_t) + sizeof(std::uint32_t) +
        sizeof(std::uint8_t);
      auto count = std::uint8_t(2);
      malformed.write(offset, &count, sizeof(count));
    }
    SUBCASE("multiple_packets") {
      auto duplicate = malformed;
      append(malformed, duplicate);
    }
    auto receive_time = fixture.m_time_client.get_time();
    fixture.m_server_channel->get_writer().write(malformed);
    flush_pending_routines();
    auto next_receive_time = receive_time + milliseconds(100);
    fixture.m_time_client.set(next_receive_time);
    fixture.m_server_channel->get_writer().write(make_packet(43));
    flush_pending_routines();
    auto timestamp = ptime();
    REQUIRE_THROWS_AS(
      fixture.m_client->read(out(timestamp)), OtcLinkParserException);
    REQUIRE(timestamp == receive_time);
    auto packet = fixture.m_client->read(out(timestamp));
    REQUIRE(timestamp == next_receive_time);
    REQUIRE(packet.get_header().m_sequence == 43);
    REQUIRE(packet.begin()->get_cursor().read_uint32() == 43);
  }

  TEST_CASE("receive_timestamp") {
    auto fixture = Fixture();
    auto receive_time = fixture.m_time_client.get_time();
    fixture.m_server_channel->get_writer().write(make_packet(42));
    flush_pending_routines();
    auto next_receive_time = receive_time + milliseconds(100);
    fixture.m_time_client.set(next_receive_time);
    fixture.m_server_channel->get_writer().write(make_packet(43));
    flush_pending_routines();
    fixture.m_time_client.set(next_receive_time + seconds(5));
    auto timestamp = ptime();
    auto first = fixture.m_client->read(out(timestamp));
    REQUIRE(timestamp == receive_time);
    REQUIRE(first.begin()->get_cursor().read_uint32() == 42);
    auto second = fixture.m_client->read(out(timestamp));
    REQUIRE(timestamp == next_receive_time);
    REQUIRE(second.begin()->get_cursor().read_uint32() == 43);
  }

  TEST_CASE("read_packet") {
    auto fixture = Fixture();
    auto source = make_packet(42);
    fixture.m_server_channel->get_writer().write(source);
    auto first = fixture.m_client->read();
    fixture.m_server_channel->get_writer().write(make_packet(43));
    flush_pending_routines();
    REQUIRE(first.get_header().m_sequence == 42);
    REQUIRE(first.get_header().m_count == 1);
    REQUIRE(first.begin()->m_type == 0xFF);
    REQUIRE(first.begin()->get_cursor().read_uint32() == 42);
    REQUIRE(first.get_payload().data() ==
      source.get_data() + OtcLinkHeader::LENGTH);
    auto second = fixture.m_client->read();
    REQUIRE(second.get_header().m_sequence == 43);
    REQUIRE(second.begin()->get_cursor().read_uint32() == 43);
  }
}
