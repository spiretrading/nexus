#ifndef NEXUS_TEST_ORDER_EXECUTION_DATA_STORE_HPP
#define NEXUS_TEST_ORDER_EXECUTION_DATA_STORE_HPP
#include <atomic>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/QueueWriterPublisher.hpp>
#include <Beam/Routines/Async.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDataStoreBox.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

namespace Nexus::OrderExecutionService::Tests {

  /** Implements an OrderExecutionDataStore for testing purposes. */
  class TestOrderExecutionDataStore {
    public:

      /** Specifies how the data store handles operations. */
      enum class Mode {

        /** Operations delegate to the source data store. */
        UNSUPERVISED,

        /** Operations await on an Async and must be explicitly handled. */
        SUPERVISED
      };

      /** Base class of an operation performed on a data store. */
      struct Operation {
        virtual ~Operation() = default;

        /** The operation's sequence number. */
        int m_sequence;
      };

      /** Stores a call to the Close method. */
      struct CloseOperation : Operation {

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

      /** Stores a call to the LoadOrder method. */
      struct LoadOrderOperation : Operation {

        /** Stores the id argument. */
        const OrderId* m_id;

        /** The OrderRecord to return to the caller. */
        Beam::Routines::Eval<boost::optional<SequencedAccountOrderRecord>>
          m_result;
      };

      /** Stores a call to the LoadOrderSubmissions method. */
      struct LoadOrderSubmissionsOperation : Operation {

        /** Stores the query argument. */
        const AccountQuery* m_query;

        /** The result to return to the caller. */
        Beam::Routines::Eval<std::vector<SequencedOrderRecord>> m_result;
      };

      /** Stores a call to the LoadExecutionReports method. */
      struct LoadExecutionReportsOperation : Operation {

        /** Stores the query argument. */
        const AccountQuery* m_query;

        /** The result to return to the caller. */
        Beam::Routines::Eval<std::vector<SequencedExecutionReport>> m_result;
      };

      /** Stores a call to Store(const SequencedAccountOrderInfo&). */
      struct StoreOrderInfoOperation : Operation {

        /** Stores the orderInfo argument. */
        const SequencedAccountOrderInfo* m_orderInfo;

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

      /** Stores a call to Store(const SequencedAccountExecutionReport&). */
      struct StoreExecutionReportOperation : Operation {

        /** Stores the orderInfo argument. */
        const SequencedAccountExecutionReport* m_executionReport;

        /** The result to return to the caller. */
        Beam::Routines::Eval<void> m_result;
      };

      /**
       * Constructs a TestOrderExecutionDataStore operating in UNSUPERVISED
       * mode.
       * @param source The data store to delegate to when operating in
       *        unsupervised mode.
       */
      TestOrderExecutionDataStore(OrderExecutionDataStoreBox source);

      /**
       * Constructs a TestOrderExecutionDataStore.
       * @param source The data store to delegate to when operating in
       *        unsupervised mode.
       * @param mode The mode to operate in.
       */
      TestOrderExecutionDataStore(OrderExecutionDataStoreBox source, Mode mode);

      ~TestOrderExecutionDataStore();

      /**
       * Returns the last sequence number used
       * (0 if no operations has been performed).
       */
      int GetSequence() const;

      /** Returns the current Mode. */
      Mode GetMode() const;

      /** Sets the current Mode. */
      void SetMode(Mode mode);

      /** Returns an object publishing pending operations. */
      const Beam::Publisher<std::shared_ptr<Operation>>& GetPublisher() const;

      boost::optional<SequencedAccountOrderRecord> LoadOrder(OrderId id);

      std::vector<SequencedOrderRecord> LoadOrderSubmissions(
        const AccountQuery& query);

      std::vector<SequencedExecutionReport> LoadExecutionReports(
        const AccountQuery& query);

      void Store(const SequencedAccountOrderInfo& orderInfo);

      void Store(const SequencedAccountExecutionReport& executionReport);

      void Close();

    private:
      std::atomic_int m_sequence;
      OrderExecutionDataStoreBox m_source;
      std::atomic<Mode> m_mode;
      Beam::QueueWriterPublisher<std::shared_ptr<Operation>> m_publisher;
      Beam::IO::OpenState m_openState;

      TestOrderExecutionDataStore(const TestOrderExecutionDataStore&) = delete;
      TestOrderExecutionDataStore& operator =(
        const TestOrderExecutionDataStore&) = delete;
  };

  /** Automatically closes a TestOrderExecutionDataStore. */
  inline void Close(TestOrderExecutionDataStore& dataStore) {
    auto operations = std::make_shared<
      Beam::Queue<std::shared_ptr<TestOrderExecutionDataStore::Operation>>>();
    dataStore.GetPublisher().Monitor(operations);
    while(true) {
      auto operation = operations->Pop();
      if(auto closeOperation =
          std::static_pointer_cast<TestOrderExecutionDataStore::CloseOperation>(
            operation)) {
        closeOperation->m_result.SetResult();
        break;
      }
    }
  }

  inline TestOrderExecutionDataStore::TestOrderExecutionDataStore(
    OrderExecutionDataStoreBox source)
    : TestOrderExecutionDataStore(std::move(source), Mode::UNSUPERVISED) {}

  inline TestOrderExecutionDataStore::TestOrderExecutionDataStore(
    OrderExecutionDataStoreBox source, Mode mode)
    : m_sequence(0),
      m_source(std::move(source)),
      m_mode(mode) {}

  inline TestOrderExecutionDataStore::~TestOrderExecutionDataStore() {
    SetMode(Mode::UNSUPERVISED);
    Close();
  }

  inline int TestOrderExecutionDataStore::GetSequence() const {
    return m_sequence.load();
  }

  inline TestOrderExecutionDataStore::Mode
      TestOrderExecutionDataStore::GetMode() const {
    return m_mode;
  }

  inline void TestOrderExecutionDataStore::SetMode(Mode mode) {
    m_mode = mode;
  }

  inline const Beam::Publisher<
      std::shared_ptr<TestOrderExecutionDataStore::Operation>>&
        TestOrderExecutionDataStore::GetPublisher() const {
    return m_publisher;
  }

  inline boost::optional<SequencedAccountOrderRecord>
      TestOrderExecutionDataStore::LoadOrder(OrderId id) {
    m_openState.EnsureOpen();
    auto result =
      Beam::Routines::Async<boost::optional<SequencedAccountOrderRecord>>();
    auto operation = std::make_shared<LoadOrderOperation>();
    operation->m_sequence = ++m_sequence;
    operation->m_id = &id;
    operation->m_result = result.GetEval();
    m_publisher.Push(operation);
    if(m_mode == Mode::UNSUPERVISED) {
      operation->m_result.SetResult(m_source.LoadOrder(id));
    }
    return result.Get();
  }

  inline std::vector<SequencedOrderRecord>
      TestOrderExecutionDataStore::LoadOrderSubmissions(
        const AccountQuery& query) {
    m_openState.EnsureOpen();
    auto result = Beam::Routines::Async<std::vector<SequencedOrderRecord>>();
    auto operation = std::make_shared<LoadOrderSubmissionsOperation>();
    operation->m_sequence = ++m_sequence;
    operation->m_query = &query;
    operation->m_result = result.GetEval();
    m_publisher.Push(operation);
    if(m_mode == Mode::UNSUPERVISED) {
      operation->m_result.SetResult(m_source.LoadOrderSubmissions(query));
    }
    return result.Get();
  }

  inline std::vector<SequencedExecutionReport>
      TestOrderExecutionDataStore::LoadExecutionReports(
        const AccountQuery& query) {
    m_openState.EnsureOpen();
    auto result =
      Beam::Routines::Async<std::vector<SequencedExecutionReport>>();
    auto operation = std::make_shared<LoadExecutionReportsOperation>();
    operation->m_sequence = ++m_sequence;
    operation->m_query = &query;
    operation->m_result = result.GetEval();
    m_publisher.Push(operation);
    if(m_mode == Mode::UNSUPERVISED) {
      operation->m_result.SetResult(m_source.LoadExecutionReports(query));
    }
    return result.Get();
  }

  inline void TestOrderExecutionDataStore::Store(
      const SequencedAccountOrderInfo& orderInfo) {
    m_openState.EnsureOpen();
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<StoreOrderInfoOperation>();
    operation->m_sequence = ++m_sequence;
    operation->m_orderInfo = &orderInfo;
    operation->m_result = result.GetEval();
    m_publisher.Push(operation);
    if(m_mode == Mode::UNSUPERVISED) {
      try {
        m_source.Store(orderInfo);
        operation->m_result.SetResult();
      } catch(const std::exception&) {
        operation->m_result.SetException(std::current_exception());
      }
    }
    result.Get();
  }

  inline void TestOrderExecutionDataStore::Store(
      const SequencedAccountExecutionReport& executionReport) {
    m_openState.EnsureOpen();
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<StoreExecutionReportOperation>();
    operation->m_sequence = ++m_sequence;
    operation->m_executionReport = &executionReport;
    operation->m_result = result.GetEval();
    m_publisher.Push(operation);
    if(m_mode == Mode::UNSUPERVISED) {
      try {
        m_source.Store(executionReport);
        operation->m_result.SetResult();
      } catch(const std::exception&) {
        operation->m_result.SetException(std::current_exception());
      }
    }
    result.Get();
  }

  inline void TestOrderExecutionDataStore::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    auto result = Beam::Routines::Async<void>();
    auto operation = std::make_shared<CloseOperation>();
    operation->m_sequence = ++m_sequence;
    operation->m_result = result.GetEval();
    m_publisher.Push(operation);
    if(m_mode == Mode::UNSUPERVISED) {
      try {
        m_source.Close();
        operation->m_result.SetResult();
      } catch(const std::exception&) {
        operation->m_result.SetException(std::current_exception());
      }
    }
    result.Get();
    m_openState.Close();
  }
}

#endif
