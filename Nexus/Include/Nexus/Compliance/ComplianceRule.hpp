#ifndef NEXUS_COMPLIANCE_RULE_HPP
#define NEXUS_COMPLIANCE_RULE_HPP
#include <memory>
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {

  /** Base class for a single compliance check. */
  class ComplianceRule {
    public:
      virtual ~ComplianceRule() = default;

      /**
       * Performs a compliance check on an Order submission.
       * @param order The Order being submitted.
       */
      virtual void submit(const std::shared_ptr<Order>& order);

      /**
       * Cancels a previously submitted Order.
       * @param order The Order to cancel.
       */
      virtual void cancel(const std::shared_ptr<Order>& order);

      /**
       * Adds an Order that successfully passed all compliance checks.
       * @param order The Order that was successfully submitted.
       */
      virtual void add(const std::shared_ptr<Order>& order);

    protected:

      /** Constructs a ComplianceRule. */
      ComplianceRule() = default;
  };

  inline void ComplianceRule::submit(const std::shared_ptr<Order>& order) {
    add(order);
  }

  inline void ComplianceRule::cancel(const std::shared_ptr<Order>& order) {}

  inline void ComplianceRule::add(const std::shared_ptr<Order>& order) {}
}

#endif
