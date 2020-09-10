#ifndef OASIS_FEES_CALCULATOR_ORDER_EXECUTION_DRIVER_HPP
#define OASIS_FEES_CALCULATOR_ORDER_EXECUTION_DRIVER_HPP
#include <tuple>
#include <unordered_set>
#include <vector>
#include <Beam/Collections/SynchronizedSet.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/AsxtFeeTable.hpp"
#include "Nexus/FeeHandling/HkexFeeTable.hpp"
#include "Nexus/FeeHandling/JpxFeeTable.hpp"
#include "Nexus/FeeHandling/ConsolidatedTmxFeeTable.hpp"
#include "Nexus/FeeHandling/ConsolidatedUsFeeTable.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus::OasisOrderExecutionService {

  /**
   * Calculates the fees for an ExecutionReport.
   * @param <O> The type of OrderExecutionDriver to pass the orders to.
   */
  template<typename O>
  class FeesCalculatorOrderExecutionDriver : private boost::noncopyable {
    public:

      /** The type of OrderExecutionDriver to pass the Orders to. */
      using OrderExecutionDriver = Beam::GetTryDereferenceType<O>;

      /**
       * Constructs a FeesCalculatorOrderExecutionDriver.
       * @param orderExecutionDriver The OrderExecutionDriver to send the
                submission to if all checks pass.
       * @param asxtFeeTable The fee table used by ASX TradeMatch.
       * @param hkexFeeTable The fee table used by HKEX.
       * @param jpxFeeTable The fee table used by JPX markets.
       * @param tmxFeeTable The fee table used by TMX markets.
       * @param usFeeTable The fee table used by US markets.
       */
      template<typename OF>
      FeesCalculatorOrderExecutionDriver(OF&& orderExecutionDriver,
        AsxtFeeTable asxFeeTable, HkexFeeTable hkexFeeTable,
        JpxFeeTable jpxFeeTable, ConsolidatedTmxFeeTable tmxFeeTable,
        ConsolidatedUsFeeTable usFeeTable);

      ~FeesCalculatorOrderExecutionDriver();

      const OrderExecutionService::Order& Recover(
        const OrderExecutionService::SequencedAccountOrderRecord& orderRecord);

      const OrderExecutionService::Order& Submit(
        const OrderExecutionService::OrderInfo& info);

      void Cancel(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId);

      void Update(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId,
        const OrderExecutionService::ExecutionReport& executionReport);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<O> m_orderExecutionDriver;
      AsxtFeeTable m_asxtFeeTable;
      HkexFeeTable m_hkexFeeTable;
      JpxFeeTable m_jpxFeeTable;
      ConsolidatedTmxFeeTable m_tmxFeeTable;
      ConsolidatedTmxFeeTable::State m_tmxState;
      ConsolidatedUsFeeTable m_usFeeTable;
      Beam::SynchronizedUnorderedSet<
        std::shared_ptr<OrderExecutionService::Order>> m_orders;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void HandleAustralianMarketFees(
        OrderExecutionService::PrimitiveOrder& order,
        const OrderExecutionService::ExecutionReport& executionReport);
      void HandleCanadianMarketFees(
        OrderExecutionService::PrimitiveOrder& order,
        const OrderExecutionService::ExecutionReport& executionReport);
      void HandleHongKongMarketFees(
        OrderExecutionService::PrimitiveOrder& order,
        const OrderExecutionService::ExecutionReport& executionReport);
      void HandleJapaneseMarketFees(
        OrderExecutionService::PrimitiveOrder& order,
        const OrderExecutionService::ExecutionReport& executionReport);
      void HandleUsMarketFees(OrderExecutionService::PrimitiveOrder& order,
        const OrderExecutionService::ExecutionReport& executionReport);
      void OnExecutionReport(
        const std::shared_ptr<OrderExecutionService::PrimitiveOrder>& order,
        const OrderExecutionService::ExecutionReport& executionReport);
  };

  template<typename O>
  template<typename OF>
  FeesCalculatorOrderExecutionDriver<O>::FeesCalculatorOrderExecutionDriver(
    OF&& orderExecutionDriver, AsxtFeeTable asxtFeeTable,
    HkexFeeTable hkexFeeTable, JpxFeeTable jpxFeeTable,
    ConsolidatedTmxFeeTable tmxFeeTable, ConsolidatedUsFeeTable usFeeTable)
    : m_orderExecutionDriver(std::forward<OF>(orderExecutionDriver)),
      m_asxtFeeTable(std::move(asxtFeeTable)),
      m_hkexFeeTable(std::move(hkexFeeTable)),
      m_jpxFeeTable(std::move(jpxFeeTable)),
      m_tmxFeeTable(std::move(tmxFeeTable)),
      m_usFeeTable(std::move(usFeeTable)) {}

  template<typename O>
  FeesCalculatorOrderExecutionDriver<O>::~FeesCalculatorOrderExecutionDriver() {
    Close();
  }

  template<typename O>
  const OrderExecutionService::Order& FeesCalculatorOrderExecutionDriver<O>::
      Recover(
      const OrderExecutionService::SequencedAccountOrderRecord& orderRecord) {
    auto& driverOrder = m_orderExecutionDriver->Recover(orderRecord);
    auto order = std::make_shared<OrderExecutionService::PrimitiveOrder>(
      **orderRecord);
    m_orders.Insert(order);
    driverOrder.GetPublisher().With([&] {
      auto existingExecutionReports = boost::optional<
        std::vector<OrderExecutionService::ExecutionReport>>();
      driverOrder.GetPublisher().Monitor(
        m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(std::bind(
        &FeesCalculatorOrderExecutionDriver::OnExecutionReport, this, order,
        std::placeholders::_1)), Beam::Store(existingExecutionReports));
      if(existingExecutionReports.is_initialized()) {
        existingExecutionReports->erase(existingExecutionReports->begin(),
          existingExecutionReports->begin() +
          (*orderRecord)->m_executionReports.size());
        for(auto& executionReport : *existingExecutionReports) {
          m_tasks.Push(
            std::bind(&FeesCalculatorOrderExecutionDriver::OnExecutionReport,
            this, order, executionReport));
        }
      }
    });
    return *order;
  }

  template<typename O>
  const OrderExecutionService::Order& FeesCalculatorOrderExecutionDriver<O>::
      Submit(const OrderExecutionService::OrderInfo& info) {
    auto& driverOrder = m_orderExecutionDriver->Submit(info);
    auto order = std::make_shared<OrderExecutionService::PrimitiveOrder>(
      driverOrder.GetInfo());
    m_orders.Insert(order);
    driverOrder.GetPublisher().Monitor(
      m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(std::bind(
      &FeesCalculatorOrderExecutionDriver::OnExecutionReport, this, order,
      std::placeholders::_1)));
    return *order;
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::Cancel(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId) {
    return m_orderExecutionDriver->Cancel(session, orderId);
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::Update(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId,
      const OrderExecutionService::ExecutionReport& executionReport) {
    return m_orderExecutionDriver->Update(session, orderId, executionReport);
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::Close() {
    m_openState.Close();
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::HandleAustralianMarketFees(
      OrderExecutionService::PrimitiveOrder& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto feesReport = CalculateFee(m_asxtFeeTable, order.GetInfo().m_fields,
      executionReport);
    order.With([&] (auto status, const auto& reports) {
      order.Update(feesReport);
    });
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::HandleCanadianMarketFees(
      OrderExecutionService::PrimitiveOrder& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto feesReport = CalculateFee(m_tmxFeeTable, m_tmxState, order,
      executionReport);
    order.With([&] (auto status, const auto& reports) {
      order.Update(feesReport);
    });
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::HandleHongKongMarketFees(
      OrderExecutionService::PrimitiveOrder& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto feesReport = CalculateFee(m_hkexFeeTable, order.GetInfo().m_fields,
      executionReport);
    order.With([&] (auto status, const auto& reports) {
      order.Update(feesReport);
    });
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::HandleJapaneseMarketFees(
      OrderExecutionService::PrimitiveOrder& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto feesReport = CalculateFee(m_jpxFeeTable, order.GetInfo().m_fields,
      executionReport);
    order.With([&] (auto status, const auto& reports) {
      order.Update(feesReport);
    });
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::HandleUsMarketFees(
      OrderExecutionService::PrimitiveOrder& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    auto feesReport = CalculateFee(m_usFeeTable, order, executionReport);
    order.With([&] (auto status, const auto& reports) {
      order.Update(feesReport);
    });
  }

  template<typename O>
  void FeesCalculatorOrderExecutionDriver<O>::OnExecutionReport(
      const std::shared_ptr<OrderExecutionService::PrimitiveOrder>& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_status == OrderStatus::PENDING_NEW) {
      return;
    }
    if(order->GetInfo().m_fields.m_security.GetCountry() ==
        DefaultCountries::AU()) {
      HandleAustralianMarketFees(*order, executionReport);
    } else if(order->GetInfo().m_fields.m_security.GetCountry() ==
        DefaultCountries::CA()) {
      HandleCanadianMarketFees(*order, executionReport);
    } else if(order->GetInfo().m_fields.m_security.GetCountry() ==
        DefaultCountries::HK()) {
      HandleHongKongMarketFees(*order, executionReport);
    } else if(order->GetInfo().m_fields.m_security.GetCountry() ==
        DefaultCountries::JP()) {
      HandleJapaneseMarketFees(*order, executionReport);
    }  else {
      HandleUsMarketFees(*order, executionReport);
    }
  }
}

#endif
