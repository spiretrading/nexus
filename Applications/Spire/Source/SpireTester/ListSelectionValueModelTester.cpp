#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Ui/ListSelectionValueModel.hpp"

using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("ListSelectionValueModel") {
  TEST_CASE("construct") {
    auto values = std::make_shared<ArrayListModel<std::string>>();
    values->push("hello");
    values->push("world");
    values->push("goodbye");
    values->push("sky");
    auto selection = std::make_shared<ArrayListModel<int>>();
    selection->push(2);
    auto selected_values =
      ListSelectionValueModel<std::string>(values, selection);
    REQUIRE(selected_values.get_size() == 1);
    REQUIRE(selected_values.get(0) == "goodbye");
    selection->push(0);
    REQUIRE(selected_values.get_size() == 2);
    REQUIRE(selected_values.get(0) == "goodbye");
    REQUIRE(selected_values.get(1) == "hello");
  }
}
