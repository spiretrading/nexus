#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/Compliance/SqlComplianceRuleDataStore.hpp"
#include "Nexus/ComplianceTests/ComplianceRuleDataStoreTestSuite.hpp"

using namespace Nexus;
using namespace Nexus::Compliance;
using namespace Nexus::Compliance::Tests;
using namespace Viper;
using namespace Viper::Sqlite3;

namespace {
  struct Builder {
    auto operator ()() const {
      return SqlComplianceRuleDataStore(
        std::make_unique<Connection>(":memory:"));
    }
  };
}

TEST_SUITE("LocalComplianceRuleDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(ComplianceRuleDataStoreTestSuite, Builder);
}
