#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskServiceTests/TestRiskDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;
using namespace Nexus::RiskService::Tests;

TEST_SUITE("TestRiskDataStore") {
  TEST_CASE("open_close") {
    auto dataStore = TestRiskDataStore();
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestRiskDataStore::Operation>>>();
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
    auto closeOperation = get<TestRiskDataStore::CloseOperation>(&*operation);
    REQUIRE(closeOperation);
    closeOperation->m_result.SetResult();
    REQUIRE_NOTHROW(closeReceiver.Get());
  }

  TEST_CASE("store") {
    auto dataStore = TestRiskDataStore();
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestRiskDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
    auto account = DirectoryEntry::MakeAccount(123, "sephi");
    auto snapshot = InventorySnapshot();
    snapshot.m_excludedOrders.push_back(12);
    snapshot.m_excludedOrders.push_back(16);
    snapshot.m_sequence = Beam::Queries::Sequence(47);
    SUBCASE("No exception") {
      auto storeReceiver = Async<void>();
      Spawn([&] {
        try {
          dataStore.Store(account, snapshot);
          storeReceiver.GetEval().SetResult();
        } catch(const std::exception&) {
          storeReceiver.GetEval().SetException(std::current_exception());
        }
      });
      auto operation = operations->Pop();
      auto storeOperation =
        get<TestRiskDataStore::StoreInventorySnapshotOperation>(&*operation);
      REQUIRE(storeOperation);
      REQUIRE(*storeOperation->m_account == account);
      REQUIRE(*storeOperation->m_snapshot == snapshot);
      storeOperation->m_result.SetResult();
      REQUIRE_NOTHROW(storeReceiver.Get());
    }
    SUBCASE("exception") {
      auto storeReceiver = Async<void>();
      Spawn([&] {
        try {
          dataStore.Store(account, snapshot);
          storeReceiver.GetEval().SetResult();
        } catch(const std::exception&) {
          storeReceiver.GetEval().SetException(std::current_exception());
        }
      });
      auto operation = operations->Pop();
      auto storeOperation =
        get<TestRiskDataStore::StoreInventorySnapshotOperation>(&*operation);
      REQUIRE(storeOperation);
      storeOperation->m_result.SetException(std::runtime_error("Fail"));
      REQUIRE_THROWS(storeReceiver.Get());
    }
  }

  TEST_CASE("load") {
    auto dataStore = TestRiskDataStore();
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestRiskDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
    auto account = DirectoryEntry::MakeAccount(123, "sephi");
    auto snapshot = InventorySnapshot();
    snapshot.m_excludedOrders.push_back(12);
    snapshot.m_excludedOrders.push_back(16);
    snapshot.m_sequence = Beam::Queries::Sequence(47);
    SUBCASE("No exception") {
      auto loadReceiver = Async<bool>();
      Spawn([&] {
        try {
          auto receivedSnapshot = dataStore.LoadInventorySnapshot(account);
          REQUIRE(receivedSnapshot == snapshot);
          loadReceiver.GetEval().SetResult(true);
        } catch(const std::exception&) {
          loadReceiver.GetEval().SetResult(false);
        }
      });
      auto operation = operations->Pop();
      auto loadOperation =
        get<TestRiskDataStore::LoadInventorySnapshotOperation>(&*operation);
      REQUIRE(loadOperation);
      REQUIRE(*loadOperation->m_account == account);
      loadOperation->m_result.SetResult(snapshot);
      REQUIRE(loadReceiver.Get());
    }
    SUBCASE("exception") {
      auto loadReceiver = Async<bool>();
      Spawn([&] {
        try {
          dataStore.LoadInventorySnapshot(account);
          loadReceiver.GetEval().SetResult(false);
        } catch(const std::exception&) {
          loadReceiver.GetEval().SetResult(true);
        }
      });
      auto operation = operations->Pop();
      auto loadOperation =
        get<TestRiskDataStore::LoadInventorySnapshotOperation>(&*operation);
      REQUIRE(loadOperation);
      REQUIRE(*loadOperation->m_account == account);
      loadOperation->m_result.SetException(std::runtime_error("Failed"));
      REQUIRE(loadReceiver.Get());
    }
  }
}
