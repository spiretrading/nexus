#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/SecuritySet.hpp"

using namespace Nexus;

TEST_SUITE("SecuritySet") {
  TEST_CASE("parsing_concrete_security") {
    auto security = ParseWildCardSecurity("ABX.TSX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    REQUIRE(security.is_initialized());
    REQUIRE(security->GetSymbol() == "ABX");
    REQUIRE(security->GetMarket() == DefaultMarkets::TSX());
    REQUIRE(security->GetCountry() == DefaultCountries::CA());
  }

  TEST_CASE("parsing_wildcard_market") {
    auto security = ParseWildCardSecurity("CNQ.*",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    REQUIRE(security.is_initialized());
    REQUIRE(security->GetSymbol() == "CNQ");
    REQUIRE(security->GetMarket() == SecuritySet::GetMarketCodeWildCard());
    REQUIRE(security->GetCountry() == SecuritySet::GetCountryCodeWildCard());
  }

  TEST_CASE("parsing_wildcard_symbol") {
    auto security = ParseWildCardSecurity("*.TSX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    REQUIRE(security.is_initialized());
    REQUIRE(security->GetSymbol() == SecuritySet::GetSymbolWildCard());
    REQUIRE(security->GetMarket() == DefaultMarkets::TSX());
    REQUIRE(security->GetCountry() == DefaultCountries::CA());
  }

  TEST_CASE("parsing_wildcard_symbol_and_market") {
    auto security = ParseWildCardSecurity("*.*",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    REQUIRE(security.is_initialized());
    REQUIRE(security->GetSymbol() == SecuritySet::GetSymbolWildCard());
    REQUIRE(security->GetMarket() == SecuritySet::GetMarketCodeWildCard());
    REQUIRE(security->GetCountry() == SecuritySet::GetCountryCodeWildCard());
  }

  TEST_CASE("parsing_wildcard_symbol_and_market_concrete_country") {
    auto security = ParseWildCardSecurity("*.*.CA",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    REQUIRE(security.is_initialized());
    REQUIRE(security->GetSymbol() == SecuritySet::GetSymbolWildCard());
    REQUIRE(security->GetMarket() == SecuritySet::GetMarketCodeWildCard());
    REQUIRE(security->GetCountry() == DefaultCountries::CA());
  }

  TEST_CASE("parsing_wildcard_concrete_country") {
    auto security = ParseWildCardSecurity("*.AU",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    REQUIRE(security.is_initialized());
    REQUIRE(security->GetSymbol() == SecuritySet::GetSymbolWildCard());
    REQUIRE(security->GetMarket() == SecuritySet::GetMarketCodeWildCard());
    REQUIRE(security->GetCountry() == DefaultCountries::AU());
  }

  TEST_CASE("concrete_security") {
    auto set = SecuritySet();
    auto securityA = *ParseWildCardSecurity("TST.TSX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    set.Add(securityA);
    REQUIRE(set.Contains(securityA));
    auto securityB = *ParseWildCardSecurity("TST.ASX",
      GetDefaultMarketDatabase(), GetDefaultCountryDatabase());
    REQUIRE(!set.Contains(securityB));
  }
}
