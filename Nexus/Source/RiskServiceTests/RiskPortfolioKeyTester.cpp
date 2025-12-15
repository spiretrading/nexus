#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <Beam/Utilities/ToString.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;
using namespace Nexus::DefaultVenues;

TEST_SUITE("RiskPortfolioKey") {
  TEST_CASE("stream") {
    auto account = DirectoryEntry::make_account(123, "test_account");
    auto security = Security("ABC", TSX);
    auto key = RiskPortfolioKey(account, security);
    REQUIRE(to_string(key) == "((ACCOUNT 123 test_account) ABC.TSX)");
    test_round_trip_shuttle(key);
  }
}
