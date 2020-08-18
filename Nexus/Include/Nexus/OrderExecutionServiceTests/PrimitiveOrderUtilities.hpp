#ifndef NEXUS_PRIMITIVE_ORDER_UTILITIES_HPP
#define NEXUS_PRIMITIVE_ORDER_UTILITIES_HPP
#include <Beam/Queues/Queue.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

namespace Nexus::OrderExecutionService::Tests {

  /**
   * Sets the OrderStatus of an Order to CANCELED.
   * @param order The Order to cancel.
   * @param timestamp The cancel timestamp.
   */
  inline void CancelOrder(PrimitiveOrder& order,
      boost::posix_time::ptime timestamp) {
    auto monitor = std::make_shared<Beam::Queue<ExecutionReport>>();
    order.GetPublisher().Monitor(monitor);
    auto lastReport = ExecutionReport();
    while(true) {
      lastReport = monitor->Pop();
      if(lastReport.m_status == OrderStatus::PENDING_CANCEL) {
        break;
      }
    }
    while(auto report = monitor->TryPop()) {
      lastReport = std::move(*report);
    }
    auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
      OrderStatus::CANCELED, timestamp);
    order.Update(updatedReport);
  }

  /**
   * Sets the OrderStatus of an Order being tested.
   * @param order The Order to set the OrderStatus for.
   * @param newStatus The OrderStatus to assign to the <i>order</i>.
   * @param timestamp The modification's timestamp.
   */
  inline void SetOrderStatus(PrimitiveOrder& order, OrderStatus newStatus,
      boost::posix_time::ptime timestamp) {
    order.With(
      [&] (auto status, auto& reports) {
        auto& lastReport = reports.back();
        auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
          newStatus, timestamp);
        order.Update(updatedReport);
      });
  }

  /**
   * Sets the OrderStatus of an Order being tested.
   * @param order The Order to set the OrderStatus for.
   * @param newStatus The OrderStatus to assign to the <i>order</i>.
   */
  inline void SetOrderStatus(PrimitiveOrder& order, OrderStatus newStatus) {
    order.With(
      [&] (auto status, auto& reports) {
        auto& lastReport = reports.back();
        auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
          newStatus, lastReport.m_timestamp);
        order.Update(updatedReport);
      });
  }

  /**
   * Sets the OrderStatus of an Order to new.
   * @param order The Order to set the OrderStatus for.
   * @param timestamp The modification's timestamp.
   */
  inline void Accept(PrimitiveOrder& order,
      boost::posix_time::ptime timestamp) {
    SetOrderStatus(order, OrderStatus::NEW, timestamp);
  }

  /**
   * Sets the OrderStatus of an Order to new.
   * @param order The Order to set the OrderStatus for.
   */
  inline void Accept(PrimitiveOrder& order) {
    SetOrderStatus(order, OrderStatus::NEW);
  }

  /**
   * Fills an Order.
   * @param order The Order to fill.
   * @param price The price of the fill.
   * @param quantity The amount to fill the order for.
   * @param timestamp The modification's timestamp.
   */
  inline void FillOrder(PrimitiveOrder& order, Money price, Quantity quantity,
      boost::posix_time::ptime timestamp) {
    order.With(
      [&] (auto status, auto& reports) {
        auto cumulativeQuantity = Quantity();
        for(auto& report : reports) {
          cumulativeQuantity += report.m_lastQuantity;
        }
        auto& lastReport = reports.back();
        assert(cumulativeQuantity + quantity <=
          order.GetInfo().m_fields.m_quantity);
        auto newStatus =
          [&] {
            if(cumulativeQuantity + quantity ==
                order.GetInfo().m_fields.m_quantity) {
              return OrderStatus::FILLED;
            } else {
              return OrderStatus::PARTIALLY_FILLED;
            }
          }();
        auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
          newStatus, timestamp);
        updatedReport.m_lastPrice = price;
        updatedReport.m_lastQuantity = quantity;
        order.Update(updatedReport);
    });
  }

  /**
   * Fills an Order.
   * @param order The Order to fill.
   * @param quantity The amount to fill the order for.
   * @param timestamp The modification's timestamp.
   */
  inline void FillOrder(PrimitiveOrder& order, Quantity quantity,
      boost::posix_time::ptime timestamp) {
    FillOrder(order, order.GetInfo().m_fields.m_price, quantity, timestamp);
  }

  /**
   * Returns <code>true</code> iff an Order is in the PENDING_CANCEL state.
   * @param order The Order to test.
   * @return <code>true</code> iff the <i>order</i> is PENDING_CANCEL.
   */
  inline bool IsPendingCancel(const PrimitiveOrder& order) {
    auto isPendingCancel = false;
    const_cast<PrimitiveOrder&>(order).With(
      [&] (auto status, auto& executionReports) {
        for(auto& executionReport : executionReports) {
          if(executionReport.m_status == OrderStatus::PENDING_CANCEL) {
            isPendingCancel = true;
            break;
          }
        }
        isPendingCancel &= !IsTerminal(status);
      });
    return isPendingCancel;
  }
}

#endif
