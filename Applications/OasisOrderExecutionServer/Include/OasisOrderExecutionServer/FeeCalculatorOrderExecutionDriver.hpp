#ifndef OASIS_FEESCALCULATORORDEREXECUTIONDRIVER_HPP
#define OASIS_FEESCALCULATORORDEREXECUTIONDRIVER_HPP
#include <tuple>
#include <unordered_set>
#include <vector>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/Utilities/SynchronizedSet.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Definitions/DefaultMarketDatabase.hpp"
#include "Nexus/Definitions/Money.hpp"
#include "Nexus/FeeHandling/AsxtFeeTable.hpp"
#include "Nexus/FeeHandling/ConsolidatedTmxFeeTable.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {
namespace OasisOrderExecutionService {

  /*! \class FeesCalculatorOrderExecutionDriver
      \brief Calculates the fees for an ExecutionReport.
      \tparam OrderExecutionDriverType The type of OrderExecutionDriver to pass
              the orders to.
   */
  template<typename OrderExecutionDriverType>
  class FeesCalculatorOrderExecutionDriver : private boost::noncopyable {
    public:

      //! The type of OrderExecutionDriver to pass the Orders to.
      using OrderExecutionDriver = Beam::GetTryDereferenceType<
        OrderExecutionDriverType>;

      //! Constructs a FeesCalculatorOrderExecutionDriver.
      /*!
        \param orderExecutionDriver The OrderExecutionDriver to send the
               submission to if all checks pass.
        \param asxtFeeTable The fee table used by ASX TradeMatch.
        \param tmxFeeTable The fee table used by TMX markets.
      */
      template<typename OrderExecutionDriverForward>
      FeesCalculatorOrderExecutionDriver(
        OrderExecutionDriverForward&& orderExecutionDriver,
        AsxtFeeTable asxFeeTable, ConsolidatedTmxFeeTable tmxFeeTable);

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

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<OrderExecutionDriverType>
        m_orderExecutionDriver;
      AsxtFeeTable m_asxtFeeTable;
      ConsolidatedTmxFeeTable m_tmxFeeTable;
      Beam::SynchronizedUnorderedSet<
        std::shared_ptr<OrderExecutionService::Order>> m_orders;
      Beam::SynchronizedUnorderedMap<OrderExecutionService::OrderId, Money>
        m_perOrderCharges;
      Beam::SynchronizedUnorderedMap<OrderExecutionService::OrderId, int>
        m_fillCount;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      void HandleAustralianMarketFees(
        OrderExecutionService::PrimitiveOrder& order,
        const OrderExecutionService::ExecutionReport& executionReport);
      void HandleCanadianMarketFees(
        OrderExecutionService::PrimitiveOrder& order,
        const OrderExecutionService::ExecutionReport& executionReport);
      void OnExecutionReport(
        const std::shared_ptr<OrderExecutionService::PrimitiveOrder>& order,
        const OrderExecutionService::ExecutionReport& executionReport);
  };

  template<typename OrderExecutionDriverType>
  template<typename OrderExecutionDriverForward>
  FeesCalculatorOrderExecutionDriver<OrderExecutionDriverType>::
      FeesCalculatorOrderExecutionDriver(OrderExecutionDriverForward&&
      orderExecutionDriver, AsxtFeeTable asxtFeeTable,
      ConsolidatedTmxFeeTable tmxFeeTable)
      : m_orderExecutionDriver(std::forward<OrderExecutionDriverForward>(
          orderExecutionDriver)),
        m_asxtFeeTable(std::move(asxtFeeTable)),
        m_tmxFeeTable(std::move(tmxFeeTable)) {}

  template<typename OrderExecutionDriverType>
  FeesCalculatorOrderExecutionDriver<OrderExecutionDriverType>::
      ~FeesCalculatorOrderExecutionDriver() {
    Close();
  }

  template<typename OrderExecutionDriverType>
  const OrderExecutionService::Order& FeesCalculatorOrderExecutionDriver<
      OrderExecutionDriverType>::Recover(
      const OrderExecutionService::SequencedAccountOrderRecord& orderRecord) {
    const auto& driverOrder = m_orderExecutionDriver->Recover(orderRecord);
    auto order = std::make_shared<OrderExecutionService::PrimitiveOrder>(
      **orderRecord);
    m_orders.Insert(order);
    driverOrder.GetPublisher().With(
      [&] {
        boost::optional<std::vector<OrderExecutionService::ExecutionReport>>
          existingExecutionReports;
        driverOrder.GetPublisher().Monitor(
          m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(std::bind(
          &FeesCalculatorOrderExecutionDriver::OnExecutionReport, this, order,
          std::placeholders::_1)), Beam::Store(existingExecutionReports));
        if(existingExecutionReports.is_initialized()) {
          existingExecutionReports->erase(existingExecutionReports->begin(),
            existingExecutionReports->begin() +
            (*orderRecord)->m_executionReports.size());
          for(const auto& executionReport : *existingExecutionReports) {
            m_tasks.Push(
              std::bind(&FeesCalculatorOrderExecutionDriver::OnExecutionReport,
              this, order, executionReport));
          }
        }
      });
    return *order;
  }

  template<typename OrderExecutionDriverType>
  const OrderExecutionService::Order& FeesCalculatorOrderExecutionDriver<
      OrderExecutionDriverType>::Submit(
      const OrderExecutionService::OrderInfo& info) {
    const auto& driverOrder = m_orderExecutionDriver->Submit(info);
    auto order = std::make_shared<OrderExecutionService::PrimitiveOrder>(
      driverOrder.GetInfo());
    m_orders.Insert(order);
    driverOrder.GetPublisher().Monitor(
      m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(std::bind(
      &FeesCalculatorOrderExecutionDriver::OnExecutionReport, this, order,
      std::placeholders::_1)));
    return *order;
  }

  template<typename OrderExecutionDriverType>
  void FeesCalculatorOrderExecutionDriver<OrderExecutionDriverType>::Cancel(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId) {
    return m_orderExecutionDriver->Cancel(session, orderId);
  }

  template<typename OrderExecutionDriverType>
  void FeesCalculatorOrderExecutionDriver<OrderExecutionDriverType>::Update(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId,
      const OrderExecutionService::ExecutionReport& executionReport) {
    return m_orderExecutionDriver->Update(session, orderId, executionReport);
  }

  template<typename OrderExecutionDriverType>
  void FeesCalculatorOrderExecutionDriver<OrderExecutionDriverType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_orderExecutionDriver->Open();
    } catch(std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename OrderExecutionDriverType>
  void FeesCalculatorOrderExecutionDriver<OrderExecutionDriverType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename OrderExecutionDriverType>
  void FeesCalculatorOrderExecutionDriver<OrderExecutionDriverType>::
      Shutdown() {
    m_openState.SetClosed();
  }

  template<typename OrderExecutionDriverType>
  void FeesCalculatorOrderExecutionDriver<OrderExecutionDriverType>::
      HandleAustralianMarketFees(OrderExecutionService::PrimitiveOrder& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    const auto SPIRE_FEE = 18 * Money::CENT;
    const auto CLEARING_FEE = boost::rational<int>{125, 1000000};
    auto feesReport = executionReport;
    feesReport.m_processingFee += CLEARING_FEE *
      (feesReport.m_lastQuantity * feesReport.m_lastPrice);
    if(feesReport.m_lastQuantity != 0) {
      feesReport.m_commission += SPIRE_FEE;
    }
    feesReport.m_executionFee += CalculateFee(m_asxtFeeTable, executionReport);
    order.With(
      [&] (OrderStatus status,
          const std::vector<OrderExecutionService::ExecutionReport>& reports) {
        order.Update(feesReport);
      });
  }

  template<typename OrderExecutionDriverType>
  void FeesCalculatorOrderExecutionDriver<OrderExecutionDriverType>::
      HandleCanadianMarketFees(OrderExecutionService::PrimitiveOrder& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    const auto SPIRE_FEE = (14 * Money::BIP) / 10;
    const auto IIROC_FEE = (132 * Money::CENT) / 10;
    const auto CDS_FEE = 13 * Money::CENT;
    const auto CDS_CAP = 5;
    const auto CLEARING_FEE = Money::BIP;
    const auto PER_ORDER_FEE = Money::BIP;
    const auto PER_ORDER_CAP = 10 * Money::CENT;
    auto feesReport = executionReport;
    feesReport.m_processingFee += feesReport.m_lastQuantity * CLEARING_FEE;
    if(feesReport.m_lastQuantity != 0) {
      auto& fillCount = m_fillCount.Get(order.GetInfo().m_orderId);
      ++fillCount;
      feesReport.m_processingFee += IIROC_FEE;
      if(fillCount <= CDS_CAP) {
        feesReport.m_processingFee += CDS_FEE;
      }
    }
    feesReport.m_commission += feesReport.m_lastQuantity * SPIRE_FEE;
    auto& perOrderCharge = m_perOrderCharges.Get(order.GetInfo().m_orderId);
    auto perOrderDelta = executionReport.m_lastQuantity * PER_ORDER_FEE;
    if(perOrderCharge + perOrderDelta > PER_ORDER_CAP) {
      perOrderDelta = PER_ORDER_CAP - perOrderCharge;
    }
    perOrderCharge += perOrderDelta;
    feesReport.m_processingFee += perOrderDelta;
    feesReport.m_executionFee += CalculateFee(m_tmxFeeTable, order,
      executionReport);
    order.With(
      [&] (OrderStatus status,
          const std::vector<OrderExecutionService::ExecutionReport>& reports) {
        order.Update(feesReport);
      });
  }

  template<typename OrderExecutionDriverType>
  void FeesCalculatorOrderExecutionDriver<OrderExecutionDriverType>::
      OnExecutionReport(
      const std::shared_ptr<OrderExecutionService::PrimitiveOrder>& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_status == OrderStatus::PENDING_NEW) {
      return;
    }
    if(order->GetInfo().m_fields.m_security.GetCountry() ==
        DefaultCountries::AU()) {
      HandleAustralianMarketFees(*order, executionReport);
    } else {
      HandleCanadianMarketFees(*order, executionReport);
    }
  }
}
}

#endif
