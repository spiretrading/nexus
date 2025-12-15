#include <doctest/doctest.h>
#include "Nexus/Compliance/CachedComplianceRuleDataStore.hpp"
#include "Nexus/Compliance/LocalComplianceRuleDataStore.hpp"
#include "Nexus/ComplianceTests/ComplianceRuleDataStoreTestSuite.hpp"

using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return CachedComplianceRuleDataStore(
        std::make_unique<LocalComplianceRuleDataStore>());
    }
  };
}

TEST_SUITE("CachedComplianceRuleDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(ComplianceRuleDataStoreTestSuite, Builder);
}
