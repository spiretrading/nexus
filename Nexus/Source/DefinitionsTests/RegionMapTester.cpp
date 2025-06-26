#include <doctest/doctest.h>
#include "Nexus/Definitions/DefaultCountryDatabase.hpp"
#include "Nexus/Definitions/DefaultVenueDatabase.hpp"
#include "Nexus/Definitions/RegionMap.hpp"

using namespace Nexus;

TEST_SUITE("RegionMap") {
  TEST_CASE("venue_region_subset_of_country_region") {
    auto map = RegionMap<int>(-1);
    map.Set(DefaultCountries::US(), 1);
    map.Set(DefaultCountries::CA(), 2);
    auto usCode = map.Get(DefaultCountries::US());
    REQUIRE(usCode == 1);
    auto caCode = map.Get(DefaultCountries::CA());
    REQUIRE(caCode == 2);
    auto brCode = map.Get(DefaultCountries::BR());
    REQUIRE(brCode == -1);
    REQUIRE(
      map.Get(GetDefaultVenueDatabase().from(DefaultVenues::NASDAQ())) == 1);
    REQUIRE(
      map.Get(GetDefaultVenueDatabase().from(DefaultVenues::TSX())) == 2);
  }

  TEST_CASE("set_country_security_venue") {
    auto map = RegionMap<int>(-1);
    auto country = DefaultCountries::CA();
    auto venue = GetDefaultVenueDatabase().from(DefaultVenues::TSX());
    auto security =
      Security("TST", DefaultVenues::TSX(), DefaultCountries::CA());
    map.Set(country, 1);
    map.Set(security, 2);
    map.Set(venue, 3);
    REQUIRE(map.Get(country) == 1);
    REQUIRE(map.Get(security) == 2);
    REQUIRE(map.Get(venue) == 3);
  }

  TEST_CASE("region_map_iterator") {
    auto map = RegionMap<int>(-1);
    map.Set(DefaultCountries::US(), 1);
    map.Set(DefaultCountries::CA(), 2);
    auto mapIterator = map.Begin();
    REQUIRE(std::get<0>(*mapIterator) == Region::Global());
    ++mapIterator;
    REQUIRE(std::get<0>(*mapIterator) == DefaultCountries::US());
  }
}
