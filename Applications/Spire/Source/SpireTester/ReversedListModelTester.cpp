#include <doctest/doctest.h>
#include "Spire/Spire/ArrayListModel.hpp"
#include "Spire/Spire/ReversedListModel.hpp"
#include "Spire/SpireTester/ListModelTester.hpp"

using namespace boost;
using namespace boost::signals2;
using namespace Spire;

TEST_SUITE("ReversedListModel") {
  TEST_CASE("constructor") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(1);
    source->push(2);
    source->push(3);
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
  TEST_CASE("remove_to_empty") {
    auto source = std::make_shared<ArrayListModel<int>>();
    source->push(123);
    auto reverse = ReversedListModel(source);
    auto operations = std::deque<ReversedListModel<int>::Operation>();
    auto connection = scoped_connection(reverse.connect_operation_signal(
      [&] (const auto& operation) {
        operations.push_back(operation);
      }));
    reverse.remove(reverse.begin());
    require_list_transaction<int>(operations,
      {
        ListModel<int>::PreRemoveOperation(0),
        ListModel<int>::RemoveOperation(0)
      });
  }
}
