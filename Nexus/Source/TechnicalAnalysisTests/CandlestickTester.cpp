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
  }

  TEST_CASE("domain_constructor") {
    auto start = from_iso_string("20240131T100000");
    auto end = from_iso_string("20240131T110000");
    auto candlestick = TestCandlestick(start, end);
    REQUIRE(candlestick.get_start() == start);
    REQUIRE(candlestick.get_end() == end);
    REQUIRE(candlestick.get_open() == Money());
    REQUIRE(candlestick.get_close() == Money());
    REQUIRE(candlestick.get_high() == Money());
    REQUIRE(candlestick.get_low() == Money());
  }

  TEST_CASE("constructor") {
    auto start = from_iso_string("20240131T100000");
    auto end = from_iso_string("20240131T110000");
    auto open = 10 * Money::ONE;
    auto close = 12 * Money::ONE;
    auto high = 15 * Money::ONE;
    auto low = 9 * Money::ONE;
    auto candlestick = TestCandlestick(start, end, open, close, high, low);
    REQUIRE(candlestick.get_start() == start);
    REQUIRE(candlestick.get_end() == end);
    REQUIRE(candlestick.get_open() == open);
    REQUIRE(candlestick.get_close() == close);
    REQUIRE(candlestick.get_high() == high);
    REQUIRE(candlestick.get_low() == low);
  }

  TEST_CASE("update") {
    auto start = from_iso_string("20240131T100000");
    auto end = from_iso_string("20240131T110000");
    auto candlestick = TestCandlestick(start, end);
    candlestick.update(10 * Money::ONE);
    REQUIRE(candlestick.get_open() == 10 * Money::ONE);
    REQUIRE(candlestick.get_close() == 10 * Money::ONE);
    REQUIRE(candlestick.get_high() == 10 * Money::ONE);
    REQUIRE(candlestick.get_low() == 10 * Money::ONE);
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

  TEST_CASE("stream") {
    auto start = from_iso_string("20240131T100000");
    auto end = from_iso_string("20240131T110000");
    auto open = 10 * Money::ONE;
    auto close = 12 * Money::ONE;
    auto high = 15 * Money::ONE;
    auto low = 9 * Money::ONE;
    auto candlestick = TestCandlestick(start, end, open, close, high, low);
    REQUIRE(to_string(candlestick) == "(2024-Jan-31 10:00:00, 2024-Jan-31 "
      "11:00:00, 10.00, 15.00, 9.00, 12.00)");
    auto empty_candlestick = TestCandlestick(start, end);
    REQUIRE(to_string(empty_candlestick) ==
      "(2024-Jan-31 10:00:00, 2024-Jan-31 11:00:00)");
    test_round_trip_shuttle(candlestick);
  }
}
