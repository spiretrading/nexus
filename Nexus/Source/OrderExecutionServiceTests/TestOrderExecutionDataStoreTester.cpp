#include <Beam/Queues/Queue.hpp>
#include <doctest/doctest.h>
#include "Nexus/OrderExecutionService/LocalOrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionServiceTests/TestOrderExecutionDataStore.hpp"

using namespace Beam;
using namespace Beam::Queries;
using namespace Beam::Routines;
using namespace Beam::ServiceLocator;
using namespace boost;
using namespace boost::posix_time;
using namespace Nexus;
using namespace Nexus::OrderExecutionService;
using namespace Nexus::OrderExecutionService::Tests;

namespace {
  const auto ACCOUNT = DirectoryEntry::MakeAccount(4001, "tester");
  const auto SECURITY = Security("TST", DefaultMarkets::NASDAQ(),
    DefaultCountries::US());

  struct Fixture {
    TestOrderExecutionDataStore m_dataStore;
    std::shared_ptr<Queue<std::shared_ptr<
      TestOrderExecutionDataStore::Operation>>> m_operations;

    Fixture()
        : m_dataStore(OrderExecutionDataStoreBox(
            std::in_place_type<LocalOrderExecutionDataStore>),
            TestOrderExecutionDataStore::Mode::SUPERVISED),
          m_operations(std::make_shared<Queue<
            std::shared_ptr<TestOrderExecutionDataStore::Operation>>>()) {
      m_dataStore.GetPublisher().Monitor(m_operations);
    }
  };

  template<typename Operation, typename M, typename... Inputs,
    typename... Outputs>
  void TestReifiedMethod(TestOrderExecutionDataStore& dataStore, M method,
      QueueReader<std::shared_ptr<TestOrderExecutionDataStore::Operation>>&
        operations, std::tuple<Inputs...> inputs,
      std::tuple<Outputs...> outputs) {
    using ReturnEval = decltype(std::declval<Operation>().m_result);
    using Return = typename ReturnEval::Type;
    auto receiver = Async<Return>();
    Spawn([&] {
      return std::apply([&] (auto&&... args) -> decltype(auto) {
        return (dataStore.*method)(std::forward<decltype(args)>(args)...);
      }, std::move(inputs));
    }, receiver.GetEval());
    auto operation = std::dynamic_pointer_cast<Operation>(operations.Pop());
    REQUIRE(operation);
    std::apply([&] (auto&&... args) {
      operation->m_result.SetResult(std::forward<decltype(args)...>(args)...);
    }, std::move(outputs));
    REQUIRE_NOTHROW(receiver.Get());
  }
}

TEST_SUITE("TestOrderExecutionDataStore") {
  TEST_CASE_FIXTURE(Fixture, "open_close") {
    TestReifiedMethod<TestOrderExecutionDataStore::CloseOperation>(m_dataStore,
      &TestOrderExecutionDataStore::Close, *m_operations, std::tuple(),
      std::tuple());
  }

  TEST_CASE_FIXTURE(Fixture, "load_order") {
    auto record = SequencedValue(IndexedValue(OrderRecord(OrderInfo(
      OrderFields::MakeLimitOrder(SECURITY, Side::BID, 230, Money::ONE), 100,
        time_from_string("2020-04-27 22:01:16")), {}),
      ACCOUNT), Beam::Queries::Sequence(200));
    TestReifiedMethod<TestOrderExecutionDataStore::LoadOrderOperation>(
      m_dataStore, &TestOrderExecutionDataStore::LoadOrder, *m_operations,
      std::tuple(OrderId(100)), std::tuple(record));
  }

  TEST_CASE_FIXTURE(Fixture, "load_order_submissions") {
    auto query = AccountQuery();
    query.SetIndex(ACCOUNT);
    query.SetRange(Range::RealTime());
    auto submissions = std::vector<SequencedOrderRecord>();
    TestReifiedMethod<
      TestOrderExecutionDataStore::LoadOrderSubmissionsOperation>(m_dataStore,
        &TestOrderExecutionDataStore::LoadOrderSubmissions, *m_operations,
        std::tuple(query), std::tuple(submissions));
  }

  TEST_CASE_FIXTURE(Fixture, "load_execution_reports") {
    auto query = AccountQuery();
    query.SetIndex(ACCOUNT);
    query.SetRange(Range::RealTime());
    auto reports = std::vector<SequencedExecutionReport>();
    TestReifiedMethod<
      TestOrderExecutionDataStore::LoadExecutionReportsOperation>(m_dataStore,
        &TestOrderExecutionDataStore::LoadExecutionReports, *m_operations,
        std::tuple(query), std::tuple(reports));
  }

  TEST_CASE_FIXTURE(Fixture, "store_order_info") {
    auto info = SequencedValue(IndexedValue(OrderInfo(
      OrderFields::MakeLimitOrder(SECURITY, Side::ASK, 100, Money::CENT), 422,
      time_from_string("2020-04-11 17:12:11")), ACCOUNT),
      Beam::Queries::Sequence(9912));
    TestReifiedMethod<TestOrderExecutionDataStore::StoreOrderInfoOperation>(
      m_dataStore, static_cast<void (TestOrderExecutionDataStore::*)(
        const SequencedAccountOrderInfo&)>(&TestOrderExecutionDataStore::Store),
      *m_operations, std::tuple(info), std::tuple());
  }

  TEST_CASE_FIXTURE(Fixture, "store_execution_report") {
    auto report = SequencedValue(IndexedValue(
      ExecutionReport::MakeInitialReport(2212,
        time_from_string("2020-04-11 17:12:11")), ACCOUNT),
      Beam::Queries::Sequence(1299));
    TestReifiedMethod<
      TestOrderExecutionDataStore::StoreExecutionReportOperation>(m_dataStore,
        static_cast<void (TestOrderExecutionDataStore::*)(
          const SequencedAccountExecutionReport&)>(
            &TestOrderExecutionDataStore::Store), *m_operations,
      std::tuple(report), std::tuple());
  }
}
