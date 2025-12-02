#include <doctest/doctest.h>
#include "Nexus/Queries/OrderInfoAccessor.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("OrderInfoAccessor") {
  TEST_CASE("from_parameter") {
    auto accessor = OrderInfoAccessor::from_parameter(0);
    auto fields_accessor = accessor.get_fields();
    REQUIRE(fields_accessor.get_name() == "fields");
    REQUIRE(fields_accessor.get_type() == typeid(OrderFields));
  }

  TEST_CASE("get_fields") {
    auto accessor = OrderInfoAccessor::from_parameter(0);
    auto member = accessor.get_fields();
    REQUIRE(member.get_name() == "fields");
    REQUIRE(member.get_type() == typeid(OrderFields));
  }

  TEST_CASE("get_submission_account") {
    auto accessor = OrderInfoAccessor::from_parameter(0);
    auto member = accessor.get_submission_account();
    REQUIRE(member.get_name() == "submission_account");
    REQUIRE(member.get_type() == typeid(DirectoryEntry));
  }

  TEST_CASE("get_order_id") {
    auto accessor = OrderInfoAccessor::from_parameter(0);
    auto member = accessor.get_order_id();
    REQUIRE(member.get_name() == "order_id");
    REQUIRE(member.get_type() == typeid(OrderId));
  }

  TEST_CASE("get_shorting_flag") {
    auto accessor = OrderInfoAccessor::from_parameter(0);
    auto member = accessor.get_shorting_flag();
    REQUIRE(member.get_name() == "shorting_flag");
    REQUIRE(member.get_type() == typeid(bool));
  }

  TEST_CASE("get_timestamp") {
    auto accessor = OrderInfoAccessor::from_parameter(0);
    auto member = accessor.get_timestamp();
    REQUIRE(member.get_name() == "timestamp");
    REQUIRE(member.get_type() == typeid(ptime));
  }
}
