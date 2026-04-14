#ifndef NEXUS_BOOKKEEPER_REACTOR_HPP
#define NEXUS_BOOKKEEPER_REACTOR_HPP
#include <optional>
#include <tuple>
#include <utility>
#include <Aspen/Aspen.hpp>
#include <Beam/Queues/PublisherReactor.hpp>
#include "Nexus/Accounting/Bookkeeper.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {

  /**
   * Returns a reactor that produces updates to a bookkeeper's inventory based
   * on a series of execution reports.
   * @param bookkeeper The initial state of the bookkeeper.
   * @param orders The reactor producing the orders to monitor for execution
   *        reports.
   */
  template<typename Orders>
  auto make_bookkeeper_reactor(IsBookkeeper auto bookkeeper, Orders orders) {
    return Aspen::lift([bookkeeper = std::move(bookkeeper)] (
        const std::tuple<std::shared_ptr<Order>,
          ExecutionReport>& update) mutable -> std::optional<Inventory> {
      auto& order = *std::get<0>(update);
      auto& report = std::get<1>(update);
      if(report.m_last_quantity == 0) {
        return std::nullopt;
      }
      bookkeeper.record(order.get_info().m_fields.m_ticker,
        order.get_info().m_fields.m_currency,
        get_direction(order.get_info().m_fields.m_side) *
          report.m_last_quantity,
        report.m_last_quantity * report.m_last_price, get_fee_total(report));
      return bookkeeper.get_inventory(order.get_info().m_fields.m_ticker);
    }, Aspen::concur(Aspen::lift([] (const std::shared_ptr<Order>& order) {
      return Aspen::Shared(Aspen::lift([=] (const ExecutionReport& report) {
        return std::tuple(order, report);
      }, Beam::publisher_reactor(order->get_publisher())));
    }, std::move(orders))));
  }

  /**
   * Returns a reactor that produces updates to a bookkeeper's inventory based
   * on a series of execution reports.
   * @param orders The reactor producing the orders to monitor for execution
   *        reports.
   */
  template<IsBookkeeper B, typename Orders>
  auto make_bookkeeper_reactor(Orders orders) {
    return make_bookkeeper_reactor(B(), std::move(orders));
  }
}

#endif
