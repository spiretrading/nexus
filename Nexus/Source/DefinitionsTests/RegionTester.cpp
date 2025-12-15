#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Region.hpp"

using namespace Beam;
using namespace Beam::Tests;
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
    require_proper_subset(ASX, AU);
  }

  TEST_CASE("security_region_subset_of_venue_region") {
    auto security = Security("TST", ASX);
    require_proper_subset(security, ASX);
  }

  TEST_CASE("security_region_subset_of_country_region") {
    auto security = Security("TST", ASX);
    require_proper_subset(security, AU);
  }

  TEST_CASE("distinct_country_regions") {
    auto au = Region(AU);
    auto ca = Region(CA);
    require_distinct_regions(au, ca);
    auto merged = au + ca;
    require_distinct_regions(merged, BR);
    require_proper_subset(au, merged);
    require_proper_subset(ca, merged);
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
    auto r1 = Region(AU);
    auto r2 = Region("AU Region");
    r2 += AU;
    REQUIRE(r1 == r2);
    REQUIRE(r1 <= r2);
    REQUIRE(r1 >= r2);
    REQUIRE(!(r1 < r2));
    REQUIRE(!(r1 > r2));
  }

  TEST_CASE("global_region_superset_of_all") {
    auto country = Region(AU);
    auto global = Region::GLOBAL;
    auto named_global = Region::make_global("All Venues");
    require_proper_subset(country, global);
    require_proper_subset(country, named_global);
    REQUIRE(global == named_global);
    REQUIRE(global <= named_global);
    REQUIRE(global >= named_global);
    REQUIRE(!(global < named_global));
    REQUIRE(!(global > named_global));
  }

  TEST_CASE("distinct_venues") {
    auto asx = Region(ASX);
    auto tsx = Region(TSX);
    require_distinct_regions(asx, tsx);
  }

  TEST_CASE("empty_subset_of_non_empty") {
    auto empty = Region();
    auto country = Region(AU);
    require_proper_subset(empty, country);
  }

  TEST_CASE("security_in_union_region") {
    auto au = Region(AU);
    auto ca = Region(CA);
    auto union_region = au + ca;
    auto security = Security("TST", ASX);
    REQUIRE(security <= union_region);
    REQUIRE(security < union_region);
    REQUIRE(union_region >= security);
    REQUIRE(union_region > security);
  }

  TEST_CASE("combine_regions_operator_plus_and_plus_eq") {
    auto au = Region(AU);
    auto ca = Region(CA);
    auto combined = au;
    combined += ca;
    auto plus_combined = au + ca;
    REQUIRE(combined == plus_combined);
    require_proper_subset(au, combined);
    require_proper_subset(ca, combined);
  }

  TEST_CASE("shuttle") {
    auto region = Region(AU);
    region += TSX;
    region += Security("TST", ASX);
    test_round_trip_shuttle(region);
  }
}
