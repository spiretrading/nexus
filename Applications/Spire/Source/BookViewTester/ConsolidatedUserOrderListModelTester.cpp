#include <doctest/doctest.h>
#include "Spire/BookView/ConsolidatedUserOrderListModel.hpp"
#include "Spire/Spire/ArrayListModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  using UserOrder = BookViewModel::UserOrder;

  auto make_order(Money price, Quantity size) {
    return UserOrder("TSX", price, size, OrderStatus::NEW);
  }

  auto make_source(std::vector<UserOrder> orders) {
    auto source = std::make_shared<ArrayListModel<UserOrder>>();
    for(auto& order : orders) {
      source->push(order);
    }
    return source;
  }

  void transition(ArrayListModel<UserOrder>& source, int index,
      OrderStatus status, Quantity size) {
    auto order = source.get(index);
    order.m_status = status;
    order.m_size = size;
    source.set(index, order);
  }
}

TEST_SUITE("ConsolidatedUserOrderListModel") {
  TEST_CASE("constructor_empty") {
    auto source = make_source({});
    auto model = ConsolidatedUserOrderListModel(source);
    REQUIRE(model.get_size() == 0);
  }

  TEST_CASE("constructor_consolidates") {
    auto source = make_source(
      {make_order(Money::ONE, 100), make_order(Money::ONE, 200)});
    auto model = ConsolidatedUserOrderListModel(source);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_price == Money::ONE);
    REQUIRE(model.get(0).m_destination == "TSX");
    REQUIRE(model.get(0).m_size == 300);
  }

  TEST_CASE("merge_same_level") {
    auto source = make_source({});
    auto model = ConsolidatedUserOrderListModel(source);
    source->push(make_order(Money::ONE, 100));
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_size == 100);
    source->push(make_order(Money::ONE, 200));
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_size == 300);
  }

  TEST_CASE("distinct_prices_are_separate_levels") {
    auto source = make_source({});
    auto model = ConsolidatedUserOrderListModel(source);
    source->push(make_order(2 * Money::ONE, 100));
    source->push(make_order(Money::ONE, 200));
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0).m_price == Money::ONE);
    REQUIRE(model.get(1).m_price == 2 * Money::ONE);
  }

  TEST_CASE("distinct_destinations_are_separate_levels") {
    auto source = make_source({});
    auto model = ConsolidatedUserOrderListModel(source);
    source->push(make_order(Money::ONE, 100));
    source->push(UserOrder("NEOE", Money::ONE, 200, OrderStatus::NEW));
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0).m_destination == "NEOE");
    REQUIRE(model.get(1).m_destination == "TSX");
  }

  TEST_CASE("remove_one_contributor") {
    auto source = make_source(
      {make_order(Money::ONE, 100), make_order(Money::ONE, 200)});
    auto model = ConsolidatedUserOrderListModel(source);
    REQUIRE(model.get(0).m_size == 300);
    source->remove(0);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_size == 200);
  }

  TEST_CASE("remove_last_contributor") {
    auto source = make_source({make_order(Money::ONE, 100)});
    auto model = ConsolidatedUserOrderListModel(source);
    REQUIRE(model.get_size() == 1);
    source->remove(0);
    REQUIRE(model.get_size() == 0);
  }

  TEST_CASE("update_size") {
    auto source = make_source({make_order(Money::ONE, 100)});
    auto model = ConsolidatedUserOrderListModel(source);
    transition(*source, 0, OrderStatus::NEW, 60);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_size == 60);
  }

  TEST_CASE("price_change_moves_contribution") {
    auto source = make_source(
      {make_order(Money::ONE, 100), make_order(Money::ONE, 200)});
    auto model = ConsolidatedUserOrderListModel(source);
    REQUIRE(model.get_size() == 1);
    auto order = source->get(0);
    order.m_price = 2 * Money::ONE;
    source->set(0, order);
    REQUIRE(model.get_size() == 2);
    REQUIRE(model.get(0).m_price == Money::ONE);
    REQUIRE(model.get(0).m_size == 200);
    REQUIRE(model.get(1).m_price == 2 * Money::ONE);
    REQUIRE(model.get(1).m_size == 100);
  }

  TEST_CASE("terminal_order_is_retained_and_stamped") {
    for(auto status : {OrderStatus::CANCELED, OrderStatus::FILLED,
        OrderStatus::REJECTED, OrderStatus::EXPIRED,
        OrderStatus::DONE_FOR_DAY}) {
      auto source = make_source({make_order(Money::ONE, 100)});
      auto model = ConsolidatedUserOrderListModel(source);
      REQUIRE(model.get(0).m_transition == 0);
      transition(*source, 0, status, 0);
      REQUIRE(model.get_size() == 1);
      REQUIRE(model.get(0).m_size == 0);
      REQUIRE(model.get(0).m_status == status);
      REQUIRE(model.get(0).m_highlight == status);
      REQUIRE(model.get(0).m_transition != 0);
    }
  }

  TEST_CASE("each_partial_fill_stamps_a_new_transition") {
    auto source = make_source({make_order(Money::ONE, 100)});
    auto model = ConsolidatedUserOrderListModel(source);
    transition(*source, 0, OrderStatus::PARTIALLY_FILLED, 60);
    auto first = model.get(0).m_transition;
    REQUIRE(first != 0);
    transition(*source, 0, OrderStatus::PARTIALLY_FILLED, 20);
    auto second = model.get(0).m_transition;
    REQUIRE(second != 0);
    REQUIRE(second != first);
  }

  TEST_CASE("cancel_of_one_contributor_stamps_the_level") {
    auto source = make_source(
      {make_order(Money::ONE, 100), make_order(Money::ONE, 200)});
    auto model = ConsolidatedUserOrderListModel(source);
    transition(*source, 0, OrderStatus::CANCELED, 0);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_size == 200);
    REQUIRE(model.get(0).m_transition != 0);
    REQUIRE(model.get(0).m_status == OrderStatus::CANCELED);
  }

  TEST_CASE("replacement_order_preserves_highlight") {
    auto source = make_source({make_order(Money::ONE, 100)});
    auto model = ConsolidatedUserOrderListModel(source);
    transition(*source, 0, OrderStatus::CANCELED, 0);
    auto highlight = model.get(0).m_transition;
    REQUIRE(highlight != 0);
    source->push(make_order(Money::ONE, 100));
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_size == 100);
    REQUIRE(model.get(0).m_transition == highlight);
    REQUIRE(model.get(0).m_highlight == OrderStatus::CANCELED);
  }

  TEST_CASE("sibling_update_preserves_highlight") {
    auto source = make_source(
      {make_order(Money::ONE, 100), make_order(Money::ONE, 200)});
    auto model = ConsolidatedUserOrderListModel(source);
    transition(*source, 0, OrderStatus::CANCELED, 0);
    auto highlight = model.get(0).m_transition;
    REQUIRE(highlight != 0);
    transition(*source, 1, OrderStatus::PENDING_CANCEL, 200);
    REQUIRE(model.get(0).m_transition == highlight);
    REQUIRE(model.get(0).m_highlight == OrderStatus::CANCELED);
  }

  TEST_CASE("transitioning_level_survives_source_removal") {
    auto source = make_source({make_order(Money::ONE, 100)});
    auto model = ConsolidatedUserOrderListModel(source);
    transition(*source, 0, OrderStatus::CANCELED, 0);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_highlight == OrderStatus::CANCELED);
    source->remove(0);
    REQUIRE(model.get_size() == 1);
    REQUIRE(model.get(0).m_highlight == OrderStatus::CANCELED);
  }
}
