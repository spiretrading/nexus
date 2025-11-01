#include <sstream>
#include <Beam/SerializationTests/ValueShuttleTests.hpp>
#include <doctest/doctest.h>
#include "Nexus/AdministrationService/EntitlementModification.hpp"

using namespace Beam;
using namespace Beam::Tests;
using namespace Nexus;

TEST_SUITE("EntitlementModification") {
  TEST_CASE("stream") {
    auto entitlements = std::vector<DirectoryEntry>();
    entitlements.push_back(DirectoryEntry::make_directory(1, "alpha"));
    entitlements.push_back(DirectoryEntry::make_directory(2, "beta"));
    auto modification = EntitlementModification(entitlements);
    test_round_trip_shuttle(modification, [&] (const auto& received) {
      REQUIRE(received.get_entitlements() == entitlements);
    });
  }
}
