#ifndef NEXUS_COMPLIANCECHECKORDEREXECUTIONDRIVER_HPP
#define NEXUS_COMPLIANCECHECKORDEREXECUTIONDRIVER_HPP
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/SynchronizedList.hpp>
#include <boost/noncopyable.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceRuleSet.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"
#include "Nexus/OrderExecutionService/OrderFields.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {
namespace Compliance {

  /*! \class ComplianceCheckOrderExecutionDriver
      \brief Performs a series of compliance checks on order execution
             operations.
      \tparam OrderExecutionDriverType The type of OrderExecutionDriver to send
              operations to to if all checks pass.
      \tparam TimeClientType The type of TimeClient used for Order timestamps.
      \tparam ComplianceRuleSetType The type of ComplianceRuleSet used to
              validate operations.
   */
  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType>
  class ComplianceCheckOrderExecutionDriver : private boost::noncopyable {
    public:

      //! The type of OrderExecutionDriver to send operations to if all checks
      //! pass.
      using OrderExecutionDriver = Beam::GetTryDereferenceType<
        OrderExecutionDriverType>;

      //! The type of TimeClient used for Order timestamps.
      using TimeClient = Beam::GetTryDereferenceType<TimeClientType>;

      //! The type of ComplianceRuleSet used to validate operations.
      using ComplianceRuleSet = Beam::GetTryDereferenceType<
        ComplianceRuleSetType>;

      //! Constructs a ComplianceCheckOrderExecutionDriver.
      /*!
        \param orderExecutionDriver The OrderExecutionDriver to send operations
               to if all checks pass.
        \param timeClient Initializes the TimeClient.
        \param complianceRuleSet Contains the set of compliance rules used to
               check order execution operations.
      */
      template<typename OrderExecutionDriverForward, typename TimeClientForward,
        typename ComplianceRuleSetForward>
      ComplianceCheckOrderExecutionDriver(
        OrderExecutionDriverForward&& orderExecutionDriver,
        TimeClientForward&& timeClient,
        ComplianceRuleSetForward&& complianceRuleSet);

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

      void Open();

      void Close();

    private:
      Beam::GetOptionalLocalPtr<OrderExecutionDriverType>
        m_orderExecutionDriver;
      Beam::GetOptionalLocalPtr<TimeClientType> m_timeClient;
      Beam::GetOptionalLocalPtr<ComplianceRuleSetType> m_complianceRuleSet;
      Beam::SynchronizedUnorderedMap<OrderExecutionService::OrderId,
        std::unique_ptr<OrderExecutionService::PrimitiveOrder>> m_orders;
      Beam::IO::OpenState m_openState;
      Beam::RoutineTaskQueue m_tasks;

      void Shutdown();
      void OnExecutionReport(OrderExecutionService::PrimitiveOrder& order,
        const OrderExecutionService::ExecutionReport& executionReport);
  };

  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType>
  template<typename OrderExecutionDriverForward, typename TimeClientForward,
    typename ComplianceRuleSetForward>
  ComplianceCheckOrderExecutionDriver<OrderExecutionDriverType,
      TimeClientType, ComplianceRuleSetType>::
      ComplianceCheckOrderExecutionDriver(
      OrderExecutionDriverForward&& orderExecutionDriver,
      TimeClientForward&& timeClient,
      ComplianceRuleSetForward&& complianceRuleSet)
      : m_orderExecutionDriver{std::forward<OrderExecutionDriverForward>(
          orderExecutionDriver)},
        m_timeClient{std::forward<TimeClientForward>(timeClient)},
        m_complianceRuleSet{std::forward<ComplianceRuleSetForward>(
          complianceRuleSet)} {}

  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType>
  ComplianceCheckOrderExecutionDriver<OrderExecutionDriverType,
      TimeClientType, ComplianceRuleSetType>::
      ~ComplianceCheckOrderExecutionDriver() {
    Close();
  }

  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType>
  const OrderExecutionService::Order& ComplianceCheckOrderExecutionDriver<
      OrderExecutionDriverType, TimeClientType, ComplianceRuleSetType>::Recover(
      const OrderExecutionService::SequencedAccountOrderRecord& orderRecord) {
    auto& order = m_orderExecutionDriver->Recover(orderRecord);
    m_complianceRuleSet->Add(order);
    return order;
  }

  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType>
  const OrderExecutionService::Order& ComplianceCheckOrderExecutionDriver<
      OrderExecutionDriverType, TimeClientType, ComplianceRuleSetType>::Submit(
      const OrderExecutionService::OrderInfo& orderInfo) {
    auto instance = std::make_unique<OrderExecutionService::PrimitiveOrder>(
      orderInfo);
    auto& order = *instance;
    m_orders.Insert(orderInfo.m_orderId, std::move(instance));
    try {
      m_complianceRuleSet->Submit(order);
    } catch(const std::exception& e) {
      order.With(
        [&] (OrderStatus status,
            const std::vector<OrderExecutionService::ExecutionReport>&
            reports) {
          auto& lastReport = reports.back();
          auto updatedReport = OrderExecutionService::ExecutionReport::
            BuildUpdatedReport(lastReport, OrderStatus::REJECTED,
            m_timeClient->GetTime());
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

  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType>
  void ComplianceCheckOrderExecutionDriver<OrderExecutionDriverType,
      TimeClientType, ComplianceRuleSetType>::Cancel(
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

  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType>
  void ComplianceCheckOrderExecutionDriver<OrderExecutionDriverType,
      TimeClientType, ComplianceRuleSetType>::Update(
      const OrderExecutionService::OrderExecutionSession& session,
      OrderExecutionService::OrderId orderId,
      const OrderExecutionService::ExecutionReport& executionReport) {
    m_orderExecutionDriver->Update(session, orderId, executionReport);
  }

  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType>
  void ComplianceCheckOrderExecutionDriver<OrderExecutionDriverType,
      TimeClientType, ComplianceRuleSetType>::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_orderExecutionDriver->Open();
      m_timeClient->Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType>
  void ComplianceCheckOrderExecutionDriver<OrderExecutionDriverType,
      TimeClientType, ComplianceRuleSetType>::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType>
  void ComplianceCheckOrderExecutionDriver<OrderExecutionDriverType,
      TimeClientType, ComplianceRuleSetType>::Shutdown() {
    m_orderExecutionDriver->Close();
    m_openState.SetClosed();
  }

  template<typename OrderExecutionDriverType, typename TimeClientType,
    typename ComplianceRuleSetType>
  void ComplianceCheckOrderExecutionDriver<OrderExecutionDriverType,
      TimeClientType, ComplianceRuleSetType>::OnExecutionReport(
      OrderExecutionService::PrimitiveOrder& order,
      const OrderExecutionService::ExecutionReport& executionReport) {
    if(executionReport.m_status == OrderStatus::PENDING_NEW) {
      return;
    }
    order.With(
      [&] (OrderStatus status,
          const std::vector<OrderExecutionService::ExecutionReport>& reports) {
        auto update = executionReport;
        update.m_sequence = reports.back().m_sequence + 1;
        order.Update(update);
      });
  }
}
}

#endif
