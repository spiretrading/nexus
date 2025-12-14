#include <doctest/doctest.h>
#include "Nexus/AdministrationService/LocalAdministrationDataStore.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationDataStoreTestSuite.hpp"

using namespace Nexus;
using namespace Nexus::Tests;

namespace {
  struct Builder {
    auto operator ()() const {
      return LocalAdministrationDataStore();
    }
  };
}

TEST_SUITE("LocalAdministrationDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(AdministrationDataStoreTestSuite, Builder);
}
