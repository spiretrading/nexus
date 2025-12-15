#include <doctest/doctest.h>
#include "Nexus/AdministrationService/CachedAdministrationDataStore.hpp"
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationDataStoreTestSuite.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return CachedAdministrationDataStore<LocalAdministrationDataStore>(
        init());
    }
  };
}

TEST_SUITE("CachedAdministrationDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(AdministrationDataStoreTestSuite, Builder);
}
