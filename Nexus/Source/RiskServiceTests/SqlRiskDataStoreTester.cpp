#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/RiskService/SqlRiskDataStore.hpp"
#include "Nexus/RiskServiceTests/RiskDataStoreTestSuite.hpp"

using namespace Nexus;
using namespace Nexus::Tests;
using namespace Viper;
using namespace Viper::Sqlite3;

namespace {
  struct Builder {
    auto operator ()() const {
      return SqlRiskDataStore(
        std::make_unique<Viper::Sqlite3::Connection>(":memory:"));
    }
  };
}

TEST_SUITE("SqlRiskDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(RiskDataStoreTestSuite, Builder);
}
