#include <sstream>
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
}
