#include <doctest/doctest.h>
#include "Spire/Spire/ListIndexTracker.hpp"

using namespace Spire;

TEST_SUITE("ListIndexTracker") {
  TEST_CASE("add") {
    auto tracker = ListIndexTracker(5);
    SUBCASE("add_at") {
      tracker.update(AnyListModel::AddOperation(5, {}));
      CHECK(tracker.get_index() == 6);
    }
    SUBCASE("add_before") {
      tracker.update(AnyListModel::AddOperation(3, {}));
      CHECK(tracker.get_index() == 6);
    }
    SUBCASE("add_after") {
      tracker.update(AnyListModel::AddOperation(7, {}));
      CHECK(tracker.get_index() == 5);
    }
  }

  TEST_CASE("remove") {
    auto tracker = ListIndexTracker(5);
    SUBCASE("remove_at") {
      tracker.update(AnyListModel::RemoveOperation(5, {}));
      CHECK(tracker.get_index() == -1);
    }
    SUBCASE("remove_before") {
      tracker.update(AnyListModel::RemoveOperation(3, {}));
      CHECK(tracker.get_index() == 4);
    }
    SUBCASE("remove_after") {
      tracker.update(AnyListModel::RemoveOperation(7, {}));
      CHECK(tracker.get_index() == 5);
    }
  }

  TEST_CASE("move") {
    auto tracker = ListIndexTracker(5);
    SUBCASE("move_forward") {
      tracker.update(AnyListModel::MoveOperation(5, 8));
      CHECK(tracker.get_index() == 8);
    }
    SUBCASE("move_backward") {
      tracker.update(AnyListModel::MoveOperation(5, 2));
      CHECK(tracker.get_index() == 2);
    }
    SUBCASE("move_before_back") {
      tracker.update(AnyListModel::MoveOperation(3, 1));
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_before_forward") {
      tracker.update(AnyListModel::MoveOperation(1, 3));
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_after_back") {
      tracker.update(AnyListModel::MoveOperation(8, 6));
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_after_forward") {
      tracker.update(AnyListModel::MoveOperation(6, 8));
      CHECK(tracker.get_index() == 5);
    }
    SUBCASE("move_before_after") {
      tracker.update(AnyListModel::MoveOperation(3, 7));
      CHECK(tracker.get_index() == 4);
    }
    SUBCASE("move_after_before") {
      tracker.update(AnyListModel::MoveOperation(7, 3));
      CHECK(tracker.get_index() == 6);
    }
    SUBCASE("move_before_at") {
      tracker.update(AnyListModel::MoveOperation(3, 5));
      CHECK(tracker.get_index() == 4);
    }
    SUBCASE("move_after_at") {
      tracker.update(AnyListModel::MoveOperation(8, 5));
      CHECK(tracker.get_index() == 6);
    }
  }
}
