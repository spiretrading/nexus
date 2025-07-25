#ifndef NEXUS_PRIMITIVE_ORDER_HPP
#define NEXUS_PRIMITIVE_ORDER_HPP
#include <memory>
#include <Beam/Queues/SequencePublisher.hpp>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"

namespace Nexus::OrderExecutionService {

  /** Implements the Order interface using basic setters. */
  class PrimitiveOrder : public Order {
    public:

      /**
       * Constructs a PrimitiveOrder.
       * @param info The OrderInfo containing the submission details.
       */
      explicit PrimitiveOrder(OrderInfo info);

      /**
       * Constructs a PrimitiveOrder.
       * @param record The OrderRecord containing the submission details.
       */
      explicit PrimitiveOrder(OrderRecord record);

      /**
       * Updates this Order.
       * @param report The ExecutionReport storing the update.
       */
      void update(const ExecutionReport& report);

      /** Provides synchronized access to this Order. */
      template<typename F>
      decltype(auto) with(F&& f);

      /** Provides synchronized access to this Order. */
      template<typename F>
      decltype(auto) with(F&& f) const;

      const OrderInfo& get_info() const override;

      const Beam::SnapshotPublisher<ExecutionReport,
        std::vector<ExecutionReport>>& get_publisher() const override;

    private:
      OrderInfo m_info;
      OrderStatus m_status;
      Beam::SequencePublisher<ExecutionReport> m_publisher;
  };

  inline PrimitiveOrder::PrimitiveOrder(OrderInfo info)
      : m_info(std::move(info)) {
    auto report = ExecutionReport(m_info.m_id, m_info.m_timestamp);
    update(report);
  }

  inline PrimitiveOrder::PrimitiveOrder(OrderRecord record)
      : m_info(std::move(record.m_info)) {
    for(auto& report : record.m_execution_reports) {
      update(report);
    }
  }

  inline void PrimitiveOrder::update(const ExecutionReport& report) {
    m_publisher.With([&] {
      if(report.m_status != OrderStatus::PARTIALLY_FILLED) {
        m_status = report.m_status;
      }
      m_publisher.Push(report);
      if(is_terminal(report.m_status)) {
        m_publisher.Break();
      }
    });
  }

  inline const OrderInfo& PrimitiveOrder::get_info() const {
    return m_info;
  }

  template<typename F>
  decltype(auto) PrimitiveOrder::with(F&& f) {
    return m_publisher.With([&] (auto reports) {
      return std::forward<F>(f)(m_status, *reports);
    });
  }

  template<typename F>
  decltype(auto) PrimitiveOrder::with(F&& f) const {
    return m_publisher.With([&] (auto reports) {
      return std::forward<F>(f)(m_status, *reports);
    });
  }

  inline const Beam::SnapshotPublisher<ExecutionReport,
      std::vector<ExecutionReport>>& PrimitiveOrder::get_publisher() const {
    return m_publisher;
  }

  /**
   * Returns a PrimitiveOrder with a rejected order status.
   * @param info The Order's submission info.
   * @param reason The reason for the rejection.
   */
  inline std::shared_ptr<PrimitiveOrder>
      make_rejected_order(OrderInfo info, const std::string& reason) {
    auto order = std::make_shared<PrimitiveOrder>(std::move(info));
    order->with([&] (auto status, const auto& reports) {
      auto& last_report = reports.back();
      auto updated_report = make_updated_execution_report(
        last_report, OrderStatus::REJECTED, order->get_info().m_timestamp);
      updated_report.m_text = reason;
      order->update(updated_report);
    });
    return order;
  }

  /**
   * Helper function to reject a cancel request on a PrimitiveOrder.
   * @param order The Order to update.
   * @param timestamp The time of the update.
   * @param reason The reason for the rejection.
   */
  inline void reject_cancel_request(PrimitiveOrder& order,
      boost::posix_time::ptime timestamp, const std::string& reason) {
    order.with([&] (auto status, const auto& reports) {
      if(is_terminal(status)) {
        return;
      }
      auto& last_report = reports.back();
      auto updated_report = make_updated_execution_report(
        last_report, OrderStatus::CANCEL_REJECT, timestamp);
      updated_report.m_text = reason;
      order.update(updated_report);
    });
  }
}

#endif
