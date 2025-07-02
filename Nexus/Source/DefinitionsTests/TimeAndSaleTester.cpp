#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/TimeAndSale.hpp"

using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("TimeAndSale") {
  TEST_CASE("stream_condition") {
    auto ss = std::stringstream();
    ss << TimeAndSale::Condition::Type::REGULAR;
    REQUIRE(ss.str() == "@");
    ss.str("");
    ss.clear();
    ss << TimeAndSale::Condition::Type::OPEN;
    REQUIRE(ss.str() == "OPEN");
    ss.str("");
    ss.clear();
    ss << TimeAndSale::Condition::Type::CLOSE;
    REQUIRE(ss.str() == "CLOSE");
  }

  TEST_CASE("stream_code") {
    auto condition =
      TimeAndSale::Condition(TimeAndSale::Condition::Type::OPEN, "X");
    auto ss = std::stringstream();
    ss << condition;
    REQUIRE(ss.str() == "(OPEN X)");
  }

  TEST_CASE("stream") {
    auto time_and_sale = TimeAndSale(ptime(date(2025, 6, 30),
      time_duration(9, 30, 0)), Money(100), Quantity(200),
      TimeAndSale::Condition(TimeAndSale::Condition::Type::CLOSE, "Z"), "NYSE",
      "BUY123", "SELL456");
    auto ss = std::stringstream();
    ss << time_and_sale;
    auto expected = std::stringstream();
    expected << '(' << time_and_sale.m_timestamp << ' ' <<
      time_and_sale.m_price << ' ' << time_and_sale.m_size << ' ' <<
      time_and_sale.m_condition << ' ' <<
      time_and_sale.m_market_center << ' ' << time_and_sale.m_buyer_mpid <<
      ' ' << time_and_sale.m_seller_mpid << ')';
    REQUIRE(ss.str() == expected.str());
  }

  TEST_CASE("shuttle") {
    Beam::Serialization::Tests::TestRoundTripShuttle(TimeAndSale(ptime(
      date(2025, 6, 30), time_duration(9, 30, 0)), Money(100), Quantity(200),
      TimeAndSale::Condition(TimeAndSale::Condition::Type::CLOSE, "Z"), "NYSE",
      "BUY123", "SELL456"));
  }
}
