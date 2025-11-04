#ifndef NEXUS_PRIMITIVE_ORDER_UTILITIES_HPP
#define NEXUS_PRIMITIVE_ORDER_UTILITIES_HPP
#include <Beam/Queues/Queue.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus::Tests {

  /**
   * Sets the OrderStatus of an Order to CANCELED.
   * @param order The Order to cancel.
   * @param timestamp The cancel timestamp.
   */
  inline ExecutionReport cancel(
      PrimitiveOrder& order, boost::posix_time::ptime timestamp) {
    auto monitor = std::make_shared<Beam::Queue<ExecutionReport>>();
    order.get_publisher().monitor(monitor);
    auto last_report = ExecutionReport();
    while(true) {
      last_report = monitor->pop();
      if(last_report.m_status == OrderStatus::PENDING_CANCEL) {
        break;
      }
    }
    while(auto report = monitor->try_pop()) {
      last_report = std::move(*report);
    }
    auto updated_report =
      make_update(last_report, OrderStatus::CANCELED, timestamp);
    order.update(updated_report);
    return updated_report;
  }

  /**
   * Sets the OrderStatus of an Order to CANCELED.
   * @param order The Order to cancel.
   */
  inline ExecutionReport cancel(PrimitiveOrder& order) {
    return cancel(
      order, order.get_publisher().get_snapshot()->back().m_timestamp);
  }

  /**
   * Sets the OrderStatus of an Order being tested.
   * @param order The Order to set the OrderStatus for.
   * @param new_status The OrderStatus to assign to the <i>order</i>.
   * @param timestamp The modification's timestamp.
   */
  inline ExecutionReport set_order_status(PrimitiveOrder& order,
      OrderStatus new_status, boost::posix_time::ptime timestamp) {
    auto updated_report = ExecutionReport();
    order.with([&] (auto status, const auto& reports) {
      auto& last_report = reports.back();
      updated_report = make_update(last_report, new_status, timestamp);
      order.update(updated_report);
    });
    return updated_report;
  }

  /**
   * Sets the OrderStatus of an Order being tested.
   * @param order The Order to set the OrderStatus for.
   * @param new_status The OrderStatus to assign to the <i>order</i>.
   */
  inline ExecutionReport set_order_status(
      PrimitiveOrder& order, OrderStatus new_status) {
    return set_order_status(order, new_status,
      order.get_publisher().get_snapshot()->back().m_timestamp);
  }

  /**
   * Sets the OrderStatus of an Order to NEW.
   * @param order The Order to set the OrderStatus for.
   * @param timestamp The modification's timestamp.
   */
  inline ExecutionReport accept(
      PrimitiveOrder& order, boost::posix_time::ptime timestamp) {
    return set_order_status(order, OrderStatus::NEW, timestamp);
  }

  /**
   * Sets the OrderStatus of an Order to NEW.
   * @param order The Order to set the OrderStatus for.
   */
  inline ExecutionReport accept(PrimitiveOrder& order) {
    return set_order_status(order, OrderStatus::NEW);
  }

  /**
   * Sets the OrderStatus of an Order to REJECTED.
   * @param order The Order to set the OrderStatus for.
   * @param timestamp The modification's timestamp.
   */
  inline ExecutionReport reject(
      PrimitiveOrder& order, boost::posix_time::ptime timestamp) {
    return set_order_status(order, OrderStatus::REJECTED, timestamp);
  }

  /**
   * Sets the OrderStatus of an Order to REJECTED.
   * @param order The Order to set the OrderStatus for.
   */
  inline ExecutionReport reject(PrimitiveOrder& order) {
    return set_order_status(order, OrderStatus::REJECTED);
  }

  /**
   * Fills an Order.
   * @param order The Order to fill.
   * @param price The price of the fill.
   * @param quantity The amount to fill the order for.
   * @param timestamp The modification's timestamp.
   */
  inline ExecutionReport fill(PrimitiveOrder& order, Money price,
      Quantity quantity, boost::posix_time::ptime timestamp) {
    auto updated_report = ExecutionReport();
    order.with([&] (auto status, const auto& reports) {
      auto cumulative_quantity = Quantity();
      for(auto& report : reports) {
        cumulative_quantity += report.m_last_quantity;
      }
      auto& last_report = reports.back();
      assert(
        cumulative_quantity + quantity <= order.get_info().m_fields.m_quantity);
      auto new_status = [&] {
        if(cumulative_quantity + quantity ==
            order.get_info().m_fields.m_quantity) {
          return OrderStatus::FILLED;
        } else {
          return OrderStatus::PARTIALLY_FILLED;
        }
      }();
      updated_report = make_update(last_report, new_status, timestamp);
      updated_report.m_last_price = price;
      updated_report.m_last_quantity = quantity;
      order.update(updated_report);
    });
    return updated_report;
  }

  /**
   * Fills an Order.
   * @param order The Order to fill.
   * @param price The price of the fill.
   * @param quantity The amount to fill the order for.
   */
  inline ExecutionReport fill(
      PrimitiveOrder& order, Money price, Quantity quantity) {
    return fill(order, price, quantity,
      order.get_publisher().get_snapshot()->back().m_timestamp);
  }

  /**
   * Fills an Order.
   * @param order The Order to fill.
   * @param quantity The amount to fill the order for.
   * @param timestamp The modification's timestamp.
   */
  inline ExecutionReport fill(PrimitiveOrder& order, Quantity quantity,
      boost::posix_time::ptime timestamp) {
    return fill(order, order.get_info().m_fields.m_price, quantity, timestamp);
  }

  /**
   * Fills an Order.
   * @param order The Order to fill.
   * @param quantity The amount to fill the order for.
   */
  inline ExecutionReport fill(PrimitiveOrder& order, Quantity quantity) {
    return fill(order, order.get_info().m_fields.m_price, quantity);
  }

  /**
   * Returns <code>true</code> iff an Order is in the PENDING_CANCEL state.
   * @param order The Order to test.
   * @return <code>true</code> iff the <i>order</i> is PENDING_CANCEL.
   */
  inline bool is_pending_cancel(const PrimitiveOrder& order) {
    return order.with([] (auto status, const auto& reports) {
      auto is_pending_cancel = false;
      for(auto& report : reports) {
        if(report.m_status == OrderStatus::PENDING_CANCEL) {
          is_pending_cancel = true;
          break;
        }
      }
      is_pending_cancel &= !is_terminal(status);
      return is_pending_cancel;
    });
  }
}

#endif
