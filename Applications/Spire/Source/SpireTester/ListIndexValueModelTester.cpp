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
    auto index = std::make_shared<ListIndexValueModel<int>>(list, value);
    REQUIRE(index->get() == 2);
    REQUIRE(index->set(-1) == QValidator::Invalid);
    REQUIRE(index->set(4) == QValidator::Invalid);
    value->set(1);
    REQUIRE(index->get() == none);
    value->set(12);
    REQUIRE(index->get() == 0);
  }
}
