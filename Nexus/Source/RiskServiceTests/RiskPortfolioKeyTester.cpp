#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskService/RiskPortfolioTypes.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::RiskService;
using namespace Nexus::DefaultVenues;

TEST_SUITE("RiskPortfolioKey") {
  TEST_CASE("stream") {
    auto account = DirectoryEntry::MakeAccount(123, "test_account");
    auto security = Security("ABC", TSX);
    auto key = RiskPortfolioKey(account, security);
    auto out = std::ostringstream();
    out << key;
    REQUIRE(out.str() == "((ACCOUNT 123 test_account) ABC.TSX)");

    SUBCASE("shuttle") {
      Beam::Serialization::Tests::TestRoundTripShuttle(key);
    }
  }
}
