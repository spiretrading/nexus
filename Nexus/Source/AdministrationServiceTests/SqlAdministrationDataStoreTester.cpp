#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/AdministrationService/SqlAdministrationDataStore.hpp"
#include "Nexus/AdministrationServiceTests/AdministrationDataStoreTestSuite.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace Viper;
using namespace Viper::Sqlite3;

namespace {
  using TestSqlAdministrationDataStore =
    SqlAdministrationDataStore<Viper::Sqlite3::Connection>;

  struct Builder {
    auto operator ()() const {
      auto directory_entries =
        std::unordered_map<unsigned int, DirectoryEntry>();
      directory_entries.insert(
        std::pair(23, DirectoryEntry::make_directory(23, "TSX")));
      directory_entries.insert(
        std::pair(100, DirectoryEntry::make_account(100, "user_a")));
      directory_entries.insert(
        std::pair(123, DirectoryEntry::make_account(123, "user1")));
      directory_entries.insert(
        std::pair(345, DirectoryEntry::make_account(345, "user2")));
      directory_entries.insert(
        std::pair(456, DirectoryEntry::make_account(456, "admin")));
      return TestSqlAdministrationDataStore(
        std::make_unique<Viper::Sqlite3::Connection>(":memory:"),
          [=] (auto id) {
            auto i = directory_entries.find(id);
            if(i != directory_entries.end()) {
              return i->second;
            }
            return DirectoryEntry();
          });
    }
  };
}

TEST_SUITE("SqlAdministrationDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(AdministrationDataStoreTestSuite, Builder);
}
