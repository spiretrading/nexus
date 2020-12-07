#ifndef NEXUS_TEST_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_TEST_ORDER_EXECUTION_DATA_STORE_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Routines/Async.hpp>
#include <boost/variant.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStore.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

namespace Nexus::OrderExecutionService::Tests {

  /** Implements an OrderExecutionDataStore for testing purposes. */
  class TestOrderExecutionDataStore {
    public:

      /** Stores a call to the Close method. */
      struct CloseOperation {

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

      /** Stores a call to the LoadOrder method. */
      struct LoadOrderOperation {

        /** Stores the id argument. */
        const OrderId* m_id;

        /** The OrderRecord to return to the caller. */
        Beam::Routines::Eval<boost::optional<SequencedOrderRecord>> m_result;
      };

      /** Stores a call to the LoadOrderSubmissions method. */
      struct LoadOrderSubmissionsOperation {

        /** Stores the query argument. */
        const AccountQuery* m_query;

        /** The result to return to the caller. */
        Beam::Routines::Eval<std::vector<SequencedOrderRecord>> m_result;
      };

      /** Stores a call to the LoadExecutionReports method. */
      struct LoadExecutionReportsOperation {

        /** Stores the query argument. */
        const AccountQuery* m_query;

        /** The result to return to the caller. */
        Beam::Routines::Eval<std::vector<SequencedExecutionReport>> m_result;
      };

      /** Stores a call to Store(const SequencedAccountOrderInfo&). */
      struct StoreOrderInfoOperation {

        /** Stores the orderInfo argument. */
        const SequencedAccountOrderInfo* m_orderInfo;

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

      /** Stores a call to Store(const SequencedAccountExecutionReport&). */
      struct StoreExecutionReportOperation {

        /** Stores the orderInfo argument. */
        const SequencedAccountExecutionReport* m_executionReport;

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

      /** A variant over all method calls. */
      using Operation = boost::variant<CloseOperation, LoadOrderOperation,
        LoadOrderSubmissionsOperation, LoadExecutionReportsOperation,
        StoreOrderInfoOperation, StoreExecutionReportOperation>;

      /** Constructs a TestOrderExecutionDataStore. */
      TestOrderExecutionDataStore() = default;

      ~TestOrderExecutionDataStore();

      /** Returns an object publishing pending operations. */
      const Beam::Publisher<std::shared_ptr<Operation>>& GetPublisher() const;

      boost::optional<SequencedOrderRecord> LoadOrder(OrderId id);

      std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query);

      std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query);

      void Store(const SequencedAccountOrderInfo& orderInfo);

      void Store(const SequencedAccountExecutionReport& executionReport);

      void Close();

    private:
      Beam::QueueWriterPublisher<std::shared_ptr<Operation>> m_publisher;
      Beam::IO::OpenState m_openState;

      TestOrderExecutionDataStore(const TestOrderExecutionDataStore&) = delete;
      TestOrderExecutionDataStore& operator =(
        const TestOrderExecutionDataStore&) = delete;
  };

  inline TestOrderExecutionDataStore::~TestOrderExecutionDataStore() {
    m_openState.Close();
  }

  inline const Beam::Publisher<
      std::shared_ptr<TestOrderExecutionDataStore::Operation>>&
        TestOrderExecutionDataStore::GetPublisher() const {
    return m_publisher;
  }

  inline boost::optional<SequencedOrderRecord>
      TestOrderExecutionDataStore::LoadOrder(OrderId id) {
    m_openState.EnsureOpen();
    auto result =
      Beam::Routines::Async<boost::optional<SequencedOrderRecord>>();
    auto operation = std::make_shared<Operation>(
      LoadOrderOperation{&id, result.GetEval()});
    m_publisher.Push(operation);
    return result.Get();
  }

  inline std::vector<SequencedOrderRecord>
      TestOrderExecutionDataStore::LoadOrderSubmissions(
        const AccountQuery& query) {
    m_openState.EnsureOpen();
    auto result = Beam::Routines::Async<std::vector<SequencedOrderRecord>>();
    auto operation = std::make_shared<Operation>(
      LoadOrderSubmissionsOperation{&query, result.GetEval()});
    m_publisher.Push(operation);
    return result.Get();
  }

  inline std::vector<SequencedExecutionReport>
      TestOrderExecutionDataStore::LoadExecutionReports(
        const AccountQuery& query) {
    m_openState.EnsureOpen();
    auto result =
      Beam::Routines::Async<std::vector<SequencedExecutionReport>>();
    auto operation = std::make_shared<Operation>(
      LoadExecutionReportsOperation{&query, result.GetEval()});
    m_publisher.Push(operation);
    return result.Get();
  }

  inline void TestOrderExecutionDataStore::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    m_openState.EnsureOpen();
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<Operation>(
      StoreOrderInfoOperation{&orderInfo, result.GetEval()});
    m_publisher.Push(operation);
    result.Get();
  }

  inline void TestOrderExecutionDataStore::Store(
      const SequencedAccountExecutionReport& executionReport) {
    m_openState.EnsureOpen();
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<Operation>(
      StoreExecutionReportOperation{&executionReport, result.GetEval()});
    m_publisher.Push(operation);
    result.Get();
  }

  inline void TestOrderExecutionDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<Operation>(
      CloseOperation{result.GetEval()});
    m_publisher.Push(operation);
    result.Get();
  }
}

#endif
