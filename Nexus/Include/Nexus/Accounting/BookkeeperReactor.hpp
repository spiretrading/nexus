#ifndef NEXUS_BOOKKEEPER_REACTOR_HPP
#define NEXUS_BOOKKEEPER_REACTOR_HPP
#include <optional>
#include <tuple>
#include <utility>
#include <Aspen/Aspen.hpp>
#include <Beam/Reactors/PublisherReactor.hpp>
#include "Nexus/Accounting/Bookkeeper.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus::Accounting {

  /**
   * Returns a reactor that produces updates to a bookkeeper's inventory based
   * on a series of execution reports.
   * @param orders The reactor producing the orders to monitor for execution
   *        reports.
   */
  template<IsBookkeeper B, typename Orders>
  auto make_bookkeeper_reactor(Orders orders) {
    return Aspen::lift([bookkeeper = B()] (
        const std::tuple<const OrderExecutionService::Order*,
          OrderExecutionService::ExecutionReport>& update) mutable ->
            std::optional<typename B::Inventory> {
      auto& order = *std::get<0>(update);
      auto& report = std::get<1>(update);
      if(report.m_last_quantity == 0) {
        return std::nullopt;
      }
      bookkeeper.record(order.get_info().m_fields.m_security,
        order.get_info().m_fields.m_currency,
        get_direction(order.get_info().m_fields.m_side) *
          report.m_last_quantity,
        report.m_last_quantity * report.m_last_price, get_fee_total(report));
      return bookkeeper.get_inventory(order.get_info().m_fields.m_security,
        order.get_info().m_fields.m_currency);
    }, Aspen::concur(Aspen::lift(
    [] (const OrderExecutionService::Order* order) {
      return Aspen::Shared(Aspen::lift(
        [=] (const OrderExecutionService::ExecutionReport& report) {
          return std::tuple(order, report);
        }, Beam::Reactors::PublisherReactor(order->get_publisher())));
    }, std::move(orders))));
  }
}

#endif
