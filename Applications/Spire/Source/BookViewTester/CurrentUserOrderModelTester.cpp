#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/BookView/CurrentUserOrderModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/TableCurrentIndexModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  auto make_book_quote(Money price, Side side) {
    return BookQuote("TSX", true, DefaultMarkets::TSX(),
      Quote(price, 100, side), time_from_string("2016-07-31 19:00:00"));
  }
}

TEST_SUITE("CurrentUserOrderModel") {
  TEST_CASE("undo_navigation") {
    auto bid_entries = std::make_shared<ArrayListModel<BookEntry>>();
    auto bid_table = std::make_shared<SortedTableModel>(
      make_book_view_table_model(bid_entries));
    auto current_bid = std::make_shared<TableCurrentIndexModel>(bid_table);
    auto ask_entries = std::make_shared<ArrayListModel<BookEntry>>();
    auto ask_table = std::make_shared<SortedTableModel>(
      make_book_view_table_model(ask_entries));
    auto current_ask = std::make_shared<TableCurrentIndexModel>(ask_table);
    auto current =
      CurrentUserOrderModel(bid_table, current_bid, ask_table, current_ask);
    SUBCASE("no_shifting") {
      bid_entries->push(
        BookViewModel::UserOrder("TSX", Money(107), 100, OrderStatus::NEW));
      bid_entries->push(make_book_quote(Money(106), Side::BID));
      bid_entries->push(make_book_quote(Money(105), Side::BID));
      bid_entries->push(make_book_quote(Money(104), Side::BID));
      bid_entries->push(
        BookViewModel::UserOrder("TSX", Money(103), 100, OrderStatus::NEW));
      bid_entries->push(make_book_quote(Money(102), Side::BID));
      bid_entries->push(make_book_quote(Money(101), Side::BID));
      bid_entries->push(make_book_quote(Money(100), Side::BID));
      ask_entries->push(make_book_quote(Money(200), Side::ASK));
      ask_entries->push(make_book_quote(Money(201), Side::ASK));
      ask_entries->push(make_book_quote(Money(202), Side::ASK));
      ask_entries->push(
        BookViewModel::UserOrder("TSX", Money(203), 100, OrderStatus::NEW));
      current_bid->set(TableIndex(4, 0));
      REQUIRE(current.get().has_value());
      REQUIRE(current.get()->m_side == Side::BID);
      REQUIRE((current.get()->m_user_order ==
        BookViewModel::UserOrder("TSX", Money(103), 100, OrderStatus::NEW)));
      current.navigate_to_asks();
      REQUIRE(current.get().has_value());
      REQUIRE(current.get()->m_side == Side::ASK);
      REQUIRE((current.get()->m_user_order ==
        BookViewModel::UserOrder("TSX", Money(203), 100, OrderStatus::NEW)));
      current.navigate_to_bids();
      REQUIRE(current.get().has_value());
      REQUIRE(current.get()->m_side == Side::BID);
      REQUIRE((current.get()->m_user_order ==
        BookViewModel::UserOrder("TSX", Money(103), 100, OrderStatus::NEW)));
      current.navigate_to_asks();
      REQUIRE(current.get().has_value());
      REQUIRE(current.get()->m_side == Side::ASK);
      REQUIRE((current.get()->m_user_order ==
        BookViewModel::UserOrder("TSX", Money(203), 100, OrderStatus::NEW)));
    }
    SUBCASE("shifting") {
      bid_entries->push(
        BookViewModel::UserOrder("TSX", Money(107), 100, OrderStatus::NEW));
      bid_entries->push(make_book_quote(Money(106), Side::BID));
      bid_entries->push(make_book_quote(Money(105), Side::BID));
      bid_entries->push(make_book_quote(Money(104), Side::BID));
      bid_entries->push(make_book_quote(Money(103), Side::BID));
      bid_entries->push(make_book_quote(Money(102), Side::BID));
      bid_entries->push(make_book_quote(Money(101), Side::BID));
      bid_entries->push(
        BookViewModel::UserOrder("TSX", Money(100), 100, OrderStatus::NEW));
      ask_entries->push(
        BookViewModel::UserOrder("TSX", Money(200), 100, OrderStatus::NEW));
      current_bid->set(TableIndex(0, 0));
      REQUIRE(current.get().has_value());
      REQUIRE(current.get()->m_side == Side::BID);
      REQUIRE((current.get()->m_user_order ==
        BookViewModel::UserOrder("TSX", Money(107), 100, OrderStatus::NEW)));
      current.navigate_to_asks();
      REQUIRE(current.get().has_value());
      REQUIRE(current.get()->m_side == Side::ASK);
      REQUIRE((current.get()->m_user_order ==
        BookViewModel::UserOrder("TSX", Money(200), 100, OrderStatus::NEW)));
      ask_entries->insert(BookEntry(make_book_quote(Money(199), Side::ASK)), 0);
      ask_entries->insert(BookEntry(make_book_quote(Money(198), Side::ASK)), 0);
      ask_entries->insert(BookEntry(make_book_quote(Money(197), Side::ASK)), 0);
      ask_entries->insert(BookEntry(make_book_quote(Money(196), Side::ASK)), 0);
      ask_entries->insert(BookEntry(make_book_quote(Money(195), Side::ASK)), 0);
      ask_entries->insert(BookEntry(make_book_quote(Money(194), Side::ASK)), 0);
      ask_entries->insert(BookEntry(
        BookViewModel::UserOrder("TSX", Money(193), 100, OrderStatus::NEW)), 0);
      current.navigate_to_bids();
      REQUIRE(current.get().has_value());
      REQUIRE(current.get()->m_side == Side::BID);
      REQUIRE((current.get()->m_user_order ==
        BookViewModel::UserOrder("TSX", Money(107), 100, OrderStatus::NEW)));
      current.navigate_to_asks();
      REQUIRE(current.get()->m_side == Side::ASK);
      REQUIRE((current.get()->m_user_order ==
        BookViewModel::UserOrder("TSX", Money(200), 100, OrderStatus::NEW)));
    }
  }
}
