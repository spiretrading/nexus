#include <doctest/doctest.h>
#include "Spire/BookView/IsTopMpidModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  const auto TEST_MPID = BookQuote("TSXID", true, Venues::TSX, Quote(),
    time_from_string("2016-07-31 19:00:00"));

  auto make_book_quote(Venue venue) {
    auto quote = TEST_MPID;
    quote.m_venue = venue;
    return quote;
  }

  auto make_book_quote(Venue venue, Money price) {
    return BookQuote("TSXID", true, venue, Quote(price, 100, Side::BID),
      time_from_string("2016-07-31 19:00:00"));
  }
}

TEST_SUITE("IsTopMpidModel") {
  TEST_CASE("constructor_empty") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto top_mpid_prices = std::make_shared<TopMpidPriceListModel>(quotes);
    auto entry = std::make_shared<LocalValueModel<BookEntry>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_prices, entry, price);
    REQUIRE(!is_top.get());
    SUBCASE("update_top_prices") {
      quotes->push(make_book_quote(Venues::TSXV, Money::ONE));
      REQUIRE(!is_top.get());
      quotes->push(make_book_quote(Venues::TSX, 2 * Money::ONE));
      REQUIRE(is_top.get());
    }
  }

  TEST_CASE("constructor_missing_mpid") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    quotes->push(make_book_quote(Venues::TSXV, Money::ONE));
    auto top_mpid_prices = std::make_shared<TopMpidPriceListModel>(quotes);
    auto entry = std::make_shared<LocalValueModel<BookEntry>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_prices, entry, price);
    REQUIRE(!is_top.get());
    SUBCASE("update_mpid") {
      entry->set(make_book_quote(Venues::TSXV));
      REQUIRE(is_top.get());
    }
  }

  TEST_CASE("constructor_missing_origin") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    quotes->push(make_book_quote(Venues::TSXV, Money::ONE));
    quotes->push(make_book_quote(Venues::TSX, 2 * Money::ONE));
    auto top_mpid_prices = std::make_shared<TopMpidPriceListModel>(quotes);
    auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
    SUBCASE("user_order") {
      auto missing_mpid =
        BookViewModel::UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW);
      auto entry = std::make_shared<LocalValueModel<BookEntry>>(missing_mpid);
      auto is_top = IsTopMpidModel(top_mpid_prices, entry, price);
      REQUIRE(!is_top.get());
    }
    SUBCASE("preview") {
      auto missing_mpid = make_limit_order_fields(
        parse_ticker("TST.TSX"), Side::BID, "TSX", 100, Money::ONE);
      auto entry = std::make_shared<LocalValueModel<BookEntry>>(missing_mpid);
      auto is_top = IsTopMpidModel(top_mpid_prices, entry, price);
      REQUIRE(!is_top.get());
    }
  }

  TEST_CASE("constructor") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    quotes->push(make_book_quote(Venues::TSXV, Money::ONE));
    quotes->push(make_book_quote(Venues::TSX, 2 * Money::ONE));
    auto top_mpid_prices = std::make_shared<TopMpidPriceListModel>(quotes);
    auto entry = std::make_shared<LocalValueModel<BookEntry>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_prices, entry, price);
    REQUIRE(is_top.get());
  }

  TEST_CASE("update_top_price") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    quotes->push(make_book_quote(Venues::TSX, 2 * Money::ONE));
    auto top_mpid_prices = std::make_shared<TopMpidPriceListModel>(quotes);
    auto entry = std::make_shared<LocalValueModel<BookEntry>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_prices, entry, price);
    REQUIRE(is_top.get());
    quotes->push(make_book_quote(Venues::TSX, 3 * Money::ONE));
    REQUIRE(!is_top.get());
    quotes->remove(1);
    REQUIRE(is_top.get());
  }

  TEST_CASE("change_to_venue_without_a_top_price") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    quotes->push(make_book_quote(Venues::TSX, 2 * Money::ONE));
    auto top_mpid_prices = std::make_shared<TopMpidPriceListModel>(quotes);
    auto entry = std::make_shared<LocalValueModel<BookEntry>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_prices, entry, price);
    REQUIRE(is_top.get());
    entry->set(make_book_quote(Venues::TSXV));
    REQUIRE(!is_top.get());
  }

  TEST_CASE("top_price_arrives_after_a_venue_change") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    quotes->push(make_book_quote(Venues::TSX, 2 * Money::ONE));
    auto top_mpid_prices = std::make_shared<TopMpidPriceListModel>(quotes);
    auto entry = std::make_shared<LocalValueModel<BookEntry>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_prices, entry, price);
    REQUIRE(!is_top.get());
    entry->set(make_book_quote(Venues::TSXV));
    REQUIRE(!is_top.get());
    quotes->push(make_book_quote(Venues::TSXV, Money::ONE));
    REQUIRE(is_top.get());
  }

  TEST_CASE("change_from_a_top_quote_to_a_user_order") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    quotes->push(make_book_quote(Venues::TSX, 2 * Money::ONE));
    auto top_mpid_prices = std::make_shared<TopMpidPriceListModel>(quotes);
    auto entry = std::make_shared<LocalValueModel<BookEntry>>(TEST_MPID);
    auto price = std::make_shared<LocalValueModel<Money>>(2 * Money::ONE);
    auto is_top = IsTopMpidModel(top_mpid_prices, entry, price);
    REQUIRE(is_top.get());
    entry->set(BookViewModel::UserOrder(
      "TSX", 2 * Money::ONE, 100, OrderStatus::NEW));
    REQUIRE(!is_top.get());
  }
}
