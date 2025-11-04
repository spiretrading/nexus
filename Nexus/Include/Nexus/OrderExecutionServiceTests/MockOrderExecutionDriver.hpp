#ifndef NEXUS_MOCK_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_MOCK_ORDER_EXECUTION_DRIVER_HPP
#include <unordered_map>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Queues/SequencePublisher.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionService/OrderUnrecoverableException.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/PrimitiveOrderUtilities.hpp"

namespace Nexus::Tests {

  /** An OrderExecutionDriver used for testing purposes. */
  class MockOrderExecutionDriver {
    public:

      /** Constructs a MockOrderExecutionDriver. */
      MockOrderExecutionDriver();

      /** Sets the state of any submitted Order to NEW upon submission. */
      void set_order_status_new_on_submission(bool value);

      /**
       * Finds an order with a specified client id.
       * @param id The Order's id.
       * @return The Order with the specified <i>id</i>.
       */
      const std::shared_ptr<PrimitiveOrder>& find(OrderId id);

      /**
       * Adds an ExecutionReport to be used to recover an Order.
       * @param report The ExecutionReport to add for recovery.
       */
      void add_recovery(const ExecutionReport& report);

      /** Returns the Publisher storing Order submissions. */
      const Beam::Publisher<std::shared_ptr<PrimitiveOrder>>&
        get_publisher() const;

      std::shared_ptr<Order> recover(const SequencedAccountOrderRecord& record);
      void add(const std::shared_ptr<Order>& order);
      std::shared_ptr<Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      bool m_set_order_status_new_on_submission;
      std::unordered_map<OrderId, std::shared_ptr<PrimitiveOrder>> m_orders;
      Beam::SequencePublisher<std::shared_ptr<PrimitiveOrder>> m_publisher;
      std::unordered_map<OrderId, std::vector<ExecutionReport>> m_recoveries;

      MockOrderExecutionDriver(const MockOrderExecutionDriver&) = delete;
      MockOrderExecutionDriver& operator =(
        const MockOrderExecutionDriver&) = delete;
  };

  inline MockOrderExecutionDriver::MockOrderExecutionDriver()
    : m_set_order_status_new_on_submission(false) {}

  inline void MockOrderExecutionDriver::set_order_status_new_on_submission(
      bool value) {
    m_set_order_status_new_on_submission = value;
  }

  inline const std::shared_ptr<PrimitiveOrder>&
      MockOrderExecutionDriver::find(OrderId id) {
    return m_orders.at(id);
  }

  inline void MockOrderExecutionDriver::add_recovery(
      const ExecutionReport& report) {
    m_recoveries[report.m_id].push_back(report);
  }

  inline const Beam::Publisher<std::shared_ptr<PrimitiveOrder>>&
      MockOrderExecutionDriver::get_publisher() const {
    return m_publisher;
  }

  inline std::shared_ptr<Order> MockOrderExecutionDriver::recover(
      const SequencedAccountOrderRecord& record) {
    {
      auto i = m_orders.find((*record)->m_info.m_id);
      if(i != m_orders.end()) {
        return i->second;
      }
    }
    auto reports = (*record)->m_execution_reports;
    auto i = m_recoveries.find((*record)->m_info.m_id);
    if(i != m_recoveries.end()) {
      reports.insert(reports.end(), i->second.begin(), i->second.end());
    }
    auto order = m_orders.insert(
      std::pair((*record)->m_info.m_id, std::make_shared<PrimitiveOrder>(
        OrderRecord((*record)->m_info, std::move(reports))))).first->second;
    return order;
  }

  inline void MockOrderExecutionDriver::add(
    const std::shared_ptr<Order>& order) {}

  inline std::shared_ptr<Order> MockOrderExecutionDriver::submit(
      const OrderInfo& info) {
    auto order = m_orders.insert(std::pair(
      info.m_id, std::make_unique<PrimitiveOrder>(info))).first->second;
    if(m_set_order_status_new_on_submission) {
      set_order_status(*order, OrderStatus::NEW, info.m_timestamp);
    }
    m_publisher.push(order);
    return order;
  }

  inline void MockOrderExecutionDriver::cancel(
      const OrderExecutionSession& session, OrderId id) {
    auto& order = m_orders.at(id);
    order->with([&] (auto status, const auto& reports) {
      auto i = std::find_if(reports.begin(), reports.end(),
        [&] (const auto& report) {
          return report.m_status == OrderStatus::PENDING_CANCEL ||
            is_terminal(report.m_status);
        });
      if(i != reports.end()) {
        return;
      }
      auto update = make_update(reports.back(), OrderStatus::PENDING_CANCEL,
        boost::posix_time::microsec_clock::universal_time());
      order->update(update);
    });
  }

  inline void MockOrderExecutionDriver::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    auto& order = m_orders.at(id);
    order->with([&] (auto status, const auto& reports) {
      if(is_terminal(status) ||
          reports.empty() && report.m_status != OrderStatus::PENDING_NEW) {
        return;
      }
      auto update = report;
      update.m_sequence = reports.back().m_sequence + 1;
      if(update.m_timestamp.is_special()) {
        update.m_timestamp =
          boost::posix_time::microsec_clock::universal_time();
      }
      order->update(update);
    });
  }

  inline void MockOrderExecutionDriver::close() {}
}

#endif
