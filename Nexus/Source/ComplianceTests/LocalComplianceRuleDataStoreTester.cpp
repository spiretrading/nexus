#include <doctest/doctest.h>
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"
#include "Nexus/ComplianceTests/ComplianceRuleDataStoreTestSuite.hpp"

using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return LocalComplianceRuleDataStore();
    }
  };
}

TEST_SUITE("LocalComplianceRuleDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(ComplianceRuleDataStoreTestSuite, Builder);
}
