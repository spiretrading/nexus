#include <boost/functional/factory.hpp>
#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionService/SqlOrderExecutionDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;

namespace {
  const auto ACCOUNT = DirectoryEntry::MakeAccount(12, "test1");
  const auto SECURITY = Security("A1", DefaultMarkets::NYSE(),
    DefaultCountries::US());

  template<typename T>
  struct SpecializedMakeDataStore {};

  template<>
  struct SpecializedMakeDataStore<LocalOrderExecutionDataStore> {
    auto operator ()() const {
      return LocalOrderExecutionDataStore();
    }
  };

  template<>
  struct SpecializedMakeDataStore<
      SqlOrderExecutionDataStore<Viper::Sqlite3::Connection>> {
    auto operator ()() const {
      return SqlOrderExecutionDataStore<Viper::Sqlite3::Connection>(
        [] {
          return Viper::Sqlite3::Connection(":memory:");
        });
    }
  };

  template<typename T>
  auto MakeDataStore() {
    return SpecializedMakeDataStore<T>()();
  };
}

TEST_CASE_TEMPLATE("query_order_records", T, LocalOrderExecutionDataStore,
    SqlOrderExecutionDataStore<Viper::Sqlite3::Connection>) {
  auto dataStore = MakeDataStore<T>();
  auto orderRecord = SequencedValue(IndexedValue(OrderInfo(
    OrderFields::BuildLimitOrder(SECURITY, Side::BID, 100, Money::ONE), 100,
    false, time_from_string("2020-02-15 3:00:01")), ACCOUNT),
    Beam::Queries::Sequence(234));
  dataStore.Store(orderRecord);
}
