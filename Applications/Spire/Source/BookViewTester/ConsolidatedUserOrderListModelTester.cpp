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

  void verify_orders_match(const BookViewModel::UserOrderListModel& orders,
      const std::vector<std::tuple<std::string, Money, int>>& expected) {
    REQUIRE(orders.get_size() == expected.size());
    for(const auto& [dest, price, size] : expected) {
      auto order = find(orders, price, dest);
      REQUIRE(order.has_value());
      REQUIRE(order->m_size == size);
    }
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

  TEST_CASE("add_update") {
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
    user_orders->set(0,
      BookViewModel::UserOrder("TSX", Money::CENT, 300, OrderStatus::NEW));
    user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 600);
  }

  TEST_CASE("remove_order") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 200, OrderStatus::NEW));
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    REQUIRE(consolidated_orders.get_size() == 1);
    user_orders->remove(0);
    REQUIRE(consolidated_orders.get_size() == 1);
    auto user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 200);
    user_orders->remove(0);
    REQUIRE(consolidated_orders.get_size() == 0);
  }

  TEST_CASE("zero_size_order") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 0, OrderStatus::NEW));
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    REQUIRE(consolidated_orders.get_size() == 1);
    auto user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 100);
    user_orders->set(
      0, BookViewModel::UserOrder("TSX", Money::CENT, 0, OrderStatus::NEW));
    REQUIRE(consolidated_orders.get_size() == 1);
    user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 0);
  }

  TEST_CASE("negative_size_order") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, -50, OrderStatus::NEW));
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    REQUIRE(consolidated_orders.get_size() == 1);
    auto user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 50);
  }

  TEST_CASE("change_price") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 200, OrderStatus::NEW));
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    REQUIRE(consolidated_orders.get_size() == 1);
    user_orders->set(0,
      BookViewModel::UserOrder("TSX", 2 * Money::CENT, 100, OrderStatus::NEW));
    REQUIRE(consolidated_orders.get_size() == 2);
    auto user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 200);
    user_order = find(consolidated_orders, 2 * Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 100);
  }

  TEST_CASE("change_destination") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 200, OrderStatus::NEW));
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    REQUIRE(consolidated_orders.get_size() == 1);
    user_orders->set(
      0, BookViewModel::UserOrder("CHIX", Money::CENT, 100, OrderStatus::NEW));
    REQUIRE(consolidated_orders.get_size() == 2);
    auto user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 200);
    user_order = find(consolidated_orders, Money::CENT, "CHIX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 100);
  }

  TEST_CASE("change_price_and_destination") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("CHIX", 2 * Money::CENT, 200, OrderStatus::NEW));
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    REQUIRE(consolidated_orders.get_size() == 2);
    user_orders->set(0,
      BookViewModel::UserOrder("CHIX", 2 * Money::CENT, 300, OrderStatus::NEW));
    REQUIRE(consolidated_orders.get_size() == 1);
    auto user_order = find(consolidated_orders, 2 * Money::CENT, "CHIX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 500);
  }

  TEST_CASE("batch_operations") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    user_orders->transact([&] {
      user_orders->push(
        BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
      user_orders->push(
        BookViewModel::UserOrder("TSX", Money::CENT, 200, OrderStatus::NEW));
      user_orders->push(BookViewModel::UserOrder(
        "CHIX", 2 * Money::CENT, 300, OrderStatus::NEW));
    });
    REQUIRE(consolidated_orders.get_size() == 2);
    auto user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 300);
    user_order = find(consolidated_orders, 2 * Money::CENT, "CHIX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 300);
    user_orders->transact([&] {
      user_orders->set(0, BookViewModel::UserOrder(
        "CHIX", 2 * Money::CENT, 100, OrderStatus::NEW));
      user_orders->set(1, BookViewModel::UserOrder(
        "CHIX", 2 * Money::CENT, 200, OrderStatus::NEW));
    });
    REQUIRE(consolidated_orders.get_size() == 1);
    user_order = find(consolidated_orders, 2 * Money::CENT, "CHIX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 600);
  }

  TEST_CASE("multiple_orders_same_price_destination") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    for(auto i = 0; i < 10; ++i) {
      user_orders->push(
        BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    }
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    REQUIRE(consolidated_orders.get_size() == 1);
    auto user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 1000);
    for(auto i = 0; i < 9; ++i) {
      user_orders->remove(0);
      REQUIRE(consolidated_orders.get_size() == 1);
      user_order = find(consolidated_orders, Money::CENT, "TSX");
      REQUIRE(user_order.has_value());
      REQUIRE(user_order->m_size == (9 - i) * 100);
    }
    user_orders->remove(0);
    REQUIRE(consolidated_orders.get_size() == 0);
  }

  TEST_CASE("empty_source_model") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    REQUIRE(consolidated_orders.get_size() == 0);
  }

  TEST_CASE("cancellation_to_zero") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    REQUIRE(consolidated_orders.get_size() == 1);
    user_orders->set(0,
      BookViewModel::UserOrder("TSX", Money::CENT, 0, OrderStatus::CANCELED));
    REQUIRE(consolidated_orders.get_size() == 1);
    auto user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 0);
    REQUIRE(user_order->m_status == OrderStatus::CANCELED);
  }

  TEST_CASE("add_after_removal") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    REQUIRE(consolidated_orders.get_size() == 1);
    user_orders->remove(0);
    REQUIRE(consolidated_orders.get_size() == 0);
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 200, OrderStatus::NEW));
    REQUIRE(consolidated_orders.get_size() == 1);
    auto user_order = find(consolidated_orders, Money::CENT, "TSX");
    REQUIRE(user_order.has_value());
    REQUIRE(user_order->m_size == 200);
  }

  TEST_CASE("mixed_operations") {
    auto user_orders = std::make_shared<UserOrderArrayListModel>();
    auto consolidated_orders = ConsolidatedUserOrderListModel(user_orders);
    auto expected = std::vector<std::tuple<std::string, Money, int>>();
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("CHIX", 2 * Money::CENT, 200, OrderStatus::NEW));
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 300, OrderStatus::NEW));
    expected = {
      {"TSX", Money::CENT, 400},
      {"CHIX", 2 * Money::CENT, 200}
    };
    verify_orders_match(consolidated_orders, expected);
    user_orders->remove(0);
    expected = {
      {"TSX", Money::CENT, 300},
      {"CHIX", 2 * Money::CENT, 200}
    };
    verify_orders_match(consolidated_orders, expected);
    user_orders->push(
      BookViewModel::UserOrder("NYSE", 3 * Money::CENT, 500, OrderStatus::NEW));
    expected = {
      {"TSX", Money::CENT, 300},
      {"CHIX", 2 * Money::CENT, 200},
      {"NYSE", 3 * Money::CENT, 500}
    };
    verify_orders_match(consolidated_orders, expected);
    user_orders->set(0,
      BookViewModel::UserOrder("NYSE", 3 * Money::CENT, 100, OrderStatus::NEW));
    expected = {
      {"TSX", Money::CENT, 300},
      {"NYSE", 3 * Money::CENT, 600}
    };
    verify_orders_match(consolidated_orders, expected);
    user_orders->transact([&] {
      while(user_orders->get_size() > 0) {
        user_orders->remove(0);
      }
    });
    REQUIRE(consolidated_orders.get_size() == 0);
    user_orders->push(
      BookViewModel::UserOrder("TSX", Money::CENT, 100, OrderStatus::NEW));
    expected = {
      {"TSX", Money::CENT, 100}
    };
    verify_orders_match(consolidated_orders, expected);
  }
}
