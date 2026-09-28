#include <Beam/IO/IOException.hpp>
#include <Beam/TimeService/FixedTimeClient.hpp>
#include <Beam/TimeService/TriggerTimer.hpp>
#include <doctest/doctest.h>
#include "OtcLinkMarketDataFeedClient/OtcLinkReferenceLoader.hpp"

using namespace Beam;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  TradingSchedule make_schedule() {
    return parse_trading_schedule(YAML::Load(R"(
- venues: [OTCM]
  time:
    weekdays: [Sat, Sun]
- venues: [OTCM]
  dates: [2026-12-25]
- venues: [OTCM]
  events:
    - code: PRE_OPEN
      time: "06:00:00"
    - code: OPEN
      time: "09:30:00"
)"));
  }
}

TEST_SUITE("OtcLinkReferenceLoader") {
  TEST_CASE("initial_reference") {
    auto timestamp = time_from_string("2026-09-24 09:59:00");
    auto snapshot = OtcLinkSnapshot(123, {SharedBuffer("reference", 9)});
    auto reference = load_initial_reference([&] {
      return snapshot;
    }, timestamp);
    REQUIRE(reference.m_sequence == snapshot.m_sequence);
    REQUIRE(reference.m_messages == snapshot.m_messages);
  }

  TEST_CASE("initial_reference_failure") {
    auto time_client = FixedTimeClient(
      time_from_string("2026-09-24 09:59:00"));
    auto reference = boost::optional<OtcLinkSnapshot>();
    REQUIRE_NOTHROW(reference = load_initial_reference(
      [] () -> OtcLinkSnapshot { throw IOException(); },
      time_client.get_time()));
    REQUIRE(reference.has_value());
    REQUIRE(reference->m_messages.empty());
    auto timer = TriggerTimer();
    auto calls = 0;
    auto loader = OtcLinkReferenceLoader(make_schedule(), &time_client,
      &timer, [&] (std::stop_token) {
        ++calls;
        reference = OtcLinkSnapshot(123, {SharedBuffer("reference", 9)});
      });
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 0);
    time_client.set(time_from_string("2026-09-24 10:00:00"));
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 1);
    REQUIRE(reference->m_sequence == 123);
    REQUIRE(reference->m_messages.size() == 1);
  }

  TEST_CASE("pre_open") {
    auto timestamp = time_from_string("2026-09-24 10:00:00");
    SUBCASE("daylight_time") {}
    SUBCASE("standard_time") {
      timestamp = time_from_string("2026-12-24 11:00:00");
    }
    auto time_client = FixedTimeClient(timestamp - seconds(1));
    auto timer = TriggerTimer();
    auto calls = 0;
    auto loader = OtcLinkReferenceLoader(make_schedule(), &time_client,
      &timer, [&] (std::stop_token) { ++calls; });
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 0);
    time_client.set(timestamp);
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 1);
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 1);
    time_client.set(timestamp - seconds(1));
    timer.trigger();
    flush_pending_routines();
    time_client.set(timestamp);
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 1);
  }

  TEST_CASE("subsequent_days") {
    auto time_client = FixedTimeClient(
      time_from_string("2026-12-24 10:59:00"));
    auto timer = TriggerTimer();
    auto calls = 0;
    auto loader = OtcLinkReferenceLoader(make_schedule(), &time_client,
      &timer, [&] (std::stop_token) { ++calls; });
    for(auto timestamp : {"2026-12-24 11:00:02", "2026-12-25 11:00:00",
        "2026-12-26 11:00:00", "2026-12-27 11:00:00"}) {
      time_client.set(time_from_string(timestamp));
      timer.trigger();
      flush_pending_routines();
      REQUIRE(calls == 1);
    }
    time_client.set(time_from_string("2026-12-28 11:00:00"));
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 2);
  }

  TEST_CASE("late_start") {
    auto time_client = FixedTimeClient(
      time_from_string("2026-09-24 14:00:00"));
    auto timer = TriggerTimer();
    auto calls = 0;
    auto loader = OtcLinkReferenceLoader(make_schedule(), &time_client,
      &timer, [&] (std::stop_token) { ++calls; });
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 0);
    time_client.set(time_from_string("2026-09-25 10:00:00"));
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 1);
  }

  TEST_CASE("refresh_failure") {
    auto time_client = FixedTimeClient(
      time_from_string("2026-09-24 09:59:00"));
    auto timer = TriggerTimer();
    auto calls = 0;
    auto loader = OtcLinkReferenceLoader(make_schedule(), &time_client,
      &timer, [&] (std::stop_token) {
        ++calls;
        throw IOException();
      });
    time_client.set(time_from_string("2026-09-24 10:00:00"));
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 1);
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 1);
    time_client.set(time_from_string("2026-09-25 10:00:00"));
    timer.trigger();
    flush_pending_routines();
    REQUIRE(calls == 2);
  }

  TEST_CASE("close") {
    auto time_client = FixedTimeClient(
      time_from_string("2026-09-24 09:59:00"));
    auto timer = TriggerTimer();
    auto completion = Async<void>();
    auto calls = 0;
    auto is_canceled = false;
    auto loader = OtcLinkReferenceLoader(make_schedule(), &time_client,
      &timer, [&] (std::stop_token token) {
        ++calls;
        auto stop = std::stop_callback(token, [&] {
          is_canceled = true;
          completion.get_eval().set();
        });
        completion.get();
        throw IOException();
      });
    SUBCASE("pending_refresh") {
      time_client.set(time_from_string("2026-09-24 10:00:00"));
      timer.trigger();
      flush_pending_routines();
      REQUIRE(calls == 1);
      loader.close();
      REQUIRE(is_canceled);
    }
    SUBCASE("before_refresh") {
      loader.close();
      time_client.set(time_from_string("2026-09-24 10:00:00"));
      timer.trigger();
      flush_pending_routines();
      REQUIRE(calls == 0);
    }
  }
}
