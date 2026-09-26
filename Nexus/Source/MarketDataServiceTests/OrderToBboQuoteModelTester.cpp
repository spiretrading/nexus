#include <doctest/doctest.h>
#include "Nexus/MarketDataService/OrderToBboQuoteModel.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  using Model = OrderToBboQuoteModel<BookQuoteOrderAdapter<int>>;
  struct Order {
    int m_price;
    Quantity m_quantity;
    std::string m_mpid;
  };

  struct Adapter {
    using OrderId = int;
    using Order = ::Order;
    const int* m_scale;

    BookQuote make_quote(const Order& order, Side side, ptime timestamp) const {
      return BookQuote(order.m_mpid, false, Venue("XTSE"),
        Quote(Money(order.m_price) / *m_scale, order.m_quantity, side),
        timestamp);
    }
  };

  struct Fixture {
    Model m_model;
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
  TEST_CASE("original_orders") {
    auto side = Side(Side::BID);
    SUBCASE("bid") {}
    SUBCASE("ask") {
      side = Side::ASK;
    }
    auto opposite = get_opposite(side);
    auto scale = 1;
    auto model = OrderToBboQuoteModel(Adapter(&scale));
    auto timestamp = time_from_string("2026-09-25 10:00:00");
    auto order = Order(10, 0, "MPID1");
    auto update = model.add(side, 1, order, timestamp);
    REQUIRE(update.m_quotes.empty());
    REQUIRE_FALSE(update.m_is_bbo_changed);
    REQUIRE(model.get_book(side).find_order(1)->m_quantity == 0);
    REQUIRE(model.add(opposite, 1, Order(11, 200, "MPID2"),
      timestamp).m_is_bbo_changed);
    auto expected = BookQuote("MPID1", false, Venue("XTSE"),
      Quote(Money(10), 100, side), timestamp);
    update = model.update(side, 1, [] (auto& order) {
      order.m_quantity = 100;
    }, timestamp);
    REQUIRE(update.m_quotes == Model::Book::Updates{expected});
    REQUIRE(update.m_is_bbo_changed);
    REQUIRE(model.get_book(side).find_order(1)->m_quantity == 100);
    REQUIRE(model.get_book(opposite).find_order(1)->m_quantity == 200);
    REQUIRE(model.get_book(side).find_order(1)->m_mpid == "MPID1");
    auto& bbo = model.get_bbo();
    REQUIRE(pick(side, bbo.m_ask, bbo.m_bid) == expected.m_quote);
    REQUIRE(pick(opposite, bbo.m_ask, bbo.m_bid) ==
      Quote(Money(11), 200, opposite));
    timestamp += seconds(1);
    update = model.update(side, 1, [] (auto& order) {
      order.m_quantity -= 40;
    }, timestamp);
    REQUIRE(update.m_is_bbo_changed);
    REQUIRE(pick(side, bbo.m_ask, bbo.m_bid).m_size == 60);
    REQUIRE(bbo.m_timestamp == timestamp);
    REQUIRE(model.remove(side, 1, timestamp).m_is_bbo_changed);
    REQUIRE_FALSE(model.get_book(side).find_order(1).has_value());
    REQUIRE(model.get_book(opposite).find_order(1).has_value());
    REQUIRE(pick(side, bbo.m_ask, bbo.m_bid).m_size == 0);
    REQUIRE(pick(opposite, bbo.m_ask, bbo.m_bid).m_size == 200);
    update = model.update(side, 1, [] (auto&) {
      FAIL("An absent order must not be updated.");
    }, timestamp);
    REQUIRE(update.m_quotes.empty());
    REQUIRE_FALSE(update.m_is_bbo_changed);
    REQUIRE_FALSE(model.remove(side, 1, timestamp).m_is_bbo_changed);
    model.add(side, 1, order, timestamp);
    REQUIRE(model.clear(timestamp).m_is_bbo_changed);
    REQUIRE(model.get_book(side).get_orders().empty());
    REQUIRE(model.get_book(opposite).get_orders().empty());
  }

  TEST_CASE("adapter") {
    auto scale = 1;
    auto model = OrderToBboQuoteModel(Adapter(&scale));
    auto timestamp = time_from_string("2026-09-25 10:00:00");
    model.add(Side::BID, 1, Order(10, 100, "MPID1"), timestamp);
    model.add(Side::ASK, 1, Order(20, 200, "MPID2"), timestamp);
    model.add(Side::ASK, 2, Order(15, 0, "MPID3"), timestamp);
    auto next_scale = 2;
    timestamp += seconds(1);
    auto update = model.set_adapter(Adapter(&next_scale), timestamp);
    REQUIRE(model.get_adapter().m_scale == &next_scale);
    REQUIRE(model.get_book(Side::ASK).get_adapter().m_scale == &next_scale);
    REQUIRE(update.m_quotes.size() == 4);
    REQUIRE(update.m_is_bbo_changed);
    REQUIRE(model.get_bbo() == BboQuote(Quote(Money(5), 100, Side::BID),
      Quote(Money(10), 200, Side::ASK), timestamp));
    REQUIRE(model.get_book(Side::BID).find_order(1)->m_price == 10);
    REQUIRE(model.get_book(Side::ASK).find_order(1)->m_price == 20);
    REQUIRE(model.get_book(Side::ASK).find_order(2)->m_price == 15);
    next_scale = 5;
    timestamp += seconds(1);
    update = model.refresh(timestamp);
    REQUIRE(update.m_quotes.size() == 4);
    REQUIRE(update.m_is_bbo_changed);
    REQUIRE(model.get_bbo() == BboQuote(Quote(Money(2), 100, Side::BID),
      Quote(Money(4), 200, Side::ASK), timestamp));
    update = model.update(Side::ASK, 2, [] (auto& order) {
      order.m_quantity = 50;
    }, timestamp);
    REQUIRE(update.m_quotes.size() == 1);
    REQUIRE(update.m_quotes.front().m_mpid == "MPID3");
    REQUIRE(update.m_quotes.front().m_quote == Quote(Money(3), 50, Side::ASK));
    REQUIRE(update.m_is_bbo_changed);
    REQUIRE(model.get_bbo().m_ask == Quote(Money(3), 50, Side::ASK));
    auto previous = model.get_bbo();
    timestamp += seconds(1);
    update = model.refresh(timestamp);
    REQUIRE(update.m_quotes.empty());
    REQUIRE_FALSE(update.m_is_bbo_changed);
    REQUIRE(model.get_bbo() == previous);
  }

  TEST_CASE("default_bbo") {
    auto model = Model();
    REQUIRE(model.get_bbo() == BboQuote());
  }

  TEST_CASE("book_updates") {
    auto side = Side(Side::BID);
    auto worse = Money(9);
    SUBCASE("bid") {}
    SUBCASE("ask") {
      side = Side::ASK;
      worse = Money(11);
    }
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    auto& book = model.get_book(side);
    auto& opposite_book = model.get_book(get_opposite(side));
    REQUIRE(book.get_side() == side);
    REQUIRE(book.empty());
    REQUIRE(opposite_book.empty());
    auto expected = fixture.order(side, Money(10), 100);
    auto update = model.add(1, expected);
    auto& quotes = update.m_quotes;
    REQUIRE(quotes == Model::Book::Updates{expected});
    REQUIRE(update.m_is_bbo_changed);
    REQUIRE(book.find_order(1).has_value());
    REQUIRE(*book.find_order(1) == expected);
    REQUIRE(book[0] == expected);
    update = model.add(2, fixture.order(side, Money(10), 200));
    expected.m_quote.m_size = 300;
    REQUIRE(quotes == Model::Book::Updates{expected});
    REQUIRE(update.m_is_bbo_changed);
    auto lower = fixture.order(side, worse, 50);
    update = model.add(3, lower);
    REQUIRE(quotes == Model::Book::Updates{lower});
    REQUIRE_FALSE(update.m_is_bbo_changed);
    fixture.m_timestamp += seconds(1);
    expected.m_timestamp = fixture.m_timestamp;
    update = model.modify_size(1, 125, fixture.m_timestamp);
    expected.m_quote.m_size = 325;
    REQUIRE(quotes == Model::Book::Updates{expected});
    REQUIRE(update.m_is_bbo_changed);
    update = model.offset_size(2, -25, fixture.m_timestamp);
    expected.m_quote.m_size = 300;
    REQUIRE(quotes == Model::Book::Updates{expected});
    REQUIRE(update.m_is_bbo_changed);
    update = model.modify_price(1, worse, fixture.m_timestamp);
    expected.m_quote.m_size = 175;
    lower.m_quote.m_size = 175;
    lower.m_timestamp = fixture.m_timestamp;
    REQUIRE(quotes ==
      Model::Book::Updates{expected, lower});
    REQUIRE(update.m_is_bbo_changed);
    REQUIRE(book.find_order(1)->m_quote == Quote(worse, 125, side));
    REQUIRE(book.get_orders().size() == 3);
    REQUIRE(book.size() == 2);
    update = model.remove(2, fixture.m_timestamp);
    expected.m_quote.m_size = 0;
    REQUIRE(quotes == Model::Book::Updates{expected});
    REQUIRE(update.m_is_bbo_changed);
    REQUIRE_FALSE(book.find_order(2).has_value());
    update = model.clear(fixture.m_timestamp);
    lower.m_quote.m_size = 0;
    REQUIRE(quotes == Model::Book::Updates{lower});
    REQUIRE(update.m_is_bbo_changed);
    REQUIRE(book.empty());
    REQUIRE(book.get_orders().empty());
    REQUIRE(opposite_book.empty());
    update = model.remove(1, fixture.m_timestamp);
    REQUIRE(quotes.empty());
    REQUIRE_FALSE(update.m_is_bbo_changed);
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
    REQUIRE(model.add(1, order).m_is_bbo_changed);
    order.m_quote.m_size = 50;
    REQUIRE(model.add(2, order).m_is_bbo_changed);
    order.m_mpid = "MPID2";
    order.m_quote.m_size = 200;
    REQUIRE(model.add(3, order).m_is_bbo_changed);
    order.m_mpid = "MPID1";
    order.m_venue = Venue("CHIC");
    order.m_quote.m_size = 75;
    REQUIRE(model.add(4, order).m_is_bbo_changed);
    REQUIRE(fixture.quote(side) == Quote(Money(10), 425, side));
    fixture.m_timestamp += seconds(1);
    REQUIRE(model.modify_size(1, 125, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(fixture.quote(side) == Quote(Money(10), 450, side));
    REQUIRE(model.get_bbo().m_timestamp == fixture.m_timestamp);
    REQUIRE(model.offset_size(3, -50, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(fixture.quote(side) == Quote(Money(10), 400, side));
    REQUIRE(model.remove(4, fixture.m_timestamp).m_is_bbo_changed);
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
    REQUIRE_FALSE(
      model.add(2, fixture.order(side, worse, 200)).m_is_bbo_changed);
    REQUIRE_FALSE(
      model.modify_size(2, 250, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE_FALSE(
      model.offset_size(2, 50, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(model.get_bbo() == previous);
    REQUIRE(
      model.modify_price(2, better, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(fixture.quote(side) == Quote(better, 300, side));
    REQUIRE(model.get_bbo().m_timestamp == fixture.m_timestamp);
    REQUIRE(
      model.modify_price(2, Money(10), fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(fixture.quote(side) == Quote(Money(10), 400, side));
    REQUIRE(model.modify_price(2, worse, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(fixture.quote(side) == Quote(Money(10), 100, side));
    previous = model.get_bbo();
    fixture.m_timestamp += seconds(1);
    REQUIRE_FALSE(model.remove(2, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(model.get_bbo() == previous);
    REQUIRE(model.add(3, fixture.order(side, better, 50)).m_is_bbo_changed);
    REQUIRE(fixture.quote(side) == Quote(better, 50, side));
    REQUIRE(model.remove(3, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(fixture.quote(side) == Quote(Money(10), 100, side));
  }

  TEST_CASE("replacement") {
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    auto order = fixture.order(Side::BID, Money(10), 100);
    model.add(1, order);
    model.add(2, order);
    auto ask = fixture.order(Side::ASK, Money(11), 200);
    model.add(3, ask);
    auto previous = model.get_bbo();
    fixture.m_timestamp += seconds(1);
    order.m_timestamp = fixture.m_timestamp;
    REQUIRE_FALSE(model.add(1, order).m_is_bbo_changed);
    order.m_mpid = "MPID2";
    REQUIRE_FALSE(model.add(1, order).m_is_bbo_changed);
    order.m_venue = Venue("CHIC");
    REQUIRE_FALSE(model.add(1, order).m_is_bbo_changed);
    order.m_is_primary_mpid = true;
    REQUIRE_FALSE(model.add(1, order).m_is_bbo_changed);
    REQUIRE(model.get_bbo() == previous);
    order.m_quote.m_size = 150;
    auto update = model.add(1, order);
    REQUIRE(update.m_is_bbo_changed);
    REQUIRE(update.m_quotes == Model::Book::Updates{order});
    REQUIRE(model.get_bbo().m_ask == ask.m_quote);
    REQUIRE(*model.get_book(Side::ASK).find_order(3) == ask);
    REQUIRE(model.get_bbo().m_bid == Quote(Money(10), 250, Side::BID));
    REQUIRE(model.get_bbo().m_timestamp == fixture.m_timestamp);
    REQUIRE(model.remove(1, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(model.get_bbo().m_bid == Quote(Money(10), 100, Side::BID));
  }

  TEST_CASE("nonpositive_size") {
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    model.add(1, fixture.order(Side::BID, Money(10), 100));
    model.add(2, fixture.order(Side::ASK, Money(10), 200));
    fixture.m_timestamp += seconds(1);
    SUBCASE("zero") {
      REQUIRE(model.modify_size(1, 0, fixture.m_timestamp).m_is_bbo_changed);
    }
    SUBCASE("negative") {
      REQUIRE(model.modify_size(1, -50, fixture.m_timestamp).m_is_bbo_changed);
    }
    SUBCASE("offset") {
      REQUIRE(model.offset_size(1, -150, fixture.m_timestamp).m_is_bbo_changed);
    }
    SUBCASE("replacement") {
      REQUIRE(
        model.add(1, fixture.order(Side::BID, Money(10), 0)).m_is_bbo_changed);
    }
    REQUIRE(model.get_bbo() == BboQuote(Quote(Money::ZERO, 0, Side::BID),
      Quote(Money(10), 200, Side::ASK), fixture.m_timestamp));
    REQUIRE_FALSE(model.remove(1, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(model.offset_size(2, -200, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(model.get_bbo().m_ask == Quote(Money::ZERO, 0, Side::ASK));
  }

  TEST_CASE("unchanged_order") {
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    model.add(1, fixture.order(Side::ASK, Money(10), 100));
    auto previous = model.get_bbo();
    fixture.m_timestamp += seconds(1);
    REQUIRE_FALSE(
      model.modify_size(1, 100, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE_FALSE(
      model.offset_size(1, 0, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE_FALSE(
      model.modify_price(1, Money(10), fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE_FALSE(
      model.modify_size(2, 200, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE_FALSE(
      model.offset_size(2, 50, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE_FALSE(
      model.modify_price(2, Money(11), fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE_FALSE(model.remove(2, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE_FALSE(
      model.add(2, fixture.order(Side::BID, Money(10), 0)).m_is_bbo_changed);
    REQUIRE(model.get_bbo() == previous);
  }

  TEST_CASE("clear") {
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    REQUIRE_FALSE(model.clear(fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(model.get_bbo() == BboQuote());
    model.add(1, fixture.order(Side::BID, Money(10), 100));
    model.add(2, fixture.order(Side::BID, Money(9), 200));
    model.add(3, fixture.order(Side::ASK, Money(11), 300));
    model.add(4, fixture.order(Side::ASK, Money(12), 400));
    fixture.m_timestamp += seconds(1);
    auto update = model.clear(fixture.m_timestamp);
    REQUIRE(update.m_is_bbo_changed);
    REQUIRE(update.m_quotes == Model::Book::Updates{
      fixture.order(Side::BID, Money(10), 0),
      fixture.order(Side::BID, Money(9), 0),
      fixture.order(Side::ASK, Money(11), 0),
      fixture.order(Side::ASK, Money(12), 0)});
    REQUIRE(model.get_bbo() == BboQuote(Quote(Money::ZERO, 0, Side::BID),
      Quote(Money::ZERO, 0, Side::ASK), fixture.m_timestamp));
    auto previous = model.get_bbo();
    fixture.m_timestamp += seconds(1);
    REQUIRE_FALSE(model.clear(fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE_FALSE(model.remove(1, fixture.m_timestamp).m_is_bbo_changed);
    REQUIRE(model.get_bbo() == previous);
    REQUIRE(
      model.add(1, fixture.order(Side::ASK, Money(11), 50)).m_is_bbo_changed);
    REQUIRE(model.get_bbo().m_ask == Quote(Money(11), 50, Side::ASK));
    REQUIRE(model.get_bbo().m_timestamp == fixture.m_timestamp);
  }
}
