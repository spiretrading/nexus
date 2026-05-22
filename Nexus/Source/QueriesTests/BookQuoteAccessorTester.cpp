#include <doctest/doctest.h>
#include "Nexus/Queries/BookQuoteAccessor.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;

TEST_SUITE("BookQuoteAccessor") {
  TEST_CASE("from_parameter") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto mpid_accessor = accessor.get_mpid();
    REQUIRE(mpid_accessor.get_name() == "mpid");
    REQUIRE(mpid_accessor.get_type() == typeid(std::string));
  }

  TEST_CASE("get_mpid") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto member = accessor.get_mpid();
    REQUIRE(member.get_name() == "mpid");
    REQUIRE(member.get_type() == typeid(std::string));
  }

  TEST_CASE("is_primary_mpid") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto member = accessor.is_primary_mpid();
    REQUIRE(member.get_name() == "is_primary_mpid");
    REQUIRE(member.get_type() == typeid(bool));
  }

  TEST_CASE("get_venue") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto member = accessor.get_venue();
    REQUIRE(member.get_name() == "venue");
    REQUIRE(member.get_type() == typeid(Venue));
  }

  TEST_CASE("get_quote") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto member = accessor.get_quote();
    REQUIRE(member.get_name() == "quote");
    REQUIRE(member.get_type() == typeid(Quote));
  }

  TEST_CASE("get_timestamp") {
    auto accessor = BookQuoteAccessor::from_parameter(0);
    auto member = accessor.get_timestamp();
    REQUIRE(member.get_name() == "timestamp");
    REQUIRE(member.get_type() == typeid(ptime));
  }
}
