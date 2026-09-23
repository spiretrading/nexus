#include <future>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <boost/endian/conversion.hpp>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkRecoveryClient.hpp"

using namespace Beam;
using namespace boost;
using namespace Nexus;

namespace {
  struct Fixture {
    LocalServerConnection m_server;
    TriggerTimer m_timer;
    OtcLinkRecoveryClient<LocalClientChannel, TriggerTimer*> m_client;
    RoutineHandler m_routine;

    Fixture()
      : m_client("SPIRE", 11, [&] (std::stop_token) {
          return std::make_shared<LocalClientChannel>("recovery", m_server);
        }, &m_timer) {}

    ~Fixture() {
      m_client.close();
      m_routine.wait();
    }

    std::future<std::vector<SharedBuffer>> request(
        std::uint32_t sequence, std::uint32_t count, std::stop_token token) {
      auto task = std::packaged_task([=, this] {
        return m_client.request(sequence, count, token);
      });
      auto future = task.get_future();
      m_routine = spawn(std::move(task));
      return future;
    }

    std::unique_ptr<LocalServerChannel> accept(
        const OtcLinkRecoveryRequest& request) {
      auto channel = m_server.accept();
      auto expected = request.encode();
      auto buffer = SharedBuffer();
      read_exact(channel->get_reader(), out(buffer), expected.size());
      REQUIRE(std::string_view(buffer.get_data(), buffer.get_size()) ==
        expected);
      return channel;
    }

    std::string response(std::string fields) {
      auto sum = 0U;
      for(auto byte : fields) {
        sum += static_cast<unsigned char>(byte);
      }
      return fields + std::format("10={:03}\x01", sum % 256);
    }

    SharedBuffer message(std::uint32_t sequence) {
      auto buffer = SharedBuffer();
      append(buffer, endian::native_to_big(static_cast<std::uint16_t>(
        OtcLinkMessage::HEADER_LENGTH + sizeof(sequence))));
      append(buffer, std::uint8_t(0xFF));
      append(buffer, endian::native_to_big(sequence));
      return buffer;
    }
  };
}

TEST_SUITE("OtcLinkRecoveryClient") {
  TEST_CASE("snapshot_request") {
    auto fixture = Fixture();
    auto task = std::packaged_task([&] {
      fixture.m_client.request_snapshot({});
    });
    auto future = task.get_future();
    fixture.m_routine = spawn(std::move(task));
    auto server = fixture.accept(OtcLinkRecoveryRequest("SPIRE", 1, 11,
      0, 0, OtcLinkRecoveryRequest::Type::SNAPSHOT));
    auto response = fixture.response("35=BX\x01" "59=SPIRE\x01"
      "1346=1\x01" "1355=11\x01" "1348=0\x01");
    server->get_writer().write(SharedBuffer(response.data(), response.size()));
    REQUIRE_NOTHROW(future.get());
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(server->get_reader().read(out(buffer)),
      EndOfFileException);
  }

  TEST_CASE("request_after_timeout") {
    auto fixture = Fixture();
    auto first = fixture.request(100, 1, {});
    auto server = fixture.accept(
      OtcLinkRecoveryRequest("SPIRE", 1, 11, 100, 1));
    fixture.m_timer.trigger();
    REQUIRE_THROWS_AS(first.get(), IOException);
    auto second = fixture.request(200, 1, {});
    server = fixture.accept(
      OtcLinkRecoveryRequest("SPIRE", 2, 11, 200, 1));
    auto response = fixture.response("35=BX\x01" "59=SPIRE\x01"
      "1346=2\x01" "1355=11\x01" "1348=0\x01" "1182=200\x01"
      "1183=200\x01");
    auto buffer = SharedBuffer(response.data(), response.size());
    append(buffer, fixture.message(200));
    server->get_writer().write(buffer);
    auto messages = second.get();
    REQUIRE(messages.size() == 1);
    REQUIRE(messages.front() == fixture.message(200));
  }

  TEST_CASE("unsent_request") {
    auto timer = TriggerTimer();
    auto connections = 0;
    auto client = OtcLinkRecoveryClient("SPIRE", 11,
      [&] (std::stop_token) -> std::shared_ptr<LocalClientChannel> {
        ++connections;
        throw IOException("Unexpected connection.");
      }, &timer);
    auto stop = std::stop_source();
    stop.request_stop();
    REQUIRE_THROWS_AS(client.request(1, 1, stop.get_token()),
      EndOfFileException);
    REQUIRE_THROWS_AS(client.request(1, 0, {}), OtcLinkParserException);
    REQUIRE_THROWS_AS(client.request(1,
      OtcLinkRecoveryRequest::MAXIMUM_COUNT + 1, {}), OtcLinkParserException);
    client.close();
    REQUIRE_THROWS_AS(client.request(1, 1, {}), EndOfFileException);
    REQUIRE(connections == 0);
  }

  TEST_CASE("connection_cancellation") {
    auto timer = TriggerTimer();
    auto entered = Async<void>();
    auto client = OtcLinkRecoveryClient<LocalClientChannel, TriggerTimer*>(
      "SPIRE", 11, [&] (std::stop_token token) ->
          std::shared_ptr<LocalClientChannel> {
        auto stopped = Async<void>();
        auto callback = std::stop_callback(token, [&] {
          stopped.get_eval().set();
        });
        entered.get_eval().set();
        stopped.get();
        throw IOException("Canceled connection.");
      }, &timer);
    auto stop = std::stop_source();
    auto task = std::packaged_task([&] {
      return client.request(1, 1, stop.get_token());
    });
    auto future = task.get_future();
    auto routine = RoutineHandler(spawn(std::move(task)));
    entered.get();
    SUBCASE("timeout") {
      timer.trigger();
    }
    SUBCASE("stop") {
      stop.request_stop();
    }
    SUBCASE("close") {
      client.close();
    }
    REQUIRE_THROWS_AS(future.get(), IOException);
    routine.wait();
  }

  TEST_CASE("cancellation") {
    auto action = 0;
    SUBCASE("timeout") {}
    SUBCASE("timer_failure") {
      action = 1;
    }
    SUBCASE("stop") {
      action = 2;
    }
    SUBCASE("close") {
      action = 3;
    }
    for(auto receive_messages : {false, true}) {
      auto fixture = Fixture();
      auto stop = std::stop_source();
      auto future = fixture.request(100, 2, stop.get_token());
      auto server = fixture.accept(
        OtcLinkRecoveryRequest("SPIRE", 1, 11, 100, 2));
      if(receive_messages) {
        auto response = fixture.response("35=BX\x01" "59=SPIRE\x01"
          "1346=1\x01" "1355=11\x01" "1348=0\x01" "1182=100\x01"
          "1183=101\x01");
        server->get_writer().write(
          SharedBuffer(response.data(), response.size()));
        server->get_writer().write(fixture.message(100));
      }
      flush_pending_routines();
      if(action == 0) {
        fixture.m_timer.trigger();
      } else if(action == 1) {
        fixture.m_timer.fail();
      } else if(action == 2) {
        stop.request_stop();
      } else {
        fixture.m_client.close();
      }
      REQUIRE_THROWS_AS(future.get(), IOException);
      auto buffer = SharedBuffer();
      REQUIRE_THROWS_AS(server->get_reader().read(out(buffer)),
        EndOfFileException);
    }
  }

  TEST_CASE("failed_request") {
    auto fixture = Fixture();
    auto future = fixture.request(100, 2, {});
    auto server = fixture.accept(
      OtcLinkRecoveryRequest("SPIRE", 1, 11, 100, 2));
    auto body = std::string("35=BX\x01" "59=SPIRE\x01" "1346=1\x01"
      "1355=11\x01" "1348=0\x01" "1182=100\x01" "1183=101\x01");
    auto messages = SharedBuffer();
    auto is_io_failure = false;
    auto is_truncated = false;
    SUBCASE("rejection") {
      body = "35=BX\x01" "59=SPIRE\x01" "1346=1\x01" "1355=11\x01"
        "1348=2\x01" "58=Unavailable\x01";
      is_io_failure = true;
    }
    SUBCASE("request_id") {
      body.replace(body.find("1346=1"), 6, "1346=2");
    }
    SUBCASE("channel") {
      body.replace(body.find("1355=11"), 7, "1355=14");
    }
    SUBCASE("range") {
      body.replace(body.find("1183=101"), 8, "1183=102");
    }
    SUBCASE("recipient") {
      body.replace(body.find("SPIRE"), 5, "OTHER");
    }
    SUBCASE("truncated_message") {
      messages = fixture.message(100).slice(0, 4);
      is_truncated = true;
      is_io_failure = true;
    }
    SUBCASE("wrong_sequence") {
      messages = fixture.message(99);
    }
    SUBCASE("short_message") {
      auto size = endian::native_to_big(std::uint16_t(1));
      messages = SharedBuffer(&size, sizeof(size));
    }
    auto response = fixture.response(body);
    auto buffer = SharedBuffer(response.data(), response.size());
    append(buffer, messages);
    server->get_writer().write(buffer);
    if(is_truncated) {
      server->get_connection().close();
    }
    if(is_io_failure) {
      REQUIRE_THROWS_AS(future.get(), IOException);
    } else {
      REQUIRE_THROWS_AS(future.get(), OtcLinkParserException);
    }
  }

  TEST_CASE("request") {
    auto fixture = Fixture();
    auto fragmented = false;
    SUBCASE("fragmented") {
      fragmented = true;
    }
    SUBCASE("coalesced") {}
    for(auto id = std::uint64_t(1); id <= 2; ++id) {
      auto future = fixture.request(100, 2, {});
      auto server = fixture.accept(OtcLinkRecoveryRequest(
        "SPIRE", id, 11, 100, 2));
      auto response = fixture.response(std::format("35=BX\x01"
        "59=SPIRE\x01" "1346={}\x01" "1355=11\x01" "1348=0\x01"
        "1182=100\x01" "1183=101\x01", id));
      if(fragmented) {
        for(auto byte : response) {
          server->get_writer().write(SharedBuffer(&byte, sizeof(byte)));
        }
        for(auto sequence : {100U, 101U}) {
          auto message = fixture.message(sequence);
          for(auto i = std::size_t(0); i < message.get_size(); ++i) {
            server->get_writer().write(message.slice(i, 1));
          }
        }
      } else {
        auto buffer = SharedBuffer(response.data(), response.size());
        for(auto sequence : {100U, 101U}) {
          append(buffer, fixture.message(sequence));
        }
        server->get_writer().write(buffer);
      }
      auto messages = future.get();
      REQUIRE(messages.size() == 2);
      REQUIRE(messages[0] == fixture.message(100));
      REQUIRE(messages[1] == fixture.message(101));
      auto buffer = SharedBuffer();
      REQUIRE_THROWS_AS(server->get_reader().read(out(buffer)),
        EndOfFileException);
    }
  }
}
