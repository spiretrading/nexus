#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/OrderExecutionService/SqlOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionDataStoreTestSuite.hpp"

using namespace Beam;
using namespace Nexus;
using namespace Nexus::Tests;
using namespace Viper;
using namespace Viper::Sqlite3;

namespace {
  using TestSqlOrderExecutionDataStore =
    SqlOrderExecutionDataStore<Viper::Sqlite3::Connection>;

  struct Builder {
    auto operator ()() const {
      auto directory_entries =
        std::unordered_map<unsigned int, DirectoryEntry>();
      directory_entries.insert(
        std::pair(123, DirectoryEntry::make_account(123, "user_a")));
      directory_entries.insert(
        std::pair(456, DirectoryEntry::make_account(456, "user_b")));
      return TestSqlOrderExecutionDataStore(
        [] {
          return Viper::Sqlite3::Connection("file::memory:?cache=shared");
        },
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

TEST_SUITE("SqlOrderExecutionDataStore") {
  TEST_CASE_TEMPLATE_INVOKE(OrderExecutionDataStoreTestSuite, Builder);
}
