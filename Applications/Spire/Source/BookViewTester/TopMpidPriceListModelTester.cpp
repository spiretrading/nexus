#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/BookView/TopMpidPriceListModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_book_quote(MarketCode market, Money price, Side side = Side::BID) {
    return BookQuote(market.GetData(), true, market,
      Quote(price, 100, side), time_from_string("2016-07-31 19:00:00"));
  }

  int find_mpid(const ListModel<TopMpidPrice>& list, MarketCode market) {
    auto i = std::find_if(list.begin(), list.end(), [&] (const auto& top) {
      return top.m_market == market;
    });
    if(i == list.end()) {
      return -1;
    }
    return std::distance(list.begin(), i);
  }
}

TEST_SUITE("TopMpidPriceListModel") {
  TEST_CASE("constructor_empty") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    REQUIRE(top_prices.get_size() == 0);
    SUBCASE("add") {
      quotes->push(make_book_quote(DefaultMarkets::TSX(), Money(100)));
      REQUIRE(top_prices.get_size() == 1);
      REQUIRE(top_prices.get(0).m_market == DefaultMarkets::TSX());
      REQUIRE(top_prices.get(0).m_price == Money(100));
    }
  }
  TEST_CASE("multiple_markets") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    quotes->push(make_book_quote(DefaultMarkets::TSX(), Money(100)));
    quotes->push(make_book_quote(DefaultMarkets::OMGA(), Money(200)));
    REQUIRE(top_prices.get_size() == 2);
    REQUIRE(find_mpid(top_prices, DefaultMarkets::TSX()) != -1);
    REQUIRE(find_mpid(top_prices, DefaultMarkets::OMGA()) != -1);
  }
  TEST_CASE("multiple_quotes") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_prices = TopMpidPriceListModel(quotes);
    quotes->push(make_book_quote(DefaultMarkets::TSX(), Money(100)));
    quotes->push(make_book_quote(DefaultMarkets::TSX(), Money(200)));
    quotes->push(make_book_quote(DefaultMarkets::TSX(), Money(150)));
    REQUIRE(top_prices.get_size() == 1);
    auto& top = top_prices.get(0);
    REQUIRE(top.m_market == DefaultMarkets::TSX());
    REQUIRE(top.m_price == Money(200));
  }
}
