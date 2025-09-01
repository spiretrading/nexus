#ifndef NEXUS_ORDER_COUNT_LIMIT_COMPLIANCE_RULE_HPP
#define NEXUS_ORDER_COUNT_LIMIT_COMPLIANCE_RULE_HPP
#include <atomic>
#include <vector>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"

namespace Nexus {

  /** Restricts the number of Orders. */
  class OrderCountLimitComplianceRule : public ComplianceRule {
    public:

      /**
       * Constructs an OrderCountLimitComplianceRule.
       * @param limit_count The maximum number of open orders at a time.
       */
      explicit OrderCountLimitComplianceRule(int limit_count);

      void submit(const std::shared_ptr<Order>& order) override;
      void add(const std::shared_ptr<Order>& order) override;

    private:
      int m_limit_count;
      std::atomic_int m_current_count;
      Beam::RoutineTaskQueue m_tasks;

      void on_execution_report(const ExecutionReport& report);
  };

  /**
   * Returns a ComplianceRuleSchema representing an
   * OrderCountLimitComplianceRule.
   */
  inline ComplianceRuleSchema make_order_count_limit_compliance_rule_schema() {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("count", Quantity(0));
    return ComplianceRuleSchema("order_count_limit", parameters);
  }

  inline OrderCountLimitComplianceRule::OrderCountLimitComplianceRule(
    int limit_count)
    : m_limit_count(limit_count),
      m_current_count(0) {}

  inline void OrderCountLimitComplianceRule::submit(
      const std::shared_ptr<Order>& order) {
    auto current_count = ++m_current_count;
    if(current_count > m_limit_count) {
      --m_current_count;
      throw ComplianceCheckException("Order limit reached.");
    }
    order->get_publisher().Monitor(m_tasks.GetSlot<ExecutionReport>(
      std::bind_front(
        &OrderCountLimitComplianceRule::on_execution_report, this)));
  }

  inline void OrderCountLimitComplianceRule::add(
      const std::shared_ptr<Order>& order) {
    ++m_current_count;
    order->get_publisher().Monitor(m_tasks.GetSlot<ExecutionReport>(
      std::bind_front(
        &OrderCountLimitComplianceRule::on_execution_report, this)));
  }

  inline void OrderCountLimitComplianceRule::on_execution_report(
      const ExecutionReport& report) {
    if(!is_terminal(report.m_status)) {
      return;
    }
    --m_current_count;
  }
}

#endif
