#include <doctest/doctest.h>
#include "Nexus/Backtester/CutoffHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/HistoricalDataStoreTestSuite.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::MarketDataService;
using namespace Nexus::MarketDataService::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return CutoffHistoricalDataStore<LocalHistoricalDataStore>(
        Initialize(), pos_infin);
    }
  };
}

TEST_SUITE("CutoffHistoricalDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(HistoricalDataStoreTestSuite, Builder);
}
