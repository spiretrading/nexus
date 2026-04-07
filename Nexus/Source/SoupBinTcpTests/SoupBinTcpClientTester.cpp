#include <future>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "Nexus/SoupBinTcp/SoupBinTcpClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::endian;
using namespace Nexus;

namespace {
  struct Fixture {
    LocalServerConnection m_server;
    optional<LocalClientChannel> m_client_channel;
    TriggerTimer m_timer;
    std::unique_ptr<LocalServerChannel> m_server_channel;

    Fixture() {
      auto server_channel_async = std::async(std::launch::async, [&] {
        return m_server.accept();
      });
      m_client_channel.emplace("soupbin_tcp", m_server);
      m_server_channel = server_channel_async.get();
    }
  };

  auto make_login_rejected_packet(char reason) {
    auto buffer = SharedBuffer();
    append(buffer, native_to_big(std::uint16_t(2)));
    append(buffer, 'J');
    append(buffer, reason);
    return buffer;
  }

  auto make_login_accepted_packet(
      const std::string& session, std::uint64_t sequence_number) {
    auto buffer = SharedBuffer();
    append(buffer, native_to_big(std::uint16_t(31)));
    append(buffer, 'A');
    for(auto i = session.size(); i < 10; ++i) {
      append(buffer, ' ');
    }
    append(buffer, session.c_str(), session.size());
    auto seq_str = std::to_string(sequence_number);
    for(auto i = seq_str.size(); i < 20; ++i) {
      append(buffer, ' ');
    }
    append(buffer, seq_str.c_str(), seq_str.size());
    return buffer;
  }

  auto make_data_packet(std::uint8_t type, const std::string& payload) {
    auto buffer = SharedBuffer();
    auto length = std::uint16_t(1 + payload.size());
    append(buffer, native_to_big(length));
    append(buffer, type);
    append(buffer, payload.c_str(), payload.size());
    return buffer;
  }
}

TEST_SUITE("SoupBinTcpClient") {
  TEST_CASE("connect_rejected_not_authorized") {
    auto fixture = Fixture();
    auto server_future = std::async(std::launch::async, [&] {
      auto buffer = SharedBuffer();
      fixture.m_server_channel->get_reader().read(out(buffer));
      auto rejected = make_login_rejected_packet('A');
      fixture.m_server_channel->get_writer().write(rejected);
    });
    REQUIRE_THROWS_AS(SoupBinTcpClient("user", "pass",
      &*fixture.m_client_channel, &fixture.m_timer), ConnectException);
    server_future.get();
  }

  TEST_CASE("connect_rejected_session_unavailable") {
    auto fixture = Fixture();
    auto server_future = std::async(std::launch::async, [&] {
      auto buffer = SharedBuffer();
      fixture.m_server_channel->get_reader().read(out(buffer));
      auto rejected = make_login_rejected_packet('S');
      fixture.m_server_channel->get_writer().write(rejected);
    });
    REQUIRE_THROWS_AS(SoupBinTcpClient("user", "pass",
      &*fixture.m_client_channel, &fixture.m_timer), ConnectException);
    server_future.get();
  }

  TEST_CASE("connect_rejected_other_reason") {
    auto fixture = Fixture();
    auto server_future = std::async(std::launch::async, [&] {
      auto buffer = SharedBuffer();
      fixture.m_server_channel->get_reader().read(out(buffer));
      auto rejected = make_login_rejected_packet('X');
      fixture.m_server_channel->get_writer().write(rejected);
    });
    REQUIRE_THROWS_AS(SoupBinTcpClient("user", "pass",
      &*fixture.m_client_channel, &fixture.m_timer), ConnectException);
    server_future.get();
  }

  TEST_CASE("connect_unrecognized_login_response") {
    auto fixture = Fixture();
    auto server_future = std::async(std::launch::async, [&] {
      auto buffer = SharedBuffer();
      fixture.m_server_channel->get_reader().read(out(buffer));
      auto unrecognized = make_data_packet('Z', "");
      fixture.m_server_channel->get_writer().write(unrecognized);
    });
    REQUIRE_THROWS_AS(SoupBinTcpClient("user", "pass",
      &*fixture.m_client_channel, &fixture.m_timer), ConnectException);
    server_future.get();
  }

  TEST_CASE("read") {
    auto fixture = Fixture();
    auto session = "SESSION";
    auto sequence_number = std::uint64_t(123);
    auto server_future = std::async(std::launch::async, [&] {
      auto buffer = SharedBuffer();
      fixture.m_server_channel->get_reader().read(out(buffer));
      auto accepted = make_login_accepted_packet(session, sequence_number);
      fixture.m_server_channel->get_writer().write(accepted);
    });
    auto client = SoupBinTcpClient(
      "user", "pass", &*fixture.m_client_channel, &fixture.m_timer);
    auto data_packet = make_data_packet('M', "PAYLOAD");
    fixture.m_server_channel->get_writer().write(data_packet);
    auto packet = client.read();
    REQUIRE(packet.m_type == 'M');
    REQUIRE(std::string(packet.m_payload, 7) == "PAYLOAD");
    server_future.get();
  }
}
