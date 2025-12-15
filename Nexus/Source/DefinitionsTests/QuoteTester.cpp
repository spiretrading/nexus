#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Quote.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;

TEST_SUITE("Quote") {
  TEST_CASE("default_constructor") {
    auto default_quote = Quote();
    REQUIRE(default_quote.m_price == Money::ZERO);
    REQUIRE(default_quote.m_size == 0);
    REQUIRE(default_quote.m_side == Side::NONE);
  }

  TEST_CASE("constructor") {
    auto price = Money::ONE;
    auto size = 100;
    auto side = Side::ASK;
    auto quote = Quote(price, size, side);
    REQUIRE(quote.m_price == price);
    REQUIRE(quote.m_size == size);
    REQUIRE(quote.m_side == side);
  }

  TEST_CASE("make_bid") {
    auto price = Money(5);
    auto size = Quantity(250);
    auto q = make_bid(price, size);
    REQUIRE(q.m_price == price);
    REQUIRE(q.m_size == size);
    REQUIRE(q.m_side == Side::BID);
  }

  TEST_CASE("make_ask") {
    auto price = Money(7);
    auto size = Quantity(400);
    auto q = make_ask(price, size);
    REQUIRE(q.m_price == price);
    REQUIRE(q.m_size == size);
    REQUIRE(q.m_side == Side::ASK);
  }

  TEST_CASE("equals") {
    auto q1 = Quote(Money(1), Quantity(100), Side::BID);
    auto q2 = Quote(Money(1), Quantity(100), Side::BID);
    auto q3 = Quote(Money(2), Quantity(100), Side::BID);
    REQUIRE(q1 == q2);
    REQUIRE_FALSE(q1 == q3);
  }

  TEST_CASE("listing_comparator_ask") {
    auto low  = Quote(Money(1), Quantity(0), Side::ASK);
    auto high = Quote(Money(2), Quantity(0), Side::ASK);
    REQUIRE(listing_comparator(low, high));
    REQUIRE_FALSE(listing_comparator(high, low));
  }

  TEST_CASE("listing_comparator_bid") {
    auto low  = Quote(Money(1), Quantity(0), Side::BID);
    auto high = Quote(Money(2), Quantity(0), Side::BID);
    REQUIRE(listing_comparator(high, low));
    REQUIRE_FALSE(listing_comparator(low, high));
  }

  TEST_CASE("offer_comparator") {
    REQUIRE(offer_comparator(Side::ASK, Money(1), Money(2)) < 0);
    REQUIRE(offer_comparator(Side::ASK, Money(2), Money(1)) > 0);
    REQUIRE(offer_comparator(Side::ASK, Money(1), Money(1)) == 0);
    REQUIRE(offer_comparator(Side::BID, Money(1), Money(2)) > 0);
    REQUIRE(offer_comparator(Side::BID, Money(2), Money(1)) < 0);
    REQUIRE(offer_comparator(Side::BID, Money(1), Money(1)) == 0);
  }

  TEST_CASE("stream") {
    auto quote = Quote(Money(1), Quantity(100), Side::BID);
    REQUIRE(to_string(quote) == "(1.00 100 BID)");
    test_round_trip_shuttle(quote);
  }
}
