#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/BookView/MergedBookQuoteListModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Spire;

TEST_SUITE("MergedBookQuoteListModel") {
  TEST_CASE("constructor_empty") {
    auto list = MergedBookQuoteListModel(
      std::make_shared<ArrayListModel<BookQuote>>(),
      std::make_shared<ArrayListModel<BookViewModel::UserOrder>>(),
      std::make_shared<LocalValueModel<optional<OrderFields>>>());
    REQUIRE(list.get_size() == 0);
  }
  TEST_CASE("constructor_book_quotes") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    quotes->push(BookQuote("TSX", false, DefaultMarkets::TSX(),
      Quote(Money::ONE, 100, Side::BID),
      time_from_string("2016-07-31 19:00:00")));
    quotes->push(BookQuote("TSX", false, DefaultMarkets::TSX(),
      Quote(2 * Money::ONE, 200, Side::BID),
      time_from_string("2016-07-31 19:01:00")));
    auto list = MergedBookQuoteListModel(quotes,
      std::make_shared<ArrayListModel<BookViewModel::UserOrder>>(),
      std::make_shared<LocalValueModel<optional<OrderFields>>>());
    REQUIRE(list.get_size() == 2);
    REQUIRE((list.get(0) == BookListing(quotes->get(0))));
    REQUIRE((list.get(1) == BookListing(quotes->get(1))));
  }
  TEST_CASE("constructor_user_orders") {
    auto orders = std::make_shared<ArrayListModel<BookViewModel::UserOrder>>();
    orders->push(
      BookViewModel::UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW));
    orders->push(BookViewModel::UserOrder(
      "TSX", 2 * Money::ONE, 200, OrderStatus::FILLED));
    orders->push(BookViewModel::UserOrder(
      "TSX", 3 * Money::ONE, 200, OrderStatus::CANCELED));
    auto list = MergedBookQuoteListModel(
      std::make_shared<ArrayListModel<BookQuote>>(), orders,
      std::make_shared<LocalValueModel<optional<OrderFields>>>());
    REQUIRE(list.get_size() == 3);
    REQUIRE((list.get(0) == BookListing(orders->get(0))));
    REQUIRE((list.get(1) == BookListing(orders->get(1))));
    REQUIRE((list.get(2) == BookListing(orders->get(2))));
  }
  TEST_CASE("constructor_preview") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    preview->set(OrderFields::MakeLimitOrder(
      ParseSecurity("ABC.TSX"), Side::BID, 100, Money::CENT));
    auto list = MergedBookQuoteListModel(
      std::make_shared<ArrayListModel<BookQuote>>(),
      std::make_shared<ArrayListModel<BookViewModel::UserOrder>>(), preview);
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookListing(*preview->get())));
  }
  TEST_CASE("constructor_full") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    quotes->push(BookQuote("TSX", false, DefaultMarkets::TSX(),
      Quote(Money::ONE, 100, Side::BID),
      time_from_string("2016-07-31 19:00:00")));
    quotes->push(BookQuote("TSX", false, DefaultMarkets::TSX(),
      Quote(2 * Money::ONE, 200, Side::BID),
      time_from_string("2016-07-31 19:01:00")));
    auto orders = std::make_shared<ArrayListModel<BookViewModel::UserOrder>>();
    orders->push(
      BookViewModel::UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW));
    orders->push(BookViewModel::UserOrder(
      "TSX", 2 * Money::ONE, 200, OrderStatus::FILLED));
    orders->push(BookViewModel::UserOrder(
      "TSX", 3 * Money::ONE, 200, OrderStatus::CANCELED));
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    preview->set(OrderFields::MakeLimitOrder(
      ParseSecurity("ABC.TSX"), Side::BID, 100, Money::CENT));
    auto list = MergedBookQuoteListModel(quotes, orders, preview);
    REQUIRE(list.get_size() == 6);
    REQUIRE((list.get(0) == BookListing(quotes->get(0))));
    REQUIRE((list.get(1) == BookListing(quotes->get(1))));
    REQUIRE((list.get(2) == BookListing(orders->get(0))));
    REQUIRE((list.get(3) == BookListing(orders->get(1))));
    REQUIRE((list.get(4) == BookListing(orders->get(2))));
    REQUIRE((list.get(5) == BookListing(*preview->get())));
  }
  TEST_CASE("update_book_quotes") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto list = MergedBookQuoteListModel(quotes,
      std::make_shared<ArrayListModel<BookViewModel::UserOrder>>(),
      std::make_shared<LocalValueModel<optional<OrderFields>>>());
    auto quote = BookQuote("TSX", false, DefaultMarkets::TSX(),
      Quote(Money::ONE, 100, Side::BID),
      time_from_string("2016-07-31 19:00:00"));
    quotes->push(quote);
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookListing(quote)));
    quotes->remove(0);
    REQUIRE(list.get_size() == 0);
  }
  TEST_CASE("update_user_orders") {
    auto orders = std::make_shared<ArrayListModel<BookViewModel::UserOrder>>();
    auto list = MergedBookQuoteListModel(
      std::make_shared<ArrayListModel<BookQuote>>(), orders,
      std::make_shared<LocalValueModel<optional<OrderFields>>>());
    auto order =
      BookViewModel::UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW);
    orders->push(order);
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookListing(order)));
    orders->remove(0);
    REQUIRE(list.get_size() == 0);
  }
  TEST_CASE("update_preview") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    auto list = MergedBookQuoteListModel(
      std::make_shared<ArrayListModel<BookQuote>>(),
      std::make_shared<ArrayListModel<BookViewModel::UserOrder>>(), preview);
    preview->set(OrderFields::MakeLimitOrder(
      ParseSecurity("ABC.TSX"), Side::BID, 100, Money::CENT));
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookListing(*preview->get())));
    preview->set(OrderFields::MakeLimitOrder(
      ParseSecurity("ABC.TSX"), Side::BID, 200, 5 * Money::CENT));
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookListing(*preview->get())));
    preview->set(none);
    REQUIRE(list.get_size() == 0);
    preview->set(none);
    REQUIRE(list.get_size() == 0);
  }
  TEST_CASE("update_mixed") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto orders = std::make_shared<ArrayListModel<BookViewModel::UserOrder>>();
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    auto list = MergedBookQuoteListModel(quotes, orders, preview);
    auto quote = BookQuote("TSX", false, DefaultMarkets::TSX(),
      Quote(Money::ONE, 100, Side::BID),
      time_from_string("2016-07-31 19:00:00"));
    quotes->push(quote);
    auto order =
      BookViewModel::UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW);
    orders->push(order);
    REQUIRE(list.get_size() == 2);
    REQUIRE((list.get(0) == BookListing(quotes->get(0))));
    REQUIRE((list.get(1) == BookListing(orders->get(0))));
    preview->set(OrderFields::MakeLimitOrder(
      ParseSecurity("ABC.TSX"), Side::BID, 100, Money::CENT));
    REQUIRE(list.get_size() == 3);
    REQUIRE((list.get(0) == BookListing(quotes->get(0))));
    REQUIRE((list.get(1) == BookListing(orders->get(0))));
    REQUIRE((list.get(2) == BookListing(*preview->get())));
  }
}
