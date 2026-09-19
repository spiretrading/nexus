#include <atomic>
#include <future>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "AsxTradeItchMarketDataFeedClient/AsxTradeItchClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace std::literals;

namespace {
  constexpr auto ONE = "T\x00\x00\x00\x01"sv;
  constexpr auto TWO = "T\x00\x00\x00\x02"sv;
  constexpr auto THREE = "T\x00\x00\x00\x03"sv;
  constexpr auto FOUR = "T\x00\x00\x00\x04"sv;
  constexpr auto FIVE = "T\x00\x00\x00\x05"sv;

  struct StubProtocolClient {
    Queue<SharedBuffer> m_packets;
    SharedBuffer m_payload;

    MoldUdp64Packet read() {
      m_payload = m_packets.pop();
      return MoldUdp64Packet::parse(
        std::string_view(m_payload.get_data(), m_payload.get_size()));
    }

    void close() {
      m_packets.close();
    }
  };

  struct StubRecoveryClient : StubProtocolClient {
    Queue<MoldUdp64Request> m_requests;
    Queue<bool> m_gate;
    std::atomic_bool m_is_blocked;

    void request(const MoldUdp64Request& request) {
      m_requests.push(request);
      if(m_is_blocked) {
        m_gate.pop();
      }
    }

    void close() {
      StubProtocolClient::close();
      m_requests.close();
      m_gate.close();
    }
  };

  struct StubGlimpseClient {
    Queue<bool> m_loads;
    Queue<AsxTradeItchSnapshot> m_snapshots;

    AsxTradeItchSnapshot load_snapshot() {
      m_loads.push(true);
      return m_snapshots.pop();
    }

    void close() {
      m_loads.close();
      m_snapshots.close();
    }
  };

  using Client = AsxTradeItchClient<StubProtocolClient*, StubRecoveryClient*,
    StubGlimpseClient*, FixedTimeClient*, TriggerTimer*>;

  struct Fixture {
    struct WithSnapshot {};
    struct WithoutSnapshot {};

    inline static const auto FEED_TIMEOUT = seconds(3);
    inline static const auto REQUEST_TIMEOUT = seconds(1);
    std::vector<std::unique_ptr<StubProtocolClient>> m_feeds;
    StubRecoveryClient m_recovery;
    optional<StubGlimpseClient> m_glimpse;
    FixedTimeClient m_time_client;
    TriggerTimer m_timer;
    optional<Client> m_client;

    Fixture()
      : Fixture(1) {}

    explicit Fixture(int feeds)
      : Fixture(feeds, WithoutSnapshot()) {}

    template<typename S> requires
      std::same_as<S, WithSnapshot> || std::same_as<S, WithoutSnapshot>
    Fixture(int feeds, S)
        : m_time_client(time_from_string("2026-09-19 10:00:00")) {
      auto clients = std::vector<StubProtocolClient*>();
      for(auto i = 0; i != feeds; ++i) {
        m_feeds.push_back(std::make_unique<StubProtocolClient>());
        clients.push_back(m_feeds.back().get());
      }
      auto glimpse = optional<StubGlimpseClient*>();
      if constexpr(std::same_as<S, WithSnapshot>) {
        m_glimpse.emplace();
        glimpse = &*m_glimpse;
      }
      m_client.emplace(FEED_TIMEOUT, REQUEST_TIMEOUT, clients, &m_recovery,
        glimpse, &m_time_client, &m_timer);
      flush_pending_routines();
    }

    SharedBuffer encode_packet(
        std::uint64_t sequence, const std::vector<std::string_view>& messages) {
      auto source = SharedBuffer();
      encode(MoldUdp64Request("SESSION123", sequence,
        static_cast<std::uint16_t>(messages.size())), out(source));
      for(auto message : messages) {
        append(source,
          endian::native_to_big(static_cast<std::uint16_t>(message.size())));
        append(source, message);
      }
      return source;
    }

    void publish(int feed, std::uint64_t sequence,
        const std::vector<std::string_view>& messages) {
      m_feeds[feed]->m_packets.push(encode_packet(sequence, messages));
      flush_pending_routines();
    }

    void recover(
        std::uint64_t sequence, const std::vector<std::string_view>& messages) {
      m_recovery.m_packets.push(encode_packet(sequence, messages));
      flush_pending_routines();
    }

    void require_message(std::uint32_t seconds) {
      REQUIRE(
        AsxTradeItchSeconds::parse(m_client->read()).m_seconds == seconds);
    }

    void require_request(std::uint64_t sequence, std::uint16_t count) {
      auto request = m_recovery.m_requests.pop();
      REQUIRE(request.m_session == "SESSION123");
      REQUIRE(request.m_sequence_number == sequence);
      REQUIRE(request.m_count == count);
    }

    void advance(time_duration duration) {
      m_time_client.set(m_time_client.get_time() + duration);
      m_timer.trigger();
      flush_pending_routines();
    }
  };
}

TEST_SUITE("AsxTradeItchClient") {
  TEST_CASE("local_channels") {
    auto server = LocalServerConnection();
    auto connect = [&] {
      auto connection = std::async(std::launch::async, [&] {
        return server.accept();
      });
      auto client = std::make_unique<LocalClientChannel>("itch", server);
      return std::pair(std::move(client), connection.get());
    };
    auto [feed_channel, feed_server] = connect();
    auto [recovery_channel, recovery_server] = connect();
    auto [glimpse_channel, glimpse_server] = connect();
    auto send = [&] (char type, std::string_view payload) {
      auto buffer = SharedBuffer();
      append(buffer, endian::native_to_big(
        static_cast<std::uint16_t>(sizeof(type) + payload.size())));
      append(buffer, type);
      append(buffer, payload);
      glimpse_server->get_writer().write(buffer);
    };
    auto feed = MoldUdp64Client(feed_channel.get());
    auto recovery = MoldUdp64Client(recovery_channel.get());
    auto glimpse_timer = TriggerTimer();
    send('A', "SESSION123                   1");
    auto glimpse = AsxTradeItchGlimpseClient(
      "user", "pass", glimpse_channel.get(), &glimpse_timer);
    auto login = SharedBuffer();
    glimpse_server->get_reader().read(out(login));
    auto time_client = FixedTimeClient(
      time_from_string("2026-09-19 10:00:00"));
    auto timer = TriggerTimer();
    auto client = AsxTradeItchClient(Fixture::FEED_TIMEOUT,
      Fixture::REQUEST_TIMEOUT, std::vector{&feed}, &recovery,
      optional(&glimpse), &time_client, &timer);
    auto packet = SharedBuffer();
    encode(MoldUdp64Request("SESSION123", 10, 1), out(packet));
    append(packet,
      endian::native_to_big(static_cast<std::uint16_t>(TWO.size())));
    append(packet, TWO);
    feed_server->get_writer().write(packet);
    send('S', ONE);
    send('S', "G                  10");
    REQUIRE(AsxTradeItchSeconds::parse(client.read()).m_seconds == 1);
    REQUIRE(AsxTradeItchSeconds::parse(client.read()).m_seconds == 2);
    client.close();
    REQUIRE_THROWS_AS(client.read(), EndOfFileException);
  }

  TEST_CASE("request_failure") {
    auto fixture = Fixture();
    fixture.publish(0, 1, {ONE});
    fixture.require_message(1);
    fixture.m_recovery.m_is_blocked = true;
    fixture.publish(0, 3, {THREE});
    fixture.require_request(2, 1);
    fixture.m_recovery.m_gate.close(
      std::make_exception_ptr(IOException("Request failed.")));
    flush_pending_routines();
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
  }

  TEST_CASE("close") {
    auto fixture = Fixture(1, Fixture::WithSnapshot());
    SUBCASE("waiting_for_feed") {}
    SUBCASE("loading_snapshot") {
      fixture.publish(0, 1, {ONE});
      REQUIRE(fixture.m_glimpse->m_loads.pop());
    }
    auto read = std::packaged_task([&] {
      return fixture.m_client->read();
    });
    auto result = read.get_future();
    auto routine = RoutineHandler(spawn(std::move(read)));
    flush_pending_routines();
    fixture.m_client->close();
    REQUIRE_THROWS_AS(result.get(), EndOfFileException);
  }

  TEST_CASE("failure") {
    auto fixture = Fixture(1, Fixture::WithSnapshot());
    fixture.publish(0, 1, {ONE});
    REQUIRE(fixture.m_glimpse->m_loads.pop());
    auto error = std::make_exception_ptr(IOException("Connection failed."));
    SUBCASE("feed") {
      fixture.m_feeds[0]->m_packets.close(error);
    }
    SUBCASE("recovery") {
      fixture.m_recovery.m_packets.close(error);
    }
    SUBCASE("snapshot") {
      fixture.m_glimpse->m_snapshots.close(error);
    }
    SUBCASE("timer") {
      fixture.m_timer.fail();
    }
    flush_pending_routines();
    REQUIRE_THROWS_AS(fixture.m_client->read(), IOException);
  }

  TEST_CASE("feed_during_request") {
    auto fixture = Fixture();
    fixture.publish(0, 1, {ONE});
    fixture.require_message(1);
    fixture.m_recovery.m_is_blocked = true;
    fixture.publish(0, 3, {THREE});
    fixture.require_request(2, 1);
    fixture.publish(0, 2, {TWO});
    fixture.require_message(2);
    fixture.require_message(3);
    fixture.publish(0, 4, {FOUR});
    fixture.require_message(4);
    fixture.advance(Fixture::REQUEST_TIMEOUT);
    REQUIRE(!fixture.m_recovery.m_requests.try_pop());
    fixture.m_client->close();
    REQUIRE_THROWS_AS(fixture.m_client->read(), EndOfFileException);
  }

  TEST_CASE("end_of_session") {
    auto fixture = Fixture();
    fixture.publish(0, 1, {ONE});
    fixture.require_message(1);
    auto source = SharedBuffer();
    encode(MoldUdp64Request("SESSION123", 3, MoldUdp64Packet::END_OF_SESSION),
      out(source));
    fixture.m_feeds[0]->m_packets.push(source);
    flush_pending_routines();
    fixture.require_request(2, 1);
    fixture.recover(2, {TWO});
    fixture.require_message(2);
    REQUIRE_THROWS_AS(fixture.m_client->read(), EndOfFileException);
  }

  TEST_CASE("malformed_packet") {
    auto fixture = Fixture();
    fixture.publish(0, 1, {ONE});
    fixture.require_message(1);
    fixture.publish(0, 4, {FOUR});
    fixture.require_request(2, 2);
    SUBCASE("feed_message") {
      fixture.publish(0, 2, {FIVE, "A"});
    }
    SUBCASE("recovery_message") {
      fixture.recover(2, {FIVE, "A"});
    }
    SUBCASE("feed_framing") {
      fixture.m_feeds[0]->m_packets.push(SharedBuffer("bad", 3));
    }
    SUBCASE("recovery_framing") {
      fixture.m_recovery.m_packets.push(SharedBuffer("bad", 3));
    }
    flush_pending_routines();
    fixture.recover(2, {TWO, THREE});
    fixture.require_message(2);
    fixture.require_message(3);
    fixture.require_message(4);
  }

  TEST_CASE("silent_feed") {
    auto fixture = Fixture(2);
    fixture.publish(0, 1, {ONE});
    fixture.publish(1, 1, {ONE});
    fixture.require_message(1);
    SUBCASE("silent") {
      fixture.publish(0, 3, {THREE});
    }
    SUBCASE("disconnected") {
      fixture.m_feeds[1]->close();
      flush_pending_routines();
      fixture.publish(0, 3, {THREE});
    }
    REQUIRE(!fixture.m_recovery.m_requests.try_pop());
    fixture.advance(Fixture::FEED_TIMEOUT + time_duration::unit());
    fixture.require_request(2, 1);
    fixture.recover(2, {TWO});
    fixture.require_message(2);
    fixture.require_message(3);
  }

  TEST_CASE("snapshot_handoff") {
    auto fixture = Fixture(1, Fixture::WithSnapshot());
    REQUIRE(!fixture.m_glimpse->m_loads.try_pop());
    fixture.publish(0, 2, {TWO, THREE});
    REQUIRE(fixture.m_glimpse->m_loads.pop());
    fixture.publish(0, 5, {FIVE});
    REQUIRE(!fixture.m_recovery.m_requests.try_pop());
    auto snapshot = AsxTradeItchSnapshot();
    snapshot.m_messages.emplace_back(ONE.data(), ONE.size());
    snapshot.m_messages.emplace_back(TWO.data(), TWO.size());
    SUBCASE("overlap") {
      snapshot.m_sequence = 3;
      fixture.m_glimpse->m_snapshots.push(snapshot);
      flush_pending_routines();
      fixture.require_request(4, 1);
      fixture.recover(4, {FOUR});
    }
    SUBCASE("ahead") {
      snapshot.m_sequence = 5;
      snapshot.m_messages.emplace_back(THREE.data(), THREE.size());
      snapshot.m_messages.emplace_back(FOUR.data(), FOUR.size());
      fixture.m_glimpse->m_snapshots.push(snapshot);
      flush_pending_routines();
    }
    SUBCASE("older") {
      snapshot.m_sequence = 1;
      snapshot.m_messages.clear();
      fixture.m_glimpse->m_snapshots.push(snapshot);
      flush_pending_routines();
      fixture.require_request(1, 1);
      fixture.recover(1, {ONE});
      fixture.require_request(4, 1);
      fixture.recover(4, {FOUR});
    }
    for(auto seconds = 1U; seconds != 6; ++seconds) {
      fixture.require_message(seconds);
    }
    REQUIRE(!fixture.m_recovery.m_requests.try_pop());
    REQUIRE(!fixture.m_glimpse->m_loads.try_pop());
  }

  TEST_CASE("gap_recovery") {
    auto fixture = Fixture(2);
    fixture.publish(0, 1, {ONE});
    fixture.publish(1, 1, {ONE});
    fixture.require_message(1);
    fixture.publish(0, 4, {FOUR});
    REQUIRE(!fixture.m_recovery.m_requests.try_pop());
    fixture.publish(1, 4, {FOUR});
    fixture.require_request(2, 2);
    SUBCASE("complete") {
      fixture.recover(2, {TWO, THREE});
      fixture.require_message(2);
    }
    SUBCASE("partial") {
      fixture.recover(2, {TWO});
      fixture.require_message(2);
      fixture.require_request(3, 1);
      fixture.recover(3, {THREE});
    }
    SUBCASE("retry") {
      fixture.publish(0, 5, {FIVE});
      REQUIRE(!fixture.m_recovery.m_requests.try_pop());
      for(auto i = 0; i != 2; ++i) {
        fixture.advance(Fixture::REQUEST_TIMEOUT);
        auto request = fixture.m_recovery.m_requests.try_pop();
        REQUIRE(request.has_value());
        REQUIRE(request->m_session == "SESSION123");
        REQUIRE(request->m_sequence_number == 2);
        REQUIRE(request->m_count == 2);
        REQUIRE(!fixture.m_recovery.m_requests.try_pop());
      }
      fixture.recover(2, {TWO, THREE});
      fixture.require_message(2);
    }
    fixture.require_message(3);
    fixture.require_message(4);
    REQUIRE(!fixture.m_recovery.m_requests.try_pop());
  }

  TEST_CASE("feed_arbitration") {
    auto fixture = Fixture(2);
    fixture.publish(0, 1, {ONE, TWO});
    fixture.publish(1, 1, {ONE, TWO, THREE});
    fixture.publish(0, 3, {THREE, FOUR});
    fixture.require_message(1);
    fixture.require_message(2);
    fixture.require_message(3);
    fixture.require_message(4);
    REQUIRE(!fixture.m_recovery.m_requests.try_pop());
  }
}
