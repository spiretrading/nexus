#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Spire/BookView/MergedBookEntryListModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

TEST_SUITE("MergedBookEntryListModel") {
  TEST_CASE("constructor_empty") {
    auto list = MergedBookEntryListModel(
      std::make_shared<ArrayListModel<BookQuote>>(),
      std::make_shared<ArrayListModel<BookViewModel::UserOrder>>(),
      std::make_shared<LocalValueModel<optional<OrderFields>>>());
    REQUIRE(list.get_size() == 0);
  }
  TEST_CASE("constructor_book_quotes") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    quotes->push(BookQuote("TSX", false, DefaultVenues::TSX,
      Quote(Money::ONE, 100, Side::BID),
      time_from_string("2016-07-31 19:00:00")));
    quotes->push(BookQuote("TSX", false, DefaultVenues::TSX,
      Quote(2 * Money::ONE, 200, Side::BID),
      time_from_string("2016-07-31 19:01:00")));
    auto list = MergedBookEntryListModel(quotes,
      std::make_shared<ArrayListModel<BookViewModel::UserOrder>>(),
      std::make_shared<LocalValueModel<optional<OrderFields>>>());
    REQUIRE(list.get_size() == 2);
    REQUIRE((list.get(0) == BookEntry(quotes->get(0))));
    REQUIRE((list.get(1) == BookEntry(quotes->get(1))));
  }
  TEST_CASE("constructor_user_orders") {
    auto orders = std::make_shared<ArrayListModel<BookViewModel::UserOrder>>();
    orders->push(
      BookViewModel::UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW));
    orders->push(BookViewModel::UserOrder(
      "TSX", 2 * Money::ONE, 200, OrderStatus::FILLED));
    orders->push(BookViewModel::UserOrder(
      "TSX", 3 * Money::ONE, 200, OrderStatus::CANCELED));
    auto list = MergedBookEntryListModel(
      std::make_shared<ArrayListModel<BookQuote>>(), orders,
      std::make_shared<LocalValueModel<optional<OrderFields>>>());
    REQUIRE(list.get_size() == 3);
    REQUIRE((list.get(0) == BookEntry(orders->get(0))));
    REQUIRE((list.get(1) == BookEntry(orders->get(1))));
    REQUIRE((list.get(2) == BookEntry(orders->get(2))));
  }
  TEST_CASE("constructor_preview") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    preview->set(make_limit_order_fields(
      parse_security("ABC.TSX"), Side::BID, 100, Money::CENT));
    auto list = MergedBookEntryListModel(
      std::make_shared<ArrayListModel<BookQuote>>(),
      std::make_shared<ArrayListModel<BookViewModel::UserOrder>>(), preview);
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookEntry(*preview->get())));
  }
  TEST_CASE("constructor_full") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    quotes->push(BookQuote("TSX", false, DefaultVenues::TSX,
      Quote(Money::ONE, 100, Side::BID),
      time_from_string("2016-07-31 19:00:00")));
    quotes->push(BookQuote("TSX", false, DefaultVenues::TSX,
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
    preview->set(make_limit_order_fields(
      parse_security("ABC.TSX"), Side::BID, 100, Money::CENT));
    auto list = MergedBookEntryListModel(quotes, orders, preview);
    REQUIRE(list.get_size() == 6);
    REQUIRE((list.get(0) == BookEntry(quotes->get(0))));
    REQUIRE((list.get(1) == BookEntry(quotes->get(1))));
    REQUIRE((list.get(2) == BookEntry(orders->get(0))));
    REQUIRE((list.get(3) == BookEntry(orders->get(1))));
    REQUIRE((list.get(4) == BookEntry(orders->get(2))));
    REQUIRE((list.get(5) == BookEntry(*preview->get())));
  }
  TEST_CASE("update_book_quotes") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto list = MergedBookEntryListModel(quotes,
      std::make_shared<ArrayListModel<BookViewModel::UserOrder>>(),
      std::make_shared<LocalValueModel<optional<OrderFields>>>());
    auto quote = BookQuote("TSX", false, DefaultVenues::TSX,
      Quote(Money::ONE, 100, Side::BID),
      time_from_string("2016-07-31 19:00:00"));
    quotes->push(quote);
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookEntry(quote)));
    quotes->remove(0);
    REQUIRE(list.get_size() == 0);
  }
  TEST_CASE("update_user_orders") {
    auto orders = std::make_shared<ArrayListModel<BookViewModel::UserOrder>>();
    auto list = MergedBookEntryListModel(
      std::make_shared<ArrayListModel<BookQuote>>(), orders,
      std::make_shared<LocalValueModel<optional<OrderFields>>>());
    auto order =
      BookViewModel::UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW);
    orders->push(order);
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookEntry(order)));
    orders->remove(0);
    REQUIRE(list.get_size() == 0);
  }
  TEST_CASE("update_preview") {
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    auto list = MergedBookEntryListModel(
      std::make_shared<ArrayListModel<BookQuote>>(),
      std::make_shared<ArrayListModel<BookViewModel::UserOrder>>(), preview);
    preview->set(make_limit_order_fields(
      parse_security("ABC.TSX"), Side::BID, 100, Money::CENT));
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookEntry(*preview->get())));
    preview->set(make_limit_order_fields(
      parse_security("ABC.TSX"), Side::BID, 200, 5 * Money::CENT));
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookEntry(*preview->get())));
    preview->set(none);
    REQUIRE(list.get_size() == 0);
    preview->set(none);
    REQUIRE(list.get_size() == 0);
  }
  TEST_CASE("update_mixed") {
    auto quotes = std::make_shared<ArrayListModel<BookQuote>>();
    auto orders = std::make_shared<ArrayListModel<BookViewModel::UserOrder>>();
    auto preview = std::make_shared<LocalValueModel<optional<OrderFields>>>();
    auto list = MergedBookEntryListModel(quotes, orders, preview);
    auto quote = BookQuote("TSX", false, DefaultVenues::TSX,
      Quote(Money::ONE, 100, Side::BID),
      time_from_string("2016-07-31 19:00:00"));
    quotes->push(quote);
    auto order =
      BookViewModel::UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW);
    orders->push(order);
    REQUIRE(list.get_size() == 2);
    REQUIRE((list.get(0) == BookEntry(quotes->get(0))));
    REQUIRE((list.get(1) == BookEntry(orders->get(0))));
    preview->set(make_limit_order_fields(
      parse_security("ABC.TSX"), Side::BID, 100, Money::CENT));
    REQUIRE(list.get_size() == 3);
    REQUIRE((list.get(0) == BookEntry(quotes->get(0))));
    REQUIRE((list.get(1) == BookEntry(orders->get(0))));
    REQUIRE((list.get(2) == BookEntry(*preview->get())));
  }
}
