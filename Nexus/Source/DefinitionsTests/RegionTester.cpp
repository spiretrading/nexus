#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/Definitions/Region.hpp"

using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultVenues;

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
    auto venue = DEFAULT_VENUES.from(NASDAQ);
    TestProperSubset(venue, US);
  }

  TEST_CASE("security_region_subset_of_venue_region") {
    auto venue = DEFAULT_VENUES.from(NASDAQ);
    auto security = Security("TST", NASDAQ);
    TestProperSubset(security, venue);
  }

  TEST_CASE("security_region_subset_of_country_region") {
    auto security = Security("TST", NASDAQ);
    TestProperSubset(security, US);
  }

  TEST_CASE("distinct_country_regions") {
    auto us = Region(US);
    auto ca = Region(CA);
    TestDistinctSets(us, ca);
    auto northAmerica = us + ca;
    TestDistinctSets(northAmerica, BR);
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
    auto r1 = Region(US);
    auto r2 = Region(US);
    r2.SetName("US Region");
    REQUIRE(r1 == r2);
    REQUIRE(r1 <= r2);
    REQUIRE(r1 >= r2);
    REQUIRE(!(r1 < r2));
    REQUIRE(!(r1 > r2));
  }

  TEST_CASE("global_region_superset_of_all") {
    auto country = Region(US);
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
    auto nasdaq = Region(DEFAULT_VENUES.from(NASDAQ));
    auto nyse = Region(DEFAULT_VENUES.from(NYSE));
    TestDistinctSets(nasdaq, nyse);
  }

  TEST_CASE("venue_entry_constructor_equivalence") {
    auto fromCodes = Region(NASDAQ, US);
    auto fromEntry = Region(DEFAULT_VENUES.from(NASDAQ));
    REQUIRE(fromCodes == fromEntry);
    REQUIRE(fromCodes <= fromEntry);
  }

  TEST_CASE("empty_subset_of_non_empty") {
    auto empty = Region();
    auto country = Region(US);
    TestProperSubset(empty, country);
  }

  TEST_CASE("security_in_union_region") {
    auto us = Region(US);
    auto ca = Region(CA);
    auto unionRegion = us + ca;
    auto security = Security("TST", NASDAQ);
    REQUIRE(security <= unionRegion);
    REQUIRE(security < unionRegion);
    REQUIRE(unionRegion >= security);
    REQUIRE(unionRegion > security);
  }

  TEST_CASE("combine_regions_operator_plus_and_plus_eq") {
    auto us = Region(US);
    auto ca = Region(CA);
    auto combined = us;
    combined += ca;
    auto plusCombined = us + ca;
    REQUIRE(combined == plusCombined);
    TestProperSubset(us, combined);
    TestProperSubset(ca, combined);
  }
}
