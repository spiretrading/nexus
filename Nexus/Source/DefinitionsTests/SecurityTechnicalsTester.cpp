#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/SecurityTechnicals.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;

TEST_SUITE("SecurityTechnicals") {
  TEST_CASE("stream") {
    auto volume = Quantity(100);
    auto high = Money(10);
    auto low = Money(5);
    auto open = Money(7);
    auto close = Money(8);
    auto technicals = SecurityTechnicals(volume, high, low, open, close);
    auto expected = std::stringstream();
    expected << '(' << volume << ' ' << high << ' ' << low << ' ' << open <<
      ' ' << close  << ')';
    CHECK(to_string(technicals) == expected.str());
    test_round_trip_shuttle(technicals, [] (const auto& technicals) {
      REQUIRE(technicals.m_volume == 100);
      REQUIRE(technicals.m_high == Money(10));
      REQUIRE(technicals.m_low == Money(5));
      REQUIRE(technicals.m_open == Money(7));
      REQUIRE(technicals.m_close == Money(8));
    });
  }
}
