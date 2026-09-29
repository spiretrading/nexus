#include <future>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "Nexus/SoupBinTcp/SoupBinTcpClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::endian;
using namespace Nexus;

namespace {
  struct BlockingWriter {
    PipedWriter* m_writer;
    Queue<bool> m_gate;
    bool m_is_login = true;

    template<IsBuffer B>
    void write(const B& buffer) {
      if(m_is_login) {
        m_is_login = false;
      } else {
        m_gate.pop();
      }
      m_writer->write(buffer);
    }
  };

  struct BlockingConnection {
    LocalConnection* m_connection;
    Queue<bool>* m_gate;

    void close() {
      m_gate->close();
      m_connection->close();
    }
  };

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

  using Client = SoupBinTcpClient<LocalClientChannel*, TriggerTimer*>;

  auto make_login_rejected_packet(char reason) {
    auto buffer = SharedBuffer();
    append(buffer, native_to_big(LoginRejectedPacket::LENGTH));
    append(buffer, LoginRejectedPacket::TYPE);
    append(buffer, reason);
    return buffer;
  }

  auto make_login_accepted_packet(
      const std::string& session, std::uint64_t sequence_number) {
    auto buffer = SharedBuffer();
    append(buffer, native_to_big(LoginAcceptedPacket::LENGTH));
    append(buffer, LoginAcceptedPacket::TYPE);
    for(auto i = session.size(); i < LoginAcceptedPacket::SESSION_LENGTH; ++i) {
      append(buffer, ' ');
    }
    append(buffer, session.c_str(), session.size());
    auto sequence = std::to_string(sequence_number);
    for(auto i = sequence.size();
        i < LoginAcceptedPacket::SEQUENCE_LENGTH; ++i) {
      append(buffer, ' ');
    }
    append(buffer, sequence.c_str(), sequence.size());
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
  TEST_CASE("login_rejection") {
    auto fixture = Fixture();
    auto reason = 'A';
    SUBCASE("unauthorized") {}
    SUBCASE("unavailable_session") {
      reason = 'S';
    }
    SUBCASE("unknown_reason") {
      reason = 'X';
    }
    fixture.m_server_channel->get_writer().write(
      make_login_rejected_packet(reason));
    REQUIRE_THROWS_AS(SoupBinTcpClient("user", "pass",
      &*fixture.m_client_channel, &fixture.m_timer), ConnectException);
  }

  TEST_CASE("login_response") {
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
    auto data_packet = make_data_packet('S', "PAYLOAD");
    fixture.m_server_channel->get_writer().write(data_packet);
    auto packet = client.read();
    REQUIRE(packet.m_type == 'S');
    REQUIRE(packet.get_payload() == "PAYLOAD");
    server_future.get();
  }

  TEST_CASE("login_timeout") {
    auto fixture = Fixture();
    auto login = std::async(std::launch::async, [&] {
      return std::make_unique<Client>("user", "pass",
          &*fixture.m_client_channel, &fixture.m_timer);
    });
    auto request = SharedBuffer();
    fixture.m_server_channel->get_reader().read(out(request));
    auto is_debug = false;
    SUBCASE("silence") {}
    SUBCASE("debug_packets") {
      is_debug = true;
    }
    for(auto i = 0; i < Client::TIMEOUT_TICKS; ++i) {
      if(is_debug) {
        fixture.m_server_channel->get_writer().write(
          make_data_packet('+', "x"));
      }
      fixture.m_timer.trigger();
      flush_pending_routines();
    }
    auto status = login.wait_for(std::chrono::seconds(1));
    fixture.m_server_channel->get_connection().close();
    REQUIRE_THROWS_AS(login.get(), ConnectException);
    REQUIRE(status == std::future_status::ready);
  }

  TEST_CASE("connection_failure") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(
      make_login_accepted_packet("SESSION123", 1));
    auto client = SoupBinTcpClient(
      "user", "pass", &*fixture.m_client_channel, &fixture.m_timer);
    auto reader = std::async(std::launch::async, [&] {
      return client.read();
    });
    SUBCASE("silence") {
      for(auto i = 0; i <= Client::TIMEOUT_TICKS; ++i) {
        fixture.m_timer.trigger();
        flush_pending_routines();
      }
    }
    SUBCASE("timer") {
      fixture.m_timer.fail();
      flush_pending_routines();
    }
    SUBCASE("heartbeat_write") {
      fixture.m_client_channel->get_writer().close(IOException());
      fixture.m_timer.trigger();
      flush_pending_routines();
    }
    SUBCASE("close") {
      client.close();
    }
    SUBCASE("framing") {
      fixture.m_server_channel->get_writer().write(SharedBuffer("\x00\x00", 2));
    }
    auto status = reader.wait_for(std::chrono::seconds(1));
    client.close();
    REQUIRE_THROWS_AS(reader.get(), IOException);
    REQUIRE(status == std::future_status::ready);
  }

  TEST_CASE("blocked_write") {
    auto fixture = Fixture();
    auto writer = BlockingWriter(&fixture.m_client_channel->get_writer());
    auto connection = BlockingConnection(
      &fixture.m_client_channel->get_connection(), &writer.m_gate);
    auto channel =
      WrapperChannel(&*fixture.m_client_channel, &writer, &connection);
    fixture.m_server_channel->get_writer().write(
      make_login_accepted_packet("SESSION", 1));
    auto client = SoupBinTcpClient("user", "pass", &channel, &fixture.m_timer);
    auto reader = std::async(std::launch::async, [&] {
      return client.read();
    });
    for(auto i = 0; i <= Client::TIMEOUT_TICKS; ++i) {
      fixture.m_timer.trigger();
      flush_pending_routines();
    }
    auto status = reader.wait_for(std::chrono::seconds(1));
    client.close();
    REQUIRE_THROWS_AS(reader.get(), IOException);
    REQUIRE(status == std::future_status::ready);
  }

  TEST_CASE("heartbeat") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(
      make_login_accepted_packet("SESSION", 1));
    auto client = SoupBinTcpClient(
      "user", "pass", &*fixture.m_client_channel, &fixture.m_timer);
    auto buffer = SharedBuffer();
    REQUIRE(read_packet(fixture.m_server_channel->get_reader(),
      out(buffer)).m_type == 'L');
    for(auto i = 0; i <= Client::TIMEOUT_TICKS; ++i) {
      fixture.m_server_channel->get_writer().write(make_data_packet('H', ""));
      REQUIRE(client.read().m_type == 'H');
      fixture.m_timer.trigger();
      flush_pending_routines();
      reset(buffer);
      auto heartbeat =
        read_packet(fixture.m_server_channel->get_reader(), out(buffer));
      REQUIRE(heartbeat.m_type == 'R');
      REQUIRE(heartbeat.get_payload().empty());
    }
    REQUIRE(client.get_sequence_number() == 1);
  }

  TEST_CASE("sequence") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(
      make_login_accepted_packet("SESSION", 123));
    auto client = SoupBinTcpClient(
      "user", "pass", &*fixture.m_client_channel, &fixture.m_timer);
    REQUIRE(client.get_session() == "SESSION");
    REQUIRE(client.get_sequence_number() == 123);
    auto sequence = std::uint64_t(123);
    for(auto type : {'H', '+', 'S', 'S', 'Z'}) {
      fixture.m_server_channel->get_writer().write(make_data_packet(type, ""));
      REQUIRE(client.read().m_type == type);
      if(type == 'S') {
        ++sequence;
      }
      REQUIRE(client.get_sequence_number() == sequence);
    }
    REQUIRE(client.get_sequence_number() == 125);
  }
}
