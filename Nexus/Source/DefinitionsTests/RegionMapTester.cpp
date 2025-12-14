#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/RegionMap.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultVenues;

TEST_SUITE("RegionMap") {
  TEST_CASE("venue_region_subset_of_country_region") {
    auto map = RegionMap(-1);
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

  TEST_CASE("set_country_security_venue") {
    auto map = RegionMap(-1);
    auto country = CA;
    auto security = Security("TST", TSX);
    map.set(country, 1);
    map.set(security, 2);
    map.set(TSX, 3);
    REQUIRE(map.get(country) == 1);
    REQUIRE(map.get(security) == 2);
    REQUIRE(map.get(TSX) == 3);
  }

  TEST_CASE("region_map_iterator") {
    auto map = RegionMap(-1);
    map.set(AU, 1);
    map.set(CA, 2);
    auto i = map.begin();
    REQUIRE(std::get<0>(*i) == Region::GLOBAL);
    ++i;
    REQUIRE(std::get<0>(*i) == AU);
  }

  TEST_CASE("shuttle") {
    auto map = RegionMap(0);
    map.set(AU, 1);
    map.set(CA, 2);
    map.set(GB, 3);
    map.set(Region(std::string("CustomRegion")), 4);
    test_round_trip_shuttle(map, [&] (const auto& result) {
      for(auto& region : map) {
        REQUIRE(result.get(std::get<0>(region)) == std::get<1>(region));
      }
      for(auto& region : result) {
        REQUIRE(map.get(std::get<0>(region)) == std::get<1>(region));
      }
    });
  }
}
