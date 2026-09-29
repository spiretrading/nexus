#include <future>
#include <Beam/IO/LocalServerConnection.hpp>
#include <doctest/doctest.h>
#include "Nexus/MoldUdp64/MoldUdp64Client.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

namespace {
  struct Fixture {
    LocalServerConnection m_server;
    optional<LocalClientChannel> m_channel;
    optional<MoldUdp64Client<LocalClientChannel*>> m_client;
    std::unique_ptr<LocalServerChannel> m_server_channel;

    Fixture() {
      auto server_channel = std::async(std::launch::async, [&] {
        return m_server.accept();
      });
      m_channel.emplace("mold_udp", m_server);
      m_client.emplace(&*m_channel);
      m_server_channel = server_channel.get();
    }
  };
}

TEST_SUITE("MoldUdp64Client") {
  TEST_CASE("read") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(SharedBuffer("ABCDEFGHIJ"
      "\x00\x00\x00\x00\x00\x00\x00\x2A\x00\x02"
      "\x00\x03ONE\x00\x03TWO", 30));
    fixture.m_server_channel->get_writer().write(SharedBuffer("KLMNOPQRST"
      "\x00\x00\x00\x00\x00\x00\x00\x01\x00\x01"
      "\x00\x01X", 23));
    auto first = fixture.m_client->read();
    REQUIRE(first.m_session == "ABCDEFGHIJ");
    REQUIRE(first.m_sequence_number == 42);
    REQUIRE(first.m_count == 2);
    auto message = first.begin();
    REQUIRE(message->get_payload() == "ONE");
    ++message;
    REQUIRE(message->get_payload() == "TWO");
    ++message;
    REQUIRE(message == first.end());
    auto second = fixture.m_client->read();
    REQUIRE(second.m_session == "KLMNOPQRST");
    REQUIRE(second.m_sequence_number == 1);
    REQUIRE(second.m_count == 1);
    REQUIRE(second.begin()->get_payload() == "X");
  }

  TEST_CASE("control_packets") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(SharedBuffer(
      "ABCDEFGHIJ" "\x00\x00\x00\x00\x00\x00\x00\x2A\x00\x00", 20));
    fixture.m_server_channel->get_writer().write(SharedBuffer(
      "ABCDEFGHIJ" "\x00\x00\x00\x00\x00\x00\x00\x2A\xFF\xFF", 20));
    fixture.m_server_channel->get_writer().write(SharedBuffer("ABCDEFGHIJ"
      "\x00\x00\x00\x00\x00\x00\x00\x29\x00\x01"
      "\x00\x00", 22));
    fixture.m_server_channel->get_connection().close();
    auto heartbeat = fixture.m_client->read();
    REQUIRE(heartbeat.is_heartbeat());
    REQUIRE(heartbeat.m_sequence_number == 42);
    REQUIRE(heartbeat.begin() == heartbeat.end());
    auto end = fixture.m_client->read();
    REQUIRE(end.is_end_of_session());
    REQUIRE(end.m_sequence_number == 42);
    REQUIRE(end.begin() == end.end());
    auto recovery = fixture.m_client->read();
    REQUIRE(recovery.m_sequence_number == 41);
    REQUIRE(recovery.m_count == 1);
    REQUIRE(recovery.begin()->get_payload().empty());
  }

  TEST_CASE("malformed_packet") {
    auto fixture = Fixture();
    SUBCASE("header") {
      fixture.m_server_channel->get_writer().write(SharedBuffer("SHORT", 5));
    }
    SUBCASE("message") {
      fixture.m_server_channel->get_writer().write(SharedBuffer("ABCDEFGHIJ"
        "\x00\x00\x00\x00\x00\x00\x00\x2A\x00\x02"
        "\x00\x01X\x00\x03Y", 26));
    }
    fixture.m_server_channel->get_writer().write(SharedBuffer("ABCDEFGHIJ"
      "\x00\x00\x00\x00\x00\x00\x00\x2A\x00\x01"
      "\x00\x01Z", 23));
    REQUIRE_THROWS_AS(fixture.m_client->read(), MoldUdp64ParserException);
    auto packet = fixture.m_client->read();
    REQUIRE(packet.m_sequence_number == 42);
    REQUIRE(packet.m_count == 1);
    REQUIRE(packet.begin()->get_payload() == "Z");
  }

  TEST_CASE("read_failure") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().close(EndOfFileException());
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
  }

  TEST_CASE("request") {
    auto fixture = Fixture();
    fixture.m_client->request(MoldUdp64Request("ABCDEFGHIJ", 42, 100));
    auto buffer = SharedBuffer();
    fixture.m_server_channel->get_reader().read(out(buffer));
    REQUIRE(buffer == std::string_view("ABCDEFGHIJ"
      "\x00\x00\x00\x00\x00\x00\x00\x2A\x00\x64", 20));
    fixture.m_server_channel->get_writer().write(SharedBuffer("ABCDEFGHIJ"
      "\x00\x00\x00\x00\x00\x00\x00\x2A\x00\x02"
      "\x00\x03ONE\x00\x03TWO", 30));
    auto packet = fixture.m_client->read();
    REQUIRE(packet.m_session == "ABCDEFGHIJ");
    REQUIRE(packet.m_sequence_number == 42);
    REQUIRE(packet.m_count == 2);
    REQUIRE(packet.begin()->get_payload() == "ONE");
    fixture.m_client->request(MoldUdp64Request("ABCDEFGHIJ", 44, 98));
    reset(buffer);
    fixture.m_server_channel->get_reader().read(out(buffer));
    REQUIRE(buffer == std::string_view("ABCDEFGHIJ"
      "\x00\x00\x00\x00\x00\x00\x00\x2C\x00\x62", 20));
    REQUIRE(packet.begin()->get_payload() == "ONE");
  }

  TEST_CASE("request_failure") {
    auto fixture = Fixture();
    fixture.m_channel->get_writer().close(EndOfFileException());
    REQUIRE_THROWS_AS(
      fixture.m_client->request(MoldUdp64Request("ABCDEFGHIJ", 42, 100)),
      IOException);
  }

  TEST_CASE("close") {
    auto fixture = Fixture();
    SUBCASE("pending_read") {
      auto reader = std::async(std::launch::async, [&] {
        return fixture.m_client->read();
      });
      fixture.m_client->close();
      REQUIRE_THROWS_AS(reader.get(), IOException);
      REQUIRE_NOTHROW(fixture.m_client->close());
    }
    SUBCASE("destruction") {
      fixture.m_client.reset();
      auto buffer = SharedBuffer();
      REQUIRE_THROWS_AS(
        fixture.m_server_channel->get_reader().read(out(buffer)),
        EndOfFileException);
    }
  }
}
