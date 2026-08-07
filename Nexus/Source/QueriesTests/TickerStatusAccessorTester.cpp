#include <doctest/doctest.h>
#include "Nexus/Queries/TickerStatusAccessor.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("TickerStatusAccessor") {
  TEST_CASE("from_parameter") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto venue_accessor = accessor.get_venue();
    REQUIRE(venue_accessor.get_name() == "venue");
    REQUIRE(venue_accessor.get_type() == typeid(Venue));
  }

  TEST_CASE("get_venue") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto member = accessor.get_venue();
    REQUIRE(member.get_name() == "venue");
    REQUIRE(member.get_type() == typeid(Venue));
  }

  TEST_CASE("get_state") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto member = accessor.get_state();
    REQUIRE(member.get_name() == "state");
    REQUIRE(member.get_type() == typeid(std::string));
  }

  TEST_CASE("get_flags") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto member = accessor.get_flags();
    REQUIRE(member.get_name() == "flags");
    REQUIRE(member.get_type() == typeid(int));
  }

  TEST_CASE("get_timestamp") {
    auto accessor = TickerStatusAccessor::from_parameter(0);
    auto member = accessor.get_timestamp();
    REQUIRE(member.get_name() == "timestamp");
    REQUIRE(member.get_type() == typeid(ptime));
  }
}
