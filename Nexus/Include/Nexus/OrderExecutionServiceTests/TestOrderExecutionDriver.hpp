#ifndef NEXUS_TEST_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_TEST_ORDER_EXECUTION_DRIVER_HPP
#include <memory>
#include <variant>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/Routines/Async.hpp>
#include <Beam/ServicesTests/TestServiceClientOperationQueue.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"

namespace Nexus::Tests {

  /**
   * Implements an OrderExecutionDriver for testing purposes.
   */
  class TestOrderExecutionDriver {
    public:

      /** Records a call to recover. */
      struct RecoverOperation {

        /** The SequencedAccountOrderRecord passed. */
        SequencedAccountOrderRecord m_order_record;

        /** The value to return. */
        Beam::Tests::ServiceResult<std::shared_ptr<Order>> m_result;
      };

      /** Records a call to submit. */
      struct SubmitOperation {

        /** The OrderInfo passed. */
        OrderInfo m_info;

        /** The value to return. */
        Beam::Tests::ServiceResult<std::shared_ptr<Order>> m_result;
      };

      /** Records a call to cancel. */
      struct CancelOperation {

        /** The session that submitted the request. */
        const OrderExecutionSession* m_session;

        /** The id of the Order to cancel. */
        OrderId m_id;

        /** The value to return. */
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to update. */
      struct UpdateOperation {

        /** The session that submitted the request. */
        const OrderExecutionSession* m_session;

        /** The id of the Order to update. */
        OrderId m_id;

        /** The ExecutionReport. */
        ExecutionReport m_report;

        /** The value to return. */
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** A variant covering all possible operations. */
      using Operation = std::variant<
        RecoverOperation, SubmitOperation, CancelOperation, UpdateOperation>;

      /** The type of Queue used to send and receive operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestOrderExecutionDriver.
       * @param operations The queue to push all operations on.
       */
      explicit TestOrderExecutionDriver(
        Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations);

      ~TestOrderExecutionDriver();

      std::shared_ptr<Order> recover(const SequencedAccountOrderRecord& record);
      void add(const std::shared_ptr<Order>& order);
      std::shared_ptr<Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      Beam::Tests::TestServiceClientOperationQueue<Operation> m_operations;
  };

  inline TestOrderExecutionDriver::TestOrderExecutionDriver(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations)
    : m_operations(std::move(operations)) {}

  inline TestOrderExecutionDriver::~TestOrderExecutionDriver() {
    close();
  }

  inline std::shared_ptr<Order> TestOrderExecutionDriver::recover(
      const SequencedAccountOrderRecord& order_record) {
    return m_operations.append_result<RecoverOperation, std::shared_ptr<Order>>(
      order_record);
  }

  inline void TestOrderExecutionDriver::add(
      const std::shared_ptr<Order>& order) {}

  inline std::shared_ptr<Order> TestOrderExecutionDriver::submit(
      const OrderInfo& info) {
    return m_operations.append_result<SubmitOperation, std::shared_ptr<Order>>(
      info);
  }

  inline void TestOrderExecutionDriver::cancel(
      const OrderExecutionSession& session, OrderId id) {
    return m_operations.append_result<CancelOperation, void>(&session, id);
  }

  inline void TestOrderExecutionDriver::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    return m_operations.append_result<UpdateOperation, void>(
      &session, id, report);
  }

  inline void TestOrderExecutionDriver::close() {
    m_operations.close();
  }
}

#endif
