#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <boost/lexical_cast.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/AccountModificationRequestCounts.hpp"

using namespace Beam::Tests;
using namespace boost;
using namespace Nexus;

TEST_SUITE("AccountModificationRequestCounts") {
  TEST_CASE("stream") {
    auto counts = AccountModificationRequestCounts(4, 2, 1);
    REQUIRE(lexical_cast<std::string>(counts) == "(4 2 1)");
    test_round_trip_shuttle(counts);
  }
}
