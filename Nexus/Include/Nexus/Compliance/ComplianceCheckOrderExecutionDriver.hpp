#ifndef NEXUS_COMPLIANCE_CHECK_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_COMPLIANCE_CHECK_ORDER_EXECUTION_DRIVER_HPP
#include <Beam/Collections/SynchronizedList.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleSet.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus::Compliance {

  /**
   * Performs a series of compliance checks on order execution operations.
   * @param <D> The type of OrderExecutionDriver to send operations to to if all
   *        checks pass.
   * @param <C> The type of TimeClient used for Order timestamps.
   * @param <S> The type of ComplianceRuleSet used to validate operations.
   */
  template<typename D, typename C, typename S>
  class ComplianceCheckOrderExecutionDriver : private boost::noncopyable {
    public:

      /**
       * The type of OrderExecutionDriver to send operations to if all checks
       * pass.
       */
      using OrderExecutionDriver = Beam::GetTryDereferenceType<D>;

      /** The type of TimeClient used for Order timestamps. */
      using TimeClient = Beam::GetTryDereferenceType<C>;

      /** The type of ComplianceRuleSet used to validate operations. */
      using ComplianceRuleSet = Beam::GetTryDereferenceType<S>;

      /**
       * Constructs a ComplianceCheckOrderExecutionDriver.
       * @param orderExecutionDriver The OrderExecutionDriver to send operations
       *        to if all checks pass.
       * @param timeClient Initializes the TimeClient.
       * @param complianceRuleSet Contains the set of compliance rules used to
       *        check order execution operations.
       */
      template<typename DF, typename CF, typename SF>
      ComplianceCheckOrderExecutionDriver(DF&& orderExecutionDriver,
        CF&& timeClient, SF&& complianceRuleSet);

      ~ComplianceCheckOrderExecutionDriver();

      const OrderExecutionService::Order& Recover(
        const OrderExecutionService::SequencedAccountOrderRecord& order);

      const OrderExecutionService::Order& Submit(
        const OrderExecutionService::OrderInfo& orderInfo);

      void Cancel(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId);

      void Update(const OrderExecutionService::OrderExecutionSession& session,
        OrderExecutionService::OrderId orderId,
        const OrderExecutionService::ExecutionReport& executionReport);

      void Close();

    private:
      Beam::GetOptionalLocalPtr<D> m_orderExecutionDriver;
      Beam::GetOptionalLocalPtr<C> m_timeClient;
      Beam::GetOptionalLocalPtr<S> m_complianceRuleSet;
      Beam::SynchronizedUnorderedMap<OrderExecutionService::OrderId,
        std::unique_ptr<OrderExecutionService::PrimitiveOrder>> m_orders;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void OnExecutionReport(OrderExecutionService::PrimitiveOrder& order,
        const OrderExecutionService::ExecutionReport& executionReport);
  };

  template<typename D, typename C, typename S>
  template<typename DF, typename CF, typename SF>
  ComplianceCheckOrderExecutionDriver<D, C, S>::
    ComplianceCheckOrderExecutionDriver(DF&& orderExecutionDriver,
    CF&& timeClient, SF&& complianceRuleSet)
    : m_orderExecutionDriver(std::forward<DF>(orderExecutionDriver)),
      m_timeClient(std::forward<CF>(timeClient)),
      m_complianceRuleSet(std::forward<SF>(complianceRuleSet)) {}

  template<typename D, typename C, typename S>
  ComplianceCheckOrderExecutionDriver<D, C, S>::
      ~ComplianceCheckOrderExecutionDriver() {
    Close();
  }

  template<typename D, typename C, typename S>
  const OrderExecutionService::Order& ComplianceCheckOrderExecutionDriver<
      D, C, S>::Recover(
      const OrderExecutionService::SequencedAccountOrderRecord& orderRecord) {
    auto& order = m_orderExecutionDriver->Recover(orderRecord);
    m_complianceRuleSet->Add(order);
    return order;
  }

  template<typename D, typename C, typename S>
  const OrderExecutionService::Order& ComplianceCheckOrderExecutionDriver<
      D, C, S>::Submit(const OrderExecutionService::OrderInfo& orderInfo) {
    auto instance = std::make_unique<OrderExecutionService::PrimitiveOrder>(
      orderInfo);
    auto& order = *instance;
    m_orders.Insert(orderInfo.m_orderId, std::move(instance));
    try {
      m_complianceRuleSet->Submit(order);
    } catch(const std::exception& e) {
      order.With(
        [&] (auto status, const auto& reports) {
          auto& lastReport = reports.back();
          auto updatedReport =
            OrderExecutionService::ExecutionReport::BuildUpdatedReport(
            lastReport, OrderStatus::REJECTED, m_timeClient->GetTime());
          updatedReport.m_text = e.what();
          order.Update(updatedReport);
        });
      return order;
    }
    auto& driverOrder = m_orderExecutionDriver->Submit(orderInfo);
    driverOrder.GetPublisher().Monitor(
      m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(std::bind(
      &ComplianceCheckOrderExecutionDriver::OnExecutionReport, this,
      std::ref(order), std::placeholders::_1)));
    return order;
  }

  template<typename D, typename C, typename S>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::Cancel(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId) {
    auto order = m_orders.Find(orderId);
    if(!order.is_initialized()) {
      m_orderExecutionDriver->Cancel(session, orderId);
      return;
    }
    try {
      m_complianceRuleSet->Cancel(session.GetAccount(), **order);
    } catch(const std::exception& e) {
      OrderExecutionService::RejectCancelRequest(**order,
        m_timeClient->GetTime(), e.what());
      return;
    }
    m_orderExecutionDriver->Cancel(session, orderId);
  }

  template<typename D, typename C, typename S>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::Update(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId,
      const OrderExecutionService::ExecutionReport& executionReport) {
    m_orderExecutionDriver->Update(session, orderId, executionReport);
  }

  template<typename D, typename C, typename S>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_orderExecutionDriver->Close();
    m_openState.Close();
  }

  template<typename D, typename C, typename S>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::OnExecutionReport(
      OrderExecutionService::PrimitiveOrder& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_status == OrderStatus::PENDING_NEW) {
      return;
    }
    order.With([&] (auto status, const auto& reports) {
      auto update = executionReport;
      update.m_sequence = reports.back().m_sequence + 1;
      order.Update(update);
    });
  }
}

#endif
