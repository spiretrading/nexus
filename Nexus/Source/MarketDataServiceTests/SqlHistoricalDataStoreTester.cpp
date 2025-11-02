#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/MarketDataService/SqlHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/HistoricalDataStoreTestSuite.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace Viper;
using namespace Viper::Sqlite3;

namespace {
  using TestSqlHistoricalDataStore =
    SqlHistoricalDataStore<Viper::Sqlite3::Connection>;

  struct Builder {
    auto operator ()() const {
      return TestSqlHistoricalDataStore(DEFAULT_VENUES, [] {
        return Viper::Sqlite3::Connection("file::memory:?cache=shared");
      });
    }
  };
}

TEST_SUITE("SqlHistoricalDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(HistoricalDataStoreTestSuite, Builder);
}
