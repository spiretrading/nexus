#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/ScopeMap.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultVenues;

TEST_SUITE("ScopeMap") {
  TEST_CASE("venue_scope_subset_of_country_scope") {
    auto map = ScopeMap(-1);
    map.set(AU, 1);
    map.set(CA, 2);
    auto au = map.get(AU);
    REQUIRE(au == 1);
    auto ca = map.get(CA);
    REQUIRE(ca == 2);
    auto br = map.get(BR);
    REQUIRE(br == -1);
    REQUIRE(map.get(ASX) == 1);
    REQUIRE(map.get(TSX) == 2);
  }

  TEST_CASE("set_country_ticker_venue") {
    auto map = ScopeMap(-1);
    auto country = CA;
    auto ticker = Ticker("TST", TSX);
    map.set(country, 1);
    map.set(ticker, 2);
    map.set(TSX, 3);
    REQUIRE(map.get(country) == 1);
    REQUIRE(map.get(ticker) == 2);
    REQUIRE(map.get(TSX) == 3);
  }

  TEST_CASE("scope_map_iterator") {
    auto map = ScopeMap(-1);
    map.set(AU, 1);
    map.set(CA, 2);
    auto i = map.begin();
    REQUIRE(std::get<0>(*i) == Scope::GLOBAL);
    ++i;
    REQUIRE(std::get<0>(*i) == AU);
  }

  TEST_CASE("shuttle") {
    auto map = ScopeMap(0);
    map.set(AU, 1);
    map.set(CA, 2);
    map.set(GB, 3);
    map.set(Scope(std::string("CustomScope")), 4);
    test_round_trip_shuttle(map, [&] (const auto& result) {
      for(auto& scope : map) {
        REQUIRE(result.get(std::get<0>(scope)) == std::get<1>(scope));
      }
      for(auto& scope : result) {
        REQUIRE(map.get(std::get<0>(scope)) == std::get<1>(scope));
      }
    });
  }
}
