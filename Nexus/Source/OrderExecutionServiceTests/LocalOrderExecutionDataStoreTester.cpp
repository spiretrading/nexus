#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionDataStoreTestSuite.hpp"

using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return LocalOrderExecutionDataStore();
    }
  };
}

TEST_SUITE("LocalOrderExecutionDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(OrderExecutionDataStoreTestSuite, Builder);
}
