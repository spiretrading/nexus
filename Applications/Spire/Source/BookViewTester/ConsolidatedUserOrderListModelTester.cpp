#include <doctest/doctest.h>
#include "Spire/BookView/ConsolidatedUserOrderListModel.hpp"

using namespace boost;
using namespace Nexus;
using namespace Spire;

namespace {
  using UserOrderArrayListModel = ArrayListModel<BookViewModel::UserOrder>;

  optional<BookViewModel::UserOrder> find(
      const BookViewModel::UserOrderListModel& user_orders, Money price,
      const std::string& destination) {
    auto i = std::find_if(user_orders.begin(), user_orders.end(),
      [&] (const BookViewModel::UserOrder& user_order) {
        return user_order.m_price == price &&
          user_order.m_destination == destination;
      });
    if(i == user_orders.end()) {
      return none;
    }
    return *i;
  }
}

TEST_SUITE("ConsolidatedUserOrderListModel") {
  TEST_CASE("constructor") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("TSX", 2 * Money::CENT, 200, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("CHIX", Money::CENT, 200, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("CHIX", 3 * Money::CENT, 300, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("TSX", 2 * Money::CENT, 400, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("CHIX", Money::CENT, 100, OrderStatus::NEW));
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    REQUIRE(consolidated_orders.get_size() == 4);
    auto user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 100);
    user_order = find(consolidated_orders, 2 * Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 600);
    user_order = find(consolidated_orders, Money::CENT, "CHIX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 300);
    user_order = find(consolidated_orders, 3 * Money::CENT, "CHIX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 300);
  }
  TEST_CASE("add") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("TSX", 2 * Money::CENT, 200, OrderStatus::NEW));
    REQUIRE(consolidated_orders.get_size() == 2);
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 300, OrderStatus::NEW));
    REQUIRE(consolidated_orders.get_size() == 2);
    auto user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 400);
  }
}
