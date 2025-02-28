#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ListCurrentIndexModel.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("ListCurrentIndexModel") {
  TEST_CASE("constructor") {
    auto list = std::make_shared<ArrayListModel<int>>();
    auto model = ListCurrentIndexModel(list);
    REQUIRE((model.get() == none));
    list->push(123);
    auto model2 = ListCurrentIndexModel(list, 0);
    REQUIRE((model2.get() == 0));
    auto model3 = ListCurrentIndexModel(list, 5);
    REQUIRE((model3.get() == none));
  }

  TEST_CASE("add_operation") {
    auto list = std::make_shared<ArrayListModel<int>>();
    list->push(1);
    list->push(2);
    list->push(3);
    auto model = ListCurrentIndexModel(list, 1);
    SUBCASE("insert_before_current") {
      list->insert(0, 0);
      REQUIRE((model.get() == 2));
    }
    SUBCASE("insert_after_current") {
      list->insert(3, 2);
      REQUIRE((model.get() == 1));
    }
  }

  TEST_CASE("remove_operation") {
    auto list = std::make_shared<ArrayListModel<int>>();
    list->push(1);
    list->push(2);
    list->push(3);
    list->push(4);
    auto model = ListCurrentIndexModel(list, 2);
    SUBCASE("remove_before_current") {
      list->remove(1);
      REQUIRE((model.get() == 1));
    }
    SUBCASE("remove_current") {
      list->remove(2);
      REQUIRE((model.get() == 2));
    }
    SUBCASE("remove_after_current") {
      list->remove(3);
      REQUIRE((model.get() == 2));
    }
  }

  TEST_CASE("move_operation") {
    auto list = std::make_shared<ArrayListModel<int>>();
    list->push(1);
    list->push(2);
    list->push(3);
    list->push(4);
    auto model = ListCurrentIndexModel(list, 2);
    SUBCASE("move_current_up") {
      list->move(2, 3);
      REQUIRE((model.get() == 3));
    }
    SUBCASE("move_above_current_down") {
      list->move(0, 2);
      REQUIRE((model.get() == 1));
    }
    SUBCASE("move_below_current_up") {
      list->move(3, 1);
      REQUIRE((model.get() == 3));
    }
  }

  TEST_CASE("transaction") {
    auto list = std::make_shared<ArrayListModel<int>>();
    auto model = ListCurrentIndexModel(list, 0);
    list->transact([&] {
      list->push(1);
      list->push(2);
      model.set(1);
      list->remove(0);
    });
    REQUIRE((model.get() == 0));
  }
}
