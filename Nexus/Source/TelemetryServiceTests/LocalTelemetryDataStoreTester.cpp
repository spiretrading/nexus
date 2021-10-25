#include <doctest/doctest.h>
#include "Nexus/TelemetryService/LocalTelemetryDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::TelemetryService;

TEST_SUITE("LocalTelemetryDataStore") {
  TEST_CASE("load_empty_events") {
    auto dataStore = LocalTelemetryDataStore();
    auto query = AccountQuery();
    query.SetIndex(DirectoryEntry::MakeAccount(123, "test"));
    query.SetRange(Range::Total());
    query.SetSnapshotLimit(SnapshotLimit::Unlimited());
    auto snapshot = dataStore.LoadTelemetryEvents(query);
    REQUIRE(snapshot.empty());
  }
}
