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
using namespace Nexus::Queries;

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

  template<typename DataStore>
  SequencedOrderRecord StoreLiveOrder(DataStore& dataStore, OrderId id,
      bool isShort, ptime timestamp, Beam::Queries::Sequence orderSequence,
      Beam::Queries::Sequence reportSequence) {
    auto side = [&] {
      if(isShort) {
        return Side::ASK;
      }
      return Side::BID;
    }();
    auto orderInfo = SequencedValue(IndexedValue(OrderInfo(
      OrderFields::MakeLimitOrder(ACCOUNT, SECURITY, side, 100, Money::ONE), id,
      isShort, timestamp), ACCOUNT), orderSequence);
    dataStore.Store(orderInfo);
    auto initialReport = SequencedValue(IndexedValue(
      ExecutionReport::MakeInitialReport(id, timestamp), ACCOUNT),
      reportSequence);
    dataStore.Store(initialReport);
    auto newReport = SequencedValue(IndexedValue(
      ExecutionReport::MakeUpdatedReport(**initialReport, OrderStatus::NEW,
        timestamp + seconds(1)), ACCOUNT),
      Beam::Queries::Increment(reportSequence));
    dataStore.Store(newReport);
    return SequencedValue(OrderRecord(
      **orderInfo, {**initialReport, **newReport}), orderSequence);
  }

  template<typename DataStore>
  SequencedOrderRecord StoreTerminalOrder(DataStore& dataStore, OrderId id,
      bool isShort, ptime timestamp, Beam::Queries::Sequence orderSequence,
      Beam::Queries::Sequence reportSequence) {
    auto record = StoreLiveOrder(dataStore, id, isShort, timestamp,
      orderSequence, reportSequence);
    auto& lastReport = record->m_executionReports.back();
    auto cancelReport = SequencedValue(IndexedValue(
      ExecutionReport::MakeUpdatedReport(lastReport, OrderStatus::CANCELED,
        lastReport.m_timestamp + seconds(1)), ACCOUNT),
      Beam::Queries::Increment(Beam::Queries::Increment(reportSequence)));
    dataStore.Store(cancelReport);
    return record;
  }
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

  TEST_CASE_TEMPLATE("load_live_orders", T, LocalOrderExecutionDataStore,
      SqlOrderExecutionDataStore<Viper::Sqlite3::Connection>) {
    auto dataStore = MakeDataStore<T>();
    StoreLiveOrder(dataStore, 100, true, time_from_string("2020-02-15 3:00:00"),
      Beam::Queries::Sequence(100), Beam::Queries::Sequence(100));
    StoreLiveOrder(dataStore, 101, false,
      time_from_string("2020-02-15 3:05:00"), Beam::Queries::Sequence(200),
      Beam::Queries::Sequence(200));
    StoreTerminalOrder(dataStore, 102, false,
      time_from_string("2020-02-15 3:10:00"), Beam::Queries::Sequence(300),
      Beam::Queries::Sequence(300));
    StoreLiveOrder(dataStore, 103, false,
      time_from_string("2020-02-15 3:15:00"), Beam::Queries::Sequence(400),
      Beam::Queries::Sequence(400));
    StoreTerminalOrder(dataStore, 104, true,
      time_from_string("2020-02-15 3:30:00"), Beam::Queries::Sequence(500),
      Beam::Queries::Sequence(500));
    SUBCASE("Load live orders.") {
      auto liveQuery = AccountQuery();
      liveQuery.SetIndex(ACCOUNT);
      liveQuery.SetRange(Range::Historical());
      liveQuery.SetSnapshotLimit(SnapshotLimit::Unlimited());
      liveQuery.SetFilter(MemberAccessExpression("is_live",
        NativeDataType<bool>(), ParameterExpression(0, OrderInfoType())));
      auto liveOrders = dataStore.LoadOrderSubmissions(liveQuery);
      REQUIRE(liveOrders.size() == 3);
      REQUIRE(liveOrders[0].GetSequence() == Beam::Queries::Sequence(100));
      REQUIRE(liveOrders[1].GetSequence() == Beam::Queries::Sequence(200));
      REQUIRE(liveOrders[2].GetSequence() == Beam::Queries::Sequence(400));
    }
    SUBCASE("Load terminal orders.") {
      auto liveQuery = AccountQuery();
      liveQuery.SetIndex(ACCOUNT);
      liveQuery.SetRange(Range::Historical());
      liveQuery.SetSnapshotLimit(SnapshotLimit::Unlimited());
      liveQuery.SetFilter(NotExpression(MemberAccessExpression("is_live",
        NativeDataType<bool>(), ParameterExpression(0, OrderInfoType()))));
      auto liveOrders = dataStore.LoadOrderSubmissions(liveQuery);
      REQUIRE(liveOrders.size() == 2);
      REQUIRE(liveOrders[0].GetSequence() == Beam::Queries::Sequence(300));
      REQUIRE(liveOrders[1].GetSequence() == Beam::Queries::Sequence(500));
    }
    SUBCASE("Load mixed orders.") {
      auto liveQuery = AccountQuery();
      liveQuery.SetIndex(ACCOUNT);
      liveQuery.SetRange(Range::Historical());
      liveQuery.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto isLiveExpression = MemberAccessExpression("is_live",
        NativeDataType<bool>(), ParameterExpression(0, OrderInfoType()));
      auto isShortExpression = MemberAccessExpression("shorting_flag",
        NativeDataType<bool>(), ParameterExpression(0, OrderInfoType()));
      liveQuery.SetFilter(FunctionExpression("==", BoolType(),
        {isLiveExpression, isShortExpression}));
      auto liveOrders = dataStore.LoadOrderSubmissions(liveQuery);
      REQUIRE(liveOrders.size() == 2);
      REQUIRE(liveOrders[0].GetSequence() == Beam::Queries::Sequence(100));
      REQUIRE(liveOrders[1].GetSequence() == Beam::Queries::Sequence(300));
    }
    SUBCASE("Load reverse mixed orders.") {
      auto liveQuery = AccountQuery();
      liveQuery.SetIndex(ACCOUNT);
      liveQuery.SetRange(Range::Historical());
      liveQuery.SetSnapshotLimit(SnapshotLimit::Unlimited());
      auto isLiveExpression = MemberAccessExpression("is_live",
        NativeDataType<bool>(), ParameterExpression(0, OrderInfoType()));
      auto isShortExpression = MemberAccessExpression("shorting_flag",
        NativeDataType<bool>(), ParameterExpression(0, OrderInfoType()));
      liveQuery.SetFilter(FunctionExpression("==", BoolType(),
        {NotExpression(isLiveExpression), isShortExpression}));
      auto liveOrders = dataStore.LoadOrderSubmissions(liveQuery);
      REQUIRE(liveOrders.size() == 3);
      REQUIRE(liveOrders[0].GetSequence() == Beam::Queries::Sequence(200));
      REQUIRE(liveOrders[1].GetSequence() == Beam::Queries::Sequence(400));
      REQUIRE(liveOrders[2].GetSequence() == Beam::Queries::Sequence(500));
    }
  }
}
