#ifndef NEXUS_BOOKKEEPER_REACTOR_HPP
#define NEXUS_BOOKKEEPER_REACTOR_HPP
#include <tuple>
#include <utility>
#include <Aspen/Aspen.hpp>
#include <Beam/Reactors/PublisherReactor.hpp>
#include "Nexus/Accounting/Accounting.hpp"
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus::Accounting {

  /**
   * Returns a reactor that produces updates to a bookkeeper's inventory based
   * on a series of execution reports.
   * @param orders The reactor producing the orders to monitor for execution
   *        reports.
   */
  template<typename Bookkeeper, typename Orders>
  auto BookkeeperReactor(Orders orders) {
    return Aspen::lift(
      [bookkeeper = Bookkeeper()] (
          const std::tuple<const OrderExecutionService::Order*,
            OrderExecutionService::ExecutionReport>& update) mutable ->
            std::optional<typename Bookkeeper::Inventory> {
        auto& order = *std::get<0>(update);
        auto& report = std::get<1>(update);
        if(report.m_lastQuantity == 0) {
          return std::nullopt;
        }
        bookkeeper.RecordTransaction(order.GetInfo().m_fields.m_security,
          order.GetInfo().m_fields.m_currency,
          get_direction(order.GetInfo().m_fields.m_side) *
            report.m_lastQuantity, report.m_lastQuantity * report.m_lastPrice,
          OrderExecutionService::GetFeeTotal(report));
        return bookkeeper.GetInventory(order.GetInfo().m_fields.m_security,
          order.GetInfo().m_fields.m_currency);
      }, Aspen::concur(Aspen::lift(
      [] (const OrderExecutionService::Order* order) {
        return Aspen::Shared(Aspen::lift(
          [=] (const OrderExecutionService::ExecutionReport& executionReport) {
            return std::tuple(order, executionReport);
          }, Beam::Reactors::PublisherReactor(order->GetPublisher())));
      }, std::move(orders))));
  }
}

#endif
