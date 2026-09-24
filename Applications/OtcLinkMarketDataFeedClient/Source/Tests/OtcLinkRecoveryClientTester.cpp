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
  struct CountingReader {
    PipedReader* m_reader;
    std::size_t* m_count;

    bool poll() const {
      return m_reader->poll();
    }

    template<IsBuffer B>
    std::size_t read(Out<B> destination, std::size_t size) {
      ++*m_count;
      return m_reader->read(destination, size);
    }
  };

  struct CountingChannel : LocalClientChannel {
    using Reader = CountingReader;
    Reader m_reader;

    CountingChannel(LocalServerConnection& server, std::size_t& reads)
      : LocalClientChannel("recovery", server),
        m_reader(&LocalClientChannel::get_reader(), &reads) {}

    Reader& get_reader() {
      return m_reader;
    }
  };

  struct BlockingWriter {
    PipedReader* m_reader;
    Async<void>* m_entered;

    void write(BufferCRef) {
      m_entered->get_eval().set();
      auto buffer = SharedBuffer();
      m_reader->read(out(buffer));
    }
  };

  struct BlockingChannel : LocalClientChannel {
    using Writer = BlockingWriter;
    Writer m_writer;

    BlockingChannel(LocalServerConnection& server, Async<void>& entered)
      : LocalClientChannel("recovery", server),
        m_writer(&LocalClientChannel::get_reader(), &entered) {}

    Writer& get_writer() {
      return m_writer;
    }
  };

  struct Fixture {
    LocalServerConnection m_server;
    TriggerTimer m_timer;
    std::size_t m_reads;
    OtcLinkRecoveryClient<CountingChannel, TriggerTimer*> m_client;
    RoutineHandler m_routine;

    Fixture()
      : m_reads(0),
        m_client("SPIRE", 11, [&] (std::stop_token) {
          return std::make_shared<CountingChannel>(m_server, m_reads);
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
  TEST_CASE("invalid_constructor") {
    auto sender = std::string("SPIRE");
    auto channel = std::uint16_t(11);
    SUBCASE("empty_sender") {
      sender.clear();
    }
    SUBCASE("sender_delimiter") {
      sender += '\x01';
    }
    SUBCASE("zero_channel") {
      channel = 0;
    }
    auto timer = TriggerTimer();
    auto connections = 0;
    auto construct = [&] {
      auto client = OtcLinkRecoveryClient(sender, channel,
        [&] (std::stop_token) -> std::shared_ptr<LocalClientChannel> {
          ++connections;
          throw IOException("Unexpected connection.");
        }, &timer);
    };
    REQUIRE_THROWS_AS(construct(), OtcLinkParserException);
    REQUIRE(connections == 0);
  }

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
    auto action = 0;
    SUBCASE("timeout") {}
    SUBCASE("stop") {
      action = 1;
    }
    SUBCASE("close") {
      action = 2;
    }
    for(auto completes : {false, true}) {
      auto server = LocalServerConnection();
      auto accepted = std::async(std::launch::async, [&] {
        return server.accept();
      });
      auto channel = std::make_shared<LocalClientChannel>("recovery", server);
      auto peer = accepted.get();
      auto timer = TriggerTimer();
      auto entered = Async<void>();
      auto client = OtcLinkRecoveryClient("SPIRE", 11,
        [&] (std::stop_token token) -> std::shared_ptr<LocalClientChannel> {
          auto stopped = Async<void>();
          auto callback = std::stop_callback(token, [&] {
            stopped.get_eval().set();
          });
          entered.get_eval().set();
          stopped.get();
          if(completes) {
            return channel;
          }
          throw IOException("Canceled connection.");
        }, &timer);
      auto stop = std::stop_source();
      auto task = std::packaged_task([&] {
        return client.request(1, 1, stop.get_token());
      });
      auto future = task.get_future();
      auto routine = RoutineHandler(spawn(std::move(task)));
      entered.get();
      if(action == 0) {
        timer.trigger();
      } else if(action == 1) {
        stop.request_stop();
      } else {
        client.close();
      }
      REQUIRE_THROWS_AS(future.get(), IOException);
      routine.wait();
      if(completes) {
        auto buffer = SharedBuffer();
        REQUIRE_THROWS_AS(peer->get_reader().read(out(buffer)),
          EndOfFileException);
        REQUIRE(buffer.get_size() == 0);
      }
    }
  }

  TEST_CASE("write_cancellation") {
    auto server = LocalServerConnection();
    auto timer = TriggerTimer();
    auto entered = Async<void>();
    auto client = OtcLinkRecoveryClient("SPIRE", 11,
      [&] (std::stop_token) {
        return std::make_shared<BlockingChannel>(server, entered);
      }, &timer);
    auto stop = std::stop_source();
    auto task = std::packaged_task([&] {
      return client.request(1, 1, stop.get_token());
    });
    auto future = task.get_future();
    auto routine = RoutineHandler(spawn(std::move(task)));
    auto peer = server.accept();
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
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(peer->get_reader().read(out(buffer)),
      EndOfFileException);
    REQUIRE(buffer.get_size() == 0);
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
        auto buffer = SharedBuffer(response.data(), response.size());
        append(buffer, fixture.message(100));
        append(buffer, fixture.message(101).slice(0, 4));
        server->get_writer().write(buffer);
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

  TEST_CASE("response_length") {
    auto fixture = Fixture();
    auto future = fixture.request(100, 1, {});
    auto server = fixture.accept(
      OtcLinkRecoveryRequest("SPIRE", 1, 11, 100, 1));
    auto body = std::string("35=BX\x01" "59=SPIRE\x01" "1346=1\x01"
      "1355=11\x01" "1348=0\x01" "1182=100\x01" "1183=100\x01"
      "58=");
    auto length = std::size_t(65536);
    auto is_oversized = false;
    SUBCASE("maximum") {}
    SUBCASE("oversized") {
      ++length;
      is_oversized = true;
    }
    body.append(length - body.size() - 8, 'x');
    body += '\x01';
    auto response = fixture.response(body);
    REQUIRE(response.size() == length);
    auto buffer = SharedBuffer(response.data(), response.size());
    append(buffer, fixture.message(100));
    server->get_writer().write(buffer);
    if(is_oversized) {
      REQUIRE_THROWS_AS(future.get(), OtcLinkParserException);
      REQUIRE(fixture.m_reads == 1);
    } else {
      auto messages = future.get();
      REQUIRE(messages.size() == 1);
      REQUIRE(messages.front() == fixture.message(100));
      REQUIRE(fixture.m_reads == 2);
    }
  }

  TEST_CASE("maximum_message_length") {
    auto fixture = Fixture();
    auto future = fixture.request(100, 2, {});
    auto server = fixture.accept(
      OtcLinkRecoveryRequest("SPIRE", 1, 11, 100, 2));
    auto response = fixture.response("35=BX\x01" "59=SPIRE\x01"
      "1346=1\x01" "1355=11\x01" "1348=0\x01" "1182=100\x01"
      "1183=101\x01");
    auto message = fixture.message(100);
    auto length = std::uint16_t(65535);
    auto padding = std::string(length - message.get_size(), 'x');
    append(message, padding.data(), padding.size());
    auto encoded = endian::native_to_big(length);
    message.write(0, &encoded, sizeof(encoded));
    auto buffer = SharedBuffer(response.data(), response.size());
    append(buffer, message);
    append(buffer, fixture.message(101));
    server->get_writer().write(buffer);
    auto messages = future.get();
    REQUIRE(messages.size() == 2);
    REQUIRE(messages[0] == message);
    REQUIRE(messages[1] == fixture.message(101));
    REQUIRE(fixture.m_reads == 3);
  }

  TEST_CASE("request") {
    auto fixture = Fixture();
    auto fragment = std::size_t(0);
    SUBCASE("fragmented") {
      fragment = 1;
    }
    SUBCASE("coalesced") {}
    SUBCASE("partial_header_surplus") {
      fragment = 2;
    }
    SUBCASE("partial_body_surplus") {
      fragment = 5;
    }
    auto reads = std::size_t(0);
    for(auto id = std::uint64_t(1); id <= 2; ++id) {
      auto future = fixture.request(100, 2, {});
      auto server = fixture.accept(OtcLinkRecoveryRequest(
        "SPIRE", id, 11, 100, 2));
      auto response = fixture.response(std::format("35=BX\x01"
        "59=SPIRE\x01" "1346={}\x01" "1355=11\x01" "1348=0\x01"
        "1182=100\x01" "1183=101\x01", id));
      auto buffer = SharedBuffer(response.data(), response.size());
      for(auto sequence : {100U, 101U}) {
        append(buffer, fixture.message(sequence));
      }
      if(fragment == 1) {
        for(auto i = std::size_t(0); i < buffer.get_size(); ++i) {
          server->get_writer().write(buffer.slice(i, 1));
        }
        reads += buffer.get_size();
      } else if(fragment != 0) {
        auto split = response.size() + fragment - 1;
        server->get_writer().write(buffer.slice(0, split));
        server->get_writer().write(
          buffer.slice(split, buffer.get_size() - split));
        reads += 2;
      } else {
        server->get_writer().write(buffer);
        ++reads;
      }
      auto messages = future.get();
      REQUIRE(messages.size() == 2);
      REQUIRE(messages[0] == fixture.message(100));
      REQUIRE(messages[1] == fixture.message(101));
      REQUIRE(fixture.m_reads == reads);
      reset(buffer);
      REQUIRE_THROWS_AS(server->get_reader().read(out(buffer)),
        EndOfFileException);
    }
  }
}
