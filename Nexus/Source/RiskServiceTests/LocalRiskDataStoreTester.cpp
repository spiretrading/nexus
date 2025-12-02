#include <doctest/doctest.h>
#include "Nexus/RiskService/LocalRiskDataStore.hpp"
#include "Nexus/RiskServiceTests/RiskDataStoreTestSuite.hpp"

using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return LocalRiskDataStore();
    }
  };
}

TEST_SUITE("LocalRiskDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(RiskDataStoreTestSuite, Builder);
}
