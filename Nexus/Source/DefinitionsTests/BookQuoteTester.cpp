#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/BookQuote.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("BookQuote") {
  TEST_CASE("listing_comparator_price") {
    auto low = BookQuote("TSX", true, Venue("TSX"), make_ask(Money(100), 100),
      time_from_string("2025-06-25 15:30:00.000"));
    auto high = BookQuote("TSX", true, Venue("TSX"), make_ask(Money(200), 100),
      time_from_string("2025-06-25 15:30:00.000"));
    REQUIRE(listing_comparator(low, high));
    REQUIRE_FALSE(listing_comparator(high, low));
  }

  TEST_CASE("listing_comparator_mpid") {
    auto a = BookQuote("AAA", true, Venue("TSX"), make_ask(Money(100), 100),
      time_from_string("2025-06-25 15:30:00.000"));
    auto b = BookQuote("BBB", true, Venue("TSX"), make_ask(Money(100), 100),
      time_from_string("2025-06-25 15:30:00.000"));
    REQUIRE(listing_comparator(a, b));
    REQUIRE_FALSE(listing_comparator(b, a));
  }

  TEST_CASE("stream") {
    auto quote =
      BookQuote("MPX", true, Venue("TEST"), make_ask(500 * Money::ONE, 25),
        time_from_string("2025-06-25 15:30:00.000"));
    auto expected = std::stringstream();
    expected << "(" << quote.m_mpid << " " << quote.m_is_primary_mpid << " " <<
      quote.m_venue << " " << "(" << quote.m_quote.m_price << " " <<
      quote.m_quote.m_size << " " << quote.m_quote.m_side << ") " <<
      quote.m_timestamp << ")";
    REQUIRE(to_string(quote) == expected.str());
    test_round_trip_shuttle(quote);
  }
}
