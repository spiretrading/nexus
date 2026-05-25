#include <doctest/doctest.h>
#include "Nexus/Queries/BboQuoteAccessor.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("BboQuoteAccessor") {
  TEST_CASE("from_parameter") {
    auto accessor = BboQuoteAccessor::from_parameter(0);
    auto bid_accessor = accessor.get_bid();
    REQUIRE(bid_accessor.get_name() == "bid");
    REQUIRE(bid_accessor.get_type() == typeid(Quote));
  }

  TEST_CASE("get_bid") {
    auto accessor = BboQuoteAccessor::from_parameter(0);
    auto member = accessor.get_bid();
    REQUIRE(member.get_name() == "bid");
    REQUIRE(member.get_type() == typeid(Quote));
  }

  TEST_CASE("get_ask") {
    auto accessor = BboQuoteAccessor::from_parameter(0);
    auto member = accessor.get_ask();
    REQUIRE(member.get_name() == "ask");
    REQUIRE(member.get_type() == typeid(Quote));
  }

  TEST_CASE("get_timestamp") {
    auto accessor = BboQuoteAccessor::from_parameter(0);
    auto member = accessor.get_timestamp();
    REQUIRE(member.get_name() == "timestamp");
    REQUIRE(member.get_type() == typeid(ptime));
  }
}
