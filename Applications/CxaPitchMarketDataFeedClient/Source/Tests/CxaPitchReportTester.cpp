#include <doctest/doctest.h>
#include "CxaPitchMarketDataFeedClient/CxaPitchReport.hpp"

using namespace Nexus;

TEST_SUITE("CxaPitchReport") {
  TEST_CASE("writer_constraints") {
    auto accepts = []<typename F> (F&&) {
      return requires { print(std::declval<F>()); };
    };
    REQUIRE(accepts([] (std::stringstream& stream) { stream << "report"; }));
    REQUIRE(!accepts(0));
    REQUIRE(!accepts(&std::ios_base::getloc));
  }
}
