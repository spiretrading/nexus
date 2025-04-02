#include <doctest/doctest.h>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/TableCurrentIndexModel.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("TableCurrentIndexModel") {
  TEST_CASE("constructor") {
    auto table = std::make_shared<ArrayTableModel>();
    auto model = TableCurrentIndexModel(table);
    REQUIRE((model.get() == none));
    table->push({123});
    auto model2 = TableCurrentIndexModel(table, TableIndex(0, 0));
    REQUIRE((model2.get() == TableIndex(0, 0)));
    auto model3 = TableCurrentIndexModel(table, TableIndex(1, 0));
    REQUIRE((model3.get() == none));
    auto model4 = TableCurrentIndexModel(table, TableIndex(0, 1));
    REQUIRE((model4.get() == none));
  }

  TEST_CASE("add_operation") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({1, 1});
    table->push({2, 2});
    table->push({3, 3});
    auto model = TableCurrentIndexModel(table, TableIndex(1, 1));
    SUBCASE("insert_before_current") {
      table->insert({0, 0}, 0);
      REQUIRE((model.get() == TableIndex(2, 1)));
    }
    SUBCASE("insert_after_current") {
      table->insert({2, 2}, 3);
      REQUIRE((model.get() == TableIndex(1, 1)));
    }
  }

  TEST_CASE("remove_operation") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({1, 1});
    table->push({2, 2});
    table->push({3, 3});
    table->push({4, 4});
    auto model = TableCurrentIndexModel(table, TableIndex(2, 0));
    SUBCASE("remove_before_current") {
      table->remove(1);
      REQUIRE((model.get() == TableIndex(1, 0)));
    }
    SUBCASE("remove_current") {
      table->remove(2);
      REQUIRE((model.get() == none));
    }
    SUBCASE("remove_after_current") {
      table->remove(3);
      REQUIRE((model.get() == TableIndex(2, 0)));
    }
  }

  TEST_CASE("move_operation") {
    auto table = std::make_shared<ArrayTableModel>();
    table->push({1, 1});
    table->push({2, 2});
    table->push({3, 3});
    table->push({4, 4});
    auto model = TableCurrentIndexModel(table, TableIndex(2, 0));
    SUBCASE("move_current_up") {
      table->move(2, 3);
      REQUIRE((model.get() == TableIndex(3, 0)));
    }
    SUBCASE("move_above_current_down") {
      table->move(0, 2);
      REQUIRE((model.get() == TableIndex(1, 0)));
    }
    SUBCASE("move_below_current_up") {
      table->move(3, 1);
      REQUIRE((model.get() == TableIndex(3, 0)));
    }
  }

  TEST_CASE("transaction") {
    auto table = std::make_shared<ArrayTableModel>();
    auto model = TableCurrentIndexModel(table, TableIndex(0, 0));
    table->transact([&] {
      table->push({1});
      table->push({2});
      model.set(TableIndex(1, 0));
      table->remove(0);
    });
    REQUIRE((model.get() == TableIndex(0, 0)));
  }
}
