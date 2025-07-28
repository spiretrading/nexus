#ifndef NEXUS_ORDER_COUNT_LIMIT_COMPLIANCE_RULE_HPP
#define NEXUS_ORDER_COUNT_LIMIT_COMPLIANCE_RULE_HPP
#include <atomic>
#include <vector>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"

namespace Nexus::Compliance {

  /** Restricts the number of Orders. */
  class OrderCountLimitComplianceRule : public ComplianceRule {
    public:

      /**
       * Constructs an OrderCountLimitComplianceRule.
       * @param parameters The list of parameters used by this rule.
       */
      explicit OrderCountLimitComplianceRule(
        const std::vector<ComplianceParameter>& parameters);

      void submit(const std::shared_ptr<
        const OrderExecutionService::Order>& order) override;
      void add(const std::shared_ptr<
        const OrderExecutionService::Order>& order) override;

    private:
      int m_limit_count;
      std::atomic_int m_current_count;
      Beam::RoutineTaskQueue m_tasks;

      void on_execution_report(
        const OrderExecutionService::ExecutionReport& report);
  };

  /**
   * Returns a ComplianceRuleSchema representing an
   * OrderCountLimitComplianceRule.
   */
  inline ComplianceRuleSchema make_order_count_limit_compliance_rule_schema() {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("count", Quantity(0));
    return ComplianceRuleSchema("orders_count_limit", parameters);
  }

  inline OrderCountLimitComplianceRule::OrderCountLimitComplianceRule(
      const std::vector<ComplianceParameter>& parameters)
      : m_limit_count(0),
        m_current_count(0) {
    for(auto& parameter : parameters) {
      if(parameter.m_name == "count") {
        m_limit_count =
          static_cast<int>(boost::get<Quantity>(parameter.m_value));
      }
    }
  }

  inline void OrderCountLimitComplianceRule::submit(
      const std::shared_ptr<const OrderExecutionService::Order>& order) {
    auto current_count = ++m_current_count;
    if(current_count > m_limit_count) {
      --m_current_count;
      throw ComplianceCheckException("Order limit reached.");
    }
    order->get_publisher().Monitor(
      m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(std::bind_front(
        &OrderCountLimitComplianceRule::on_execution_report, this)));
  }

  inline void OrderCountLimitComplianceRule::add(
      const std::shared_ptr<const OrderExecutionService::Order>& order) {
    ++m_current_count;
    order->get_publisher().Monitor(
      m_tasks.GetSlot<OrderExecutionService::ExecutionReport>(std::bind_front(
        &OrderCountLimitComplianceRule::on_execution_report, this)));
  }

  inline void OrderCountLimitComplianceRule::on_execution_report(
      const OrderExecutionService::ExecutionReport& report) {
    if(!is_terminal(report.m_status)) {
      return;
    }
    --m_current_count;
  }
}

#endif
