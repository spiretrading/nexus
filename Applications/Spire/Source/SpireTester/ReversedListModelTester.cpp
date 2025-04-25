#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ReversedListModel.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("ReversedListModel") {
  TEST_CASE("constructor") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->insert(1, 0);
    source->insert(2, 1);
    source->insert(3, 2);
    auto reverse = ReversedListModel(source);
    REQUIRE(reverse.get_size() == 3);
    REQUIRE(reverse.get(0) == 3);
    REQUIRE(reverse.get(1) == 2);
    REQUIRE(reverse.get(2) == 1);
    SUBCASE("insert_end") {
      reverse.insert(4, 0);
      REQUIRE(source->get(3) == 4);
      REQUIRE(reverse.get(0) == 4);
    }
    SUBCASE("insert_front") {
      reverse.insert(6, reverse.get_size());
      REQUIRE(source->get(0) == 6);
      REQUIRE(reverse.get(reverse.get_size() - 1) == 6);
    }
    SUBCASE("remove") {
      reverse.remove(0);
      REQUIRE(source->get_size() == 2);
      REQUIRE(reverse.get(0) == 2);
    }
    SUBCASE("set") {
      reverse.set(1, 9);
      REQUIRE(source->get(1) == 9);
      REQUIRE(reverse.get(1) == 9);
    }
  }
}
