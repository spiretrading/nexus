#include <doctest/doctest.h>
#include "Nexus/Definitions/Country.hpp"
#include "Nexus/Definitions/Currency.hpp"
#include "Nexus/Definitions/DatabaseFacet.hpp"
#include "Nexus/Definitions/Venue.hpp"

using namespace Nexus;

TEST_SUITE("DatabaseFacet") {
  TEST_CASE("make_locale") {
    auto currencies = CurrencyDatabase();
    auto entry = CurrencyDatabase::Entry();
    entry.m_id = Currencies::CAD;
    entry.m_code = "AAA";
    currencies.add(entry);
    SUBCASE("single_database") {
      auto locale = make_locale(currencies);
      currencies.remove(entry.m_id);
      REQUIRE(std::use_facet<DatabaseFacet<CurrencyDatabase>>(locale).
        get_database().from(entry.m_id).m_code == "AAA");
      REQUIRE(!std::has_facet<DatabaseFacet<CountryDatabase>>(locale));
      REQUIRE(!std::has_facet<DatabaseFacet<VenueDatabase>>(locale));
      REQUIRE(std::use_facet<std::numpunct<char>>(locale).decimal_point() == '.');
      REQUIRE(std::use_facet<std::numpunct<char>>(locale).grouping().empty());
    }
    SUBCASE("multiple_databases") {
      auto locale = make_locale(currencies, VENUES, COUNTRIES);
      REQUIRE(std::use_facet<DatabaseFacet<CurrencyDatabase>>(locale).
        get_database().from(entry.m_id).m_code == "AAA");
      REQUIRE(std::use_facet<DatabaseFacet<VenueDatabase>>(locale).
        get_database().from(Venues::TSX) == VENUES.from(Venues::TSX));
      REQUIRE(std::use_facet<DatabaseFacet<CountryDatabase>>(locale).
        get_database().from(CountryCode(124)) ==
          COUNTRIES.from(CountryCode(124)));
    }
    SUBCASE("repeated_database") {
      auto locale = make_locale(CURRENCIES, currencies);
      REQUIRE(std::use_facet<DatabaseFacet<CurrencyDatabase>>(locale).
        get_database().from(entry.m_id).m_code == "AAA");
    }
  }

  TEST_CASE("make_locale_with_base") {
    auto base = make_locale(CURRENCIES, COUNTRIES);
    auto currencies = CurrencyDatabase();
    auto entry = CurrencyDatabase::Entry();
    entry.m_id = Currencies::CAD;
    entry.m_code = "AAA";
    currencies.add(entry);
    SUBCASE("single_database") {
      auto locale = make_locale(base, currencies);
      REQUIRE(std::use_facet<DatabaseFacet<CurrencyDatabase>>(locale).
        get_database().from(entry.m_id).m_code == "AAA");
      REQUIRE(&std::use_facet<DatabaseFacet<CountryDatabase>>(locale) ==
        &std::use_facet<DatabaseFacet<CountryDatabase>>(base));
    }
    SUBCASE("multiple_databases") {
      auto locale = make_locale(base, currencies, VENUES);
      REQUIRE(std::use_facet<DatabaseFacet<CurrencyDatabase>>(locale).
        get_database().from(entry.m_id).m_code == "AAA");
      REQUIRE(std::use_facet<DatabaseFacet<CurrencyDatabase>>(base).
        get_database().from(entry.m_id).m_code == "CAD");
      REQUIRE(&std::use_facet<DatabaseFacet<CountryDatabase>>(locale) ==
        &std::use_facet<DatabaseFacet<CountryDatabase>>(base));
      REQUIRE(std::use_facet<DatabaseFacet<VenueDatabase>>(locale).
        get_database().from(Venues::TSX) == VENUES.from(Venues::TSX));
      REQUIRE(!std::has_facet<DatabaseFacet<VenueDatabase>>(base));
    }
  }
}
