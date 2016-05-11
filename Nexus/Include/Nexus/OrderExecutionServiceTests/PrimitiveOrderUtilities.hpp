#ifndef NEXUS_PRIMITIVEORDERUTILITIES_HPP
#define NEXUS_PRIMITIVEORDERUTILITIES_HPP
#include <Beam/Queues/Queue.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"
#include "Nexus/OrderExecutionServiceTests/OrderExecutionServiceTests.hpp"

namespace Nexus {
namespace OrderExecutionService {
namespace Tests {

  //! Sets the OrderStatus of an Order to CANCELED.
  /*!
    \param order The Order to cancel.
    \param timestamp The cancel timestamp.
  */
  inline void CancelOrder(PrimitiveOrder& order,
      const boost::posix_time::ptime& timestamp) {
    auto monitor = std::make_shared<Beam::Queue<ExecutionReport>>();
    order.GetPublisher().Monitor(monitor);
    ExecutionReport lastReport;
    while(true) {
      monitor->Emplace(Beam::Store(lastReport));
      if(lastReport.m_status == OrderStatus::PENDING_CANCEL) {
        break;
      }
    }
    while(!monitor->IsEmpty()) {
      monitor->Emplace(Beam::Store(lastReport));
    }
    auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
      OrderStatus::CANCELED, timestamp);
    order.Update(updatedReport);
  }

  //! Sets the OrderStatus of an Order being tested.
  /*!
    \param order The Order to set the OrderStatus for.
    \param newStatus The OrderStatus to assign to the <i>order</i>.
    \param timestamp The modification's timestamp.
  */
  inline void SetOrderStatus(PrimitiveOrder& order, OrderStatus newStatus,
      const boost::posix_time::ptime& timestamp) {
    order.With(
      [&] (OrderStatus status, const std::vector<ExecutionReport>& reports) {
        auto& lastReport = reports.back();
        auto updatedReport = ExecutionReport::BuildUpdatedReport(lastReport,
          newStatus, timestamp);
        order.Update(updatedReport);
      });
  }

  //! Fills an Order.
  /*!
    \param order The Order to fill.
    \param price The price of the fill.
    \param quantity The amount to fill the order for.
    \param timestamp The modification's timestamp.
  */
  inline void FillOrder(PrimitiveOrder& order, Money price, Quantity quantity,
      const boost::posix_time::ptime& timestamp) {
    order.With(
      [&] (OrderStatus status, const std::vector<ExecutionReport>& reports) {
        Quantity cumulativeQuantity = 0;
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

  //! Fills an Order.
  /*!
    \param order The Order to fill.
    \param quantity The amount to fill the order for.
    \param timestamp The modification's timestamp.
  */
  inline void FillOrder(PrimitiveOrder& order, Quantity quantity,
      const boost::posix_time::ptime& timestamp) {
    FillOrder(order, order.GetInfo().m_fields.m_price, quantity, timestamp);
  }
}
}
}

#endif
