#ifndef NEXUS_TEST_COMPLIANCE_RULE_HPP
#define NEXUS_TEST_COMPLIANCE_RULE_HPP
#include <memory>
#include <variant>
#include <Beam/Queues/Queue.hpp>
#include <Beam/Queues/ScopedQueueWriter.hpp>
#include <Beam/ServicesTests/ServiceResult.hpp>
#include <Beam/ServicesTests/TestServiceClientOperationQueue.hpp>
#include "Nexus/Compliance/ComplianceRule.hpp"
#include "Nexus/OrderExecutionService/PrimitiveOrder.hpp"

namespace Nexus::Tests {

  /**
   * Implements a ComplianceRule for testing by pushing all operations
   * performed onto a queue.
   */
  class TestComplianceRule : public ComplianceRule {
    public:

      /** Records a call to submit(). */
      struct SubmitOperation {

        /** The Order being submitted. */
        std::shared_ptr<Order> m_order;

        /** The value to return to the caller. */
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to cancel(). */
      struct CancelOperation {

        /** The Order being canceled. */
        std::shared_ptr<Order> m_order;

        /** The value to return to the caller. */
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** Records a call to add(). */
      struct AddOperation {

        /** The Order that was successfully submitted. */
        std::shared_ptr<Order> m_order;

        /** The value to return to the caller. */
        Beam::Tests::ServiceResult<void> m_result;
      };

      /** A variant covering all possible TestComplianceRule operations. */
      using Operation =
        std::variant<SubmitOperation, CancelOperation, AddOperation>;

      /** The type of Queue used to send and receive operations. */
      using Queue = Beam::Queue<std::shared_ptr<Operation>>;

      /**
       * Constructs a TestComplianceRule.
       * @param operations The queue to push all operations on.
       */
      explicit TestComplianceRule(
        Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations)
          noexcept;

      void submit(const std::shared_ptr<Order>& order) override;
      void cancel(const std::shared_ptr<Order>& order) override;
      void add(const std::shared_ptr<Order>& order) override;

    private:
      Beam::Tests::TestServiceClientOperationQueue<Operation> m_queue;
  };

  inline TestComplianceRule::TestComplianceRule(
    Beam::ScopedQueueWriter<std::shared_ptr<Operation>> operations) noexcept
    : m_queue(std::move(operations)) {}

  inline void TestComplianceRule::submit(const std::shared_ptr<Order>& order) {
    m_queue.append_result<SubmitOperation, void>(order);
  }

  inline void TestComplianceRule::cancel(const std::shared_ptr<Order>& order) {
    m_queue.append_result<CancelOperation, void>(order);
  }

  inline void TestComplianceRule::add(const std::shared_ptr<Order>& order) {
    m_queue.append_result<AddOperation, void>(order);
  }
}

#endif
