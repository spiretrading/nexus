#ifndef NEXUS_PRIMITIVE_ORDER_HPP
#define NEXUS_PRIMITIVE_ORDER_HPP
#include <Beam/Queues/SequencePublisher.hpp>
#include "Nexus/OrderExecutionService/Order.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderRecord.hpp"

namespace Nexus::OrderExecutionService {

  /** Implements the Order interface using basic setters. */
  class PrimitiveOrder : public Order {
    public:

      /**
       * Constructs a PrimitiveOrder.
       * @param info The OrderInfo containing the submission details.
       */
      PrimitiveOrder(OrderInfo info);

      /**
       * Constructs a PrimitiveOrder.
       * @param orderRecord The OrderRecord containing the submission details.
       */
      PrimitiveOrder(OrderRecord orderRecord);

      /**
       * Updates this Order.
       * @param report The ExecutionReport storing the update.
       */
      void Update(const ExecutionReport& report);

      /** Provides synchronized access to this Order. */
      template<typename F>
      decltype(auto) With(F&& f);

      /** Provides synchronized access to this Order. */
      template<typename F>
      decltype(auto) With(F&& f) const;

      const OrderInfo& GetInfo() const override;

      const Beam::SnapshotPublisher<ExecutionReport,
        std::vector<ExecutionReport>>& GetPublisher() const override;

    private:
      OrderInfo m_info;
      OrderStatus m_status;
      Beam::SequencePublisher<ExecutionReport> m_publisher;
  };

  inline PrimitiveOrder::PrimitiveOrder(OrderInfo info)
      : m_info(std::move(info)) {
    auto report = ExecutionReport::MakeInitialReport(m_info.m_orderId,
      m_info.m_timestamp);
    Update(report);
  }

  inline PrimitiveOrder::PrimitiveOrder(OrderRecord orderRecord)
      : m_info(std::move(orderRecord.m_info)) {
    for(auto& executionReport : orderRecord.m_executionReports) {
      Update(executionReport);
    }
  }

  inline void PrimitiveOrder::Update(const ExecutionReport& report) {
    m_publisher.With([&] {
      if(report.m_status != OrderStatus::PARTIALLY_FILLED) {
        m_status = report.m_status;
      }
      m_publisher.Push(report);
      if(IsTerminal(report.m_status)) {
        m_publisher.Break();
      }
    });
  }

  inline const OrderInfo& PrimitiveOrder::GetInfo() const {
    return m_info;
  }

  template<typename F>
  decltype(auto) PrimitiveOrder::With(F&& f) {
    return m_publisher.With([&] (auto executionReports) {
      return std::forward<F>(f)(m_status, *executionReports);
    });
  }

  template<typename F>
  decltype(auto) PrimitiveOrder::With(F&& f) const {
    return m_publisher.With([&] (auto executionReports) {
      return std::forward<F>(f)(m_status, *executionReports);
    });
  }

  inline const Beam::SnapshotPublisher<ExecutionReport,
      std::vector<ExecutionReport>>& PrimitiveOrder::GetPublisher() const {
    return m_publisher;
  }

  /**
   * Returns a PrimitiveOrder with a rejected order status.
   * @param info The Order's submission info.
   * @param reason The reason for the rejection.
   */
  inline std::unique_ptr<PrimitiveOrder> MakeRejectedOrder(OrderInfo info,
      const std::string& reason) {
    auto order = std::make_unique<PrimitiveOrder>(std::move(info));
    order->With([&] (auto status, const auto& reports) {
      auto& lastReport = reports.back();
      auto updatedReport = ExecutionReport::MakeUpdatedReport(lastReport,
        OrderStatus::REJECTED, order->GetInfo().m_timestamp);
      updatedReport.m_text = reason;
      order->Update(updatedReport);
    });
    return order;
  }

  /**
   * Helper function to reject a cancel request on a PrimitiveOrder.
   * @param order The Order to update.
   * @param timestamp The time of the update.
   * @param reason The reason for the rejection.
   */
  inline void RejectCancelRequest(PrimitiveOrder& order,
      boost::posix_time::ptime timestamp, std::string reason) {
    order.With([&] (auto status, const auto& reports) {
      if(IsTerminal(status)) {
        return;
      }
      auto& lastReport = reports.back();
      auto updatedReport = ExecutionReport::MakeUpdatedReport(lastReport,
        OrderStatus::CANCEL_REJECT, timestamp);
      updatedReport.m_text = std::move(reason);
      order.Update(updatedReport);
    });
  }
}

#endif
