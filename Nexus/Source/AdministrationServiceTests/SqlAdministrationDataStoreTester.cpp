#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/AdministrationService/SqlAdministrationDataStore.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationDataStoreTestSuite.hpp"

using namespace Beam;
using namespace Beam::ServiceLocator;
using namespace Nexus;
using namespace Nexus::AdministrationService;
using namespace Nexus::AdministrationService::Tests;
using namespace Viper;
using namespace Viper::Sqlite3;

namespace {
  using TestSqlAdministrationDataStore = SqlAdministrationDataStore<Connection>;
}

namespace {
  struct Builder {
    auto operator ()() const {
      return TestSqlAdministrationDataStore(
        std::make_unique<Connection>(":memory:"), [] (auto id) {
          auto name = [&] {
            if(id == 123) {
              return "user1";
            } else if(id == 456) {
              return "admin";
            }
            return "";
          }();
          return DirectoryEntry::MakeAccount(id, name);
        });
    }
  };
}

TEST_SUITE("SqlAdministrationDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(AdministrationDataStoreTestSuite, Builder);
}
