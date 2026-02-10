#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/ScopeTable.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultCountries;
using namespace Nexus::DefaultVenues;

TEST_SUITE("ScopeTable") {
  TEST_CASE("venue_scope_subset_of_country_scope") {
    auto table = ScopeTable(-1);
    table.set(AU, 1);
    table.set(CA, 2);
    auto au = table.get(AU);
    REQUIRE(au == 1);
    auto ca = table.get(CA);
    REQUIRE(ca == 2);
    auto br = table.get(BR);
    REQUIRE(br == -1);
    REQUIRE(table.get(ASX) == 1);
    REQUIRE(table.get(TSX) == 2);
  }

  TEST_CASE("set_country_ticker_venue") {
    auto table = ScopeTable(-1);
    auto country = CA;
    auto ticker = Ticker("TST", TSX);
    table.set(country, 1);
    table.set(ticker, 2);
    table.set(TSX, 3);
    REQUIRE(table.get(country) == 1);
    REQUIRE(table.get(ticker) == 2);
    REQUIRE(table.get(TSX) == 3);
  }

  TEST_CASE("scope_table_iterator") {
    auto table = ScopeTable(-1);
    table.set(AU, 1);
    table.set(CA, 2);
    auto i = table.begin();
    REQUIRE(std::get<0>(*i) == Scope::GLOBAL);
    ++i;
    REQUIRE(std::get<0>(*i) == AU);
  }

  TEST_CASE("shuttle") {
    auto table = ScopeTable(0);
    table.set(AU, 1);
    table.set(CA, 2);
    table.set(GB, 3);
    table.set(Scope(std::string("CustomScope")), 4);
    test_round_trip_shuttle(table, [&] (const auto& result) {
      for(auto& scope : table) {
        REQUIRE(result.get(std::get<0>(scope)) == std::get<1>(scope));
      }
      for(auto& scope : result) {
        REQUIRE(table.get(std::get<0>(scope)) == std::get<1>(scope));
      }
    });
  }
}
