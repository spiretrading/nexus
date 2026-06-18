#include <doctest/doctest.h>
#include "Spire/BookView/PriceLevelModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace Nexus;
using namespace Spire;

TEST_SUITE("PriceLevelModel") {
  TEST_CASE("constructor_empty") {
    auto bid_levels = PriceLevelModel(std::make_shared<ArrayListModel<Money>>(),
      std::make_shared<LocalValueModel<int>>(5));
    REQUIRE(bid_levels.get_size() == 0);
  }
  TEST_CASE("constructor") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    prices->push(5 * Money::ONE);
    prices->push(5 * Money::ONE);
    prices->push(5 * Money::ONE);
    prices->push(4 * Money::ONE);
    prices->push(4 * Money::ONE);
    prices->push(3 * Money::ONE);
    prices->push(2 * Money::ONE);
    auto levels =
      PriceLevelModel(prices, std::make_shared<LocalValueModel<int>>(5));
    REQUIRE(levels.get_size() == 7);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 0);
    REQUIRE(levels.get(2) == 0);
    REQUIRE(levels.get(3) == 1);
    REQUIRE(levels.get(4) == 1);
    REQUIRE(levels.get(5) == 2);
    REQUIRE(levels.get(6) == 3);
  }
  TEST_CASE("constructor_max_level") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    prices->push(5 * Money::ONE);
    prices->push(5 * Money::ONE);
    prices->push(5 * Money::ONE);
    prices->push(4 * Money::ONE);
    prices->push(4 * Money::ONE);
    prices->push(3 * Money::ONE);
    prices->push(2 * Money::ONE);
    prices->push(2 * Money::ONE);
    prices->push(2 * Money::ONE);
    prices->push(2 * Money::ONE);
    prices->push(Money::ONE);
    prices->push(Money::ONE);
    prices->push(Money::CENT);
    auto levels =
      PriceLevelModel(prices, std::make_shared<LocalValueModel<int>>(3));
    REQUIRE(levels.get_size() == 13);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 0);
    REQUIRE(levels.get(2) == 0);
    REQUIRE(levels.get(3) == 1);
    REQUIRE(levels.get(4) == 1);
    REQUIRE(levels.get(5) == 2);
    REQUIRE(levels.get(6) == 3);
    REQUIRE(levels.get(7) == 3);
    REQUIRE(levels.get(8) == 3);
    REQUIRE(levels.get(9) == 3);
    REQUIRE(levels.get(10) == 3);
    REQUIRE(levels.get(11) == 3);
    REQUIRE(levels.get(12) == 3);
  }
  TEST_CASE("add") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    prices->push(1 * Money::ONE);
    prices->push(1 * Money::ONE);
    prices->push(2 * Money::ONE);
    prices->push(2 * Money::ONE);
    prices->push(3 * Money::ONE);
    prices->push(3 * Money::ONE);
    auto levels =
      PriceLevelModel(prices, std::make_shared<LocalValueModel<int>>(10));
    SUBCASE("insert_front_existing_price") {
      prices->insert(1 * Money::ONE, 0);
      REQUIRE(levels.get_size() == 7);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 0);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 1);
      REQUIRE(levels.get(5) == 2);
      REQUIRE(levels.get(6) == 2);
    }
    SUBCASE("insert_front_new_price") {
      prices->insert(Money::CENT, 0);
      REQUIRE(levels.get_size() == 7);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 1);
      REQUIRE(levels.get(2) == 1);
      REQUIRE(levels.get(3) == 2);
      REQUIRE(levels.get(4) == 2);
      REQUIRE(levels.get(5) == 3);
      REQUIRE(levels.get(6) == 3);
    }
    SUBCASE("insert_back_existing_price") {
      prices->push(3 * Money::ONE);
      REQUIRE(levels.get_size() == 7);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 1);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 2);
      REQUIRE(levels.get(5) == 2);
      REQUIRE(levels.get(6) == 2);
    }
    SUBCASE("insert_back_new_price") {
      prices->push(4 * Money::ONE);
      REQUIRE(levels.get_size() == 7);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 1);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 2);
      REQUIRE(levels.get(5) == 2);
      REQUIRE(levels.get(6) == 3);
    }
    SUBCASE("insert_top_price") {
      prices->insert(2 * Money::ONE, 2);
      REQUIRE(levels.get_size() == 7);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 1);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 1);
      REQUIRE(levels.get(5) == 2);
      REQUIRE(levels.get(6) == 2);
    }
    SUBCASE("insert_mid_price") {
      prices->insert(2 * Money::ONE, 3);
      REQUIRE(levels.get_size() == 7);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 1);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 1);
      REQUIRE(levels.get(5) == 2);
      REQUIRE(levels.get(6) == 2);
    }
    SUBCASE("insert_bottom_price") {
      prices->insert(2 * Money::ONE, 4);
      REQUIRE(levels.get_size() == 7);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 1);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 1);
      REQUIRE(levels.get(5) == 2);
      REQUIRE(levels.get(6) == 2);
    }
    SUBCASE("insert_new_price") {
      prices->insert(Money::ONE + Money::CENT, 2);
      REQUIRE(levels.get_size() == 7);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 1);
      REQUIRE(levels.get(3) == 2);
      REQUIRE(levels.get(4) == 2);
      REQUIRE(levels.get(5) == 3);
      REQUIRE(levels.get(6) == 3);
    }
  }
  TEST_CASE("remove") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    prices->push(1 * Money::ONE);
    prices->push(1 * Money::ONE);
    prices->push(1 * Money::ONE);
    prices->push(2 * Money::ONE);
    prices->push(2 * Money::ONE);
    prices->push(2 * Money::ONE);
    prices->push(3 * Money::ONE);
    prices->push(3 * Money::ONE);
    prices->push(3 * Money::ONE);
    auto levels =
      PriceLevelModel(prices, std::make_shared<LocalValueModel<int>>(10));
    SUBCASE("remove_front") {
      prices->remove(0);
      REQUIRE(levels.get_size() == 8);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 1);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 1);
      REQUIRE(levels.get(5) == 2);
      REQUIRE(levels.get(6) == 2);
      REQUIRE(levels.get(7) == 2);
      prices->remove(0);
      prices->remove(0);
      REQUIRE(levels.get_size() == 6);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 0);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 1);
      REQUIRE(levels.get(5) == 1);
    }
    SUBCASE("remove_back") {
      prices->remove(8);
      REQUIRE(levels.get_size() == 8);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 0);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 1);
      REQUIRE(levels.get(5) == 1);
      REQUIRE(levels.get(6) == 2);
      REQUIRE(levels.get(7) == 2);
      prices->remove(6);
      REQUIRE(levels.get_size() == 7);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 0);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 1);
      REQUIRE(levels.get(5) == 1);
      REQUIRE(levels.get(6) == 2);
      prices->remove(6);
      REQUIRE(levels.get_size() == 6);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 0);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 1);
      REQUIRE(levels.get(5) == 1);
    }
    SUBCASE("remove_middle") {
      prices->remove(4);
      REQUIRE(levels.get_size() == 8);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 0);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 1);
      REQUIRE(levels.get(5) == 2);
      REQUIRE(levels.get(6) == 2);
      REQUIRE(levels.get(7) == 2);
      prices->remove(3);
      REQUIRE(levels.get_size() == 7);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 0);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 2);
      REQUIRE(levels.get(5) == 2);
      REQUIRE(levels.get(6) == 2);
      prices->remove(3);
      REQUIRE(levels.get_size() == 6);
      REQUIRE(levels.get(0) == 0);
      REQUIRE(levels.get(1) == 0);
      REQUIRE(levels.get(2) == 0);
      REQUIRE(levels.get(3) == 1);
      REQUIRE(levels.get(4) == 1);
      REQUIRE(levels.get(5) == 1);
    }
  }
  TEST_CASE("decrease_max_level") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    prices->push(10 * Money::ONE);
    prices->push(11 * Money::ONE);
    prices->push(12 * Money::ONE);
    prices->push(13 * Money::ONE);
    auto max_level = std::make_shared<LocalValueModel<int>>(3);
    auto levels = PriceLevelModel(prices, max_level);
    max_level->set(2);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 1);
    REQUIRE(levels.get(2) == 2);
    REQUIRE(levels.get(3) == 2);
  }
  TEST_CASE("increase_max_level") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    prices->push(10 * Money::ONE);
    prices->push(11 * Money::ONE);
    prices->push(12 * Money::ONE);
    prices->push(13 * Money::ONE);
    prices->push(14 * Money::ONE);
    prices->push(15 * Money::ONE);
    prices->push(16 * Money::ONE);
    auto max_level = std::make_shared<LocalValueModel<int>>(3);
    auto levels = PriceLevelModel(prices, max_level);
    max_level->set(5);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 1);
    REQUIRE(levels.get(2) == 2);
    REQUIRE(levels.get(3) == 3);
    REQUIRE(levels.get(4) == 4);
    REQUIRE(levels.get(5) == 5);
    REQUIRE(levels.get(6) == 5);
  }
  TEST_CASE("remove_with_max_level") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    prices->push(10 * Money::ONE);
    prices->push(11 * Money::ONE);
    prices->push(12 * Money::ONE);
    prices->push(12 * Money::ONE);
    prices->push(12 * Money::ONE);
    prices->push(12 * Money::ONE);
    prices->push(12 * Money::ONE);
    auto max_level = std::make_shared<LocalValueModel<int>>(4);
    auto levels = PriceLevelModel(prices, max_level);
    prices->remove(0);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 1);
    REQUIRE(levels.get(2) == 1);
    REQUIRE(levels.get(3) == 1);
    REQUIRE(levels.get(4) == 1);
    REQUIRE(levels.get(5) == 1);
  }
  TEST_CASE("clear_prices") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    auto max_level = std::make_shared<LocalValueModel<int>>(3);
    auto levels = PriceLevelModel(prices, max_level);
    prices->push(Money(10 * Money::ONE));
    prices->remove(0);
    REQUIRE(levels.get_size() == 0);
  }
  TEST_CASE("last_price") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    auto max_level = std::make_shared<LocalValueModel<int>>(3);
    auto levels = PriceLevelModel(prices, max_level);
    prices->push(10 * Money::ONE);
    prices->push(11 * Money::ONE);
    prices->push(12 * Money::ONE);
    prices->push(12 * Money::ONE);
    REQUIRE(levels.get(3) == 2);
  }
  TEST_CASE("last_price_at_max_level") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    auto max_level = std::make_shared<LocalValueModel<int>>(2);
    auto levels = PriceLevelModel(prices, max_level);
    prices->push(10 * Money::ONE);
    prices->push(11 * Money::ONE);
    prices->push(12 * Money::ONE);
    prices->push(13 * Money::ONE);
    prices->push(14 * Money::ONE);
    REQUIRE(levels.get(4) == 2);
  }
  TEST_CASE("remove_same_price") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    auto max_level = std::make_shared<LocalValueModel<int>>(3);
    auto levels = PriceLevelModel(prices, max_level);
    prices->push(10 * Money::ONE);
    prices->push(10 * Money::ONE);
    prices->push(11 * Money::ONE);
    prices->push(11 * Money::ONE);
    prices->push(12 * Money::ONE);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 0);
    REQUIRE(levels.get(2) == 1);
    REQUIRE(levels.get(3) == 1);
    REQUIRE(levels.get(4) == 2);
    prices->remove(1);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 1);
    REQUIRE(levels.get(2) == 1);
    REQUIRE(levels.get(3) == 2);
  }
  TEST_CASE("remove_max_price") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    auto max_level = std::make_shared<LocalValueModel<int>>(2);
    auto levels = PriceLevelModel(prices, max_level);
    prices->push(10 * Money::ONE);
    prices->push(11 * Money::ONE);
    prices->push(12 * Money::ONE);
    prices->push(13 * Money::ONE);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 1);
    REQUIRE(levels.get(2) == 2);
    REQUIRE(levels.get(3) == 2);
    prices->remove(2);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 1);
    REQUIRE(levels.get(2) == 2);
  }
  TEST_CASE("insert_middle") {
    auto prices = std::make_shared<ArrayListModel<Money>>();
    auto max_level = std::make_shared<LocalValueModel<int>>(3);
    auto levels = PriceLevelModel(prices, max_level);
    prices->push(10 * Money::ONE);
    prices->push(12 * Money::ONE);
    prices->push(14 * Money::ONE);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 1);
    REQUIRE(levels.get(2) == 2);
    prices->insert(11 * Money::ONE, 1);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 1);
    REQUIRE(levels.get(2) == 2);
    REQUIRE(levels.get(3) == 3);
    prices->insert(11 * Money::ONE, 2);
    REQUIRE(levels.get(0) == 0);
    REQUIRE(levels.get(1) == 1);
    REQUIRE(levels.get(2) == 1);
    REQUIRE(levels.get(3) == 2);
    REQUIRE(levels.get(4) == 3);
  }
}
