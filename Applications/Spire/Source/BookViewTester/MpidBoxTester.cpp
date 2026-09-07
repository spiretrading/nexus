#include <doctest/doctest.h>
#include "Spire/BookView/MpidBox.hpp"
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/SpireTester/SpireTester.hpp"
#include "Spire/Styles/Stylist.hpp"

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
}

TEST_SUITE("MpidBox") {
  TEST_CASE("settled_highlight_survives_unrelated_update") {
    run_test([] {
      auto entry = make_entry(make_order(OrderStatus::FILLED, 5));
      auto box = make_box(entry);
      REQUIRE(is_match(box, SettledRow()));
      REQUIRE(is_match(box, UserOrderRow(OrderStatus::FILLED)));
      auto order = make_order(OrderStatus::FILLED, 5);
      order.m_status = OrderStatus::PENDING_CANCEL;
      entry->set(order);
      REQUIRE(is_match(box, SettledRow()));
      REQUIRE(is_match(box, UserOrderRow(OrderStatus::FILLED)));
    });
  }

  TEST_CASE("new_transition_replays_highlight") {
    run_test([] {
      auto entry = make_entry(make_order(OrderStatus::FILLED, 5));
      auto box = make_box(entry);
      REQUIRE(is_match(box, SettledRow()));
      entry->set(make_order(OrderStatus::FILLED, 7));
      REQUIRE(!is_match(box, SettledRow()));
      REQUIRE(is_match(box, UserOrderRow(OrderStatus::FILLED)));
    });
  }

  TEST_CASE("expired_highlight_is_cleared") {
    run_test([] {
      auto entry = make_entry(make_order(OrderStatus::FILLED, 5));
      auto box = make_box(entry);
      REQUIRE(is_match(box, SettledRow()));
      entry->set(make_order(OrderStatus::NONE, 0));
      REQUIRE(!is_match(box, SettledRow()));
      REQUIRE(!is_match(box, UserOrderRow(OrderStatus::FILLED)));
    });
  }
}
