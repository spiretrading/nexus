#include <doctest/doctest.h>
#include "Nexus/Definitions/Quote.hpp"
 
using namespace Nexus;

TEST_SUITE("Quote") {
  TEST_CASE("default_constructor") {
    auto default_quote = Quote();
    CHECK(default_quote.m_price == Money::ZERO);
    CHECK(default_quote.m_size == 0);
    CHECK(default_quote.m_side == Side::NONE);
  }

  TEST_CASE("constructor") {
    auto price = Money::ONE;
    auto size = 100;
    auto side = Side::ASK;
    auto quote = Quote(price, size, side);
    CHECK(quote.m_price == price);
    CHECK(quote.m_size == size);
    CHECK(quote.m_side == side);
  }

  TEST_CASE("equals") {
    auto q1 = Quote(Money(1), Quantity(100), Side::BID);
    auto q2 = Quote(Money(1), Quantity(100), Side::BID);
    auto q3 = Quote(Money(2), Quantity(100), Side::BID);
    CHECK(q1 == q2);
    CHECK_FALSE(q1 == q3);
  }

  TEST_CASE("listing_comparator_ask") {
    auto low  = Quote(Money(1), Quantity(0), Side::ASK);
    auto high = Quote(Money(2), Quantity(0), Side::ASK);
    CHECK(listing_comparator(low, high));
    CHECK_FALSE(listing_comparator(high, low));
  }

  TEST_CASE("listing_comparator_bid") {
    auto low  = Quote(Money(1), Quantity(0), Side::BID);
    auto high = Quote(Money(2), Quantity(0), Side::BID);
    CHECK(listing_comparator(high, low));
    CHECK_FALSE(listing_comparator(low, high));
  }

  TEST_CASE("offer_comparator") {
    CHECK(offer_comparator(Side::ASK, Money(1), Money(2)) < 0);
    CHECK(offer_comparator(Side::ASK, Money(2), Money(1)) > 0);
    CHECK(offer_comparator(Side::ASK, Money(1), Money(1)) == 0);
    CHECK(offer_comparator(Side::BID, Money(1), Money(2)) > 0);
    CHECK(offer_comparator(Side::BID, Money(2), Money(1)) < 0);
    CHECK(offer_comparator(Side::BID, Money(1), Money(1)) == 0);
  }

  TEST_CASE("stream") {
    auto quote = Quote(Money(1), Quantity(100), Side::BID);
    auto ss = std::ostringstream();
    ss << quote;
    CHECK(ss.str() == "(1.00 100 BID)");
  }
}
