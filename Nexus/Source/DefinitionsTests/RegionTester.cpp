#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Region.hpp"

using namespace Nexus;

namespace {
  void TestProperSubset(const Region& subset, const Region& superset) {
    REQUIRE(subset < superset);
    REQUIRE(subset <= superset);
    REQUIRE(!(subset == superset));
    REQUIRE(subset != superset);
    REQUIRE(!(subset >= superset));
    REQUIRE(!(subset > superset));
    REQUIRE(!(superset < subset));
    REQUIRE(!(superset <= subset));
    REQUIRE(!(superset == subset));
    REQUIRE(superset != subset);
    REQUIRE(superset >= subset);
    REQUIRE(superset > subset);
  }

  void TestDistinctSets(const Region& a, const Region& b) {
    REQUIRE(!(a < b));
    REQUIRE(!(a <= b));
    REQUIRE(!(a == b));
    REQUIRE(a != b);
    REQUIRE(!(a >= b));
    REQUIRE(!(a > b));
    REQUIRE(!(b < a));
    REQUIRE(!(b <= a));
    REQUIRE(!(b == a));
    REQUIRE(b != a);
    REQUIRE(!(b >= a));
    REQUIRE(!(b > a));
  }
}

TEST_SUITE("Region") {
  TEST_CASE("market_region_subset_of_country_region") {
    auto country = DefaultCountries::US();
    auto market = GetDefaultMarketDatabase().FromCode(DefaultMarkets::NASDAQ());
    TestProperSubset(market, country);
  }

  TEST_CASE("security_region_subset_of_market_region") {
    auto market = GetDefaultMarketDatabase().FromCode(DefaultMarkets::NASDAQ());
    auto security =
      Security("TST", DefaultMarkets::NASDAQ(), DefaultCountries::US());
    TestProperSubset(security, market);
  }

  TEST_CASE("security_region_subset_of_country_region") {
    auto country = DefaultCountries::US();
    auto security =
      Security("TST", DefaultMarkets::NASDAQ(), DefaultCountries::US());
    TestProperSubset(security, country);
  }

  TEST_CASE("distinct_country_regions") {
    auto us = Region(DefaultCountries::US());
    auto ca = Region(DefaultCountries::CA());
    TestDistinctSets(us, ca);
    auto northAmerica = us + ca;
    auto br = DefaultCountries::BR();
    TestDistinctSets(northAmerica, br);
    TestProperSubset(us, northAmerica);
    TestProperSubset(ca, northAmerica);
  }
}
