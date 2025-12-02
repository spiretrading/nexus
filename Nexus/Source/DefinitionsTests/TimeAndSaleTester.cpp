#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/TimeAndSale.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("TimeAndSale") {
  TEST_CASE("stream_condition") {
    REQUIRE(to_string(TimeAndSale::Condition::Type::REGULAR) == "@");
    REQUIRE(to_string(TimeAndSale::Condition::Type::OPEN) == "OPEN");
    REQUIRE(to_string(TimeAndSale::Condition::Type::CLOSE) == "CLOSE");
  }

  TEST_CASE("stream_code") {
    auto condition =
      TimeAndSale::Condition(TimeAndSale::Condition::Type::OPEN, "X");
    REQUIRE(to_string(condition) == "(OPEN X)");
  }

  TEST_CASE("stream") {
    auto time_and_sale = TimeAndSale(
      time_from_string("2025-06-30 09:30:00"), Money(100), Quantity(200),
      TimeAndSale::Condition(TimeAndSale::Condition::Type::CLOSE, "Z"), "NYSE",
      "BUY123", "SELL456");
    auto expected = std::stringstream();
    expected << '(' << time_and_sale.m_timestamp << ' ' <<
      time_and_sale.m_price << ' ' << time_and_sale.m_size << ' ' <<
      time_and_sale.m_condition << ' ' <<
      time_and_sale.m_market_center << ' ' << time_and_sale.m_buyer_mpid <<
      ' ' << time_and_sale.m_seller_mpid << ')';
    REQUIRE(to_string(time_and_sale) == expected.str());
    test_round_trip_shuttle(time_and_sale);
  }
}
