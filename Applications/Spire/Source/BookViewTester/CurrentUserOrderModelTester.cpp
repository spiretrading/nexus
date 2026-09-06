#include <doctest/doctest.h>
#include "Spire/BookView/BookViewCurrentTableModel.hpp"
#include "Spire/BookView/BookViewTableModel.hpp"
#include "Spire/BookView/CurrentUserOrderModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;

namespace {
  using UserOrder = BookViewModel::UserOrder;

  auto make_book_quote(Money price, Side side) {
    return BookQuote("TSX", true, Venues::TSX, Quote(price, 100, side),
      time_from_string("2016-07-31 19:00:00"));
  }

  auto make_user_order(Money price) {
    return UserOrder("TSX", price, 100, OrderStatus::NEW);
  }

  auto make_side() {
    auto entries = std::make_shared<ArrayListModel<BookEntry>>();
    auto table = std::make_shared<SortedTableModel>(
      make_book_view_table_model(entries));
    return std::tuple(
      entries, table, std::make_shared<BookViewCurrentTableModel>(table));
  }

  void require_current(
      const CurrentUserOrderModel& model, Side side, const UserOrder& order) {
    REQUIRE(model.get().has_value());
    REQUIRE(model.get()->m_side == side);
    REQUIRE((model.get()->m_user_order == order));
  }
}

TEST_SUITE("CurrentUserOrderModel") {
  TEST_CASE("undo_navigation") {
    auto [bid_entries, bid_table, current_bid] = make_side();
    auto [ask_entries, ask_table, current_ask] = make_side();
    auto current =
      CurrentUserOrderModel(bid_table, current_bid, ask_table, current_ask);
    SUBCASE("no_shifting") {
      bid_entries->push(make_user_order(Money(107)));
      bid_entries->push(make_book_quote(Money(106), Side::BID));
      bid_entries->push(make_book_quote(Money(105), Side::BID));
      bid_entries->push(make_book_quote(Money(104), Side::BID));
      bid_entries->push(make_user_order(Money(103)));
      bid_entries->push(make_book_quote(Money(102), Side::BID));
      bid_entries->push(make_book_quote(Money(101), Side::BID));
      bid_entries->push(make_book_quote(Money(100), Side::BID));
      ask_entries->push(make_book_quote(Money(200), Side::ASK));
      ask_entries->push(make_book_quote(Money(201), Side::ASK));
      ask_entries->push(make_book_quote(Money(202), Side::ASK));
      ask_entries->push(make_user_order(Money(203)));
      current_bid->set(TableIndex(4, 0));
      require_current(current, Side::BID, make_user_order(Money(103)));
      current.navigate_to_asks();
      require_current(current, Side::ASK, make_user_order(Money(203)));
      current.navigate_to_bids();
      require_current(current, Side::BID, make_user_order(Money(103)));
      current.navigate_to_asks();
      require_current(current, Side::ASK, make_user_order(Money(203)));
    }
    SUBCASE("shifting") {
      bid_entries->push(make_user_order(Money(107)));
      bid_entries->push(make_book_quote(Money(106), Side::BID));
      bid_entries->push(make_book_quote(Money(105), Side::BID));
      bid_entries->push(make_book_quote(Money(104), Side::BID));
      bid_entries->push(make_book_quote(Money(103), Side::BID));
      bid_entries->push(make_book_quote(Money(102), Side::BID));
      bid_entries->push(make_book_quote(Money(101), Side::BID));
      bid_entries->push(make_user_order(Money(100)));
      ask_entries->push(make_user_order(Money(200)));
      current_bid->set(TableIndex(0, 0));
      require_current(current, Side::BID, make_user_order(Money(107)));
      current.navigate_to_asks();
      require_current(current, Side::ASK, make_user_order(Money(200)));
      for(auto price : {199, 198, 197, 196, 195, 194}) {
        ask_entries->insert(
          BookEntry(make_book_quote(Money(price), Side::ASK)), 0);
      }
      ask_entries->insert(BookEntry(make_user_order(Money(193))), 0);
      current.navigate_to_bids();
      require_current(current, Side::BID, make_user_order(Money(107)));
      current.navigate_to_asks();
      require_current(current, Side::ASK, make_user_order(Money(200)));
    }
  }

  TEST_CASE("navigate_from_a_row_beyond_the_target_size") {
    auto [bid_entries, bid_table, current_bid] = make_side();
    auto [ask_entries, ask_table, current_ask] = make_side();
    auto current =
      CurrentUserOrderModel(bid_table, current_bid, ask_table, current_ask);
    auto bid_order = make_user_order(Money(75));
    for(auto i = 0; i != 30; ++i) {
      if(i == 25) {
        bid_entries->push(bid_order);
      } else {
        bid_entries->push(make_book_quote(Money(100 - i), Side::BID));
      }
    }
    auto ask_order = make_user_order(Money(203));
    for(auto i = 0; i != 8; ++i) {
      if(i == 3) {
        ask_entries->push(ask_order);
      } else {
        ask_entries->push(make_book_quote(Money(200 + i), Side::ASK));
      }
    }
    current_bid->set(TableIndex(25, 0));
    require_current(current, Side::BID, bid_order);
    current.navigate_to_asks();
    require_current(current, Side::ASK, ask_order);
  }

  TEST_CASE("navigate_after_a_crossing_that_selected_nothing") {
    auto [bid_entries, bid_table, current_bid] = make_side();
    auto [ask_entries, ask_table, current_ask] = make_side();
    auto current =
      CurrentUserOrderModel(bid_table, current_bid, ask_table, current_ask);
    auto bid_order = make_user_order(Money(102));
    bid_entries->push(make_book_quote(Money(104), Side::BID));
    bid_entries->push(make_book_quote(Money(103), Side::BID));
    bid_entries->push(bid_order);
    bid_entries->push(make_book_quote(Money(101), Side::BID));
    bid_entries->push(make_book_quote(Money(100), Side::BID));
    for(auto i = 0; i != 10; ++i) {
      ask_entries->push(make_book_quote(Money(200 + i), Side::ASK));
    }
    current_bid->set(TableIndex(2, 0));
    require_current(current, Side::BID, bid_order);
    current.navigate_to_asks();
    require_current(current, Side::BID, bid_order);
    auto ask_order = make_user_order(Money(205));
    ask_entries->set(5, ask_order);
    current.navigate_to_asks();
    require_current(current, Side::ASK, ask_order);
  }
}
