#include <cstddef>
#include <cstdint>
#include <future>
#include <memory>
#include <stop_token>
#include <string>
#include <string_view>
#include <Beam/IO/AsyncWriter.hpp>
#include <Beam/IO/ConnectException.hpp>
#include <Beam/IO/IOException.hpp>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/IO/SharedBuffer.hpp>
#include <Beam/IO/WrapperChannel.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/optional/optional.hpp>
#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchSessionClient.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

namespace {
  const auto LOGIN_SIZE = CxaPitchHeader::LENGTH + CxaPitchLogin::LENGTH;
  const auto GAP_RESPONSE =
    std::string_view("\x0a\x04" "\x01" "\x3b\x10\x00\x00" "\x32\x00" "A", 10);

  SharedBuffer encode(std::string_view messages, std::uint8_t count) {
    auto block = std::string();
    auto length =
      static_cast<std::uint16_t>(CxaPitchHeader::LENGTH + messages.size());
    block += static_cast<char>(length & 0xFF);
    block += static_cast<char>((length >> 8) & 0xFF);
    block += static_cast<char>(count);
    block += char(0);
    block.append(4, char(0));
    block += messages;
    return SharedBuffer(block.data(), block.size());
  }

  struct GatedWriter {
    LocalClientChannel::Writer* m_writer;
    std::shared_ptr<Queue<int>> m_gate;
    std::shared_ptr<Queue<int>> m_writes;

    void write(BufferCRef buffer) {
      m_writes->push(0);
      m_gate->pop();
      m_writer->write(buffer);
    }
  };

  using GatedChannel =
    WrapperChannel<LocalClientChannel*, AsyncWriter<GatedWriter>>;

  std::string read_exactly(LocalServerChannel& channel, std::size_t size) {
    auto buffer = SharedBuffer();
    while(buffer.get_size() < size) {
      channel.get_reader().read(out(buffer), size - buffer.get_size());
    }
    return std::string(buffer.get_data(), buffer.get_size());
  }

  struct Fixture {
    LocalServerConnection m_server;
    optional<LocalClientChannel> m_channel;
    TriggerTimer m_timer;
    std::string m_login;
    std::unique_ptr<LocalServerChannel> m_server_channel;
    optional<
      CxaPitchSessionClient<LocalClientChannel*, TriggerTimer*>> m_client;

    explicit Fixture(char status) {
      auto server_channel = std::async(std::launch::async, [&] {
        auto channel = m_server.accept();
        m_login = read_exactly(*channel, LOGIN_SIZE);
        auto response = std::string("\x03\x02", 2);
        response += status;
        channel->get_writer().write(encode(response, 1));
        return channel;
      });
      m_channel.emplace("cxa", m_server);
      auto login = CxaPitchLogin();
      login.m_session_sub_id = "0001";
      login.m_username = "FIRM";
      login.m_password = "ABCD00";
      m_client.emplace(login, &*m_channel, &m_timer);
      m_server_channel = server_channel.get();
    }

    Fixture()
      : Fixture(CxaPitchLoginResponse::ACCEPTED) {}
  };
}

TEST_SUITE("CxaPitchSessionClient") {
  TEST_CASE("log_in") {
    auto fixture = Fixture();
    REQUIRE(fixture.m_login == std::string_view(
      "\x1e\x00" "\x01" "\x00" "\x00\x00\x00\x00"
      "\x16\x01" "0001" "FIRM" "  " "ABCD00    ", 30));
  }

  TEST_CASE("reject_login") {
    REQUIRE_THROWS_AS(Fixture('N'), ConnectException);
  }

  TEST_CASE("cancel_pending_login") {
    auto server = LocalServerConnection();
    auto stop_source = std::stop_source();
    auto accepting = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      read_exactly(*channel, LOGIN_SIZE);
      stop_source.request_stop();
      return channel;
    });
    auto channel = LocalClientChannel("cxa", server);
    auto timer = TriggerTimer();
    REQUIRE_THROWS_AS(CxaPitchSessionClient(CxaPitchLogin(), &channel, &timer,
      stop_source.get_token()), ConnectException);
    auto server_channel = accepting.get();
    server_channel->get_connection().close();
  }

  TEST_CASE("cancel_before_login") {
    auto server = LocalServerConnection();
    auto accepting = std::async(std::launch::async, [&] {
      return server.accept();
    });
    auto channel = LocalClientChannel("cxa", server);
    auto server_channel = accepting.get();
    auto stop_source = std::stop_source();
    stop_source.request_stop();
    auto timer = TriggerTimer();
    REQUIRE_THROWS_AS(CxaPitchSessionClient(CxaPitchLogin(), &channel, &timer,
      stop_source.get_token()), ConnectException);
    server_channel->get_connection().close();
  }

  TEST_CASE("timeout_pending_login") {
    auto server = LocalServerConnection();
    auto timer = TriggerTimer();
    auto accepting = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      read_exactly(*channel, LOGIN_SIZE);
      return channel;
    });
    auto channel = LocalClientChannel("cxa", server);
    auto results = Queue<bool>();
    auto connecting = RoutineHandler(spawn([&] {
      try {
        auto client = CxaPitchSessionClient(CxaPitchLogin(), &channel, &timer);
        results.push(false);
      } catch(const ConnectException&) {
        results.push(true);
      } catch(const std::exception&) {
        results.push(false);
      }
    }));
    auto server_channel = accepting.get();
    flush_pending_routines();
    using Client = CxaPitchSessionClient<LocalClientChannel*, TriggerTimer*>;
    for(auto i = 0; i != Client::SILENT_HEARTBEAT_LIMIT; ++i) {
      timer.trigger();
      flush_pending_routines();
    }
    auto result = results.try_pop();
    server_channel->get_connection().close();
    connecting.wait();
    REQUIRE(result.has_value());
    REQUIRE(*result);
  }

  TEST_CASE("timeout_silent_session") {
    auto fixture = Fixture();
    fixture.m_timer.trigger();
    flush_pending_routines();
    using Client = CxaPitchSessionClient<LocalClientChannel*, TriggerTimer*>;
    for(auto i = 0; i != Client::SILENT_HEARTBEAT_LIMIT; ++i) {
      fixture.m_timer.trigger();
      flush_pending_routines();
    }
    REQUIRE_THROWS(fixture.m_server_channel->get_writer().write(
      encode(GAP_RESPONSE, 1)));
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
  }

  TEST_CASE("timer_failure") {
    auto fixture = Fixture();
    fixture.m_timer.fail();
    flush_pending_routines();
    REQUIRE_THROWS(fixture.m_server_channel->get_writer().write(
      encode(GAP_RESPONSE, 1)));
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
  }

  TEST_CASE("timeout_blocked_write") {
    auto server = LocalServerConnection();
    auto accepting = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      read_exactly(*channel, LOGIN_SIZE);
      auto response = std::string("\x03\x02", 2);
      response += CxaPitchLoginResponse::ACCEPTED;
      channel->get_writer().write(encode(response, 1));
      return channel;
    });
    auto channel = LocalClientChannel("cxa", server);
    auto gate = std::make_shared<Queue<int>>();
    auto writes = std::make_shared<Queue<int>>();
    gate->push(0);
    auto wrapper = GatedChannel(
      &channel, GatedWriter(&channel.get_writer(), gate, writes));
    auto timer = TriggerTimer();
    auto client = CxaPitchSessionClient(CxaPitchLogin(), &wrapper, &timer);
    auto server_channel = accepting.get();
    writes->pop();
    timer.trigger();
    flush_pending_routines();
    writes->pop();
    using Client = decltype(client);
    for(auto i = 0; i != Client::SILENT_HEARTBEAT_LIMIT; ++i) {
      timer.trigger();
      flush_pending_routines();
    }
    gate->close();
    REQUIRE_THROWS(server_channel->get_writer().write(encode(GAP_RESPONSE, 1)));
    REQUIRE_THROWS_AS(client.read(), IOException);
    server_channel->get_connection().close();
  }

  TEST_CASE("read_message") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(encode(GAP_RESPONSE, 1));
    auto message = fixture.m_client->read();
    REQUIRE(message.m_type == CxaPitchGapResponse::TYPE);
    auto response = CxaPitchGapResponse::parse(message);
    REQUIRE(response.m_unit == 1);
    REQUIRE(response.m_sequence == 4155);
    REQUIRE(response.m_count == 50);
    REQUIRE(response.m_status == CxaPitchGapResponse::ACCEPTED);
  }

  TEST_CASE("read_fragmented_message") {
    auto fixture = Fixture();
    auto block = encode(GAP_RESPONSE, 1);
    fixture.m_server_channel->get_writer().write(
      SharedBuffer(block.get_data(), 5));
    fixture.m_server_channel->get_writer().write(
      SharedBuffer(block.get_data() + 5, block.get_size() - 5));
    auto message = fixture.m_client->read();
    REQUIRE(message.m_type == CxaPitchGapResponse::TYPE);
    REQUIRE(CxaPitchGapResponse::parse(message).m_sequence == 4155);
  }

  TEST_CASE("read_multiple_messages") {
    auto fixture = Fixture();
    auto messages = std::string(GAP_RESPONSE) + std::string(GAP_RESPONSE);
    fixture.m_server_channel->get_writer().write(encode(messages, 2));
    REQUIRE(fixture.m_client->read().m_type == CxaPitchGapResponse::TYPE);
    REQUIRE(fixture.m_client->read().m_type == CxaPitchGapResponse::TYPE);
  }

  TEST_CASE("skip_heartbeat") {
    auto fixture = Fixture();
    fixture.m_server_channel->get_writer().write(encode("", 0));
    fixture.m_server_channel->get_writer().write(encode(GAP_RESPONSE, 1));
    REQUIRE(fixture.m_client->read().m_type == CxaPitchGapResponse::TYPE);
  }

  TEST_CASE("read_consecutive_blocks") {
    auto fixture = Fixture();
    auto messages = std::string(GAP_RESPONSE) + std::string(GAP_RESPONSE);
    auto buffer = encode(messages, 2);
    append(buffer, encode("", 0));
    append(buffer, encode(std::string_view("\x03\x02" "A", 3), 1));
    fixture.m_server_channel->get_writer().write(buffer);
    auto first = fixture.m_client->read();
    auto second = fixture.m_client->read();
    REQUIRE(CxaPitchGapResponse::parse(first).m_sequence == 4155);
    REQUIRE(CxaPitchGapResponse::parse(second).m_sequence == 4155);
    REQUIRE(second.m_payload == first.m_payload + first.m_length);
    auto response = CxaPitchLoginResponse::parse(fixture.m_client->read());
    REQUIRE(response.m_status == CxaPitchLoginResponse::ACCEPTED);
  }

  TEST_CASE("read_miscounted_block") {
    auto trailing = std::string(GAP_RESPONSE) + std::string("\x02\x04", 2);
    for(auto& block : {encode(GAP_RESPONSE, 0), encode(GAP_RESPONSE, 2),
        encode(trailing, 1)}) {
      auto fixture = Fixture();
      fixture.m_server_channel->get_writer().write(block);
      REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
    }
  }

  TEST_CASE("write_gap_request") {
    auto fixture = Fixture();
    auto request = CxaPitchGapRequest();
    request.m_unit = 1;
    request.m_sequence = 4155;
    request.m_count = 50;
    fixture.m_client->write(request);
    REQUIRE(read_exactly(*fixture.m_server_channel,
      CxaPitchHeader::LENGTH + CxaPitchGapRequest::LENGTH) == std::string_view(
        "\x11\x00" "\x01" "\x00" "\x00\x00\x00\x00"
        "\x09\x03" "\x01" "\x3b\x10\x00\x00" "\x32\x00", 17));
  }

  TEST_CASE("queued_writes") {
    auto server = LocalServerConnection();
    auto accepting = std::async(std::launch::async, [&] {
      auto channel = server.accept();
      read_exactly(*channel, LOGIN_SIZE);
      auto response = std::string("\x03\x02", 2);
      response += CxaPitchLoginResponse::ACCEPTED;
      channel->get_writer().write(encode(response, 1));
      return channel;
    });
    auto channel = LocalClientChannel("cxa", server);
    auto gate = std::make_shared<Queue<int>>();
    auto writes = std::make_shared<Queue<int>>();
    gate->push(0);
    auto wrapper = GatedChannel(
      &channel, GatedWriter(&channel.get_writer(), gate, writes));
    auto timer = TriggerTimer();
    auto client = CxaPitchSessionClient(CxaPitchLogin(), &wrapper, &timer);
    auto server_channel = accepting.get();
    writes->pop();
    auto completed = Queue<int>();
    auto writer = RoutineHandler(spawn([&] {
      auto request = CxaPitchGapRequest(1, 100, 1);
      client.write(request);
      request.m_sequence = 200;
      request.m_count = 2;
      client.write(request);
      completed.push(0);
    }));
    writes->pop();
    flush_pending_routines();
    auto is_completed = completed.try_pop().has_value();
    gate->push(0);
    gate->push(0);
    writer.wait();
    auto size = CxaPitchHeader::LENGTH + CxaPitchGapRequest::LENGTH;
    auto first = read_exactly(*server_channel, size);
    auto second = read_exactly(*server_channel, size);
    REQUIRE(is_completed);
    auto cursor = (*CxaPitchBlock::parse(first).begin()).get_cursor();
    REQUIRE(cursor.read_uint8() == 1);
    REQUIRE(cursor.read_uint32() == 100);
    REQUIRE(cursor.read_uint16() == 1);
    cursor = (*CxaPitchBlock::parse(second).begin()).get_cursor();
    REQUIRE(cursor.read_uint8() == 1);
    REQUIRE(cursor.read_uint32() == 200);
    REQUIRE(cursor.read_uint16() == 2);
  }

  TEST_CASE("write_heartbeat") {
    auto fixture = Fixture();
    fixture.m_timer.trigger();
    REQUIRE(read_exactly(*fixture.m_server_channel, CxaPitchHeader::LENGTH) ==
      std::string_view("\x08\x00" "\x00" "\x00" "\x00\x00\x00\x00", 8));
  }
}
