#include <doctest/doctest.h>
#include "Spire/Spire/ArrayTableModel.hpp"
#include "Spire/Spire/TableRowIndexTracker.hpp"

using namespace boost;
using namespace Spire;

TEST_SUITE("TableRowIndexTracker") {
  TEST_CASE("add") {
    auto table = ArrayTableModel();
    for(auto i = 0; i < 10; ++i) {
      table.push({i});
    }
    auto tracker = TableRowIndexTracker(table, 5);
    SUBCASE("add_at") {
      table.insert({42}, 5);
      CHECK(tracker.get_index() == 6);
    }
    SUBCASE("add_before") {
      table.insert({42}, 3);
      CHECK(tracker.get_index() == 6);
    }
    SUBCASE("add_after") {
      table.insert({42}, 7);
      CHECK(tracker.get_index() == 5);
    }
  }

  TEST_CASE("remove") {
    auto table = ArrayTableModel();
    for(auto i = 0; i < 10; ++i) {
      table.push({i});
    }
    auto tracker = TableRowIndexTracker(table, 5);
    SUBCASE("remove_at") {
      table.remove(5);
      CHECK(tracker.get_index() == -1);
    }
    SUBCASE("remove_before") {
      table.remove(3);
      CHECK(tracker.get_index() == 4);
    }
    SUBCASE("remove_after") {
      table.remove(7);
      CHECK(tracker.get_index() == 5);
    }
  }

  TEST_CASE("move") {
    auto table = ArrayTableModel();
    for(auto i = 0; i < 10; ++i) {
      table.push({i});
    }
    auto tracker = TableRowIndexTracker(table, 5);
    SUBCASE("move_forward") {
      table.move(5, 8);
      CHECK(tracker.get_index() == 8);
    }
    SUBCASE("move_backward") {
      table.move(5, 2);
      CHECK(tracker.get_index() == 2);
    }
    SUBCASE("move_before_back") {
      table.move(3, 1);
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_before_forward") {
      table.move(1, 3);
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_after_back") {
      table.move(8, 6);
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_after_forward") {
      table.move(6, 8);
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_before_after") {
      table.move(3, 7);
      CHECK(tracker.get_index() == 4);
    }
    SUBCASE("move_after_before") {
      table.move(7, 3);
      CHECK(tracker.get_index() == 6);
    }
    SUBCASE("move_before_at") {
      table.move(3, 5);
      CHECK(tracker.get_index() == 4);
    }
    SUBCASE("move_after_at") {
      table.move(8, 5);
      CHECK(tracker.get_index() == 6);
    }
  }
}
