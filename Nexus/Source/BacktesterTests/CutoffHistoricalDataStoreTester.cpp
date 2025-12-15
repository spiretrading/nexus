#include <doctest/doctest.h>
#include "Nexus/Backtester/CutoffHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/HistoricalDataStoreTestSuite.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return CutoffHistoricalDataStore<LocalHistoricalDataStore>(
        init(), pos_infin);
    }
  };
}

TEST_SUITE("CutoffHistoricalDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(HistoricalDataStoreTestSuite, Builder);
}
