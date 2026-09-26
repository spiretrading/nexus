#include <doctest/doctest.h>
#include "Nexus/MarketDataService/BookQuoteToBboQuoteModel.hpp"
#include "Nexus/MarketDataService/OrderToBookQuoteModel.hpp"

using namespace boost::posix_time;
using namespace Nexus;

namespace {
  struct Fixture {
    OrderToBookQuoteModel<int> m_model;
    ptime m_timestamp = time_from_string("2026-09-25 10:00:00");

    BookQuote order(Quantity size) {
      return BookQuote("MPID1", false, Venue("XTSE"),
        Quote(Money(10), size, Side::BID), m_timestamp);
    }
  };
}

TEST_SUITE("OrderToBookQuoteModel") {
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
    SUBCASE("side") {
      second.m_quote.m_side = Side::ASK;
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
      replacement.m_quote.m_side = Side::ASK;
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
