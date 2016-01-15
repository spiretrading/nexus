#include "Nexus/DefinitionsTests/RegionMapTester.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/RegionMap.hpp"

using namespace Nexus;
using namespace Nexus::Tests;
using namespace std;

void RegionMapTester::TestMarketRegionSubsetOfCountryRegion() {
  RegionMap<int> map(-1);
  map.Set(DefaultCountries::US(), 1);
  map.Set(DefaultCountries::CA(), 2);
  int usCode = map.Get(DefaultCountries::US());
  CPPUNIT_ASSERT(usCode == 1);
  int caCode = map.Get(DefaultCountries::CA());
  CPPUNIT_ASSERT(caCode == 2);
  int brCode = map.Get(DefaultCountries::BR());
  CPPUNIT_ASSERT(brCode == -1);
  CPPUNIT_ASSERT(map.Get(GetDefaultMarketDatabase().FromCode(
    DefaultMarkets::NASDAQ())) == 1);
  CPPUNIT_ASSERT(map.Get(GetDefaultMarketDatabase().FromCode(
    DefaultMarkets::TSX())) == 2);
}

void RegionMapTester::TestSetCountrySecurityMarket() {
  RegionMap<int> map(-1);
  Region country = DefaultCountries::CA();
  Region market = GetDefaultMarketDatabase().FromCode(DefaultMarkets::TSX());
  Region security = Security("TST", DefaultMarkets::TSX(),
    DefaultCountries::CA());
  map.Set(country, 1);
  map.Set(security, 2);
  map.Set(market, 3);
  CPPUNIT_ASSERT(map.Get(country) == 1);
  CPPUNIT_ASSERT(map.Get(security) == 2);
  CPPUNIT_ASSERT(map.Get(market) == 3);
}

void RegionMapTester::TestRegionMapIterator() {
  RegionMap<int> map(-1);
  map.Set(DefaultCountries::US(), 1);
  map.Set(DefaultCountries::CA(), 2);
  auto mapIterator = map.Begin();
  CPPUNIT_ASSERT(get<0>(*mapIterator) == Region(Region::GlobalTag()));
  ++mapIterator;
  CPPUNIT_ASSERT(get<0>(*mapIterator) == DefaultCountries::US());
}
