#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/Scope.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultVenues;

namespace {
  void require_proper_subset(const Scope& subset, const Scope& superset) {
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

  void require_distinct_scopes(const Scope& a, const Scope& b) {
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

TEST_SUITE("Scope") {
  TEST_CASE("venue_scope_subset_of_country_scope") {
    require_proper_subset(ASX, AU);
  }

  TEST_CASE("ticker_scope_subset_of_venue_scope") {
    auto ticker = Ticker("TST", ASX);
    require_proper_subset(ticker, ASX);
  }

  TEST_CASE("ticker_scope_subset_of_country_scope") {
    auto ticker = Ticker("TST", ASX);
    require_proper_subset(ticker, AU);
  }

  TEST_CASE("distinct_country_scopes") {
    auto au = Scope(AU);
    auto ca = Scope(CA);
    require_distinct_scopes(au, ca);
    auto merged = au + ca;
    require_distinct_scopes(merged, BR);
    require_proper_subset(au, merged);
    require_proper_subset(ca, merged);
  }

  TEST_CASE("empty_scopes_are_equal_and_subsets") {
    auto empty1 = Scope();
    auto empty2 = Scope("");
    REQUIRE(empty1 == empty2);
    REQUIRE(empty1 <= empty2);
    REQUIRE(empty1 >= empty2);
    REQUIRE(!(empty1 < empty2));
    REQUIRE(!(empty1 > empty2));
  }

  TEST_CASE("named_and_unnamed_scope_equality") {
    auto s1 = Scope(AU);
    auto s2 = Scope("AU Scope");
    s2 += AU;
    REQUIRE(s1 == s2);
    REQUIRE(s1 <= s2);
    REQUIRE(s1 >= s2);
    REQUIRE(!(s1 < s2));
    REQUIRE(!(s1 > s2));
  }

  TEST_CASE("global_scope_superset_of_all") {
    auto country = Scope(AU);
    auto global = Scope::GLOBAL;
    auto named_global = Scope::make_global("All Venues");
    require_proper_subset(country, global);
    require_proper_subset(country, named_global);
    REQUIRE(global == named_global);
    REQUIRE(global <= named_global);
    REQUIRE(global >= named_global);
    REQUIRE(!(global < named_global));
    REQUIRE(!(global > named_global));
  }

  TEST_CASE("distinct_venues") {
    auto asx = Scope(ASX);
    auto tsx = Scope(TSX);
    require_distinct_scopes(asx, tsx);
  }

  TEST_CASE("empty_subset_of_non_empty") {
    auto empty = Scope();
    auto country = Scope(AU);
    require_proper_subset(empty, country);
  }

  TEST_CASE("ticker_in_union_scope") {
    auto au = Scope(AU);
    auto ca = Scope(CA);
    auto union_scope = au + ca;
    auto ticker = Ticker("TST", ASX);
    REQUIRE(ticker <= union_scope);
    REQUIRE(ticker < union_scope);
    REQUIRE(union_scope >= ticker);
    REQUIRE(union_scope > ticker);
  }

  TEST_CASE("combine_scopes_operator_plus_and_plus_eq") {
    auto au = Scope(AU);
    auto ca = Scope(CA);
    auto combined = au;
    combined += ca;
    auto plus_combined = au + ca;
    REQUIRE(combined == plus_combined);
    require_proper_subset(au, combined);
    require_proper_subset(ca, combined);
  }

  TEST_CASE("shuttle") {
    auto scope = Scope(AU);
    scope += TSX;
    scope += Ticker("TST", ASX);
    test_round_trip_shuttle(scope);
  }
}
