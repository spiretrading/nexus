#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/Definitions/Region.hpp"

using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultVenues;

namespace {
  void require_proper_subset(const Region& subset, const Region& superset) {
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

  void require_distinct_regions(const Region& a, const Region& b) {
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
    require_proper_subset(NASDAQ, US);
  }

  TEST_CASE("security_region_subset_of_venue_region") {
    auto security = Security("TST", NASDAQ);
    require_proper_subset(security, NASDAQ);
  }

  TEST_CASE("security_region_subset_of_country_region") {
    auto security = Security("TST", NASDAQ);
    require_proper_subset(security, US);
  }

  TEST_CASE("distinct_country_regions") {
    auto us = Region(US);
    auto ca = Region(CA);
    require_distinct_regions(us, ca);
    auto northAmerica = us + ca;
    require_distinct_regions(northAmerica, BR);
    require_proper_subset(us, northAmerica);
    require_proper_subset(ca, northAmerica);
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
    auto r2 = Region("US Region");
    r2 += US;
    REQUIRE(r1 == r2);
    REQUIRE(r1 <= r2);
    REQUIRE(r1 >= r2);
    REQUIRE(!(r1 < r2));
    REQUIRE(!(r1 > r2));
  }

  TEST_CASE("global_region_superset_of_all") {
    auto country = Region(US);
    auto global = Region::Global();
    auto named_global = Region::Global("All Venues");
    require_proper_subset(country, global);
    require_proper_subset(country, named_global);
    REQUIRE(global == named_global);
    REQUIRE(global <= named_global);
    REQUIRE(global >= named_global);
    REQUIRE(!(global < named_global));
    REQUIRE(!(global > named_global));
  }

  TEST_CASE("distinct_venues") {
    auto nasdaq = Region(NASDAQ);
    auto nyse = Region(NYSE);
    require_distinct_regions(nasdaq, nyse);
  }

  TEST_CASE("empty_subset_of_non_empty") {
    auto empty = Region();
    auto country = Region(US);
    require_proper_subset(empty, country);
  }

  TEST_CASE("security_in_union_region") {
    auto us = Region(US);
    auto ca = Region(CA);
    auto union_region = us + ca;
    auto security = Security("TST", NASDAQ);
    REQUIRE(security <= union_region);
    REQUIRE(security < union_region);
    REQUIRE(union_region >= security);
    REQUIRE(union_region > security);
  }

  TEST_CASE("combine_regions_operator_plus_and_plus_eq") {
    auto us = Region(US);
    auto ca = Region(CA);
    auto combined = us;
    combined += ca;
    auto plus_combined = us + ca;
    REQUIRE(combined == plus_combined);
    require_proper_subset(us, combined);
    require_proper_subset(ca, combined);
  }
}
