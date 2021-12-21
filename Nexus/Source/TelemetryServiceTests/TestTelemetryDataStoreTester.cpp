#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/TelemetryServiceTests/TestTelemetryDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::TelemetryService;
using namespace Nexus::TelemetryService::Tests;

TEST_SUITE("TestTelemetryDataStore") {
  TEST_CASE("open_close") {
    auto dataStore = TestTelemetryDataStore();
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestTelemetryDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
    auto closeReceiver = Async<void>();
    Spawn([&] {
      try {
        dataStore.Close();
        closeReceiver.GetEval().SetResult();
      } catch(const std::exception&) {
        closeReceiver.GetEval().SetException(std::current_exception());
      }
    });
    auto operation = operations->Pop();
    auto closeOperation =
      get<TestTelemetryDataStore::CloseOperation>(&*operation);
    REQUIRE(closeOperation);
    closeOperation->m_result.SetResult();
    REQUIRE_NOTHROW(closeReceiver.Get());
  }

  TEST_CASE("load") {
    auto dataStore = TestTelemetryDataStore();
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestTelemetryDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
    auto query = AccountQuery();
    query.SetIndex(DirectoryEntry::MakeAccount(123, "sephi"));
    auto snapshot = std::vector<SequencedTelemetryEvent>();
    snapshot.emplace_back(SequencedValue(TelemetryEvent(
      "abcd", "spire.blotter.size", time_from_string("2021-10-23 13:01:12"),
      JsonObject()), Queries::Sequence(123)));
    SUBCASE("No exception") {
      auto loadReceiver = Async<bool>();
      Spawn([&] {
        try {
          auto receivedSnapshot = dataStore.LoadTelemetryEvents(query);
          REQUIRE(receivedSnapshot == snapshot);
          loadReceiver.GetEval().SetResult(true);
        } catch(const std::exception&) {
          loadReceiver.GetEval().SetResult(false);
        }
      });
      auto operation = operations->Pop();
      auto loadOperation =
        get<TestTelemetryDataStore::LoadTelemetryEventsOperation>(&*operation);
      REQUIRE(loadOperation);
      REQUIRE(loadOperation->m_query == &query);
      loadOperation->m_result.SetResult(snapshot);
      REQUIRE(loadReceiver.Get());
    }
    SUBCASE("exception") {
      auto loadReceiver = Async<bool>();
      Spawn([&] {
        try {
          dataStore.LoadTelemetryEvents(query);
          loadReceiver.GetEval().SetResult(false);
        } catch(const std::exception&) {
          loadReceiver.GetEval().SetResult(true);
        }
      });
      auto operation = operations->Pop();
      auto loadOperation =
        get<TestTelemetryDataStore::LoadTelemetryEventsOperation>(&*operation);
      REQUIRE(loadOperation);
      REQUIRE(loadOperation->m_query == &query);
      loadOperation->m_result.SetException(std::runtime_error("Failed"));
      REQUIRE(loadReceiver.Get());
    }
  }

  TEST_CASE("store") {
    auto dataStore = TestTelemetryDataStore();
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestTelemetryDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
    auto account = DirectoryEntry::MakeAccount(123, "sephi");
    auto event = SequencedValue(IndexedValue(TelemetryEvent(
      "abcd", "spire.blotter.size", time_from_string("2021-10-23 13:01:12"),
      JsonObject()), account), Queries::Sequence(123));
    SUBCASE("No exception") {
      auto storeReceiver = Async<void>();
      Spawn([&] {
        try {
          dataStore.Store(event);
          storeReceiver.GetEval().SetResult();
        } catch(const std::exception&) {
          storeReceiver.GetEval().SetException(std::current_exception());
        }
      });
      auto operation = operations->Pop();
      auto storeOperation =
        get<TestTelemetryDataStore::StoreEventOperation>(&*operation);
      REQUIRE(storeOperation);
      REQUIRE(storeOperation->m_event == &event);
      storeOperation->m_result.SetResult();
      REQUIRE_NOTHROW(storeReceiver.Get());
    }
    SUBCASE("exception") {
      auto storeReceiver = Async<void>();
      Spawn([&] {
        try {
          dataStore.Store(event);
          storeReceiver.GetEval().SetResult();
        } catch(const std::exception&) {
          storeReceiver.GetEval().SetException(std::current_exception());
        }
      });
      auto operation = operations->Pop();
      auto storeOperation =
        get<TestTelemetryDataStore::StoreEventOperation>(&*operation);
      REQUIRE(storeOperation);
      storeOperation->m_result.SetException(std::runtime_error("Fail"));
      REQUIRE_THROWS(storeReceiver.Get());
    }
  }

  TEST_CASE("store_list") {
    auto dataStore = TestTelemetryDataStore();
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestTelemetryDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
    auto account = DirectoryEntry::MakeAccount(123, "sephi");
    auto events = std::vector<SequencedAccountTelemetryEvent>();
    events.push_back(SequencedValue(IndexedValue(TelemetryEvent(
      "abcd", "spire.blotter.size", time_from_string("2021-10-23 13:01:12"),
      JsonObject()), account), Queries::Sequence(123)));
    events.push_back(SequencedValue(IndexedValue(TelemetryEvent(
      "abcd", "spire.blotter.size", time_from_string("2021-10-23 13:01:13"),
      JsonObject()), account), Queries::Sequence(124)));
    SUBCASE("No exception") {
      auto storeReceiver = Async<void>();
      Spawn([&] {
        try {
          dataStore.Store(events);
          storeReceiver.GetEval().SetResult();
        } catch(const std::exception&) {
          storeReceiver.GetEval().SetException(std::current_exception());
        }
      });
      auto operation = operations->Pop();
      auto storeOperation =
        get<TestTelemetryDataStore::StoreEventListOperation>(&*operation);
      REQUIRE(storeOperation);
      REQUIRE(storeOperation->m_events == &events);
      storeOperation->m_result.SetResult();
      REQUIRE_NOTHROW(storeReceiver.Get());
    }
    SUBCASE("exception") {
      auto storeReceiver = Async<void>();
      Spawn([&] {
        try {
          dataStore.Store(events);
          storeReceiver.GetEval().SetResult();
        } catch(const std::exception&) {
          storeReceiver.GetEval().SetException(std::current_exception());
        }
      });
      auto operation = operations->Pop();
      auto storeOperation =
        get<TestTelemetryDataStore::StoreEventListOperation>(&*operation);
      REQUIRE(storeOperation);
      storeOperation->m_result.SetException(std::runtime_error("Fail"));
      REQUIRE_THROWS(storeReceiver.Get());
    }
  }
}
