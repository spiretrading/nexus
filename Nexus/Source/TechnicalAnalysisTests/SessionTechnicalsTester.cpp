#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Nexus/TechnicalAnalysis/SessionTechnicals.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace Nexus;

namespace {
  auto make_time_and_sale(
      Money price, Quantity size, std::string market_center) {
    auto time_and_sale = TimeAndSale();
    time_and_sale.m_price = price;
    time_and_sale.m_size = size;
    time_and_sale.m_market_center = market_center;
    return time_and_sale;
  }
}

TEST_SUITE("SessionTechnicals") {
  TEST_CASE("update") {
    auto technicals = SessionTechnicals();
    update(technicals, make_time_and_sale(10 * Money::ONE, 100, "XNYS"), "TSE");
    REQUIRE(!technicals.m_open);
    REQUIRE(technicals.m_high == 10 * Money::ONE);
    REQUIRE(technicals.m_low == 10 * Money::ONE);
    REQUIRE(technicals.m_volume == 100);
    update(technicals, make_time_and_sale(11 * Money::ONE, 50, "TSE"), "TSE");
    REQUIRE(technicals.m_open == 11 * Money::ONE);
    REQUIRE(technicals.m_high == 11 * Money::ONE);
    REQUIRE(technicals.m_low == 10 * Money::ONE);
    REQUIRE(technicals.m_volume == 150);
    update(technicals, make_time_and_sale(12 * Money::ONE, 25, "TSE"), "TSE");
    REQUIRE(technicals.m_open == 11 * Money::ONE);
    REQUIRE(technicals.m_high == 12 * Money::ONE);
    REQUIRE(technicals.m_low == 10 * Money::ONE);
    REQUIRE(technicals.m_volume == 175);
    update(technicals, make_time_and_sale(8 * Money::ONE, 25, "XNYS"), "TSE");
    REQUIRE(technicals.m_open == 11 * Money::ONE);
    REQUIRE(technicals.m_high == 12 * Money::ONE);
    REQUIRE(technicals.m_low == 8 * Money::ONE);
    REQUIRE(technicals.m_volume == 200);
  }

  TEST_CASE("stream") {
    auto technicals = SessionTechnicals();
    technicals.m_open = 10 * Money::ONE;
    technicals.m_previous_close = 9 * Money::ONE;
    technicals.m_high = 12 * Money::ONE;
    technicals.m_low = 8 * Money::ONE;
    technicals.m_volume = Quantity(5000);
    REQUIRE(to_string(technicals) == "(10.00 9.00 12.00 8.00 5000)");
    auto empty = SessionTechnicals();
    REQUIRE(to_string(empty) == "(none none none none 0)");
    test_round_trip_shuttle(technicals);
  }
}
