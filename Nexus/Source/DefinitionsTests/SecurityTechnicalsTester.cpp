#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/Definitions/SecurityTechnicals.hpp"

using namespace Nexus;

TEST_SUITE("SecurityTechnicals") {
  TEST_CASE("stream") {
    auto volume = Quantity(100);
    auto high = Money(10);
    auto low = Money(5);
    auto open = Money(7);
    auto close = Money(8);
    auto technicals = SecurityTechnicals(volume, high, low, open, close);
    auto ss = std::stringstream();
    ss << technicals;
    auto expected = std::stringstream();
    expected << '(' << volume << ' ' << high << ' ' << low << ' ' << open <<
      ' ' << close  << ')';
    CHECK(ss.str() == expected.str());
  }

  TEST_CASE("shuttle") {
    Beam::Serialization::Tests::TestRoundTripShuttle(
      SecurityTechnicals(100, Money(10), Money(5), Money(7), Money(8)),
      [] (const auto& technicals) {
        REQUIRE(technicals.m_volume == 100);
        REQUIRE(technicals.m_high == Money(10));
        REQUIRE(technicals.m_low == Money(5));
        REQUIRE(technicals.m_open == Money(7));
        REQUIRE(technicals.m_close == Money(8));
      });
  }
}
