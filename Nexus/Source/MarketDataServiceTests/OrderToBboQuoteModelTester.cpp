#include <doctest/doctest.h>
#include "Nexus/MarketDataService/OrderToBboQuoteModel.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct Fixture {
    OrderToBboQuoteModel<int> m_model;
    ptime m_timestamp = time_from_string("2026-09-25 10:00:00");

    BookQuote order(Side side, Money price, Quantity size) {
      return BookQuote("MPID1", false, Venue("XTSE"),
        Quote(price, size, side), m_timestamp);
    }

    const Quote& quote(Side side) const {
      return pick(side, m_model.get_bbo().m_ask, m_model.get_bbo().m_bid);
    }
  };
}

TEST_SUITE("OrderToBboQuoteModel") {
  TEST_CASE("default_bbo") {
    auto model = OrderToBboQuoteModel<int>();
    REQUIRE(model.get_bbo() == BboQuote());
  }

  TEST_CASE("aggregate") {
    auto side = Side(Side::BID);
    SUBCASE("bid") {}
    SUBCASE("ask") {
      side = Side::ASK;
    }
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    auto order = fixture.order(side, Money(10), 100);
    REQUIRE(model.add(1, order));
    order.m_quote.m_size = 50;
    REQUIRE(model.add(2, order));
    order.m_mpid = "MPID2";
    order.m_quote.m_size = 200;
    REQUIRE(model.add(3, order));
    order.m_mpid = "MPID1";
    order.m_venue = Venue("CHIC");
    order.m_quote.m_size = 75;
    REQUIRE(model.add(4, order));
    REQUIRE(fixture.quote(side) == Quote(Money(10), 425, side));
    fixture.m_timestamp += seconds(1);
    REQUIRE(model.modify_size(1, 125, fixture.m_timestamp));
    REQUIRE(fixture.quote(side) == Quote(Money(10), 450, side));
    REQUIRE(model.get_bbo().m_timestamp == fixture.m_timestamp);
    REQUIRE(model.offset_size(3, -50, fixture.m_timestamp));
    REQUIRE(fixture.quote(side) == Quote(Money(10), 400, side));
    REQUIRE(model.remove(4, fixture.m_timestamp));
    REQUIRE(fixture.quote(side) == Quote(Money(10), 325, side));
  }

  TEST_CASE("price") {
    auto side = Side(Side::BID);
    auto worse = Money(9);
    auto better = Money(11);
    SUBCASE("bid") {}
    SUBCASE("ask") {
      side = Side::ASK;
      std::swap(worse, better);
    }
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    model.add(1, fixture.order(side, Money(10), 100));
    auto previous = model.get_bbo();
    fixture.m_timestamp += seconds(1);
    REQUIRE_FALSE(model.add(2, fixture.order(side, worse, 200)));
    REQUIRE_FALSE(model.modify_size(2, 250, fixture.m_timestamp));
    REQUIRE_FALSE(model.offset_size(2, 50, fixture.m_timestamp));
    REQUIRE(model.get_bbo() == previous);
    REQUIRE(model.modify_price(2, better, fixture.m_timestamp));
    REQUIRE(fixture.quote(side) == Quote(better, 300, side));
    REQUIRE(model.get_bbo().m_timestamp == fixture.m_timestamp);
    REQUIRE(model.modify_price(2, Money(10), fixture.m_timestamp));
    REQUIRE(fixture.quote(side) == Quote(Money(10), 400, side));
    REQUIRE(model.modify_price(2, worse, fixture.m_timestamp));
    REQUIRE(fixture.quote(side) == Quote(Money(10), 100, side));
    previous = model.get_bbo();
    fixture.m_timestamp += seconds(1);
    REQUIRE_FALSE(model.remove(2, fixture.m_timestamp));
    REQUIRE(model.get_bbo() == previous);
    REQUIRE(model.add(3, fixture.order(side, better, 50)));
    REQUIRE(fixture.quote(side) == Quote(better, 50, side));
    REQUIRE(model.remove(3, fixture.m_timestamp));
    REQUIRE(fixture.quote(side) == Quote(Money(10), 100, side));
  }

  TEST_CASE("replacement") {
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    auto order = fixture.order(Side::BID, Money(10), 100);
    model.add(1, order);
    model.add(2, order);
    auto previous = model.get_bbo();
    fixture.m_timestamp += seconds(1);
    order.m_timestamp = fixture.m_timestamp;
    REQUIRE_FALSE(model.add(1, order));
    order.m_mpid = "MPID2";
    REQUIRE_FALSE(model.add(1, order));
    order.m_venue = Venue("CHIC");
    REQUIRE_FALSE(model.add(1, order));
    order.m_is_primary_mpid = true;
    REQUIRE_FALSE(model.add(1, order));
    REQUIRE(model.get_bbo() == previous);
    order.m_quote.m_size = 150;
    REQUIRE(model.add(1, order));
    REQUIRE(model.get_bbo().m_bid == Quote(Money(10), 250, Side::BID));
    REQUIRE(model.get_bbo().m_timestamp == fixture.m_timestamp);
    REQUIRE(model.remove(1, fixture.m_timestamp));
    REQUIRE(model.get_bbo().m_bid == Quote(Money(10), 100, Side::BID));
  }

  TEST_CASE("side_replacement") {
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    model.add(1, fixture.order(Side::BID, Money(10), 100));
    model.add(2, fixture.order(Side::BID, Money(9), 50));
    model.add(3, fixture.order(Side::ASK, Money(12), 300));
    fixture.m_timestamp += seconds(1);
    REQUIRE(model.add(1, fixture.order(Side::ASK, Money(11), 200)));
    REQUIRE(model.get_bbo() == BboQuote(Quote(Money(9), 50, Side::BID),
      Quote(Money(11), 200, Side::ASK), fixture.m_timestamp));
    REQUIRE(model.offset_size(1, 25, fixture.m_timestamp));
    REQUIRE(model.get_bbo().m_ask == Quote(Money(11), 225, Side::ASK));
    REQUIRE(model.get_bbo().m_bid == Quote(Money(9), 50, Side::BID));
    REQUIRE(model.add(1, fixture.order(Side::BID, Money(10), 75)));
    REQUIRE(model.get_bbo() == BboQuote(Quote(Money(10), 75, Side::BID),
      Quote(Money(12), 300, Side::ASK), fixture.m_timestamp));
    REQUIRE(model.remove(1, fixture.m_timestamp));
    REQUIRE(model.get_bbo().m_bid == Quote(Money(9), 50, Side::BID));
    REQUIRE(model.get_bbo().m_ask == Quote(Money(12), 300, Side::ASK));
  }

  TEST_CASE("nonpositive_size") {
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    model.add(1, fixture.order(Side::BID, Money(10), 100));
    model.add(2, fixture.order(Side::ASK, Money(10), 200));
    fixture.m_timestamp += seconds(1);
    SUBCASE("zero") {
      REQUIRE(model.modify_size(1, 0, fixture.m_timestamp));
    }
    SUBCASE("negative") {
      REQUIRE(model.modify_size(1, -50, fixture.m_timestamp));
    }
    SUBCASE("offset") {
      REQUIRE(model.offset_size(1, -150, fixture.m_timestamp));
    }
    SUBCASE("replacement") {
      REQUIRE(model.add(1, fixture.order(Side::BID, Money(10), 0)));
    }
    SUBCASE("opposite_side_replacement") {
      REQUIRE(model.add(1, fixture.order(Side::ASK, Money(11), 0)));
    }
    REQUIRE(model.get_bbo() == BboQuote(Quote(Money::ZERO, 0, Side::BID),
      Quote(Money(10), 200, Side::ASK), fixture.m_timestamp));
    REQUIRE_FALSE(model.remove(1, fixture.m_timestamp));
    REQUIRE(model.offset_size(2, -200, fixture.m_timestamp));
    REQUIRE(model.get_bbo().m_ask == Quote(Money::ZERO, 0, Side::ASK));
  }

  TEST_CASE("unchanged_order") {
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    model.add(1, fixture.order(Side::ASK, Money(10), 100));
    auto previous = model.get_bbo();
    fixture.m_timestamp += seconds(1);
    REQUIRE_FALSE(model.modify_size(1, 100, fixture.m_timestamp));
    REQUIRE_FALSE(model.offset_size(1, 0, fixture.m_timestamp));
    REQUIRE_FALSE(model.modify_price(1, Money(10), fixture.m_timestamp));
    REQUIRE_FALSE(model.modify_size(2, 200, fixture.m_timestamp));
    REQUIRE_FALSE(model.offset_size(2, 50, fixture.m_timestamp));
    REQUIRE_FALSE(model.modify_price(2, Money(11), fixture.m_timestamp));
    REQUIRE_FALSE(model.remove(2, fixture.m_timestamp));
    REQUIRE_FALSE(model.add(2, fixture.order(Side::BID, Money(10), 0)));
    REQUIRE(model.get_bbo() == previous);
  }

  TEST_CASE("clear") {
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    REQUIRE_FALSE(model.clear(fixture.m_timestamp));
    REQUIRE(model.get_bbo() == BboQuote());
    model.add(1, fixture.order(Side::BID, Money(10), 100));
    model.add(2, fixture.order(Side::BID, Money(9), 200));
    model.add(3, fixture.order(Side::ASK, Money(11), 300));
    model.add(4, fixture.order(Side::ASK, Money(12), 400));
    fixture.m_timestamp += seconds(1);
    REQUIRE(model.clear(fixture.m_timestamp));
    REQUIRE(model.get_bbo() == BboQuote(Quote(Money::ZERO, 0, Side::BID),
      Quote(Money::ZERO, 0, Side::ASK), fixture.m_timestamp));
    auto previous = model.get_bbo();
    fixture.m_timestamp += seconds(1);
    REQUIRE_FALSE(model.clear(fixture.m_timestamp));
    REQUIRE_FALSE(model.remove(1, fixture.m_timestamp));
    REQUIRE(model.get_bbo() == previous);
    REQUIRE(model.add(1, fixture.order(Side::ASK, Money(11), 50)));
    REQUIRE(model.get_bbo().m_ask == Quote(Money(11), 50, Side::ASK));
    REQUIRE(model.get_bbo().m_timestamp == fixture.m_timestamp);
  }
}
