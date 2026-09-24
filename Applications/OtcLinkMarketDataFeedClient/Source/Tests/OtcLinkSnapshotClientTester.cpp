#include <future>
#include <Beam/IO/LocalServerConnection.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkSnapshotClient.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using ProtocolClient =
    OtcLinkProtocolClient<LocalClientChannel*, FixedTimeClient*>;
  using Client = OtcLinkSnapshotClient<ProtocolClient*, TriggerTimer*>;

  struct Fixture {
    LocalServerConnection m_server;
    FixedTimeClient m_time_client;
    TriggerTimer m_timer;
    optional<LocalClientChannel> m_channel;
    std::unique_ptr<LocalServerChannel> m_server_channel;
    optional<ProtocolClient> m_protocol_client;
    optional<Client> m_client;
    Async<void> m_acknowledgement;
    RoutineHandler m_routine;

    Fixture() {
      auto server = std::async(std::launch::async, [&] {
        return m_server.accept();
      });
      m_channel.emplace("snapshot", m_server);
      m_server_channel = server.get();
      m_protocol_client.emplace(&*m_channel, &m_time_client);
      m_client.emplace(OtcLinkSpinType::MARKET_DATA,
        [&] (std::stop_token token) {
          auto stop = std::stop_callback(token, [&] {
            m_acknowledgement.get_eval().set_exception(EndOfFileException());
          });
          m_acknowledgement.get();
        }, &*m_protocol_client, &m_timer);
    }

    ~Fixture() {
      m_client->close();
      m_routine.wait();
    }

    std::future<OtcLinkSnapshot> load(std::stop_token token) {
      auto task = std::packaged_task([=, this] {
        return m_client->load_snapshot(token);
      });
      auto future = task.get_future();
      m_routine = spawn(std::move(task));
      flush_pending_routines();
      return future;
    }

    SharedBuffer message(std::uint8_t type, const SharedBuffer& payload) {
      auto buffer = SharedBuffer();
      append(buffer, endian::native_to_big(static_cast<std::uint16_t>(
        OtcLinkMessage::HEADER_LENGTH + payload.get_size())));
      append(buffer, type);
      append(buffer, payload);
      return buffer;
    }

    SharedBuffer data(std::uint32_t sequence) {
      auto payload = SharedBuffer();
      append(payload, endian::native_to_big(sequence));
      return message(0xFF, payload);
    }

    SharedBuffer marker(bool is_end, OtcLinkSpinType type,
        std::uint32_t last_sequence) {
      auto payload = SharedBuffer();
      append(payload, endian::native_to_big(std::uint32_t(1)));
      append(payload, static_cast<std::uint8_t>(type));
      auto message_type = OtcLinkSpinStart::TYPE;
      if(is_end) {
        message_type = OtcLinkSpinEnd::TYPE;
        append(payload, endian::native_to_big(std::uint32_t(1)));
      }
      append(payload, endian::native_to_big(std::uint64_t(1000)));
      append(payload, endian::native_to_big(last_sequence));
      return message(message_type, payload);
    }

    void publish(std::uint32_t sequence,
        std::initializer_list<SharedBuffer> messages) {
      auto payload = SharedBuffer();
      for(auto& message : messages) {
        append(payload, message);
      }
      auto buffer = SharedBuffer();
      append(buffer, endian::native_to_big(static_cast<std::uint16_t>(
        OtcLinkHeader::LENGTH + payload.get_size())));
      append(buffer, endian::native_to_big(sequence));
      auto flags = std::uint8_t(0);
      if(messages.size() == 0) {
        flags = static_cast<std::uint8_t>(OtcLinkHeader::Flag::HEARTBEAT);
      }
      append(buffer, flags);
      append(buffer, static_cast<std::uint8_t>(messages.size()));
      append(buffer, std::uint32_t(0));
      append(buffer, payload);
      m_server_channel->get_writer().write(buffer);
      flush_pending_routines();
    }
  };
}

TEST_SUITE("OtcLinkSnapshotClient") {
  TEST_CASE("timeout_diagnostics") {
    auto fixture = Fixture();
    auto future = fixture.load({});
    auto is_acknowledged = true;
    auto is_timer_failure = false;
    auto progress = std::string("packets=0 messages=0 heartbeats=0 "
      "last_packet=none expected_spin=market_data last_marker=none");
    SUBCASE("awaiting_acknowledgement") {
      is_acknowledged = false;
    }
    SUBCASE("no_packets") {}
    SUBCASE("heartbeats") {
      fixture.publish(8, {});
      progress = "packets=1 messages=0 heartbeats=1 last_packet=8 "
        "expected_spin=market_data last_marker=none";
    }
    SUBCASE("reference_complete") {
      fixture.publish(10, {
        fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
        fixture.data(91),
        fixture.marker(true, OtcLinkSpinType::REFERENCE, 100)});
      progress = "packets=1 messages=3 heartbeats=0 last_packet=10 "
        "expected_spin=market_data last_marker=end/reference/100";
    }
    SUBCASE("incomplete_spin") {
      fixture.publish(11, {
        fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 100),
        fixture.data(92)});
      progress = "packets=1 messages=2 heartbeats=0 last_packet=11 "
        "expected_spin=market_data last_marker=start/market_data/100";
    }
    SUBCASE("other_spin") {
      fixture.publish(12, {
        fixture.marker(false, OtcLinkSpinType::OPENING, 100),
        fixture.marker(true, OtcLinkSpinType::OPENING, 100)});
      progress = "packets=1 messages=2 heartbeats=0 last_packet=12 "
        "expected_spin=market_data last_marker=end/opening/100";
    }
    SUBCASE("completed_before_acknowledgement") {
      is_acknowledged = false;
      fixture.publish(13, {
        fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 100),
        fixture.data(93),
        fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 100)});
      progress = "packets=1 messages=3 heartbeats=0 last_packet=13 "
        "expected_spin=market_data last_marker=end/market_data/100";
    }
    SUBCASE("timer_failure") {
      is_timer_failure = true;
    }
    if(is_acknowledged) {
      fixture.m_acknowledgement.get_eval().set();
      flush_pending_routines();
    }
    auto expected = std::string();
    if(is_timer_failure) {
      fixture.m_timer.fail();
      expected = "OTC Link snapshot timer failed.";
    } else {
      fixture.m_timer.trigger();
      expected = "OTC Link snapshot deadline expired.";
    }
    if(is_acknowledged) {
      expected += " acknowledgement=received ";
    } else {
      expected += " acknowledgement=pending ";
    }
    expected += progress;
    auto actual = [&] {
      try {
        future.get();
      } catch(const IOException& exception) {
        return std::string(exception.what());
      }
      return std::string();
    }();
    REQUIRE(actual == expected);
  }

  TEST_CASE("acknowledged_snapshot") {
    auto is_empty = false;
    SUBCASE("empty") {
      is_empty = true;
    }
    SUBCASE("timeout") {}
    auto fixture = Fixture();
    auto future = fixture.load({});
    fixture.m_acknowledgement.get_eval().set();
    fixture.publish(1, {
      fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 0)});
    if(is_empty) {
      fixture.publish(2, {
        fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 0)});
      auto snapshot = future.get();
      REQUIRE(snapshot.m_sequence == 1);
      REQUIRE(snapshot.m_messages.empty());
    } else {
      fixture.m_timer.trigger();
      REQUIRE_THROWS_AS(future.get(), IOException);
    }
  }

  TEST_CASE("incomplete_snapshot") {
    auto action = 0;
    SUBCASE("packet_gap") {}
    SUBCASE("marker_mismatch") {
      action = 1;
    }
    SUBCASE("timeout") {
      action = 2;
    }
    SUBCASE("stop") {
      action = 3;
    }
    SUBCASE("close") {
      action = 4;
    }
    SUBCASE("request_failure") {
      action = 5;
    }
    auto fixture = Fixture();
    auto stop = std::stop_source();
    auto future = fixture.load(stop.get_token());
    fixture.publish(1, {
      fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 100),
      fixture.data(1)});
    if(action == 0) {
      fixture.publish(3, {
        fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 100)});
    } else if(action == 1) {
      fixture.publish(2, {
        fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 101)});
    } else if(action == 2) {
      fixture.m_timer.trigger();
    } else if(action == 3) {
      stop.request_stop();
    } else if(action == 4) {
      fixture.m_client->close();
    } else {
      fixture.m_acknowledgement.get_eval().set_exception(
        IOException("Snapshot unavailable."));
    }
    REQUIRE_THROWS_AS(future.get(), std::exception);
  }

  TEST_CASE("snapshot") {
    auto before_acknowledgement = false;
    SUBCASE("before_acknowledgement") {
      before_acknowledgement = true;
    }
    SUBCASE("after_acknowledgement") {}
    auto fixture = Fixture();
    auto future = fixture.load({});
    if(!before_acknowledgement) {
      fixture.m_acknowledgement.get_eval().set();
    }
    fixture.publish(10, {fixture.data(90)});
    fixture.publish(11, {
      fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
      fixture.data(91),
      fixture.marker(true, OtcLinkSpinType::REFERENCE, 100)});
    fixture.publish(12, {});
    fixture.publish(12, {
      fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 100),
      fixture.data(92)});
    fixture.publish(12, {fixture.data(92)});
    fixture.publish(13, {
      fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 100)});
    if(before_acknowledgement) {
      REQUIRE(future.wait_for(std::chrono::seconds(0)) ==
        std::future_status::timeout);
      fixture.m_acknowledgement.get_eval().set();
    }
    auto snapshot = future.get();
    REQUIRE(snapshot.m_sequence == 101);
    REQUIRE(snapshot.m_messages.size() == 2);
    REQUIRE(snapshot.m_messages[0] == fixture.data(91));
    REQUIRE(snapshot.m_messages[1] == fixture.data(92));
    REQUIRE_THROWS_AS(fixture.m_client->load_snapshot({}), IOException);
  }
}
