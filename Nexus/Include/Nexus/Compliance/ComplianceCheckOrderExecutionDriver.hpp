#ifndef NEXUS_COMPLIANCE_CHECK_ORDER_EXECUTION_DRIVER_HPP
#define NEXUS_COMPLIANCE_CHECK_ORDER_EXECUTION_DRIVER_HPP
#include <Beam/Collections/SynchronizedMap.hpp>
#include <Beam/IO/OpenState.hpp>
#include <Beam/Pointers/Dereference.hpp>
#include <Beam/Pointers/LocalPtr.hpp>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <Beam/TimeService/TimeClient.hpp>
#include <Beam/Utilities/TypeTraits.hpp>
#include "Nexus/Compliance/ComplianceRuleSet.hpp"
#include "Nexus/OrderExecutionService/AccountQuery.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionDriver.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus {

  /**
   * Performs a series of compliance checks on order execution operations.
   * @param <D> The type of OrderExecutionDriver to send operations to to if all
   *        checks pass.
   * @param <C> The type of TimeClient used for Order timestamps.
   * @param <S> The type of ComplianceRuleSet used to validate operations.
   */
  template<IsOrderExecutionDriver D, typename C,
    Beam::IsInstanceOrIndirect<ComplianceRuleSet> S>
  class ComplianceCheckOrderExecutionDriver {
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
       * @param driver The OrderExecutionDriver to send operations to if all
       *        checks pass.
       * @param time_client Initializes the TimeClient.
       * @param compliance_rule_set Contains the set of compliance rules used to
       *        check order execution operations.
       */
      template<Beam::Initializes<D> DF, Beam::Initializes<C> CF,
        Beam::Initializes<S> SF>
      ComplianceCheckOrderExecutionDriver(
        DF&& driver, CF&& time_client, SF&& compliance_rule_set);

      ~ComplianceCheckOrderExecutionDriver();
      std::shared_ptr<Order> recover(const SequencedAccountOrderRecord& record);
      void add(const std::shared_ptr<Order>& order);
      std::shared_ptr<Order> submit(const OrderInfo& info);
      void cancel(const OrderExecutionSession& session, OrderId id);
      void update(const OrderExecutionSession& session, OrderId id,
        const ExecutionReport& report);
      void close();

    private:
      Beam::GetOptionalLocalPtr<D> m_driver;
      Beam::GetOptionalLocalPtr<C> m_time_client;
      Beam::GetOptionalLocalPtr<S> m_compliance_rule_set;
      Beam::SynchronizedUnorderedMap<OrderId, std::shared_ptr<PrimitiveOrder>>
        m_orders;
      Beam::IO::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      ComplianceCheckOrderExecutionDriver(
        const ComplianceCheckOrderExecutionDriver&) = delete;
      ComplianceCheckOrderExecutionDriver& operator =(
        const ComplianceCheckOrderExecutionDriver&) = delete;
      void on_execution_report(
        PrimitiveOrder& order, const ExecutionReport& executionReport);
  };

  template<IsOrderExecutionDriver D, typename C,
    Beam::IsInstanceOrIndirect<ComplianceRuleSet> S>
  template<Beam::Initializes<D> DF, Beam::Initializes<C> CF,
    Beam::Initializes<S> SF>
  ComplianceCheckOrderExecutionDriver<D, C, S>::
      ComplianceCheckOrderExecutionDriver(
        DF&& driver, CF&& time_client, SF&& compliance_rule_set)
    : m_driver(std::forward<DF>(driver)),
      m_time_client(std::forward<CF>(time_client)),
      m_compliance_rule_set(std::forward<SF>(compliance_rule_set)) {}

  template<IsOrderExecutionDriver D, typename C,
    Beam::IsInstanceOrIndirect<ComplianceRuleSet> S>
  ComplianceCheckOrderExecutionDriver<D, C, S>::
      ~ComplianceCheckOrderExecutionDriver() {
    close();
  }

  template<IsOrderExecutionDriver D, typename C,
    Beam::IsInstanceOrIndirect<ComplianceRuleSet> S>
  std::shared_ptr<Order> ComplianceCheckOrderExecutionDriver<D, C, S>::recover(
      const SequencedAccountOrderRecord& record) {
    auto order = m_driver->recover(record);
    m_compliance_rule_set->add(order);
    return order;
  }

  template<IsOrderExecutionDriver D, typename C,
    Beam::IsInstanceOrIndirect<ComplianceRuleSet> S>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::add(
      const std::shared_ptr<Order>& order) {
    m_driver->add(order);
    m_compliance_rule_set->add(order);
  }

  template<IsOrderExecutionDriver D, typename C,
    Beam::IsInstanceOrIndirect<ComplianceRuleSet> S>
  std::shared_ptr<Order> ComplianceCheckOrderExecutionDriver<D, C, S>::submit(
      const OrderInfo& info) {
    auto order = std::make_shared<PrimitiveOrder>(info);
    m_orders.Insert(info.m_id, order);
    try {
      m_compliance_rule_set->submit(order);
    } catch(const std::exception& e) {
      order->with([&] (auto status, const auto& reports) {
        auto& last_report = reports.back();
        auto update = make_update(
          last_report, OrderStatus::REJECTED, m_time_client->GetTime());
        update.m_text = e.what();
        order->update(update);
      });
      return order;
    }
    auto driver_order = m_driver->submit(info);
    driver_order->get_publisher().Monitor(m_tasks.GetSlot<ExecutionReport>(
      std::bind_front(&ComplianceCheckOrderExecutionDriver::on_execution_report,
        this, std::ref(*order))));
    return order;
  }

  template<IsOrderExecutionDriver D, typename C,
    Beam::IsInstanceOrIndirect<ComplianceRuleSet> S>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::cancel(
      const OrderExecutionSession& session, OrderId id) {
    auto order = m_orders.Find(id);
    if(!order) {
      m_driver->cancel(session, id);
      return;
    }
    try {
      m_compliance_rule_set->cancel(session.GetAccount(), *order);
    } catch(const std::exception& e) {
      reject_cancel_request(**order, m_time_client->GetTime(), e.what());
      return;
    }
    m_driver->cancel(session, id);
  }

  template<IsOrderExecutionDriver D, typename C,
    Beam::IsInstanceOrIndirect<ComplianceRuleSet> S>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    m_driver->update(session, id, report);
  }

  template<IsOrderExecutionDriver D, typename C,
    Beam::IsInstanceOrIndirect<ComplianceRuleSet> S>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::close() {
    if(m_open_state.SetClosing()) {
      return;
    }
    m_tasks.Break();
    m_tasks.Wait();
    m_driver->close();
    m_open_state.Close();
  }

  template<IsOrderExecutionDriver D, typename C,
    Beam::IsInstanceOrIndirect<ComplianceRuleSet> S>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::on_execution_report(
      PrimitiveOrder& order, const ExecutionReport& report) {
    if(report.m_status == OrderStatus::PENDING_NEW) {
      return;
    }
    order.with([&] (auto status, const auto& reports) {
      auto update = report;
      update.m_sequence = reports.back().m_sequence + 1;
      order.update(update);
    });
  }
}

#endif
