#include <doctest/doctest.h>
#include "Nexus/Queries/TimeAndSaleAccessor.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("TimeAndSaleAccessor") {
  TEST_CASE("from_parameter") {
    auto accessor = TimeAndSaleAccessor::from_parameter(0);
    auto timestamp_accessor = accessor.get_timestamp();
    REQUIRE(timestamp_accessor.get_name() == "timestamp");
    REQUIRE(timestamp_accessor.get_type() == typeid(ptime));
  }

  TEST_CASE("get_timestamp") {
    auto accessor = TimeAndSaleAccessor::from_parameter(0);
    auto member = accessor.get_timestamp();
    REQUIRE(member.get_name() == "timestamp");
    REQUIRE(member.get_type() == typeid(ptime));
  }

  TEST_CASE("get_price") {
    auto accessor = TimeAndSaleAccessor::from_parameter(0);
    auto member = accessor.get_price();
    REQUIRE(member.get_name() == "price");
    REQUIRE(member.get_type() == typeid(Money));
  }

  TEST_CASE("get_size") {
    auto accessor = TimeAndSaleAccessor::from_parameter(0);
    auto member = accessor.get_size();
    REQUIRE(member.get_name() == "size");
    REQUIRE(member.get_type() == typeid(Quantity));
  }

  TEST_CASE("get_market_center") {
    auto accessor = TimeAndSaleAccessor::from_parameter(0);
    auto member = accessor.get_market_center();
    REQUIRE(member.get_name() == "market_center");
    REQUIRE(member.get_type() == typeid(std::string));
  }

  TEST_CASE("get_buyer_mpid") {
    auto accessor = TimeAndSaleAccessor::from_parameter(0);
    auto member = accessor.get_buyer_mpid();
    REQUIRE(member.get_name() == "buyer_mpid");
    REQUIRE(member.get_type() == typeid(std::string));
  }

  TEST_CASE("get_seller_mpid") {
    auto accessor = TimeAndSaleAccessor::from_parameter(0);
    auto member = accessor.get_seller_mpid();
    REQUIRE(member.get_name() == "seller_mpid");
    REQUIRE(member.get_type() == typeid(std::string));
  }
}
