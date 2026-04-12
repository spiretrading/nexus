#include <Beam/Serialization/ShuttleDateTime.hpp>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/TechnicalAnalysis/Candlestick.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using TestCandlestick = Candlestick<ptime, Money>;
}

TEST_SUITE("Candlestick") {
  TEST_CASE("default_constructor") {
    auto candlestick = TestCandlestick();
    REQUIRE(candlestick.get_start() == ptime());
    REQUIRE(candlestick.get_end() == ptime());
    REQUIRE(candlestick.get_open() == Money());
    REQUIRE(candlestick.get_close() == Money());
    REQUIRE(candlestick.get_high() == Money());
    REQUIRE(candlestick.get_low() == Money());
    REQUIRE(candlestick.get_volume() == 0);
  }

  TEST_CASE("domain_constructor") {
    auto start = time_from_string("2024-01-31 10:00:00");
    auto end = time_from_string("2024-01-31 11:00:00");
    auto candlestick = TestCandlestick(start, end);
    REQUIRE(candlestick.get_start() == start);
    REQUIRE(candlestick.get_end() == end);
    REQUIRE(candlestick.get_open() == Money());
    REQUIRE(candlestick.get_close() == Money());
    REQUIRE(candlestick.get_high() == Money());
    REQUIRE(candlestick.get_low() == Money());
    REQUIRE(candlestick.get_volume() == 0);
  }

  TEST_CASE("constructor") {
    auto start = time_from_string("2024-01-31 10:00:00");
    auto end = time_from_string("2024-01-31 11:00:00");
    auto open = 10 * Money::ONE;
    auto close = 12 * Money::ONE;
    auto high = 15 * Money::ONE;
    auto low = 9 * Money::ONE;
    auto volume = Quantity(5000);
    auto candlestick =
      TestCandlestick(start, end, open, close, high, low, volume);
    REQUIRE(candlestick.get_start() == start);
    REQUIRE(candlestick.get_end() == end);
    REQUIRE(candlestick.get_open() == open);
    REQUIRE(candlestick.get_close() == close);
    REQUIRE(candlestick.get_high() == high);
    REQUIRE(candlestick.get_low() == low);
    REQUIRE(candlestick.get_volume() == volume);
  }

  TEST_CASE("update") {
    auto start = time_from_string("2024-01-31 10:00:00");
    auto end = time_from_string("2024-01-31 11:00:00");
    auto candlestick = TestCandlestick(start, end);
    candlestick.update(10 * Money::ONE);
    REQUIRE(candlestick.get_open() == 10 * Money::ONE);
    REQUIRE(candlestick.get_close() == 10 * Money::ONE);
    REQUIRE(candlestick.get_high() == 10 * Money::ONE);
    REQUIRE(candlestick.get_low() == 10 * Money::ONE);
    REQUIRE(candlestick.get_volume() == 0);
    candlestick.update(12 * Money::ONE);
    REQUIRE(candlestick.get_open() == 10 * Money::ONE);
    REQUIRE(candlestick.get_close() == 12 * Money::ONE);
    REQUIRE(candlestick.get_high() == 12 * Money::ONE);
    REQUIRE(candlestick.get_low() == 10 * Money::ONE);
    candlestick.update(8 * Money::ONE);
    REQUIRE(candlestick.get_open() == 10 * Money::ONE);
    REQUIRE(candlestick.get_close() == 8 * Money::ONE);
    REQUIRE(candlestick.get_high() == 12 * Money::ONE);
    REQUIRE(candlestick.get_low() == 8 * Money::ONE);
  }

  TEST_CASE("update_with_volume") {
    auto start = time_from_string("2024-01-31 10:00:00");
    auto end = time_from_string("2024-01-31 11:00:00");
    auto candlestick = TestCandlestick(start, end);
    candlestick.update(10 * Money::ONE, 100);
    REQUIRE(candlestick.get_open() == 10 * Money::ONE);
    REQUIRE(candlestick.get_volume() == 100);
    candlestick.update(12 * Money::ONE, 200);
    REQUIRE(candlestick.get_close() == 12 * Money::ONE);
    REQUIRE(candlestick.get_high() == 12 * Money::ONE);
    REQUIRE(candlestick.get_volume() == 300);
    candlestick.update(8 * Money::ONE, 50);
    REQUIRE(candlestick.get_low() == 8 * Money::ONE);
    REQUIRE(candlestick.get_volume() == 350);
  }

  TEST_CASE("stream") {
    auto start = time_from_string("2024-01-31 10:00:00");
    auto end = time_from_string("2024-01-31 11:00:00");
    auto open = 10 * Money::ONE;
    auto close = 12 * Money::ONE;
    auto high = 15 * Money::ONE;
    auto low = 9 * Money::ONE;
    auto volume = Quantity(5000);
    auto candlestick =
      TestCandlestick(start, end, open, close, high, low, volume);
    REQUIRE(to_string(candlestick) == "(2024-Jan-31 10:00:00, 2024-Jan-31 "
      "11:00:00, 10.00, 15.00, 9.00, 12.00, 5000)");
    auto empty_candlestick = TestCandlestick(start, end);
    REQUIRE(to_string(empty_candlestick) ==
      "(2024-Jan-31 10:00:00, 2024-Jan-31 11:00:00)");
    test_round_trip_shuttle(candlestick);
  }
}
