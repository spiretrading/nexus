#ifndef NEXUS_TEST_ORDER_EXECUTION_CLIENT_HPP
#define NEXUS_TEST_ORDER_EXECUTION_CLIENT_HPP
#include <variant>
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/EndOfFileException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionClient.hpp"

namespace Nexus::Tests {

  /**
   * Implements an OrderExecutionClient for testing purposes by pushing all
   * operations performed on this client onto a queue.
   */
  class TestOrderExecutionClient {
    public:

      /** Records a call to submit(...). */
      struct SubmitOperation {

        /** The fields used to submit the order. */
        OrderFields m_fields;

        /** The result to return to the caller. */
        Beam::Services::Tests::ServiceResult<std::shared_ptr<Order>> m_result;
      };

      /** Records a call to cancel(...). */
      struct CancelOperation {

        /** The id of the Order to cancel. */
        OrderId m_id;
      };

      /** Records a call to update(...). */
      struct UpdateOperation {

        /** The id of the Order to update. */
        OrderId m_id;

        /** The ExecutionReport containing the update. */
        ExecutionReport m_report;
      };

      /** Records a call to load_order(...). */
      struct LoadOrderOperation {

        /** The id of the Order to load. */
        OrderId m_id;

        /** The result to return to the caller. */
        Beam::Services::Tests::ServiceResult<std::shared_ptr<Order>> m_result;
      };

      /** Records a call to query(..., SequencedOrderRecord). */
      struct QuerySequencedOrderRecordOperation {

        /** The AccountQuery passed. */
        AccountQuery m_query;

        /** The queue writer for SequencedOrderRecord. */
        Beam::ScopedQueueWriter<SequencedOrderRecord> m_queue;
      };

      /** Records a call to query(..., OrderRecord). */
      struct QueryOrderRecordOperation {

        /** The AccountQuery passed. */
        AccountQuery m_query;

        /** The queue writer for OrderRecord. */
        Beam::ScopedQueueWriter<OrderRecord> m_queue;
      };

      /** Records a call to query(..., SequencedOrder). */
      struct QuerySequencedOrderOperation {

        /** The AccountQuery passed. */
        AccountQuery m_query;

        /** The queue writer for SequencedOrder. */
        Beam::ScopedQueueWriter<SequencedOrder> m_queue;
      };

      /** Records a call to query(..., Order). */
      struct QueryOrderOperation {

        /** The AccountQuery passed. */
        AccountQuery m_query;

        /** The queue writer for Order. */
        Beam::ScopedQueueWriter<std::shared_ptr<Order>> m_queue;
      };

      /** Records a call to query(..., SequencedExecutionReport). */
      struct QuerySequencedExecutionReportOperation {

        /** The AccountQuery passed. */
        AccountQuery m_query;

        /** The queue writer for SequencedExecutionReport. */
        Beam::ScopedQueueWriter<SequencedExecutionReport> m_queue;
      };

      /** Records a call to query(..., ExecutionReport). */
      struct QueryExecutionReportOperation {

        /** The AccountQuery passed. */
        AccountQuery m_query;

        /** The queue writer for ExecutionReport. */
        Beam::ScopedQueueWriter<ExecutionReport> m_queue;
      };

      /** Records a call to close(). */
      struct CloseOperation {};

      /**
       * A variant covering all possible TestOrderExecutionClient operations.
       */
      using Operation = std::variant<SubmitOperation, CancelOperation,
        UpdateOperation, LoadOrderOperation, QuerySequencedOrderRecordOperation,
        QueryOrderRecordOperation, QuerySequencedOrderOperation,
        QueryOrderOperation, QuerySequencedExecutionReportOperation,
        QueryExecutionReportOperation, CloseOperation>;

      /** The type of Queue used to send and receive operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestOrderExecutionClient.
       * @param operations The queue to push all operations on.
       */
      explicit TestOrderExecutionClient(
        Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations)
          noexcept;

      ~TestOrderExecutionClient();

      std::shared_ptr<Order> submit(const OrderFields& fields);
      void cancel(const std::shared_ptr<Order>& order);
      void cancel(const Order& order);
      void update(OrderId id, const ExecutionReport& report);
      std::shared_ptr<Order> load_order(OrderId id);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrderRecord> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<OrderRecord> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedOrder> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<SequencedExecutionReport> queue);
      void query(const AccountQuery& query,
        Beam::ScopedQueueWriter<ExecutionReport> queue);
      void close();

    private:
      Beam::ScopedQueueWriter<std::shared_ptr<Operation>> m_operations;
      Beam::SynchronizedVector<std::weak_ptr<Beam::BaseQueue>> m_queues;
      Beam::SynchronizedUnorderedSet<Beam::Services::Tests::BaseServiceResult*>
        m_pending_results;
      Beam::IO::OpenState m_open_state;

      TestOrderExecutionClient(const TestOrderExecutionClient&) = delete;
      TestOrderExecutionClient& operator =(
        const TestOrderExecutionClient&) = delete;

      template<typename T>
      void append_queue(std::shared_ptr<Operation> operation);
      template<typename T, typename R, typename... Args>
      R append_result(Args&&... args);
  };

  inline TestOrderExecutionClient::TestOrderExecutionClient(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_operations(std::move(operations)) {}

  inline TestOrderExecutionClient::~TestOrderExecutionClient() {
    close();
  }

  inline std::shared_ptr<Order>
      TestOrderExecutionClient::submit(const OrderFields& fields) {
    return append_result<SubmitOperation, std::shared_ptr<Order>>(fields);
  }

  inline void TestOrderExecutionClient::cancel(
      const std::shared_ptr<Order>& order) {
    cancel(*order);
  }

  inline void TestOrderExecutionClient::cancel(const Order& order) {
    auto operation =
      std::make_shared<Operation>(CancelOperation(order.get_info().m_id));
    m_operations.Push(operation);
  }

  inline void TestOrderExecutionClient::update(
      OrderId id, const ExecutionReport& report) {
    auto operation = std::make_shared<Operation>(UpdateOperation(id, report));
    m_operations.Push(operation);
  }

  inline std::shared_ptr<Order>
      TestOrderExecutionClient::load_order(OrderId id) {
    return append_result<LoadOrderOperation, std::shared_ptr<Order>>(id);
  }

  inline void TestOrderExecutionClient::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrderRecord> queue) {
    append_queue<QuerySequencedOrderRecordOperation>(
      std::make_shared<Operation>(
        QuerySequencedOrderRecordOperation(query, std::move(queue))));
  }

  inline void TestOrderExecutionClient::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<OrderRecord> queue) {
    append_queue<QueryOrderRecordOperation>(std::make_shared<Operation>(
      QueryOrderRecordOperation(query, std::move(queue))));
  }

  inline void TestOrderExecutionClient::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedOrder> queue) {
    append_queue<QuerySequencedOrderOperation>(std::make_shared<Operation>(
      QuerySequencedOrderOperation(query, std::move(queue))));
  }

  inline void TestOrderExecutionClient::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<std::shared_ptr<Order>> queue) {
    append_queue<QueryOrderOperation>(std::make_shared<Operation>(
      QueryOrderOperation(query, std::move(queue))));
  }

  inline void TestOrderExecutionClient::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<SequencedExecutionReport> queue) {
    append_queue<QuerySequencedExecutionReportOperation>(
      std::make_shared<Operation>(
        QuerySequencedExecutionReportOperation(query, std::move(queue))));
  }

  inline void TestOrderExecutionClient::query(const AccountQuery& query,
      Beam::ScopedQueueWriter<ExecutionReport> queue) {
    append_queue<QueryExecutionReportOperation>(std::make_shared<Operation>(
      QueryExecutionReportOperation(query, std::move(queue))));
  }

  inline void TestOrderExecutionClient::close() {
    if(m_open_state.SetClosing()) {
      m_queues.ForEach([] (const auto& q) {
        if(auto queue = q.lock()) {
          queue->Break();
        }
      });
      m_queues.Clear();
      m_pending_results.With([] (auto& results) {
        for(auto& result : results) {
          result->set(std::make_exception_ptr(Beam::IO::EndOfFileException()));
        }
      });
      m_pending_results.Clear();
    }
    m_open_state.Close();
    auto operation = std::make_shared<Operation>(CloseOperation());
    m_operations.Push(operation);
  }

  template<typename T>
  void TestOrderExecutionClient::append_queue(
      std::shared_ptr<Operation> operation) {
    auto queue = std::shared_ptr<Beam::BaseQueue>(
      operation, &std::get<T>(*operation).m_queue);
    m_queues.PushBack(queue);
    if(!m_open_state.IsOpen()) {
      m_queues.RemoveIf([&] (const auto& weak_queue) {
        auto q = weak_queue.lock();
        return !q || q == queue;
      });
      queue->Break();
      return;
    }
    m_operations.Push(operation);
  }

  template<typename T, typename R, typename... Args>
  R TestOrderExecutionClient::append_result(Args&&... args) {
    auto async = Beam::Routines::Async<R>();
    auto operation = std::make_shared<Operation>(
      std::in_place_type<T>, std::forward<Args>(args)..., async.GetEval());
    m_pending_results.Insert(&std::get<T>(*operation).m_result);
    if(!m_open_state.IsOpen()) {
      m_pending_results.Erase(&std::get<T>(*operation).m_result);
      BOOST_THROW_EXCEPTION(Beam::IO::EndOfFileException());
    }
    m_operations.Push(operation);
    auto result = std::move(async.Get());
    m_pending_results.Erase(&std::get<T>(*operation).m_result);
    return result;
  }
}

#endif
