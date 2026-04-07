#include <doctest/doctest.h>
#include "Nexus/Queries/OrderFieldsAccessor.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("OrderFieldsAccessor") {
  TEST_CASE("from_parameter") {
    auto accessor = OrderFieldsAccessor::from_parameter(0);
    auto account_accessor = accessor.get_account();
    REQUIRE(account_accessor.get_name() == "account");
    REQUIRE(account_accessor.get_type() == typeid(DirectoryEntry));
  }

  TEST_CASE("get_account") {
    auto accessor = OrderFieldsAccessor::from_parameter(0);
    auto member = accessor.get_account();
    REQUIRE(member.get_name() == "account");
    REQUIRE(member.get_type() == typeid(DirectoryEntry));
  }

  TEST_CASE("get_security") {
    auto accessor = OrderFieldsAccessor::from_parameter(0);
    auto member = accessor.get_security();
    REQUIRE(member.get_name() == "security");
    REQUIRE(member.get_type() == typeid(Security));
  }

  TEST_CASE("get_currency") {
    auto accessor = OrderFieldsAccessor::from_parameter(0);
    auto member = accessor.get_currency();
    REQUIRE(member.get_name() == "currency");
    REQUIRE(member.get_type() == typeid(CurrencyId));
  }

  TEST_CASE("get_type") {
    auto accessor = OrderFieldsAccessor::from_parameter(0);
    auto member = accessor.get_type();
    REQUIRE(member.get_name() == "type");
    REQUIRE(member.get_type() == typeid(OrderType));
  }

  TEST_CASE("get_side") {
    auto accessor = OrderFieldsAccessor::from_parameter(0);
    auto member = accessor.get_side();
    REQUIRE(member.get_name() == "side");
    REQUIRE(member.get_type() == typeid(Side));
  }

  TEST_CASE("get_destination") {
    auto accessor = OrderFieldsAccessor::from_parameter(0);
    auto member = accessor.get_destination();
    REQUIRE(member.get_name() == "destination");
    REQUIRE(member.get_type() == typeid(Destination));
  }

  TEST_CASE("get_quantity") {
    auto accessor = OrderFieldsAccessor::from_parameter(0);
    auto member = accessor.get_quantity();
    REQUIRE(member.get_name() == "quantity");
    REQUIRE(member.get_type() == typeid(Quantity));
  }

  TEST_CASE("get_price") {
    auto accessor = OrderFieldsAccessor::from_parameter(0);
    auto member = accessor.get_price();
    REQUIRE(member.get_name() == "price");
    REQUIRE(member.get_type() == typeid(Money));
  }

  TEST_CASE("get_time_in_force") {
    auto accessor = OrderFieldsAccessor::from_parameter(0);
    auto member = accessor.get_time_in_force();
    REQUIRE(member.get_name() == "time_in_force");
    REQUIRE(member.get_type() == typeid(TimeInForce));
  }
}
