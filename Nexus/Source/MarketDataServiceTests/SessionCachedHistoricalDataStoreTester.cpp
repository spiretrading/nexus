#include <doctest/doctest.h>
#include "Nexus/MarketDataService/LocalHistoricalDataStore.hpp"
#include "Nexus/MarketDataService/SessionCachedHistoricalDataStore.hpp"
#include "Nexus/MarketDataServiceTests/HistoricalDataStoreTestSuite.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return SessionCachedHistoricalDataStore<LocalHistoricalDataStore>(
        init(), 1000);
    }
  };

  TickerInfo make_ticker_info(
      const std::string& ticker, const std::string& name) {
    auto info = TickerInfo();
    info.m_ticker = parse_ticker(ticker);
    info.m_name = name;
    info.m_sector = "Technology";
    return info;
  }

  TickerInfoQuery make_global_ticker_info_query() {
    auto query = TickerInfoQuery();
    query.set_index(Scope::GLOBAL);
    query.set_snapshot_limit(SnapshotLimit::UNLIMITED);
    return query;
  }
}

TEST_SUITE("SessionCachedHistoricalDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(HistoricalDataStoreTestSuite, Builder);

  TEST_CASE("load_warmed_ticker_info") {
    auto source = LocalHistoricalDataStore();
    auto info = make_ticker_info("AAA.TSX", "Company A");
    source.store(info);
    auto cache = SessionCachedHistoricalDataStore<LocalHistoricalDataStore*>(
      &source, 1000);
    auto result = cache.load_ticker_info(make_global_ticker_info_query());
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == info);
  }

  TEST_CASE("load_ticker_info_added_to_source") {
    auto source = LocalHistoricalDataStore();
    auto info = make_ticker_info("AAA.TSX", "Company A");
    source.store(info);
    auto cache = SessionCachedHistoricalDataStore<LocalHistoricalDataStore*>(
      &source, 1000);
    source.store(make_ticker_info("BBB.TSX", "Company B"));
    auto result = cache.load_ticker_info(make_global_ticker_info_query());
    REQUIRE(result.size() == 1);
    REQUIRE(result.front() == info);
  }

  TEST_CASE("write_through_ticker_info") {
    auto source = LocalHistoricalDataStore();
    auto info_a = make_ticker_info("AAA.TSX", "Company A");
    source.store(info_a);
    auto cache = SessionCachedHistoricalDataStore<LocalHistoricalDataStore*>(
      &source, 1000);
    auto info_b = make_ticker_info("BBB.TSX", "Company B");
    cache.store(info_b);
    auto cached = cache.load_ticker_info(make_global_ticker_info_query());
    REQUIRE(cached.size() == 2);
    REQUIRE(cached.front() == info_a);
    REQUIRE(cached.back() == info_b);
    auto persisted = source.load_ticker_info(make_global_ticker_info_query());
    REQUIRE(persisted.size() == 2);
  }
}
