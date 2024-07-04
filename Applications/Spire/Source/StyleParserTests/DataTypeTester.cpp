#include <doctest/doctest.h>
#include "Spire/StyleParser/DataTypes/FloatType.hpp"
#include "Spire/StyleParser/DataTypes/IntegerType.hpp"
#include "Spire/StyleParser/DataTypes/StringType.hpp"

using namespace Spire;

TEST_SUITE("DataTypes") {
  TEST_CASE("equality") {
    auto float1 = FloatType();
    auto float2 = FloatType();
    REQUIRE(float1 == float2);
    REQUIRE_FALSE(float1 != float2);
    auto integer1 = IntegerType();
    auto integer2 = IntegerType();
    REQUIRE(integer1 == integer2);
    REQUIRE_FALSE(integer1 != integer2);
    auto string1 = StringType();
    auto string2 = StringType();
    REQUIRE(string1 == string2);
    REQUIRE_FALSE(string1 != string2);
    REQUIRE(integer1 != float1);
    REQUIRE(integer1 != string1);
    REQUIRE(float1 != string1);
  }
}
