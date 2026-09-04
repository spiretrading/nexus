#include <doctest/doctest.h>
#include "Spire/BookView/MergedBookEntryListModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  using UserOrder = BookViewModel::UserOrder;

  auto make_book_quote(Money price, Quantity size) {
    return BookQuote("TSX", false, Venues::TSX,
      Quote(price, size, Side::BID),
      time_from_string("2016-07-31 19:00:00"));
  }

  auto make_quotes() {
    return std::make_shared<ArrayListModel<BookQuote>>();
  }

  auto make_orders() {
    return std::make_shared<ArrayListModel<UserOrder>>();
  }

  auto make_preview() {
    return std::make_shared<LocalValueModel<optional<OrderFields>>>();
  }

  auto make_fields(Quantity quantity, Money price) {
    return make_limit_order_fields(
      parse_ticker("ABC.TSX"), Side::BID, quantity, price);
  }
}

TEST_SUITE("MergedBookEntryListModel") {
  TEST_CASE("constructor_empty") {
    auto list =
      MergedBookEntryListModel(make_quotes(), make_orders(), make_preview());
    REQUIRE(list.get_size() == 0);
  }

  TEST_CASE("constructor_book_quotes") {
    auto quotes = make_quotes();
    quotes->push(make_book_quote(Money::ONE, 100));
    quotes->push(make_book_quote(2 * Money::ONE, 200));
    auto list =
      MergedBookEntryListModel(quotes, make_orders(), make_preview());
    REQUIRE(list.get_size() == 2);
    REQUIRE((list.get(0) == BookEntry(quotes->get(0))));
    REQUIRE((list.get(1) == BookEntry(quotes->get(1))));
  }

  TEST_CASE("constructor_user_orders") {
    auto orders = make_orders();
    orders->push(UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW));
    orders->push(UserOrder("TSX", 2 * Money::ONE, 200, OrderStatus::FILLED));
    orders->push(
      UserOrder("TSX", 3 * Money::ONE, 200, OrderStatus::CANCELED));
    auto list =
      MergedBookEntryListModel(make_quotes(), orders, make_preview());
    REQUIRE(list.get_size() == 3);
    REQUIRE((list.get(0) == BookEntry(orders->get(0))));
    REQUIRE((list.get(1) == BookEntry(orders->get(1))));
    REQUIRE((list.get(2) == BookEntry(orders->get(2))));
  }

  TEST_CASE("constructor_preview") {
    auto preview = make_preview();
    preview->set(make_fields(100, Money::CENT));
    auto list =
      MergedBookEntryListModel(make_quotes(), make_orders(), preview);
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookEntry(*preview->get())));
  }

  TEST_CASE("constructor_full") {
    auto quotes = make_quotes();
    quotes->push(make_book_quote(Money::ONE, 100));
    quotes->push(make_book_quote(2 * Money::ONE, 200));
    auto orders = make_orders();
    orders->push(UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW));
    orders->push(UserOrder("TSX", 2 * Money::ONE, 200, OrderStatus::FILLED));
    orders->push(
      UserOrder("TSX", 3 * Money::ONE, 200, OrderStatus::CANCELED));
    auto preview = make_preview();
    preview->set(make_fields(100, Money::CENT));
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
    auto quotes = make_quotes();
    auto list =
      MergedBookEntryListModel(quotes, make_orders(), make_preview());
    auto quote = make_book_quote(Money::ONE, 100);
    quotes->push(quote);
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookEntry(quote)));
    quotes->remove(0);
    REQUIRE(list.get_size() == 0);
  }

  TEST_CASE("update_user_orders") {
    auto orders = make_orders();
    auto list =
      MergedBookEntryListModel(make_quotes(), orders, make_preview());
    auto order = UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW);
    orders->push(order);
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookEntry(order)));
    orders->remove(0);
    REQUIRE(list.get_size() == 0);
  }

  TEST_CASE("update_preview") {
    auto preview = make_preview();
    auto list =
      MergedBookEntryListModel(make_quotes(), make_orders(), preview);
    preview->set(make_fields(100, Money::CENT));
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookEntry(*preview->get())));
    preview->set(make_fields(200, 5 * Money::CENT));
    REQUIRE(list.get_size() == 1);
    REQUIRE((list.get(0) == BookEntry(*preview->get())));
    preview->set(none);
    REQUIRE(list.get_size() == 0);
    preview->set(none);
    REQUIRE(list.get_size() == 0);
  }

  TEST_CASE("update_mixed") {
    auto quotes = make_quotes();
    auto orders = make_orders();
    auto preview = make_preview();
    auto list = MergedBookEntryListModel(quotes, orders, preview);
    auto quote = make_book_quote(Money::ONE, 100);
    quotes->push(quote);
    auto order = UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW);
    orders->push(order);
    REQUIRE(list.get_size() == 2);
    REQUIRE((list.get(0) == BookEntry(quotes->get(0))));
    REQUIRE((list.get(1) == BookEntry(orders->get(0))));
    preview->set(make_fields(100, Money::CENT));
    REQUIRE(list.get_size() == 3);
    REQUIRE((list.get(0) == BookEntry(quotes->get(0))));
    REQUIRE((list.get(1) == BookEntry(orders->get(0))));
    REQUIRE((list.get(2) == BookEntry(*preview->get())));
  }

  TEST_CASE("get_reference_survives_subsequent_reads") {
    auto quotes = make_quotes();
    for(auto i = 0; i != 12; ++i) {
      quotes->push(make_book_quote((i + 1) * Money::ONE, 100));
    }
    auto list =
      MergedBookEntryListModel(quotes, make_orders(), make_preview());
    auto expected = list.get(0);
    auto& entry = list.get(0);
    for(auto i = 1; i != list.get_size(); ++i) {
      list.get(i);
    }
    REQUIRE((entry == expected));
  }
}
