#ifndef OASIS_FEES_CALCULATOR_ORDER_EXECUTION_DRIVER_HPP
#define OASIS_FEES_CALCULATOR_ORDER_EXECUTION_DRIVER_HPP
#include <tuple>
#include <unordered_set>
#include <vector>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/AsxTradeMatchFeeTable.hpp"
#include "Nexus/FeeHandling/ConsolidatedTmxFeeTable.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {

  /**
   * Calculates the fees for an ExecutionReport.
   * @param <O> The type of OrderExecutionDriver to pass the orders to.
   */
  template<typename O>
  class FeesCalculatorOrderExecutionDriver {
    public:

      /** The type of OrderExecutionDriver to pass the Orders to. */
      using OrderExecutionDriver = Beam::dereference_t<O>;

      /**
       * Constructs a FeesCalculatorOrderExecutionDriver.
       * @param driver The OrderExecutionDriver to send the submission to if all
       *        checks pass.
       * @param asx_trade_match_fee_table The fee table used by ASX TradeMatch.
       * @param tmx_fee_table The fee table used by TMX markets.
       */
      template<typename OF>
      FeesCalculatorOrderExecutionDriver(
        OF&& driver, AsxTradeMatchFeeTable asx_fee_table,
        ConsolidatedTmxFeeTable tmx_fee_table);

      ~FeesCalculatorOrderExecutionDriver();

      std::shared_ptr<Order> recover(const SequencedAccountOrderRecord& record);
      void add(const std::shared_ptr<Order>& order);
      std::shared_ptr<Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      Beam::local_ptr_t<O> m_driver;
      AsxTradeMatchFeeTable m_asx_fee_table;
      ConsolidatedTmxFeeTable m_tmx_fee_table;
      ConsolidatedTmxFeeTable::State m_tmx_state;
      Beam::SynchronizedUnorderedSet<std::shared_ptr<Order>> m_orders;
      Beam::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      void handle_australian_market_fees(
        PrimitiveOrder& order, const ExecutionReport& report);
      void handle_canadian_market_fees(
        PrimitiveOrder& order, const ExecutionReport& report);
      void on_execution_report(const std::shared_ptr<PrimitiveOrder>& order,
        const ExecutionReport& report);
  };

  template<typename O>
  template<typename OF>
  FeesCalculatorOrderExecutionDriver<O>::FeesCalculatorOrderExecutionDriver(
    OF&& driver, AsxTradeMatchFeeTable asx_trade_match_fee_table,
    ConsolidatedTmxFeeTable tmx_fee_table)
    : m_driver(std::forward<OF>(driver)),
      m_asx_fee_table(std::move(asx_trade_match_fee_table)),
      m_tmx_fee_table(std::move(tmx_fee_table)) {}

  template<typename O>
  FeesCalculatorOrderExecutionDriver<O>::~FeesCalculatorOrderExecutionDriver() {
    close();
  }

  template<typename O>
  std::shared_ptr<Order> FeesCalculatorOrderExecutionDriver<O>::recover(
      const SequencedAccountOrderRecord& record) {
    auto driver_order = m_driver->recover(record);
    auto order = std::make_shared<PrimitiveOrder>(**record);
    m_orders.insert(order);
    driver_order->get_publisher().with([&] {
      auto existing_reports = boost::optional<std::vector<ExecutionReport>>();
      driver_order->get_publisher().monitor(m_tasks.get_slot<ExecutionReport>(
        std::bind(&FeesCalculatorOrderExecutionDriver::on_execution_report,
          this, order, std::placeholders::_1)), Beam::out(existing_reports));
      if(existing_reports) {
        existing_reports->erase(existing_reports->begin(),
          existing_reports->begin() + (*record)->m_execution_reports.size());
        for(auto& report : *existing_reports) {
          m_tasks.push(
            std::bind(&FeesCalculatorOrderExecutionDriver::on_execution_report,
              this, order, report));
        }
      }
    });
    return order;
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::add(
      const std::shared_ptr<Order>& order) {
    m_driver->add(order);
  }

  template<typename O>
  std::shared_ptr<Order> FeesCalculatorOrderExecutionDriver<O>::submit(
      const OrderInfo& info) {
    auto driver_order = m_driver->submit(info);
    auto order = std::make_shared<PrimitiveOrder>(driver_order->get_info());
    m_orders.insert(order);
    driver_order->get_publisher().monitor(m_tasks.get_slot<ExecutionReport>(
      std::bind_front(&FeesCalculatorOrderExecutionDriver::on_execution_report,
        this, order)));
    return order;
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::cancel(
      const OrderExecutionSession& session, OrderId id) {
    return m_driver->cancel(session, id);
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    return m_driver->update(session, id, report);
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::close() {
    m_open_state.close();
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::handle_australian_market_fees(
      PrimitiveOrder& order, const ExecutionReport& report) {
    auto fees_report =
      calculate_fee(m_asx_fee_table, order.get_info().m_fields, report);
    order.with([&] (auto status, const auto& reports) {
      order.update(fees_report);
    });
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::handle_canadian_market_fees(
      PrimitiveOrder& order, const ExecutionReport& report) {
    auto fees_report =
      calculate_fee(m_tmx_fee_table, m_tmx_state, order, report);
    order.with([&] (auto status, const auto& reports) {
      order.update(fees_report);
    });
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::on_execution_report(
      const std::shared_ptr<PrimitiveOrder>& order,
      const ExecutionReport& report) {
    if(report.m_status == OrderStatus::PENDING_NEW) {
      return;
    }
    auto venue = order->get_info().m_fields.m_security.get_venue();
    if(venue == DefaultVenues::ASX || venue == DefaultVenues::CXA) {
      handle_australian_market_fees(*order, report);
    } else {
      handle_canadian_market_fees(*order, report);
    }
  }
}

#endif
