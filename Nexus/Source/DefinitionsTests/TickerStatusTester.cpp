#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/TickerStatus.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("TickerStatus") {
  TEST_CASE("aggregate_initialization") {
    auto timestamp = time_from_string("2026-05-07 09:30:00.000");
    auto status = TickerStatus(Venue("TSE"), "Authorized",
      TickerStatus::Flag::IS_MATCHING |
        TickerStatus::Flag::IS_ACCEPTING_ORDERS |
        TickerStatus::Flag::IS_ACCEPTING_CANCELS, timestamp);
    REQUIRE(status.m_venue == Venue("TSE"));
    REQUIRE(status.m_state == "Authorized");
    REQUIRE(has(status.m_flags, TickerStatus::Flag::IS_MATCHING));
    REQUIRE(has(status.m_flags, TickerStatus::Flag::IS_ACCEPTING_ORDERS));
    REQUIRE(has(status.m_flags, TickerStatus::Flag::IS_ACCEPTING_CANCELS));
    REQUIRE(!has(status.m_flags, TickerStatus::Flag::IS_AUCTION));
    REQUIRE(status.m_timestamp == timestamp);
  }

  TEST_CASE("flag_operators") {
    auto flags =
      TickerStatus::Flag::IS_MATCHING | TickerStatus::Flag::IS_ACCEPTING_ORDERS;
    REQUIRE(has(flags, TickerStatus::Flag::IS_MATCHING));
    REQUIRE(has(flags, TickerStatus::Flag::IS_ACCEPTING_ORDERS));
    REQUIRE(!has(flags, TickerStatus::Flag::IS_ACCEPTING_CANCELS));
    REQUIRE(!has(flags, TickerStatus::Flag::IS_AUCTION));
    REQUIRE((flags & TickerStatus::Flag::IS_MATCHING) ==
      TickerStatus::Flag::IS_MATCHING);
    REQUIRE(
      (flags & TickerStatus::Flag::IS_AUCTION) == TickerStatus::Flag::NONE);
  }

  TEST_CASE("flag_none") {
    REQUIRE(!has(TickerStatus::Flag::NONE, TickerStatus::Flag::IS_MATCHING));
    REQUIRE(
      !has(TickerStatus::Flag::NONE, TickerStatus::Flag::IS_ACCEPTING_ORDERS));
    REQUIRE(
      !has(TickerStatus::Flag::NONE, TickerStatus::Flag::IS_ACCEPTING_CANCELS));
    REQUIRE(!has(TickerStatus::Flag::NONE, TickerStatus::Flag::IS_AUCTION));
  }

  TEST_CASE("flag_stream_none") {
    REQUIRE(to_string(TickerStatus::Flag::NONE) == "NONE");
  }

  TEST_CASE("flag_stream_single") {
    REQUIRE(to_string(TickerStatus::Flag::IS_MATCHING) == "IS_MATCHING");
    REQUIRE(to_string(TickerStatus::Flag::IS_AUCTION) == "IS_AUCTION");
  }

  TEST_CASE("flag_stream_multiple") {
    auto flags =
      TickerStatus::Flag::IS_MATCHING | TickerStatus::Flag::IS_ACCEPTING_ORDERS;
    REQUIRE(to_string(flags) == "(IS_MATCHING | IS_ACCEPTING_ORDERS)");
  }

  TEST_CASE("is_continuous") {
    auto flags = TickerStatus::Flag::IS_CONTINUOUS;
    REQUIRE(has(flags, TickerStatus::Flag::IS_MATCHING));
    REQUIRE(has(flags, TickerStatus::Flag::IS_ACCEPTING_ORDERS));
    REQUIRE(has(flags, TickerStatus::Flag::IS_ACCEPTING_CANCELS));
    REQUIRE(!has(flags, TickerStatus::Flag::IS_AUCTION));
    REQUIRE(flags == (TickerStatus::Flag::IS_MATCHING |
      TickerStatus::Flag::IS_ACCEPTING_ORDERS |
      TickerStatus::Flag::IS_ACCEPTING_CANCELS));
  }

  TEST_CASE("stream") {
    auto timestamp = time_from_string("2026-05-07 09:30:00.000");
    auto status = TickerStatus(
      Venue("TSE"), "Authorized", TickerStatus::Flag::IS_MATCHING, timestamp);
    REQUIRE(to_string(status) ==
      "(TSE Authorized IS_MATCHING " + to_string(timestamp) + ")");
  }

  TEST_CASE("shuttle") {
    auto timestamp = time_from_string("2026-05-07 14:00:00.000");
    auto status = TickerStatus(Venue("CHI"), "Open",
      TickerStatus::Flag::IS_MATCHING |
        TickerStatus::Flag::IS_ACCEPTING_ORDERS |
        TickerStatus::Flag::IS_ACCEPTING_CANCELS, timestamp);
    test_round_trip_shuttle(status);
  }
}
