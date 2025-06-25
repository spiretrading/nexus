#include <doctest/doctest.h>
#include "Nexus/Definitions/Venue.hpp"

using namespace Beam;
using namespace Nexus;

TEST_SUITE("Venue") {
  TEST_CASE("equality") {
    auto default_v1 = Venue();
    auto default_v2 = Venue();
    CHECK(default_v1 == default_v2);
    auto mic = FixedString<4>("ABCD");
    auto v1 = Venue(mic);
    auto v2 = Venue("ABCD");
    CHECK(v1 == v2);
    auto v3 = Venue("WXYZ");
    CHECK_FALSE(v1 == v3);
  }
}
