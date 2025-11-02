#include <doctest/doctest.h>
#include "Nexus/MarketDataService/CachedHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/HistoricalDataStoreTestSuite.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return CachedHistoricalDataStore<LocalHistoricalDataStore>(init(), 1000);
    }
  };
}

TEST_SUITE("CachedHistoricalDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(HistoricalDataStoreTestSuite, Builder);
}
