#include "Nexus/DefinitionsTests/SecuritySetTester.hpp"
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"

using namespace Nexus;
using namespace Nexus::Tests;
using namespace std;

void SecuritySetTester::TestParsingConcreteSecurity() {
  auto security = ParseWildCardSecurity("ABX.TSX",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
  CPPUNIT_ASSERT(security.is_initialized());
  CPPUNIT_ASSERT(security->GetSymbol() == "ABX");
  CPPUNIT_ASSERT(security->GetMarket() == DefaultMarkets::TSX());
  CPPUNIT_ASSERT(security->GetCountry() == DefaultCountries::CA());
}

void SecuritySetTester::TestParsingWildcardMarket() {
  auto security = ParseWildCardSecurity("CNQ.*",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
  CPPUNIT_ASSERT(security.is_initialized());
  CPPUNIT_ASSERT(security->GetSymbol() == "CNQ");
  CPPUNIT_ASSERT(security->GetMarket() == SecuritySet::GetMarketCodeWildCard());
  CPPUNIT_ASSERT(security->GetCountry() ==
    SecuritySet::GetCountryCodeWildCard());
}

void SecuritySetTester::TestParsingWildcardSymbol() {
  auto security = ParseWildCardSecurity("*.TSX",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
  CPPUNIT_ASSERT(security.is_initialized());
  CPPUNIT_ASSERT(security->GetSymbol() == SecuritySet::GetSymbolWildCard());
  CPPUNIT_ASSERT(security->GetMarket() == DefaultMarkets::TSX());
  CPPUNIT_ASSERT(security->GetCountry() == DefaultCountries::CA());
}

void SecuritySetTester::TestParsingWildcardSymbolAndMarket() {
  auto security = ParseWildCardSecurity("*.*",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
  CPPUNIT_ASSERT(security.is_initialized());
  CPPUNIT_ASSERT(security->GetSymbol() == SecuritySet::GetSymbolWildCard());
  CPPUNIT_ASSERT(security->GetMarket() == SecuritySet::GetMarketCodeWildCard());
  CPPUNIT_ASSERT(security->GetCountry() ==
    SecuritySet::GetCountryCodeWildCard());
}

void SecuritySetTester::TestParsingWildcardSymbolAndMarketConcreteCountry() {
  auto security = ParseWildCardSecurity("*.*.CA",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
  CPPUNIT_ASSERT(security.is_initialized());
  CPPUNIT_ASSERT(security->GetSymbol() == SecuritySet::GetSymbolWildCard());
  CPPUNIT_ASSERT(security->GetMarket() == SecuritySet::GetMarketCodeWildCard());
  CPPUNIT_ASSERT(security->GetCountry() == DefaultCountries::CA());
}

void SecuritySetTester::TestParsingWildcardConcreteCountry() {
  auto security = ParseWildCardSecurity("*.AU",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
  CPPUNIT_ASSERT(security.is_initialized());
  CPPUNIT_ASSERT(security->GetSymbol() == SecuritySet::GetSymbolWildCard());
  CPPUNIT_ASSERT(security->GetMarket() == SecuritySet::GetMarketCodeWildCard());
  CPPUNIT_ASSERT(security->GetCountry() == DefaultCountries::AU());
}

void SecuritySetTester::TestConcreteSecurity() {
  SecuritySet set;
  auto securityA = *ParseWildCardSecurity("TST.TSX", GetDefaultMarketDatabase(),
    GetDefaultCountryDatabase());
  set.Add(securityA);
  CPPUNIT_ASSERT(set.Contains(securityA));
  auto securityB = *ParseWildCardSecurity("TST.ASX",
    GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
  CPPUNIT_ASSERT(!set.Contains(securityB));
}
