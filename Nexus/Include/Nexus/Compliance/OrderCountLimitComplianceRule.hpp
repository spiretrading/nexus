#ifndef NEXUS_ORDER_COUNT_LIMIT_COMPLIANCE_RULE_HPP
#define NEXUS_ORDER_COUNT_LIMIT_COMPLIANCE_RULE_HPP
#include <atomic>
#include <vector>
#include <Beam/Queues/RoutineTaskQueue.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Compliance/ComplianceCheckException.hpp"
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/Compliance/ComplianceRuleSchema.hpp"
#include "Nexus/Compliance/MapComplianceRule.hpp"

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
   * The standard name used to identify the OrderCountLimitComplianceRule.
   */
  inline auto ORDER_COUNT_LIMIT_RULE_NAME = std::string("order_count_limit");

  /**
   * Returns a ComplianceRuleSchema representing an
   * OrderCountLimitComplianceRule.
   */
  inline ComplianceRuleSchema make_order_count_limit_compliance_rule_schema() {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("count", Quantity(0));
    return ComplianceRuleSchema(
      ORDER_COUNT_LIMIT_RULE_NAME, std::move(parameters));
  }

  /**
   * Makes a new OrderCountLimitComplianceRule from a list of
   * ComplianceParameters.
   * @param parameters The parameters to construct the rule from.
   */
  inline auto make_order_count_limit_compliance_rule(
      const std::vector<ComplianceParameter>& parameters) {
    auto count = Quantity(0);
    for(auto& parameter : parameters) {
      if(parameter.m_name == "count") {
        count = boost::get<Quantity>(parameter.m_value);
      }
    }
    return std::make_unique<OrderCountLimitComplianceRule>(
      static_cast<int>(count));
  }

  /**
   * The standard name used to identify the
   * OrderCountLimitPerSideComplianceRule.
   */
  inline auto ORDER_COUNT_LIMIT_PER_SIDE_RULE_NAME =
    std::string("order_count_limit_per_side");

  /**
   * Returns a ComplianceRuleSchema representing an
   * OrderCountLimitPerSideComplianceRule.
   */
  inline ComplianceRuleSchema
      make_order_count_limit_per_side_compliance_rule_schema() {
    auto parameters = std::vector<ComplianceParameter>();
    parameters.emplace_back("count", Quantity(0));
    return ComplianceRuleSchema(
      ORDER_COUNT_LIMIT_PER_SIDE_RULE_NAME, parameters);
  }

  /**
   * Makes a new OrderCountLimitPerSideComplianceRule from a list of
   * ComplianceParameters.
   * @param parameters The parameters to construct the rule from.
   */
  inline auto make_order_count_limit_per_side_compliance_rule(
      const std::vector<ComplianceParameter>& parameters) {
    auto schema = ComplianceRuleSchema(ORDER_COUNT_LIMIT_RULE_NAME, parameters);
    return make_per_side_compliance_rule(std::move(schema),
      [] (const auto& schema) {
        return make_order_count_limit_compliance_rule(schema.get_parameters());
      });
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
      boost::throw_with_location(
        ComplianceCheckException("Order limit reached."));
    }
    order->get_publisher().monitor(m_tasks.get_slot<ExecutionReport>(
      std::bind_front(
        &OrderCountLimitComplianceRule::on_execution_report, this)));
  }

  inline void OrderCountLimitComplianceRule::add(
      const std::shared_ptr<Order>& order) {
    ++m_current_count;
    order->get_publisher().monitor(m_tasks.get_slot<ExecutionReport>(
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
