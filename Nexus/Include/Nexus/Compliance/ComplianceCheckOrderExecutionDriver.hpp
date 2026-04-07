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
  template<typename D, typename C, typename S> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
    Beam::IsTimeClient<Beam::dereference_t<C>>
  class ComplianceCheckOrderExecutionDriver {
    public:

      /**
       * The type of OrderExecutionDriver to send operations to if all checks
       * pass.
       */
      using OrderExecutionDriver = Beam::dereference_t<D>;

      /** The type of TimeClient used for Order timestamps. */
      using TimeClient = Beam::dereference_t<C>;

      /** The type of ComplianceRuleSet used to validate operations. */
      using ComplianceRuleSet = Beam::dereference_t<S>;

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
      Beam::local_ptr_t<D> m_driver;
      Beam::local_ptr_t<C> m_time_client;
      Beam::local_ptr_t<S> m_compliance_rule_set;
      Beam::SynchronizedUnorderedMap<OrderId, std::shared_ptr<PrimitiveOrder>>
        m_orders;
      Beam::OpenState m_open_state;
      Beam::RoutineTaskQueue m_tasks;

      ComplianceCheckOrderExecutionDriver(
        const ComplianceCheckOrderExecutionDriver&) = delete;
      ComplianceCheckOrderExecutionDriver& operator =(
        const ComplianceCheckOrderExecutionDriver&) = delete;
      void on_execution_report(
        PrimitiveOrder& order, const ExecutionReport& executionReport);
  };

  template<typename D, typename C, typename S> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<C>>
  template<Beam::Initializes<D> DF, Beam::Initializes<C> CF,
    Beam::Initializes<S> SF>
  ComplianceCheckOrderExecutionDriver<D, C, S>::
      ComplianceCheckOrderExecutionDriver(
        DF&& driver, CF&& time_client, SF&& compliance_rule_set)
    : m_driver(std::forward<DF>(driver)),
      m_time_client(std::forward<CF>(time_client)),
      m_compliance_rule_set(std::forward<SF>(compliance_rule_set)) {}

  template<typename D, typename C, typename S> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<C>>
  ComplianceCheckOrderExecutionDriver<D, C, S>::
      ~ComplianceCheckOrderExecutionDriver() {
    close();
  }

  template<typename D, typename C, typename S> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<C>>
  std::shared_ptr<Order> ComplianceCheckOrderExecutionDriver<D, C, S>::recover(
      const SequencedAccountOrderRecord& record) {
    auto order = m_driver->recover(record);
    m_compliance_rule_set->add(order);
    return order;
  }

  template<typename D, typename C, typename S> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<C>>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::add(
      const std::shared_ptr<Order>& order) {
    m_driver->add(order);
    m_compliance_rule_set->add(order);
  }

  template<typename D, typename C, typename S> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<C>>
  std::shared_ptr<Order> ComplianceCheckOrderExecutionDriver<D, C, S>::submit(
      const OrderInfo& info) {
    auto order = std::make_shared<PrimitiveOrder>(info);
    m_orders.insert(info.m_id, order);
    try {
      m_compliance_rule_set->submit(order);
    } catch(const std::exception& e) {
      order->with([&] (auto status, const auto& reports) {
        auto& last_report = reports.back();
        auto update = make_update(
          last_report, OrderStatus::REJECTED, m_time_client->get_time());
        update.m_text = e.what();
        order->update(update);
      });
      return order;
    }
    auto driver_order = m_driver->submit(info);
    driver_order->get_publisher().monitor(m_tasks.get_slot<ExecutionReport>(
      std::bind_front(&ComplianceCheckOrderExecutionDriver::on_execution_report,
        this, std::ref(*order))));
    return order;
  }

  template<typename D, typename C, typename S> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<C>>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::cancel(
      const OrderExecutionSession& session, OrderId id) {
    auto order = m_orders.find(id);
    if(!order) {
      m_driver->cancel(session, id);
      return;
    }
    try {
      m_compliance_rule_set->cancel(session.get_account(), *order);
    } catch(const std::exception& e) {
      reject_cancel_request(**order, m_time_client->get_time(), e.what());
      return;
    }
    m_driver->cancel(session, id);
  }

  template<typename D, typename C, typename S> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<C>>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::update(
      const OrderExecutionSession& session, OrderId id,
      const ExecutionReport& report) {
    m_driver->update(session, id, report);
  }

  template<typename D, typename C, typename S> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<C>>
  void ComplianceCheckOrderExecutionDriver<D, C, S>::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_tasks.close();
    m_tasks.wait();
    m_driver->close();
    m_open_state.close();
  }

  template<typename D, typename C, typename S> requires
    IsOrderExecutionDriver<Beam::dereference_t<D>> &&
      Beam::IsTimeClient<Beam::dereference_t<C>>
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
