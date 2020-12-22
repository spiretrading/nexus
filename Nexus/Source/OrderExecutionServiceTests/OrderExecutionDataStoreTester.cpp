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
          return Viper::Sqlite3::Connection("file::memory:?cache=shared");
        });
    }
  };

  template<typename T>
  auto MakeDataStore() {
    return SpecializedMakeDataStore<T>()();
  };
}

TEST_SUITE("OrderExecutionDataStore") {
  TEST_CASE_TEMPLATE("load_orders", T, LocalOrderExecutionDataStore,
      SqlOrderExecutionDataStore<Viper::Sqlite3::Connection>) {
    auto dataStore = MakeDataStore<T>();
    auto orderInfo = SequencedValue(IndexedValue(OrderInfo(
      OrderFields::MakeLimitOrder(ACCOUNT, SECURITY, Side::BID, 100,
        Money::ONE), 100, false, time_from_string("2020-02-15 3:00:01")),
      ACCOUNT), Beam::Queries::Sequence(234));
    dataStore.Store(orderInfo);
    auto initialReport = SequencedValue(IndexedValue(
      ExecutionReport::MakeInitialReport(100,
        time_from_string("2020-02-15 3:00:01")), ACCOUNT),
      Beam::Queries::Sequence(432));
    dataStore.Store(initialReport);
    auto newReport = SequencedValue(IndexedValue(
      ExecutionReport::MakeUpdatedReport(**initialReport, OrderStatus::NEW,
        time_from_string("2020-02-15 3:00:02")), ACCOUNT),
      Beam::Queries::Sequence(433));
    dataStore.Store(newReport);
    auto fullReport = SequencedValue(IndexedValue(
      ExecutionReport::MakeUpdatedReport(**newReport, OrderStatus::FILLED,
        time_from_string("2020-02-15 3:00:03")), ACCOUNT),
      Beam::Queries::Sequence(434));
    (*fullReport)->m_lastQuantity = 100;
    (*fullReport)->m_lastPrice = Money::CENT;
    dataStore.Store(fullReport);
    auto query = AccountQuery();
    query.SetIndex(ACCOUNT); 
    query.SetRange(Beam::Queries::Sequence(234), Beam::Queries::Sequence(234));
    query.SetSnapshotLimit(Beam::Queries::SnapshotLimit::Unlimited());
    auto records = dataStore.LoadOrderSubmissions(query);
    REQUIRE(records.size() == 1);
    REQUIRE(records.front().GetSequence() == Beam::Queries::Sequence(234));
    REQUIRE(records.front()->m_executionReports.size() == 3);
    REQUIRE(records.front()->m_executionReports[0] == **initialReport);
    REQUIRE(records.front()->m_executionReports[1] == **newReport);
    REQUIRE(records.front()->m_executionReports[2] == **fullReport);
    auto loadedRecord = dataStore.LoadOrder(100);
    REQUIRE(loadedRecord.is_initialized());
    REQUIRE(loadedRecord->GetSequence() == Beam::Queries::Sequence(234));
    REQUIRE((**loadedRecord)->m_executionReports ==
      records.front()->m_executionReports);
  }
}
