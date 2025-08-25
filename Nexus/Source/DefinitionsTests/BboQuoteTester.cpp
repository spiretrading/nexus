#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/BboQuote.hpp"

using namespace Nexus;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("BboQuote") {
  TEST_CASE("default_constructor") {
    auto bbq = BboQuote();
    REQUIRE(bbq.m_bid.m_side == Side::BID);
    REQUIRE(bbq.m_ask.m_side == Side::ASK);
    REQUIRE(bbq.m_bid.m_price == Money::ZERO);
    REQUIRE(bbq.m_ask.m_price == Money::ZERO);
    REQUIRE(bbq.m_bid.m_size == Quantity(0));
    REQUIRE(bbq.m_ask.m_size == Quantity(0));
    REQUIRE(bbq.m_timestamp.is_not_a_date_time());
  }

  TEST_CASE("constructor") {
    auto bid = make_bid(Money(10), Quantity(1));
    auto ask = make_ask(Money(20), Quantity(2));
    auto timestamp = time_from_string("2025-06-25 15:30:00.000");
    auto bbo = BboQuote(bid, ask, timestamp);
    REQUIRE(bbo.m_bid == bid);
    REQUIRE(bbo.m_ask == ask);
    REQUIRE(bbo.m_timestamp == timestamp);
  }

  TEST_CASE("stream") {
    auto bid = make_bid(Money(5), Quantity(50));
    auto ask = make_ask(Money(6), Quantity(60));
    auto timestamp = time_from_string("2025-06-24 09:15:42.123");
    auto bbo = BboQuote(bid, ask, timestamp);
    auto ss = std::ostringstream();
    ss << bbo;
    auto expected = "(" + std::string("(") +
      lexical_cast<std::string>(bid.m_price) + " " +
      lexical_cast<std::string>(bid.m_size) + " BID) " +
      std::string("(") + lexical_cast<std::string>(ask.m_price) + " " +
      lexical_cast<std::string>(ask.m_size) + " ASK) " +
      lexical_cast<std::string>(timestamp) + ")";
    REQUIRE(ss.str() == expected);
  }

  TEST_CASE("shuttle") {
    auto bid = make_bid(Money(10), Quantity(1));
    auto ask = make_ask(Money(20), Quantity(2));
    auto timestamp = time_from_string("2025-06-25 15:30:00.000");
    auto bbo = BboQuote(bid, ask, timestamp);
    Beam::Serialization::Tests::TestRoundTripShuttle(bbo);
  }
}
