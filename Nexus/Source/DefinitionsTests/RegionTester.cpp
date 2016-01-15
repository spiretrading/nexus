#include "Nexus/DefinitionsTests/RegionTester.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Region.hpp"

using namespace Nexus;
using namespace Nexus::Tests;
using namespace std;

namespace {
  void TestProperSubset(const Region& subset, const Region& superset) {
    CPPUNIT_ASSERT(subset < superset);
    CPPUNIT_ASSERT(subset <= superset);
    CPPUNIT_ASSERT(!(subset == superset));
    CPPUNIT_ASSERT(subset != superset);
    CPPUNIT_ASSERT(!(subset >= superset));
    CPPUNIT_ASSERT(!(subset > superset));
    CPPUNIT_ASSERT(!(superset < subset));
    CPPUNIT_ASSERT(!(superset <= subset));
    CPPUNIT_ASSERT(!(superset == subset));
    CPPUNIT_ASSERT(superset != subset);
    CPPUNIT_ASSERT(superset >= subset);
    CPPUNIT_ASSERT(superset > subset);
  }

  void TestDistinctSets(const Region& a, const Region& b) {
    CPPUNIT_ASSERT(!(a < b));
    CPPUNIT_ASSERT(!(a <= b));
    CPPUNIT_ASSERT(!(a == b));
    CPPUNIT_ASSERT(a != b);
    CPPUNIT_ASSERT(!(a >= b));
    CPPUNIT_ASSERT(!(a > b));
    CPPUNIT_ASSERT(!(b < a));
    CPPUNIT_ASSERT(!(b <= a));
    CPPUNIT_ASSERT(!(b == a));
    CPPUNIT_ASSERT(b != a);
    CPPUNIT_ASSERT(!(b >= a));
    CPPUNIT_ASSERT(!(b > a));
  }
}

void RegionTester::TestMarketRegionSubsetOfCountryRegion() {
  Region country = DefaultCountries::US();
  Region market = GetDefaultMarketDatabase().FromCode(DefaultMarkets::NASDAQ());
  TestProperSubset(market, country);
}

void RegionTester::TestSecurityRegionSubsetOfMarketRegion() {
  Region market = GetDefaultMarketDatabase().FromCode(DefaultMarkets::NASDAQ());
  Region security = Security("TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  TestProperSubset(security, market);
}

void RegionTester::TestSecurityRegionSubsetOfCountryRegion() {
  Region country = DefaultCountries::US();
  Region security = Security("TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());
  TestProperSubset(security, country);
}

void RegionTester::TestDistinctCountryRegions() {
  Region us = DefaultCountries::US();
  Region ca = DefaultCountries::CA();
  TestDistinctSets(us, ca);
  Region northAmerica = us + ca;
  Region br = DefaultCountries::BR();
  TestDistinctSets(northAmerica, br);
  TestProperSubset(us, northAmerica);
  TestProperSubset(ca, northAmerica);
}
