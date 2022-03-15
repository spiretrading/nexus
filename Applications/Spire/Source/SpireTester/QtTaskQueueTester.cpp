#include <doctest/doctest.h>
#include <deque>
#include "Spire/Spire/QtTaskQueue.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Beam;
using namespace Spire;

TEST_SUITE("QtTaskQueue") {
  TEST_CASE("push") {
    run_test([] {
      auto tasks = QtTaskQueue();
      auto values = std::deque<int>();
      auto slot = tasks.get_slot<int>([&] (auto value) {
        values.push_back(value);
      });
      slot.Push(123);
      QApplication::processEvents();
      REQUIRE(values.size() == 1);
      REQUIRE(values.front() == 123);
      values.pop_front();
      tasks.close();
      REQUIRE_THROWS(slot.Push(11));
    });
  }
}
