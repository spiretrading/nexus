#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/BookQuote.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("BookQuote") {
  TEST_CASE("listing_comparator_price") {
    auto low =
      BookQuote("TSX", true, Venue("TSX"), Quote(Money(100), 100, Side::ASK),
        time_from_string("2025-06-25 15:30:00.000"));
    auto high =
      BookQuote("TSX", true, Venue("TSX"), Quote(Money(200), 100, Side::ASK),
        time_from_string("2025-06-25 15:30:00.000"));
    REQUIRE(listing_comparator(low, high));
    REQUIRE_FALSE(listing_comparator(high, low));
  }

  TEST_CASE("listing_comparator_mpid") {
    auto a =
      BookQuote("AAA", true, Venue("TSX"), Quote(Money(100), 100, Side::ASK),
        time_from_string("2025-06-25 15:30:00.000"));
    auto b =
      BookQuote("BBB", true, Venue("TSX"), Quote(Money(100), 100, Side::ASK),
        time_from_string("2025-06-25 15:30:00.000"));
    REQUIRE(listing_comparator(a, b));
    REQUIRE_FALSE(listing_comparator(b, a));
  }

  TEST_CASE("stream") {
    auto quote = BookQuote("MPX", true, Venue("TEST"),
      Quote(500 * Money::ONE, 25, Side::ASK),
        time_from_string("2025-06-25 15:30:00.000"));
    auto ss = std::stringstream();
    ss << quote;
    auto expected = std::stringstream();
    expected << "(" << quote.m_mpid << " " << quote.m_is_primary_mpid << " " <<
      quote.m_venue << " " << "(" << quote.m_quote.m_price << " " <<
      quote.m_quote.m_size << " " << quote.m_quote.m_side << ") " <<
      quote.m_timestamp << ")";
    REQUIRE(ss.str() == expected.str());
  }
}
