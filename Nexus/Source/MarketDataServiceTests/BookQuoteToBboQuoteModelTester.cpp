#include <doctest/doctest.h>
#include "Nexus/MarketDataService/BookQuoteToBboQuoteModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

namespace {
  BookQuote make_book_quote(Side side, Money price, Quantity size,
      std::string mpid, ptime timestamp) {
    return BookQuote(
      std::move(mpid), false, Venue(), Quote(price, size, side), timestamp);
  }
}

TEST_SUITE("BookQuoteToBboQuoteModel") {
  TEST_CASE("default_bbo") {
    auto model = BookQuoteToBboQuoteModel();
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money::ZERO);
    REQUIRE(bbo.m_bid.m_size == 0);
    REQUIRE(bbo.m_bid.m_side == Side::BID);
    REQUIRE(bbo.m_ask.m_price == Money::ZERO);
    REQUIRE(bbo.m_ask.m_size == 0);
    REQUIRE(bbo.m_ask.m_side == Side::ASK);
  }

  TEST_CASE("single_bid_update") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    auto quote = make_book_quote(Side::BID, Money(10), 100, "MPID1", timestamp);
    auto changed = model.update(quote);
    REQUIRE(changed);
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money(10));
    REQUIRE(bbo.m_bid.m_size == 100);
    REQUIRE(bbo.m_timestamp == timestamp);
  }

  TEST_CASE("single_ask_update") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    auto quote = make_book_quote(Side::ASK, Money(11), 200, "MPID1", timestamp);
    auto changed = model.update(quote);
    REQUIRE(changed);
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_ask.m_price == Money(11));
    REQUIRE(bbo.m_ask.m_size == 200);
    REQUIRE(bbo.m_timestamp == timestamp);
  }

  TEST_CASE("best_bid_by_price") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    model.update(
      make_book_quote(Side::BID, Money(10), 100, "MPID1", timestamp));
    model.update(make_book_quote(Side::BID, Money(11), 50, "MPID2", timestamp));
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money(11));
    REQUIRE(bbo.m_bid.m_size == 50);
  }

  TEST_CASE("best_ask_by_price") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    model.update(
      make_book_quote(Side::ASK, Money(12), 100, "MPID1", timestamp));
    model.update(make_book_quote(Side::ASK, Money(11), 50, "MPID2", timestamp));
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_ask.m_price == Money(11));
    REQUIRE(bbo.m_ask.m_size == 50);
  }

  TEST_CASE("aggregate_size") {
    auto side = Side(Side::BID);
    SUBCASE("bid") {}
    SUBCASE("ask") {
      side = Side::ASK;
    }
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00");
    model.update(make_book_quote(side, Money(10), 100, "MPID1", timestamp));
    REQUIRE(model.update(
      make_book_quote(side, Money(10), 200, "MPID2", timestamp)));
    auto quote = [&] {
      return pick(side, model.get_bbo().m_ask, model.get_bbo().m_bid);
    };
    REQUIRE(quote() == Quote(Money(10), 300, side));
    timestamp += seconds(1);
    REQUIRE(model.update(
      make_book_quote(side, Money(10), 300, "MPID1", timestamp)));
    REQUIRE(quote() == Quote(Money(10), 500, side));
    REQUIRE(model.get_bbo().m_timestamp == timestamp);
    auto previous = model.get_bbo();
    timestamp += seconds(1);
    REQUIRE_FALSE(model.update(
      make_book_quote(side, Money(10), 200, "MPID2", timestamp)));
    REQUIRE(model.get_bbo() == previous);
    REQUIRE(model.update(
      make_book_quote(side, Money(10), 50, "MPID1", timestamp)));
    REQUIRE(quote() == Quote(Money(10), 250, side));
    REQUIRE(model.update(
      make_book_quote(side, Money(10), 0, "MPID1", timestamp)));
    REQUIRE(quote() == Quote(Money(10), 200, side));
    REQUIRE(model.update(
      make_book_quote(side, Money(10), 0, "MPID2", timestamp)));
    REQUIRE(quote() == Quote(Money::ZERO, 0, side));
  }

  TEST_CASE("same_mpid_across_venues") {
    auto side = Side(Side::BID);
    SUBCASE("bid") {}
    SUBCASE("ask") {
      side = Side::ASK;
    }
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00");
    auto first = BookQuote("MPID1", false, Venue("XTSE"),
      Quote(Money(10), 100, side), timestamp);
    auto second = BookQuote("MPID1", false, Venue("CHIC"),
      Quote(Money(10), 200, side), timestamp);
    REQUIRE(model.update(first));
    REQUIRE(model.update(second));
    auto quote = [&] {
      return pick(side, model.get_bbo().m_ask, model.get_bbo().m_bid);
    };
    REQUIRE(quote() == Quote(Money(10), 300, side));
    first.m_quote.m_size = 150;
    REQUIRE(model.update(first));
    REQUIRE(quote() == Quote(Money(10), 350, side));
    second.m_quote.m_size = 250;
    REQUIRE(model.update(second));
    REQUIRE(quote() == Quote(Money(10), 400, side));
    first.m_quote.m_size = 0;
    REQUIRE(model.update(first));
    REQUIRE(quote() == Quote(Money(10), 250, side));
    REQUIRE_FALSE(model.update(first));
    REQUIRE(quote() == Quote(Money(10), 250, side));
    second.m_quote.m_size = 0;
    REQUIRE(model.update(second));
    REQUIRE(quote() == Quote(Money::ZERO, 0, side));
  }

  TEST_CASE("replace_existing_quote") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp1 = time_from_string("2025-01-15 10:00:00.000");
    auto timestamp2 = time_from_string("2025-01-15 10:00:01.000");
    model.update(
      make_book_quote(Side::BID, Money(10), 100, "MPID1", timestamp1));
    auto changed = model.update(
      make_book_quote(Side::BID, Money(10), 150, "MPID1", timestamp2));
    REQUIRE(changed);
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money(10));
    REQUIRE(bbo.m_bid.m_size == 150);
    REQUIRE(bbo.m_timestamp == timestamp2);
  }

  TEST_CASE("delete_quote") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    model.update(
      make_book_quote(Side::BID, Money(10), 100, "MPID1", timestamp));
    model.update(make_book_quote(Side::BID, Money(9), 50, "MPID2", timestamp));
    auto changed = model.update(
      make_book_quote(Side::BID, Money(10), 0, "MPID1", timestamp));
    REQUIRE(changed);
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money(9));
    REQUIRE(bbo.m_bid.m_size == 50);
  }

  TEST_CASE("delete_nonexistent_quote") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    auto changed = model.update(
      make_book_quote(Side::BID, Money(10), 0, "MPID1", timestamp));
    REQUIRE_FALSE(changed);
  }

  TEST_CASE("delete_last_quote") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    model.update(
      make_book_quote(Side::BID, Money(10), 100, "MPID1", timestamp));
    auto changed = model.update(
      make_book_quote(Side::BID, Money(10), 0, "MPID1", timestamp));
    REQUIRE(changed);
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money::ZERO);
    REQUIRE(bbo.m_bid.m_size == 0);
  }

  TEST_CASE("no_change_when_not_at_best_price") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    model.update(
      make_book_quote(Side::BID, Money(10), 100, "MPID1", timestamp));
    auto changed = model.update(
      make_book_quote(Side::BID, Money(8), 50, "MPID2", timestamp));
    REQUIRE_FALSE(changed);
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money(10));
    REQUIRE(bbo.m_bid.m_size == 100);
  }

  TEST_CASE("update_at_best_price_with_smaller_size") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    model.update(
      make_book_quote(Side::BID, Money(10), 200, "MPID1", timestamp));
    model.update(
      make_book_quote(Side::BID, Money(10), 100, "MPID2", timestamp));
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money(10));
    REQUIRE(bbo.m_bid.m_size == 300);
  }

  TEST_CASE("bid_and_ask_independent") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    model.update(
      make_book_quote(Side::BID, Money(10), 100, "MPID1", timestamp));
    model.update(
      make_book_quote(Side::ASK, Money(11), 200, "MPID2", timestamp));
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money(10));
    REQUIRE(bbo.m_bid.m_size == 100);
    REQUIRE(bbo.m_ask.m_price == Money(11));
    REQUIRE(bbo.m_ask.m_size == 200);
  }

  TEST_CASE("delete_at_non_best_price") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    model.update(
      make_book_quote(Side::BID, Money(10), 100, "MPID1", timestamp));
    model.update(
      make_book_quote(Side::BID, Money(8), 50, "MPID2", timestamp));
    auto changed = model.update(
      make_book_quote(Side::BID, Money(8), 0, "MPID2", timestamp));
    REQUIRE_FALSE(changed);
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money(10));
    REQUIRE(bbo.m_bid.m_size == 100);
  }

  TEST_CASE("replace_best_with_smaller_size") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    model.update(
      make_book_quote(Side::BID, Money(10), 200, "MPID1", timestamp));
    model.update(
      make_book_quote(Side::BID, Money(10), 150, "MPID2", timestamp));
    auto changed = model.update(
      make_book_quote(Side::BID, Money(10), 50, "MPID1", timestamp));
    REQUIRE(changed);
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money(10));
    REQUIRE(bbo.m_bid.m_size == 200);
  }

  TEST_CASE("new_better_price_replaces_bbo") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    model.update(
      make_book_quote(Side::ASK, Money(12), 100, "MPID1", timestamp));
    auto changed = model.update(
      make_book_quote(Side::ASK, Money(11), 50, "MPID2", timestamp));
    REQUIRE(changed);
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_ask.m_price == Money(11));
    REQUIRE(bbo.m_ask.m_size == 50);
  }

  TEST_CASE("multiple_mpids_delete_best_size") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp = time_from_string("2025-01-15 10:00:00.000");
    model.update(
      make_book_quote(Side::BID, Money(10), 300, "MPID1", timestamp));
    model.update(
      make_book_quote(Side::BID, Money(10), 200, "MPID2", timestamp));
    model.update(
      make_book_quote(Side::BID, Money(10), 100, "MPID3", timestamp));
    auto changed = model.update(
      make_book_quote(Side::BID, Money(10), 0, "MPID1", timestamp));
    REQUIRE(changed);
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money(10));
    REQUIRE(bbo.m_bid.m_size == 300);
  }

  TEST_CASE("replace_smaller_mpid") {
    auto model = BookQuoteToBboQuoteModel();
    auto timestamp1 = time_from_string("2025-01-15 10:00:00.000");
    auto timestamp2 = time_from_string("2025-01-15 10:00:01.000");
    model.update(
      make_book_quote(Side::BID, Money(10), 200, "MPID1", timestamp1));
    model.update(
      make_book_quote(Side::BID, Money(10), 50, "MPID2", timestamp1));
    auto changed = model.update(
      make_book_quote(Side::BID, Money(10), 100, "MPID2", timestamp2));
    REQUIRE(changed);
    auto bbo = model.get_bbo();
    REQUIRE(bbo.m_bid.m_price == Money(10));
    REQUIRE(bbo.m_bid.m_size == 300);
  }
}
