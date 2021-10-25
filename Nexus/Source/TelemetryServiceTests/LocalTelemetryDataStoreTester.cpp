#include <doctest/doctest.h>
#include "Nexus/TelemetryService/LocalTelemetryDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::TelemetryService;

TEST_SUITE("LocalTelemetryDataStore") {
  TEST_CASE("load_events") {
    auto dataStore = LocalTelemetryDataStore();
  }
}
