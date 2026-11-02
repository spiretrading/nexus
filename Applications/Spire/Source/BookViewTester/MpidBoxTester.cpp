#include <doctest/doctest.h>
#include "Spire/BookView/MpidBox.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/Stylist.hpp"
#include "Spire/Ui/TableBody.hpp"

using namespace boost::posix_time;
using namespace Nexus;
using namespace Spire;
using namespace Spire::Styles;

namespace {
  using UserOrder = BookViewModel::UserOrder;

  auto make_order(OrderStatus highlight, std::uint64_t transition) {
    auto order = UserOrder("TSX", Money::ONE, 100, OrderStatus::NEW);
    order.m_highlight = highlight;
    order.m_transition = transition;
    return order;
  }

  auto make_entry(const UserOrder& order) {
    return make_local_value_model<BookEntry>(order);
  }

  auto make_box(std::shared_ptr<LocalValueModel<BookEntry>> entry) {
    return MpidBox(std::move(entry), make_local_value_model(0),
      make_local_value_model(false));
  }

  auto make_quote_box(std::shared_ptr<LocalValueModel<int>> level) {
    return new MpidBox(make_local_value_model<BookEntry>(BookQuote("A", true,
      Venue("XTSE"), Quote(Money::ONE, 100, Side::BID),
      second_clock::universal_time())), std::move(level),
      make_local_value_model(false));
  }

  auto make_row(QWidget& table) {
    auto row = new QWidget(&table);
    match(*row, Row());
    return row;
  }
}

TEST_SUITE("MpidBox") {
  UI_TEST_CASE("settled_highlight_survives_unrelated_update") {
    auto entry = make_entry(make_order(OrderStatus::FILLED, 5));
    auto box = make_box(entry);
    REQUIRE(is_match(box, SettledRow()));
    REQUIRE(is_match(box, UserOrderRow(OrderStatus::FILLED)));
    auto order = make_order(OrderStatus::FILLED, 5);
    order.m_status = OrderStatus::PENDING_CANCEL;
    entry->set(order);
    REQUIRE(is_match(box, SettledRow()));
    REQUIRE(is_match(box, UserOrderRow(OrderStatus::FILLED)));
  }

  UI_TEST_CASE("new_transition_replays_highlight") {
    auto entry = make_entry(make_order(OrderStatus::FILLED, 5));
    auto box = make_box(entry);
    REQUIRE(is_match(box, SettledRow()));
    entry->set(make_order(OrderStatus::FILLED, 7));
    REQUIRE(!is_match(box, SettledRow()));
    REQUIRE(is_match(box, UserOrderRow(OrderStatus::FILLED)));
  }

  UI_TEST_CASE("expired_highlight_is_cleared") {
    auto entry = make_entry(make_order(OrderStatus::FILLED, 5));
    auto box = make_box(entry);
    REQUIRE(is_match(box, SettledRow()));
    entry->set(make_order(OrderStatus::NONE, 0));
    REQUIRE(!is_match(box, SettledRow()));
    REQUIRE(!is_match(box, UserOrderRow(OrderStatus::FILLED)));
  }

  UI_TEST_CASE("row_states") {
    auto table = QWidget();
    auto first_row = make_row(table);
    auto first_item = new QWidget(first_row);
    auto second_row = make_row(table);
    auto second_item = new QWidget(second_row);
    auto plain_item = new QWidget(&table);
    auto level = make_local_value_model(0);
    auto box = make_quote_box(level);
    REQUIRE(is_match(*box, PriceLevelRow(0)));
    box->setParent(plain_item);
    REQUIRE(!is_match(table, PriceLevelRow(0)));
    box->setParent(first_item);
    REQUIRE(is_match(*first_row, PriceLevelRow(0)));
    level->set(1);
    REQUIRE(!is_match(*first_row, PriceLevelRow(0)));
    REQUIRE(is_match(*first_row, PriceLevelRow(1)));
    REQUIRE(is_match(*box, PriceLevelRow(1)));
    box->setParent(second_item);
    REQUIRE(!is_match(*first_row, PriceLevelRow(1)));
    REQUIRE(is_match(*second_row, PriceLevelRow(1)));
    delete box;
    REQUIRE(!is_match(*second_row, PriceLevelRow(1)));
  }

  UI_TEST_CASE("shared_row_states") {
    auto table = QWidget();
    auto row = make_row(table);
    auto item = new QWidget(row);
    auto first_box = make_quote_box(make_local_value_model(0));
    first_box->setParent(item);
    auto second_box = make_quote_box(make_local_value_model(0));
    second_box->setParent(item);
    REQUIRE(is_match(*row, PriceLevelRow(0)));
    delete first_box;
    REQUIRE(is_match(*row, PriceLevelRow(0)));
    delete second_box;
    REQUIRE(!is_match(*row, PriceLevelRow(0)));
  }
}
