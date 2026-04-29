#include <doctest/doctest.h>
#include "Nexus/Queries/TickerAccessor.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("TickerAccessor") {
  TEST_CASE("from_parameter") {
    auto accessor = TickerAccessor::from_parameter(0);
    auto symbol_accessor = accessor.get_symbol();
    REQUIRE(symbol_accessor.get_name() == "symbol");
    REQUIRE(symbol_accessor.get_type() == typeid(std::string));
  }

  TEST_CASE("get_symbol") {
    auto accessor = TickerAccessor::from_parameter(0);
    auto member = accessor.get_symbol();
    REQUIRE(member.get_name() == "symbol");
    REQUIRE(member.get_type() == typeid(std::string));
  }

  TEST_CASE("get_venue") {
    auto accessor = TickerAccessor::from_parameter(0);
    auto member = accessor.get_venue();
    REQUIRE(member.get_name() == "venue");
    REQUIRE(member.get_type() == typeid(std::string));
  }
}
