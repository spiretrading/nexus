#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ListIndexValueModel.hpp"
#include "Spire/Spire/LocalValueModel.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("ListIndexValueModel") {
  TEST_CASE("get_set") {
    auto list = std::make_shared<ArrayListModel<int>>();
    list->push(12);
    list->push(4);
    list->push(8);
    list->push(42);
    auto value = std::make_shared<LocalValueModel<int>>();
    value->set(8);
    auto index = ListIndexValueModel(list, value);
    REQUIRE(index.get() == 2);
    REQUIRE(index.set(-1) == QValidator::Invalid);
    REQUIRE(index.set(4) == QValidator::Invalid);
    value->set(1);
    REQUIRE(index.get() == none);
    value->set(12);
    REQUIRE(index.get() == 0);
  }

  TEST_CASE("list_modifications") {
    auto list = std::make_shared<ArrayListModel<int>>();
    list->push(12);
    list->push(4);
    list->push(6);
    list->push(4);
    auto value = std::make_shared<LocalValueModel<int>>();
    value->set(4);
    auto index = ListIndexValueModel(list, value);
    REQUIRE(index.get() == 1);

    SUBCASE("add_preceeding_value") {
      list->insert(42, 0);
      REQUIRE(index.get() == 2);
    }

    SUBCASE("add_proceeding_value") {
      list->push(42);
      REQUIRE(index.get() == 1);
    }

    SUBCASE("remove_preceeding_value") {
      list->remove(0);
      REQUIRE(index.get() == 0);
    }

    SUBCASE("remove_proceeding_value") {
      list->remove(2);
      REQUIRE(index.get() == 1);
    }

    SUBCASE("remove_value") {
      list->remove(1);
      REQUIRE(index.get() == 2);
      list->remove(2);
      REQUIRE(index.get() == none);
    }

    SUBCASE("update_value") {
      list->set(0, 12);
      REQUIRE(index.get() == 1);
      list->set(1, 13);
      REQUIRE(index.get() == 3);
      list->set(3, 42);
      REQUIRE(index.get() == none);
    }
  }
}
