#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/RiskService/TestRiskDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::RiskService;

namespace {
  void Open(TestRiskDataStore& dataStore) {
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestRiskDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
    auto openReceiver = Async<void>();
    Spawn([&] {
      dataStore.Open();
      openReceiver.GetEval().SetResult();
    });
    while(true) {
      auto operation = operations->Pop();
      if(auto openOperation =
          get<TestRiskDataStore::OpenOperation>(&*operation)) {
        openOperation->m_result.SetResult();
        openReceiver.Get();
        break;
      }
    }
  }
}

TEST_SUITE("TestRiskDataStore") {
  TEST_CASE("open_close") {
    auto dataStore = TestRiskDataStore();
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestRiskDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
    auto openReceiver = Async<void>();
    Spawn([&] {
      try {
        dataStore.Open();
        openReceiver.GetEval().SetResult();
      } catch(const std::exception&) {
        openReceiver.GetEval().SetException(std::current_exception());
      }
    });
    auto operation = operations->Pop();
    auto openOperation = get<TestRiskDataStore::OpenOperation>(&*operation);
    REQUIRE(openOperation);
    openOperation->m_result.SetResult();
    REQUIRE_NOTHROW(openReceiver.Get());
    auto closeReceiver = Async<void>();
    Spawn([&] {
      try {
        dataStore.Close();
        closeReceiver.GetEval().SetResult();
      } catch(const std::exception&) {
        closeReceiver.GetEval().SetException(std::current_exception());
      }
    });
    operation = operations->Pop();
    auto closeOperation = get<TestRiskDataStore::CloseOperation>(&*operation);
    REQUIRE(closeOperation);
    closeOperation->m_result.SetResult();
    REQUIRE_NOTHROW(closeReceiver.Get());
  }

  TEST_CASE("store") {
    auto dataStore = TestRiskDataStore();
    Open(dataStore);
    auto operations = std::make_shared<
      Queue<std::shared_ptr<TestRiskDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
  }
}
