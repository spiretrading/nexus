#include <doctest/doctest.h>
#include "Spire/Spire/TableRowIndexTracker.hpp"

using namespace Spire;

TEST_SUITE("TableRowIndexTracker") {
  TEST_CASE("add") {
    auto tracker = TableRowIndexTracker(5);
    SUBCASE("add_at") {
      tracker.update(TableModel::AddOperation(5, {}));
      CHECK(tracker.get_index() == 6);
    }
    SUBCASE("add_before") {
      tracker.update(TableModel::AddOperation(3, {}));
      CHECK(tracker.get_index() == 6);
    }
    SUBCASE("add_after") {
      tracker.update(TableModel::AddOperation(7, {}));
      CHECK(tracker.get_index() == 5);
    }
  }

  TEST_CASE("remove") {
    auto tracker = TableRowIndexTracker(5);
    SUBCASE("remove_at") {
      tracker.update(TableModel::RemoveOperation(5, {}));
      CHECK(tracker.get_index() == -1);
    }
    SUBCASE("remove_before") {
      tracker.update(TableModel::RemoveOperation(3, {}));
      CHECK(tracker.get_index() == 4);
    }
    SUBCASE("remove_after") {
      tracker.update(TableModel::RemoveOperation(7, {}));
      CHECK(tracker.get_index() == 5);
    }
  }

  TEST_CASE("move") {
    auto tracker = TableRowIndexTracker(5);
    SUBCASE("move_forward") {
      tracker.update(TableModel::MoveOperation(5, 8));
      CHECK(tracker.get_index() == 8);
    }
    SUBCASE("move_backward") {
      tracker.update(TableModel::MoveOperation(5, 2));
      CHECK(tracker.get_index() == 2);
    }
    SUBCASE("move_before_back") {
      tracker.update(TableModel::MoveOperation(3, 1));
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_before_forward") {
      tracker.update(TableModel::MoveOperation(1, 3));
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_after_back") {
      tracker.update(TableModel::MoveOperation(8, 6));
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_after_forward") {
      tracker.update(TableModel::MoveOperation(6, 8));
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_before_after") {
      tracker.update(TableModel::MoveOperation(3, 7));
      CHECK(tracker.get_index() == 4);
    }
    SUBCASE("move_after_before") {
      tracker.update(TableModel::MoveOperation(7, 3));
      CHECK(tracker.get_index() == 6);
    }
    SUBCASE("move_before_at") {
      tracker.update(TableModel::MoveOperation(3, 5));
      CHECK(tracker.get_index() == 4);
    }
    SUBCASE("move_after_at") {
      tracker.update(TableModel::MoveOperation(8, 5));
      CHECK(tracker.get_index() == 6);
    }
  }
}
