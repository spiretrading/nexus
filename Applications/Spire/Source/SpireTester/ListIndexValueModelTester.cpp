#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Spire/Spire/LocalValueModel.hpp"
#include "Spire/Ui/ArrayListModel.hpp"
#include "Spire/Ui/ListIndexValueModel.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("ListIndexValueModel") {
  TEST_CASE("get_set") {
    auto list = std::make_shared<ArrayListModel>();
    list->push(12);
    list->push(4);
    list->push(8);
    list->push(42);
    auto value = std::make_shared<LocalValueModel<int>>();
    value->set_current(8);
    auto index = std::make_shared<ListIndexValueModel<int>>(list, value);
    REQUIRE(index->get() == 2);
    REQUIRE(index->set_current(-1) == QValidator::Invalid);
    REQUIRE(index->set_current(4) == QValidator::Invalid);
    value->set_current(1);
    REQUIRE(index->get() == none);
    value->set_current(12);
    REQUIRE(index->get() == 0);
  }
}
