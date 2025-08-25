#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/EntitlementModification.hpp"

using namespace Beam::ServiceLocator;
using namespace Nexus;

TEST_SUITE("EntitlementModification") {
  TEST_CASE("stream") {
    auto entitlements = std::vector<DirectoryEntry>();
    entitlements.push_back(DirectoryEntry::MakeDirectory(1, "alpha"));
    entitlements.push_back(DirectoryEntry::MakeDirectory(2, "beta"));
    auto modification = EntitlementModification(entitlements);
    Beam::Serialization::Tests::TestRoundTripShuttle(modification,
      [&](const auto& received) {
        REQUIRE(received.get_entitlements() == entitlements);
      });
  }
}
