#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
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
  TEST_CASE("venue_region_subset_of_country_region") {
    auto country = DefaultCountries::US;
    auto venue = GetDefaultVenueDatabase().from(DefaultVenues::NASDAQ());
    TestProperSubset(venue, country);
  }

  TEST_CASE("security_region_subset_of_venue_region") {
    auto venue = GetDefaultVenueDatabase().from(DefaultVenues::NASDAQ());
    auto security =
      Security("TST", DefaultVenues::NASDAQ(), DefaultCountries::US);
    TestProperSubset(security, venue);
  }

  TEST_CASE("security_region_subset_of_country_region") {
    auto country = DefaultCountries::US;
    auto security =
      Security("TST", DefaultVenues::NASDAQ(), DefaultCountries::US);
    TestProperSubset(security, country);
  }

  TEST_CASE("distinct_country_regions") {
    auto us = Region(DefaultCountries::US);
    auto ca = Region(DefaultCountries::CA);
    TestDistinctSets(us, ca);
    auto northAmerica = us + ca;
    auto br = DefaultCountries::BR;
    TestDistinctSets(northAmerica, br);
    TestProperSubset(us, northAmerica);
    TestProperSubset(ca, northAmerica);
  }

  TEST_CASE("empty_regions_are_equal_and_subsets") {
    auto empty1 = Region();
    auto empty2 = Region("");
    REQUIRE(empty1 == empty2);
    REQUIRE(empty1 <= empty2);
    REQUIRE(empty1 >= empty2);
    REQUIRE(!(empty1 < empty2));
    REQUIRE(!(empty1 > empty2));
  }

  TEST_CASE("named_and_unnamed_region_equality") {
    auto r1 = Region(DefaultCountries::US);
    auto r2 = Region(DefaultCountries::US);
    r2.SetName("US Region");
    REQUIRE(r1 == r2);
    REQUIRE(r1 <= r2);
    REQUIRE(r1 >= r2);
    REQUIRE(!(r1 < r2));
    REQUIRE(!(r1 > r2));
  }

  TEST_CASE("global_region_superset_of_all") {
    auto country = Region(DefaultCountries::US);
    auto global = Region::Global();
    auto namedGlobal = Region::Global("All Venues");
    TestProperSubset(country, global);
    TestProperSubset(country, namedGlobal);
    REQUIRE(global == namedGlobal);
    REQUIRE(global <= namedGlobal);
    REQUIRE(global >= namedGlobal);
    REQUIRE(!(global < namedGlobal));
    REQUIRE(!(global > namedGlobal));
  }

  TEST_CASE("distinct_venues") {
    auto nasdaq =
      Region(GetDefaultVenueDatabase().from(DefaultVenues::NASDAQ()));
    auto nyse = Region(GetDefaultVenueDatabase().from(DefaultVenues::NYSE()));
    TestDistinctSets(nasdaq, nyse);
  }

  TEST_CASE("venue_entry_constructor_equivalence") {
    auto fromCodes = Region(DefaultVenues::NASDAQ(), DefaultCountries::US);
    auto fromEntry =
      Region(GetDefaultVenueDatabase().from(DefaultVenues::NASDAQ()));
    REQUIRE(fromCodes == fromEntry);
    REQUIRE(fromCodes <= fromEntry);
  }

  TEST_CASE("empty_subset_of_non_empty") {
    auto empty = Region();
    auto country = Region(DefaultCountries::US);
    TestProperSubset(empty, country);
  }

  TEST_CASE("security_in_union_region") {
    auto us = Region(DefaultCountries::US);
    auto ca = Region(DefaultCountries::CA);
    auto unionRegion = us + ca;
    auto security =
      Security("TST", DefaultVenues::NASDAQ(), DefaultCountries::US);
    REQUIRE(security <= unionRegion);
    REQUIRE(security < unionRegion);
    REQUIRE(unionRegion >= security);
    REQUIRE(unionRegion > security);
  }

  TEST_CASE("combine_regions_operator_plus_and_plus_eq") {
    auto us = Region(DefaultCountries::US);
    auto ca = Region(DefaultCountries::CA);
    auto combined = us;
    combined += ca;
    auto plusCombined = us + ca;
    REQUIRE(combined == plusCombined);
    TestProperSubset(us, combined);
    TestProperSubset(ca, combined);
  }
}
