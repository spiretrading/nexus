#ifndef NEXUS_TEST_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_TEST_ORDER_EXECUTION_DRIVER_HPP
#include <memory>
#include <variant>
#include <Beam/IO/NotConnectedException.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/Routines/Async.hpp>
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionSession.hpp"

namespace Nexus::OrderExecutionService::Tests {

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
        Beam::Routines::Eval<std::shared_ptr<const Order>> m_result;
      };

      /** Records a call to submit. */
      struct SubmitOperation {

        /** The OrderInfo passed. */
        OrderInfo m_info;

        /** The value to return. */
        Beam::Routines::Eval<std::shared_ptr<const Order>> m_result;
      };

      /** Records a call to cancel. */
      struct CancelOperation {

        /** The session that submitted the request. */
        const OrderExecutionSession* m_session;

        /** The id of the Order to cancel. */
        OrderId m_id;

        /** The value to return. */
        Beam::Routines::Eval<void> m_result;
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
        Beam::Routines::Eval<void> m_result;
      };

      /** A variant covering all possible operations. */
      using Operation = std::variant<
        RecoverOperation, SubmitOperation, CancelOperation, UpdateOperation>;

      /**
       * Constructs a TestOrderExecutionDriver.
       * @param operations The queue to push all operations on.
       */
      explicit TestOrderExecutionDriver(
        Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations);

      ~TestOrderExecutionDriver();

      std::shared_ptr<const Order> recover(
        const SequencedAccountOrderRecord& record);

      std::shared_ptr<const Order> submit(const OrderInfo& info);

      void cancel(const OrderExecutionSession& session, OrderId id);

      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);

      void close();

    private:
      Beam::ScopedQueueWriter<std::shared_ptr<Operation>> m_operations;
      Beam::IO::OpenState m_open_state;

      template<typename T, typename R, typename... Args>
      R append_result(Args&&... args);
  };

  inline TestOrderExecutionDriver::TestOrderExecutionDriver(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations)
    : m_operations(std::move(operations)) {}

  inline TestOrderExecutionDriver::~TestOrderExecutionDriver() {
    close();
  }

  inline std::shared_ptr<const Order> TestOrderExecutionDriver::recover(
      const SequencedAccountOrderRecord& order_record) {
    return append_result<RecoverOperation, std::shared_ptr<const Order>>(
      order_record);
  }

  inline std::shared_ptr<const Order> TestOrderExecutionDriver::submit(
      const OrderInfo& info) {
    return append_result<SubmitOperation, std::shared_ptr<const Order>>(info);
  }

  inline void TestOrderExecutionDriver::cancel(
      const OrderExecutionSession& session, OrderId id) {
    return append_result<CancelOperation, void>(&session, id);
  }

  inline void TestOrderExecutionDriver::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    return append_result<UpdateOperation, void>(&session, id, report);
  }

  inline void TestOrderExecutionDriver::close() {
    m_open_state.Close();
  }

  template<typename T, typename R, typename... Args>
  R TestOrderExecutionDriver::append_result(Args&&... args) {
    auto async = Beam::Routines::Async<R>();
    auto operation = std::make_shared<Operation>(
      std::in_place_type<T>, std::forward<Args>(args)..., async.GetEval());
    if(!m_open_state.IsOpen()) {
      BOOST_THROW_EXCEPTION(Beam::IO::NotConnectedException());
    }
    m_operations.Push(operation);
    if constexpr(std::is_same_v<R, void>) {
      async.Get();
      return;
    } else {
      return async.Get();
    }
  }
}

#endif
