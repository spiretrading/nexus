#include <doctest/doctest.h>
#include "Nexus/MarketDataService/AsyncHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/HistoricalDataStoreTestSuite.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return AsyncHistoricalDataStore<LocalHistoricalDataStore>(Initialize());
    }
  };
}

TEST_SUITE("AsyncHistoricalDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(HistoricalDataStoreTestSuite, Builder);
}
