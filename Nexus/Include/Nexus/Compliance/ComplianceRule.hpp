#ifndef NEXUS_COMPLIANCE_RULE_HPP
#define NEXUS_COMPLIANCE_RULE_HPP
#include <vector>
#include <boost/noncopyable.hpp>
#include "Nexus/Compliance/Compliance.hpp"
#include "Nexus/Compliance/ComplianceParameter.hpp"
#include "Nexus/Compliance/ComplianceRuleEntry.hpp"
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::Compliance {

  /** Base class for a single compliance check. */
  class ComplianceRule : private boost::noncopyable {
    public:
      virtual ~ComplianceRule() = default;

      /**
       * Performs a compliance check on an Order submission.
       * @param order The Order being submitted.
       */
      virtual void Submit(const OrderExecutionService::Order& order);

      /**
       * Cancels a previously submitted Order.
       * @param order The Order to cancel.
       */
      virtual void Cancel(const OrderExecutionService::Order& order);

      /**
       * Adds an Order that successfully passed all compliance checks.
       * @param order The Order that was successfully submitted.
       */
      virtual void Add(const OrderExecutionService::Order& order);

    protected:

      /** Constructs a ComplianceRule. */
      ComplianceRule() = default;
  };

  inline void ComplianceRule::Submit(
      const OrderExecutionService::Order& order) {
    Add(order);
  }

  inline void ComplianceRule::Cancel(
    const OrderExecutionService::Order& order) {}

  inline void ComplianceRule::Add(const OrderExecutionService::Order& order) {}
}

#endif
