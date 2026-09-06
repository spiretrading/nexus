#include <algorithm>
#include <doctest/doctest.h>
#include "Spire/BookView/TopMpidPriceListModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_book_quote(Venue venue, Money price) {
    return BookQuote(venue.get_code().get_data(), true, venue,
      Quote(price, 100, Side::BID), time_from_string("2016-07-31 19:00:00"));
  }

  bool has_venue(const ListModel<TopMpidPrice>& list, Venue venue) {
    return std::any_of(list.begin(), list.end(), [&] (const auto& top) {
      return top.m_venue == venue;
    });
  }
}

TEST_SUITE("TopMpidPriceListModel") {
  TEST_CASE("constructor_empty") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    REQUIRE(top_prices.get_size() == 0);
    SUBCASE("add") {
      quotes->push(make_book_quote(Venues::TSX, Money(100)));
      REQUIRE(top_prices.get_size() == 1);
      REQUIRE(top_prices.get(0).m_venue == Venues::TSX);
      REQUIRE(top_prices.get(0).m_price == Money(100));
    }
  }

  TEST_CASE("multiple_venues") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    quotes->push(make_book_quote(Venues::TSX, Money(100)));
    quotes->push(make_book_quote(Venues::OMGA, Money(200)));
    REQUIRE(top_prices.get_size() == 2);
    REQUIRE(has_venue(top_prices, Venues::TSX));
    REQUIRE(has_venue(top_prices, Venues::OMGA));
  }

  TEST_CASE("multiple_quotes") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    quotes->push(make_book_quote(Venues::TSX, Money(100)));
    quotes->push(make_book_quote(Venues::TSX, Money(200)));
    quotes->push(make_book_quote(Venues::TSX, Money(150)));
    REQUIRE(top_prices.get_size() == 1);
    auto& top = top_prices.get(0);
    REQUIRE(top.m_venue == Venues::TSX);
    REQUIRE(top.m_price == Money(200));
  }

  TEST_CASE("remove_quote_below_top") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    quotes->push(make_book_quote(Venues::TSX, Money(200)));
    quotes->push(make_book_quote(Venues::TSX, Money(100)));
    REQUIRE(top_prices.get(0).m_price == Money(200));
    quotes->remove(1);
    REQUIRE(top_prices.get_size() == 1);
    REQUIRE(top_prices.get(0).m_price == Money(200));
  }

  TEST_CASE("remove_top_quote_falls_back_to_next") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    quotes->push(make_book_quote(Venues::TSX, Money(200)));
    quotes->push(make_book_quote(Venues::TSX, Money(100)));
    REQUIRE(top_prices.get(0).m_price == Money(200));
    quotes->remove(0);
    REQUIRE(top_prices.get_size() == 1);
    REQUIRE(top_prices.get(0).m_price == Money(100));
  }

  TEST_CASE("remove_one_of_two_quotes_sharing_the_top_price") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    quotes->push(make_book_quote(Venues::TSX, Money(200)));
    quotes->push(make_book_quote(Venues::TSX, Money(200)));
    quotes->push(make_book_quote(Venues::TSX, Money(100)));
    REQUIRE(top_prices.get(0).m_price == Money(200));
    quotes->remove(1);
    REQUIRE(top_prices.get_size() == 1);
    REQUIRE(top_prices.get(0).m_price == Money(200));
  }

  TEST_CASE("remove_last_quote_for_venue") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    quotes->push(make_book_quote(Venues::TSX, Money(200)));
    quotes->push(make_book_quote(Venues::OMGA, Money(100)));
    REQUIRE(top_prices.get_size() == 2);
    quotes->remove(0);
    REQUIRE(!has_venue(top_prices, Venues::TSX));
    REQUIRE(has_venue(top_prices, Venues::OMGA));
  }

  TEST_CASE("re_add_quote_below_a_removed_top") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    quotes->push(make_book_quote(Venues::TSX, Money(200)));
    quotes->remove(0);
    quotes->push(make_book_quote(Venues::TSX, Money(100)));
    REQUIRE(top_prices.get_size() == 1);
    REQUIRE(top_prices.get(0).m_price == Money(100));
  }

  TEST_CASE("update_quote_price") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    quotes->push(make_book_quote(Venues::TSX, Money(200)));
    quotes->push(make_book_quote(Venues::TSX, Money(100)));
    REQUIRE(top_prices.get(0).m_price == Money(200));
    SUBCASE("lower_the_top") {
      quotes->set(0, make_book_quote(Venues::TSX, Money(50)));
      REQUIRE(top_prices.get_size() == 1);
      REQUIRE(top_prices.get(0).m_price == Money(100));
    }
    SUBCASE("raise_below_the_top") {
      quotes->set(1, make_book_quote(Venues::TSX, Money(300)));
      REQUIRE(top_prices.get_size() == 1);
      REQUIRE(top_prices.get(0).m_price == Money(300));
    }
  }

  TEST_CASE("remove_non_primary_quote") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    quotes->push(make_book_quote(Venues::TSX, Money(100)));
    auto secondary = make_book_quote(Venues::TSX, Money(200));
    secondary.m_is_primary_mpid = false;
    quotes->push(secondary);
    REQUIRE(top_prices.get(0).m_price == Money(100));
    quotes->remove(1);
    REQUIRE(top_prices.get_size() == 1);
    REQUIRE(top_prices.get(0).m_price == Money(100));
  }
}
