#include <doctest/doctest.h>
#include "Nexus/Queries/OrderImbalanceAccessor.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("OrderImbalanceAccessor") {
  TEST_CASE("from_parameter") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto ticker_accessor = accessor.get_ticker();
    REQUIRE(ticker_accessor.get_name() == "ticker");
    REQUIRE(ticker_accessor.get_type() == typeid(Ticker));
  }

  TEST_CASE("get_ticker") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto member = accessor.get_ticker();
    REQUIRE(member.get_name() == "ticker");
    REQUIRE(member.get_type() == typeid(Ticker));
  }

  TEST_CASE("get_side") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto member = accessor.get_side();
    REQUIRE(member.get_name() == "side");
    REQUIRE(member.get_type() == typeid(Side));
  }

  TEST_CASE("get_size") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto member = accessor.get_size();
    REQUIRE(member.get_name() == "size");
    REQUIRE(member.get_type() == typeid(Quantity));
  }

  TEST_CASE("get_reference_price") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto member = accessor.get_reference_price();
    REQUIRE(member.get_name() == "reference_price");
    REQUIRE(member.get_type() == typeid(Money));
  }

  TEST_CASE("get_timestamp") {
    auto accessor = OrderImbalanceAccessor::from_parameter(0);
    auto member = accessor.get_timestamp();
    REQUIRE(member.get_name() == "timestamp");
    REQUIRE(member.get_type() == typeid(ptime));
  }
}
