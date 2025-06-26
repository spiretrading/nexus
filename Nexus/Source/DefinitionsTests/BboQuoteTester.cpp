#include <sstream>
#include <doctest/doctest.h>
#include "Nexus/Definitions/BboQuote.hpp"
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace Nexus;
using namespace boost;
using namespace boost::posix_time;

TEST_SUITE("BboQuote") {
  TEST_CASE("default_constructor") {
    auto bbq = BboQuote();
    CHECK(bbq.m_bid.m_side == Side::BID);
    CHECK(bbq.m_ask.m_side == Side::ASK);
    CHECK(bbq.m_bid.m_price == Money::ZERO);
    CHECK(bbq.m_ask.m_price == Money::ZERO);
    CHECK(bbq.m_bid.m_size == Quantity(0));
    CHECK(bbq.m_ask.m_size == Quantity(0));
    CHECK(bbq.m_timestamp.is_not_a_date_time());
  }

  TEST_CASE("constructor") {
    auto bid = Quote(Money(10), Quantity(1), Side::BID);
    auto ask = Quote(Money(20), Quantity(2), Side::ASK);
    auto timestamp = time_from_string("2025-06-25 15:30:00.000");
    auto bbo = BboQuote(bid, ask, timestamp);
    CHECK(bbo.m_bid == bid);
    CHECK(bbo.m_ask == ask);
    CHECK(bbo.m_timestamp == timestamp);
  }

  TEST_CASE("stream") {
    auto bid = Quote(Money(5), Quantity(50), Side::BID);
    auto ask = Quote(Money(6), Quantity(60), Side::ASK);
    auto timestamp = time_from_string("2025-06-24 09:15:42.123");
    auto bbo = BboQuote(bid, ask, timestamp);
    auto ss = std::ostringstream();
    ss << timestamp;
    auto timestamp_as_string = ss.str();
    ss = std::ostringstream();
    auto expected = "(" + std::string("(") +
      ToString(bid.m_price) + " " + ToString(bid.m_size) + " BID) " +
      std::string("(") + ToString(ask.m_price) + " " +
      ToString(ask.m_size) + " ASK) " + timestamp_as_string + ")";
    CHECK(ss.str() == expected);
  }
}
