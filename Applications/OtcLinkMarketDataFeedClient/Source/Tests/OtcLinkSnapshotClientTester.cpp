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

    Fixture() : Fixture(OtcLinkSpinType::MARKET_DATA) {}

    explicit Fixture(OtcLinkSpinType type) {
      auto server = std::async(std::launch::async, [&] {
        return m_server.accept();
      });
      m_channel.emplace("snapshot", m_server);
      m_server_channel = server.get();
      m_protocol_client.emplace(&*m_channel, &m_time_client);
      m_client.emplace(type,
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

    void tick() {
      m_timer.trigger();
      flush_pending_routines();
    }

    void expire(std::future<OtcLinkSnapshot>& future) {
      for(auto i = 0; i < 3 && future.wait_for(std::chrono::seconds(0)) ==
          std::future_status::timeout; ++i) {
        tick();
      }
      REQUIRE(future.wait_for(std::chrono::seconds(0)) ==
        std::future_status::ready);
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
      return marker(is_end, type, last_sequence, 1000);
    }

    SharedBuffer marker(bool is_end, OtcLinkSpinType type,
        std::uint32_t last_sequence, std::uint64_t timestamp) {
      auto payload = SharedBuffer();
      append(payload, endian::native_to_big(std::uint32_t(1)));
      append(payload, static_cast<std::uint8_t>(type));
      auto message_type = OtcLinkSpinStart::TYPE;
      if(is_end) {
        message_type = OtcLinkSpinEnd::TYPE;
        append(payload, endian::native_to_big(std::uint32_t(1)));
      }
      append(payload, endian::native_to_big(timestamp));
      append(payload, endian::native_to_big(last_sequence));
      return message(message_type, payload);
    }

    void publish(std::uint32_t sequence,
        std::initializer_list<SharedBuffer> messages) {
      auto flags = std::uint8_t(0);
      if(messages.size() == 0) {
        flags = static_cast<std::uint8_t>(OtcLinkHeader::Flag::HEARTBEAT);
      }
      publish(sequence, messages, flags);
    }

    void publish(std::uint32_t sequence,
        std::initializer_list<SharedBuffer> messages, std::uint8_t flags) {
      auto payload = SharedBuffer();
      for(auto& message : messages) {
        append(payload, message);
      }
      auto buffer = SharedBuffer();
      append(buffer, endian::native_to_big(static_cast<std::uint16_t>(
        OtcLinkHeader::LENGTH + payload.get_size())));
      append(buffer, endian::native_to_big(sequence));
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
  TEST_CASE("snapshot_retry") {
    auto failure = std::string("gap");
    SUBCASE("gap") {}
    SUBCASE("timeout") {
      failure = "timeout";
    }
    SUBCASE("request") {
      failure = "request";
    }
    for(auto type : {OtcLinkSpinType::REFERENCE, OtcLinkSpinType::MARKET_DATA}) {
      auto attempts = 0;
      auto snapshot = load_snapshot(1, [&] (std::stop_token token) {
        ++attempts;
        auto fixture = Fixture(type);
        auto future = fixture.load(token);
        if(attempts == 1 && failure == "request") {
          fixture.m_acknowledgement.get_eval().set_exception(IOException());
        } else {
          fixture.m_acknowledgement.get_eval().set();
          if(attempts == 1) {
            fixture.publish(1, {
              fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
              fixture.data(91)});
            if(failure == "gap") {
              fixture.publish(3, {fixture.data(92)});
            } else {
              fixture.expire(future);
            }
          } else {
            fixture.publish(10, {
              fixture.marker(false, OtcLinkSpinType::REFERENCE, 200),
              fixture.data(191),
              fixture.marker(true, OtcLinkSpinType::REFERENCE, 200)});
            if(type == OtcLinkSpinType::MARKET_DATA) {
              fixture.publish(11, {
                fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 200),
                fixture.data(192),
                fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 200)});
            }
          }
        }
        return future.get();
      }, {});
      REQUIRE(attempts == 2);
      REQUIRE(snapshot.m_sequence == 201);
      auto fixture = Fixture(type);
      if(type == OtcLinkSpinType::REFERENCE) {
        REQUIRE(snapshot.m_messages.size() == 1);
      } else {
        REQUIRE(snapshot.m_messages.size() == 2);
        REQUIRE(snapshot.m_messages[1] == fixture.data(192));
      }
      REQUIRE(snapshot.m_messages[0] == fixture.data(191));
    }
  }

  TEST_CASE("snapshot_retry_limit") {
    for(auto retries : {0, 1, 3}) {
      auto attempts = 0;
      REQUIRE_THROWS_AS(load_snapshot(retries,
        [&] (std::stop_token) -> OtcLinkSnapshot {
          ++attempts;
          throw IOException();
        }, {}), IOException);
      REQUIRE(attempts == retries + 1);
    }
    auto attempts = 0;
    auto snapshot = load_snapshot(1, [&] (std::stop_token) {
      ++attempts;
      return OtcLinkSnapshot(123, {});
    }, {});
    REQUIRE(attempts == 1);
    REQUIRE(snapshot.m_sequence == 123);
  }

  TEST_CASE("snapshot_retry_cancellation") {
    auto stop = std::stop_source();
    auto is_canceled = false;
    SUBCASE("before_attempt") {
      stop.request_stop();
      is_canceled = true;
    }
    SUBCASE("during_attempt") {}
    auto attempts = 0;
    REQUIRE_THROWS_AS(load_snapshot(1,
      [&] (std::stop_token) -> OtcLinkSnapshot {
        ++attempts;
        stop.request_stop();
        throw EndOfFileException();
      }, stop.get_token()), EndOfFileException);
    if(is_canceled) {
      REQUIRE(attempts == 0);
    } else {
      REQUIRE(attempts == 1);
    }
  }

  TEST_CASE("initial_inactivity") {
    auto is_acknowledged = false;
    SUBCASE("awaiting_acknowledgement") {}
    SUBCASE("acknowledged") {
      is_acknowledged = true;
    }
    auto fixture = Fixture();
    auto future = fixture.load({});
    if(is_acknowledged) {
      fixture.m_acknowledgement.get_eval().set();
      flush_pending_routines();
    }
    fixture.tick();
    REQUIRE(future.wait_for(std::chrono::seconds(0)) ==
      std::future_status::timeout);
    fixture.tick();
    REQUIRE_THROWS_AS(future.get(), IOException);
  }

  TEST_CASE("snapshot_activity") {
    auto fixture = Fixture();
    auto future = fixture.load({});
    fixture.m_acknowledgement.get_eval().set();
    fixture.tick();
    REQUIRE(future.wait_for(std::chrono::seconds(0)) ==
      std::future_status::timeout);
    auto messages = std::vector<SharedBuffer>{
      fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
      fixture.data(91),
      fixture.marker(true, OtcLinkSpinType::REFERENCE, 100),
      fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 100),
      fixture.data(92), fixture.data(93)};
    auto sequence = std::uint32_t(1);
    for(auto& message : messages) {
      fixture.publish(sequence, {message});
      ++sequence;
      fixture.tick();
      fixture.tick();
      REQUIRE(future.wait_for(std::chrono::seconds(0)) ==
        std::future_status::timeout);
    }
    fixture.publish(sequence, {
      fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 100)});
    auto snapshot = future.get();
    REQUIRE(snapshot.m_sequence == 101);
    REQUIRE(snapshot.m_messages.size() == 3);
    REQUIRE(snapshot.m_messages[0] == fixture.data(91));
    REQUIRE(snapshot.m_messages[1] == fixture.data(92));
    REQUIRE(snapshot.m_messages[2] == fixture.data(93));
  }

  TEST_CASE("snapshot_inactivity") {
    auto flag = OtcLinkHeader::Flag::HEARTBEAT;
    auto is_duplicate = false;
    SUBCASE("heartbeats") {}
    SUBCASE("test") {
      flag = OtcLinkHeader::Flag::TEST;
    }
    SUBCASE("replay") {
      flag = OtcLinkHeader::Flag::REPLAY;
    }
    SUBCASE("duplicate") {
      is_duplicate = true;
    }
    auto fixture = Fixture();
    auto future = fixture.load({});
    fixture.m_acknowledgement.get_eval().set();
    fixture.publish(1, {
      fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
      fixture.data(91)});
    fixture.tick();
    fixture.tick();
    REQUIRE(future.wait_for(std::chrono::seconds(0)) ==
      std::future_status::timeout);
    if(is_duplicate) {
      fixture.publish(1, {fixture.data(91)});
    } else if(flag == OtcLinkHeader::Flag::HEARTBEAT) {
      fixture.publish(2, {});
    } else {
      fixture.publish(2, {fixture.data(92)}, static_cast<std::uint8_t>(flag));
    }
    fixture.tick();
    REQUIRE_THROWS_AS(future.get(), IOException);
  }

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
        fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
        fixture.marker(true, OtcLinkSpinType::REFERENCE, 100),
        fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 100),
        fixture.data(92)});
      progress = "packets=1 messages=4 heartbeats=0 last_packet=11 "
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
        fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
        fixture.marker(true, OtcLinkSpinType::REFERENCE, 100),
        fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 100),
        fixture.data(93),
        fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 100)});
      progress = "packets=1 messages=5 heartbeats=0 last_packet=13 "
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
      fixture.expire(future);
      expected = "OTC Link snapshot inactivity timeout expired.";
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
      fixture.marker(false, OtcLinkSpinType::REFERENCE, 0),
      fixture.marker(true, OtcLinkSpinType::REFERENCE, 0),
      fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 0)});
    if(is_empty) {
      fixture.publish(2, {
        fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 0)});
      auto snapshot = future.get();
      REQUIRE(snapshot.m_sequence == 1);
      REQUIRE(snapshot.m_messages.empty());
    } else {
      fixture.expire(future);
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
      fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
      fixture.marker(true, OtcLinkSpinType::REFERENCE, 100),
      fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 100),
      fixture.data(1)});
    if(action == 0) {
      fixture.publish(3, {
        fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 100)});
    } else if(action == 1) {
      fixture.publish(2, {
        fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 101)});
    } else if(action == 2) {
      fixture.expire(future);
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

  TEST_CASE("ignored_packets") {
    auto flag = OtcLinkHeader::Flag::TEST;
    SUBCASE("test") {}
    SUBCASE("replay") {
      flag = OtcLinkHeader::Flag::REPLAY;
    }
    auto fixture = Fixture(OtcLinkSpinType::REFERENCE);
    auto future = fixture.load({});
    fixture.m_acknowledgement.get_eval().set();
    fixture.publish(1, {
      fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
      fixture.data(90),
      fixture.marker(true, OtcLinkSpinType::REFERENCE, 100)},
      static_cast<std::uint8_t>(flag));
    REQUIRE(future.wait_for(std::chrono::seconds(0)) ==
      std::future_status::timeout);
    fixture.publish(10, {
      fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
      fixture.data(91)});
    fixture.publish(30, {fixture.data(99)}, static_cast<std::uint8_t>(flag));
    fixture.publish(11, {
      fixture.data(92),
      fixture.marker(true, OtcLinkSpinType::REFERENCE, 100)});
    auto snapshot = future.get();
    REQUIRE(snapshot.m_sequence == 101);
    REQUIRE(snapshot.m_messages.size() == 2);
    REQUIRE(snapshot.m_messages[0] == fixture.data(91));
    REQUIRE(snapshot.m_messages[1] == fixture.data(92));
  }

  TEST_CASE("snapshot_reset") {
    auto is_capturing = false;
    SUBCASE("before_capture") {}
    SUBCASE("during_capture") {
      is_capturing = true;
    }
    auto fixture = Fixture(OtcLinkSpinType::REFERENCE);
    auto future = fixture.load({});
    fixture.m_acknowledgement.get_eval().set();
    if(is_capturing) {
      fixture.publish(10, {
        fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
        fixture.data(91)});
    }
    fixture.publish(0, {},
      static_cast<std::uint8_t>(OtcLinkHeader::Flag::SEQUENCE_RESET));
    if(is_capturing) {
      REQUIRE_THROWS_AS(future.get(), IOException);
    } else {
      fixture.publish(1, {
        fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
        fixture.data(91),
        fixture.marker(true, OtcLinkSpinType::REFERENCE, 100)});
      auto snapshot = future.get();
      REQUIRE(snapshot.m_sequence == 101);
      REQUIRE(snapshot.m_messages.size() == 1);
      REQUIRE(snapshot.m_messages[0] == fixture.data(91));
    }
  }

  TEST_CASE("acknowledged_cancellation") {
    auto is_closed = false;
    SUBCASE("stop") {}
    SUBCASE("close") {
      is_closed = true;
    }
    auto fixture = Fixture();
    auto stop = std::stop_source();
    auto future = fixture.load(stop.get_token());
    fixture.m_acknowledgement.get_eval().set();
    fixture.publish(1, {
      fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
      fixture.data(91)});
    REQUIRE(future.wait_for(std::chrono::seconds(0)) ==
      std::future_status::timeout);
    if(is_closed) {
      fixture.m_client->close();
    } else {
      stop.request_stop();
    }
    REQUIRE_THROWS_AS(future.get(), EndOfFileException);
  }

  TEST_CASE("spin_markers") {
    auto is_end = true;
    auto type = OtcLinkSpinType::REFERENCE;
    auto timestamp = std::uint64_t(1000);
    SUBCASE("overlapping_starts") {
      is_end = false;
    }
    SUBCASE("different_types") {
      type = OtcLinkSpinType::MARKET_DATA;
    }
    SUBCASE("earlier_end") {
      --timestamp;
    }
    auto fixture = Fixture(OtcLinkSpinType::REFERENCE);
    auto future = fixture.load({});
    fixture.m_acknowledgement.get_eval().set();
    fixture.publish(1, {
      fixture.marker(false, OtcLinkSpinType::REFERENCE, 100),
      fixture.data(91)});
    fixture.publish(2, {fixture.marker(is_end, type, 100, timestamp)});
    REQUIRE_THROWS_AS(future.get(), OtcLinkParserException);
  }

  TEST_CASE("partial_reference") {
    auto fixture = Fixture();
    auto future = fixture.load({});
    fixture.m_acknowledgement.get_eval().set();
    fixture.publish(10, {
      fixture.data(90),
      fixture.marker(true, OtcLinkSpinType::REFERENCE, 100)});
    fixture.publish(11, {
      fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 100),
      fixture.data(91),
      fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 100)});
    REQUIRE(future.wait_for(std::chrono::seconds(0)) ==
      std::future_status::timeout);
    SUBCASE("next_cycle") {
      fixture.publish(12, {
        fixture.marker(false, OtcLinkSpinType::REFERENCE, 200),
        fixture.data(190)});
      fixture.publish(13, {
        fixture.data(191),
        fixture.marker(true, OtcLinkSpinType::REFERENCE, 200)});
      REQUIRE(future.wait_for(std::chrono::seconds(0)) ==
        std::future_status::timeout);
      fixture.publish(14, {
        fixture.marker(false, OtcLinkSpinType::MARKET_DATA, 200),
        fixture.data(192),
        fixture.marker(true, OtcLinkSpinType::MARKET_DATA, 200)});
      auto snapshot = future.get();
      REQUIRE(snapshot.m_sequence == 201);
      REQUIRE(snapshot.m_messages.size() == 3);
      REQUIRE(snapshot.m_messages[0] == fixture.data(190));
      REQUIRE(snapshot.m_messages[1] == fixture.data(191));
      REQUIRE(snapshot.m_messages[2] == fixture.data(192));
    }
    SUBCASE("inactivity") {
      fixture.expire(future);
      auto actual = [&] {
        try {
          future.get();
        } catch(const IOException& exception) {
          return std::string(exception.what());
        }
        return std::string();
      }();
      REQUIRE(actual == "OTC Link snapshot inactivity timeout expired. "
        "acknowledgement=received packets=2 messages=5 heartbeats=0 "
        "last_packet=11 expected_spin=market_data "
        "last_marker=end/market_data/100");
    }
  }

  TEST_CASE("reference_snapshot") {
    auto fixture = Fixture(OtcLinkSpinType::REFERENCE);
    auto future = fixture.load({});
    fixture.m_acknowledgement.get_eval().set();
    fixture.publish(10, {
      fixture.data(90),
      fixture.marker(true, OtcLinkSpinType::REFERENCE, 100)});
    REQUIRE(future.wait_for(std::chrono::seconds(0)) ==
      std::future_status::timeout);
    fixture.publish(11, {
      fixture.marker(false, OtcLinkSpinType::REFERENCE, 200),
      fixture.data(191),
      fixture.marker(true, OtcLinkSpinType::REFERENCE, 200)});
    auto snapshot = future.get();
    REQUIRE(snapshot.m_sequence == 201);
    REQUIRE(snapshot.m_messages.size() == 1);
    REQUIRE(snapshot.m_messages[0] == fixture.data(191));
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
