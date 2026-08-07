#include <doctest/doctest.h>
#include "Nexus/Queries/QuoteAccessor.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("QuoteAccessor") {
  TEST_CASE("from_parameter") {
    auto accessor = QuoteAccessor::from_parameter(0);
    auto price_accessor = accessor.get_price();
    REQUIRE(price_accessor.get_name() == "price");
    REQUIRE(price_accessor.get_type() == typeid(Money));
  }

  TEST_CASE("get_price") {
    auto accessor = QuoteAccessor::from_parameter(0);
    auto member = accessor.get_price();
    REQUIRE(member.get_name() == "price");
    REQUIRE(member.get_type() == typeid(Money));
  }

  TEST_CASE("get_size") {
    auto accessor = QuoteAccessor::from_parameter(0);
    auto member = accessor.get_size();
    REQUIRE(member.get_name() == "size");
    REQUIRE(member.get_type() == typeid(Quantity));
  }

  TEST_CASE("get_side") {
    auto accessor = QuoteAccessor::from_parameter(0);
    auto member = accessor.get_side();
    REQUIRE(member.get_name() == "side");
    REQUIRE(member.get_type() == typeid(Side));
  }
}
