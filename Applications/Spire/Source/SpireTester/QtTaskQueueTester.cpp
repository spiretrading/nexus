#include <doctest/doctest.h>
#include "Spire/Spire/QtTaskQueue.hpp"
#include "Spire/SpireTester/SpireTester.hpp"

using namespace Spire;

TEST_SUITE("QtTaslQueue") {
  TEST_CASE("push") {
    run_test([] {
      auto queue = QtTaskQueue();
    });
  }
}
