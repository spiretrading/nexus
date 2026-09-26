#include <doctest/doctest.h>
#include "Nexus/MarketDataService/BookQuoteToBboQuoteModel.hpp"
#include "Nexus/MarketDataService/OrderToBookQuoteModel.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct Fixture {
    OrderToBookQuoteModel<int> m_model;
    ptime m_timestamp;

    Fixture()
      : Fixture(Side::BID) {}

    explicit Fixture(Side side)
      : m_model(side),
        m_timestamp(time_from_string("2026-09-25 10:00:00")) {}


    BookQuote order(Quantity size) {
      return BookQuote("MPID1", false, Venue("XTSE"),
        Quote(Money(10), size, m_model.get_side()), m_timestamp);
    }
  };
}

TEST_SUITE("OrderToBookQuoteModel") {
  TEST_CASE("side") {
    auto bids = Fixture(Side::BID);
    auto asks = Fixture(Side::ASK);
    REQUIRE(bids.m_model.get_side() == Side::BID);
    REQUIRE(asks.m_model.get_side() == Side::ASK);
    bids.m_model.add(1, bids.order(100));
    asks.m_model.add(1, asks.order(200));
    REQUIRE(bids.m_model[0] == bids.order(100));
    REQUIRE(asks.m_model[0] == asks.order(200));
  }

  TEST_CASE("orders") {
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    REQUIRE_FALSE(model.find_order(1));
    REQUIRE(model.get_orders().empty());
    auto first = fixture.order(100);
    auto second = fixture.order(200);
    model.add(1, first);
    model.add(2, second);
    REQUIRE(model.find_order(1));
    REQUIRE(*model.find_order(1) == first);
    REQUIRE(model.get_orders() ==
      OrderToBookQuoteModel<int>::Orders{{1, first}, {2, second}});
    REQUIRE(model[0].m_quote.m_size == 300);
    fixture.m_timestamp += seconds(1);
    model.modify_size(1, 150, fixture.m_timestamp);
    first.m_quote.m_size = 150;
    first.m_timestamp = fixture.m_timestamp;
    REQUIRE(*model.find_order(1) == first);
    model.modify_price(1, Money(11), fixture.m_timestamp);
    first.m_quote.m_price = Money(11);
    REQUIRE(model.get_orders().at(1) == first);
    second.m_mpid = "MPID2";
    model.add(2, second);
    REQUIRE(*model.find_order(2) == second);
    model.remove(1, fixture.m_timestamp);
    REQUIRE_FALSE(model.find_order(1));
    REQUIRE(model.get_orders().size() == 1);
    model.clear(fixture.m_timestamp);
    REQUIRE_FALSE(model.find_order(2));
    REQUIRE(model.get_orders().empty());
  }

  TEST_CASE("book") {
    auto side = Side(Side::BID);
    SUBCASE("bid") {}
    SUBCASE("ask") {
      side = Side::ASK;
    }
    auto fixture = Fixture(side);
    auto& model = fixture.m_model;
    REQUIRE(model.empty());
    REQUIRE(model.size() == 0);
    REQUIRE(model.begin() == model.end());
    auto first = fixture.order(100);
    auto second = fixture.order(200);
    second.m_quote.m_price = Money(12);
    auto third = fixture.order(300);
    third.m_quote.m_price = Money(11);
    model.add(1, first);
    model.add(2, second);
    model.add(3, third);
    auto expected = std::vector<BookQuote>{first, third, second};
    if(side == Side::BID) {
      std::ranges::reverse(expected);
    }
    REQUIRE_FALSE(model.empty());
    REQUIRE(model.size() == 3);
    REQUIRE(std::ranges::equal(model, expected));
    for(auto i = std::size_t(0); i < expected.size(); ++i) {
      REQUIRE(model[i] == expected[i]);
    }
    model.remove(3, fixture.m_timestamp);
    expected.erase(expected.begin() + 1);
    REQUIRE(std::ranges::equal(model, expected));
    auto price = Money(9);
    if(side == Side::BID) {
      price = Money(13);
    }
    model.modify_price(1, price, fixture.m_timestamp);
    first.m_quote.m_price = price;
    REQUIRE(model.size() == 2);
    REQUIRE(model[0] == first);
    REQUIRE(model[1] == second);
    model.add(4, first);
    first.m_quote.m_size = 200;
    REQUIRE(model.size() == 2);
    REQUIRE(model[0] == first);
    auto updates = model.clear(fixture.m_timestamp);
    REQUIRE(updates.size() == 2);
    REQUIRE(updates[0].m_quote.m_price == first.m_quote.m_price);
    REQUIRE(updates[1].m_quote.m_price == second.m_quote.m_price);
    REQUIRE(model.empty());
    REQUIRE(model.begin() == model.end());
  }

  TEST_CASE("book_entry_input") {
    auto fixture = Fixture();
    auto& model = fixture.m_model;
    model.add(1, fixture.order(100));
    model.add(2, model[0]);
    REQUIRE(model[0] == fixture.order(200));
    model.add(1, model[0]);
    REQUIRE(model[0] == fixture.order(300));
    model.remove(1, fixture.m_timestamp);
    REQUIRE(model.size() == 1);
    REQUIRE(model[0] == fixture.order(100));
  }

  TEST_CASE("aggregate") {
    auto fixture = Fixture();
    auto order = fixture.order(100);
    auto updates = fixture.m_model.add(1, order);
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == order);
    updates = fixture.m_model.add(2, fixture.order(200));
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == fixture.order(300));
    auto timestamp = fixture.m_timestamp + seconds(1);
    updates = fixture.m_model.remove(1, timestamp);
    auto expected = fixture.order(200);
    expected.m_timestamp = timestamp;
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == expected);
    updates = fixture.m_model.remove(2, timestamp);
    expected.m_quote.m_size = 0;
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == expected);
    REQUIRE(fixture.m_model.clear(timestamp).empty());
  }

  TEST_CASE("entry_key") {
    auto fixture = Fixture();
    auto first = fixture.order(100);
    auto second = fixture.order(200);
    SUBCASE("venue") {
      second.m_venue = Venue("CHIC");
    }
    SUBCASE("mpid") {
      second.m_mpid = "MPID2";
    }
    SUBCASE("price") {
      second.m_quote.m_price = Money(11);
    }
    fixture.m_model.add(1, first);
    auto updates = fixture.m_model.add(2, second);
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == second);
    updates = fixture.m_model.remove(1, fixture.m_timestamp);
    first.m_quote.m_size = 0;
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == first);
    updates = fixture.m_model.modify_size(2, 300, fixture.m_timestamp);
    second.m_quote.m_size = 300;
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == second);
  }

  TEST_CASE("replace") {
    auto fixture = Fixture();
    fixture.m_model.add(1, fixture.order(100));
    fixture.m_model.add(2, fixture.order(200));
    auto replacement = fixture.order(150);
    SUBCASE("same_entry") {
      auto updates = fixture.m_model.add(1, replacement);
      REQUIRE(updates.size() == 1);
      REQUIRE(updates.front() == fixture.order(350));
    }
    SUBCASE("different_entry") {
      replacement.m_venue = Venue("CHIC");
      replacement.m_mpid = "MPID2";
      replacement.m_quote.m_price = Money(11);
      replacement.m_timestamp += seconds(1);
      auto updates = fixture.m_model.add(1, replacement);
      auto previous = fixture.order(200);
      previous.m_timestamp = replacement.m_timestamp;
      REQUIRE(updates.size() == 2);
      REQUIRE(updates[0] == previous);
      REQUIRE(updates[1] == replacement);
      updates = fixture.m_model.remove(1, replacement.m_timestamp);
      replacement.m_quote.m_size = 0;
      REQUIRE(updates.size() == 1);
      REQUIRE(updates.front() == replacement);
    }
  }

  TEST_CASE("modify_size") {
    auto fixture = Fixture();
    fixture.m_model.add(1, fixture.order(100));
    fixture.m_model.add(2, fixture.order(200));
    auto timestamp = fixture.m_timestamp + seconds(1);
    auto updates = fixture.m_model.modify_size(1, 50, timestamp);
    auto expected = fixture.order(250);
    expected.m_timestamp = timestamp;
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == expected);
    updates = fixture.m_model.offset_size(1, 25, timestamp);
    expected.m_quote.m_size = 275;
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == expected);
    updates = fixture.m_model.offset_size(2, -50, timestamp);
    expected.m_quote.m_size = 225;
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == expected);
  }

  TEST_CASE("nonpositive_size") {
    auto fixture = Fixture();
    fixture.m_model.add(1, fixture.order(100));
    fixture.m_model.add(2, fixture.order(200));
    auto updates = OrderToBookQuoteModel<int>::Updates();
    SUBCASE("zero") {
      updates = fixture.m_model.modify_size(1, 0, fixture.m_timestamp);
    }
    SUBCASE("negative") {
      updates = fixture.m_model.modify_size(1, -1, fixture.m_timestamp);
    }
    SUBCASE("execution") {
      updates = fixture.m_model.offset_size(1, -100, fixture.m_timestamp);
    }
    SUBCASE("excess_execution") {
      updates = fixture.m_model.offset_size(1, -150, fixture.m_timestamp);
    }
    SUBCASE("replace") {
      updates = fixture.m_model.add(1, fixture.order(0));
    }
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == fixture.order(200));
    REQUIRE(fixture.m_model.remove(1, fixture.m_timestamp).empty());
    REQUIRE(fixture.m_model.add(3, fixture.order(0)).empty());
  }

  TEST_CASE("modify_price") {
    auto fixture = Fixture();
    fixture.m_model.add(1, fixture.order(100));
    auto destination = fixture.order(200);
    destination.m_quote.m_price = Money(11);
    fixture.m_model.add(2, destination);
    auto timestamp = fixture.m_timestamp + seconds(1);
    auto updates = fixture.m_model.modify_price(1, Money(11), timestamp);
    auto previous = fixture.order(0);
    previous.m_timestamp = timestamp;
    destination.m_quote.m_size = 300;
    destination.m_timestamp = timestamp;
    REQUIRE(updates.size() == 2);
    REQUIRE(updates[0] == previous);
    REQUIRE(updates[1] == destination);
    updates = fixture.m_model.remove(1, timestamp);
    destination.m_quote.m_size = 200;
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == destination);
  }

  TEST_CASE("unchanged_order") {
    auto fixture = Fixture();
    fixture.m_model.add(1, fixture.order(100));
    fixture.m_timestamp += seconds(1);
    REQUIRE(fixture.m_model.add(1, fixture.order(100)).empty());
    REQUIRE(fixture.m_model.modify_size(1, 100, fixture.m_timestamp).empty());
    REQUIRE(fixture.m_model.offset_size(1, 0, fixture.m_timestamp).empty());
    REQUIRE(fixture.m_model.modify_price(
      1, Money(10), fixture.m_timestamp).empty());
  }

  TEST_CASE("unknown_order") {
    auto fixture = Fixture();
    REQUIRE(fixture.m_model.modify_size(1, 100, fixture.m_timestamp).empty());
    REQUIRE(fixture.m_model.offset_size(1, 100, fixture.m_timestamp).empty());
    REQUIRE(fixture.m_model.modify_price(
      1, Money(10), fixture.m_timestamp).empty());
    REQUIRE(fixture.m_model.remove(1, fixture.m_timestamp).empty());
    REQUIRE(fixture.m_model.clear(fixture.m_timestamp).empty());
  }

  TEST_CASE("primary_mpid") {
    auto fixture = Fixture();
    fixture.m_model.add(1, fixture.order(100));
    auto order = fixture.order(200);
    order.m_is_primary_mpid = true;
    auto updates = fixture.m_model.add(2, order);
    REQUIRE(updates.front().m_is_primary_mpid);
    REQUIRE(updates.front().m_quote.m_size == 300);
    updates = fixture.m_model.modify_size(1, 150, fixture.m_timestamp);
    REQUIRE(updates.front().m_is_primary_mpid);
    REQUIRE(updates.front().m_quote.m_size == 350);
    order.m_is_primary_mpid = false;
    updates = fixture.m_model.add(2, order);
    REQUIRE_FALSE(updates.front().m_is_primary_mpid);
    REQUIRE(updates.front().m_quote.m_size == 350);
    order.m_is_primary_mpid = true;
    fixture.m_model.add(2, order);
    fixture.m_model.add(3, order);
    updates = fixture.m_model.remove(2, fixture.m_timestamp);
    REQUIRE(updates.front().m_is_primary_mpid);
    REQUIRE(updates.front().m_quote.m_size == 350);
    updates = fixture.m_model.remove(3, fixture.m_timestamp);
    REQUIRE_FALSE(updates.front().m_is_primary_mpid);
    REQUIRE(updates.front().m_quote.m_size == 150);
  }

  TEST_CASE("clear") {
    auto fixture = Fixture();
    for(auto i = 0; i < 4; ++i) {
      auto order = fixture.order(100);
      order.m_quote.m_price = Money(10 + i);
      fixture.m_model.add(i, order);
    }
    auto timestamp = fixture.m_timestamp + seconds(1);
    auto updates = fixture.m_model.clear(timestamp);
    REQUIRE(updates.size() == 4);
    std::ranges::sort(updates, std::ranges::less(), [] (const auto& quote) {
      return quote.m_quote.m_price;
    });
    for(auto i = 0; i < 4; ++i) {
      auto expected = fixture.order(0);
      expected.m_quote.m_price = Money(10 + i);
      expected.m_timestamp = timestamp;
      REQUIRE(updates[i] == expected);
      REQUIRE(fixture.m_model.remove(i, timestamp).empty());
    }
    REQUIRE(fixture.m_model.clear(timestamp).empty());
    updates = fixture.m_model.add(0, fixture.order(50));
    REQUIRE(updates.size() == 1);
    REQUIRE(updates.front() == fixture.order(50));
  }

  TEST_CASE("bbo_composition") {
    auto fixture = Fixture();
    auto bbo = BookQuoteToBboQuoteModel();
    auto apply = [&] (const auto& updates) {
      for(auto& quote : updates) {
        bbo.update(quote);
      }
    };
    apply(fixture.m_model.add(1, fixture.order(100)));
    apply(fixture.m_model.add(2, fixture.order(200)));
    REQUIRE(bbo.get_bbo().m_bid == Quote(Money(10), 300, Side::BID));
    apply(fixture.m_model.modify_price(1, Money(11), fixture.m_timestamp));
    REQUIRE(bbo.get_bbo().m_bid == Quote(Money(11), 100, Side::BID));
    apply(fixture.m_model.remove(1, fixture.m_timestamp));
    REQUIRE(bbo.get_bbo().m_bid == Quote(Money(10), 200, Side::BID));
    apply(fixture.m_model.clear(fixture.m_timestamp));
    REQUIRE(bbo.get_bbo().m_bid == Quote(Money::ZERO, 0, Side::BID));
  }
}
