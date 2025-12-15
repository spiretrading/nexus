#ifndef NEXUS_ORDER_SUBMISSION_CHECK_HPP
#define NEXUS_ORDER_SUBMISSION_CHECK_HPP
#include "Nexus/OrderExecutionService/Order.hpp"

namespace Nexus {

  /** Interface used to check if an Order submission is valid. */
  class OrderSubmissionCheck {
    public:
      virtual ~OrderSubmissionCheck() = default;

      /**
       * Performs a check on a submission.
       * @param info The OrderInfo being submitted.
       */
      virtual void submit(const OrderInfo& info) = 0;

      /**
       * Adds an Order that successfully passed all submission checks.
       * @param order The successfully submitted Order.
       */
      virtual void add(const std::shared_ptr<Order>& order);

      /**
       * Indicates that a submission was rejected.
       * @param info The OrderInfo being rejected.
       */
      virtual void reject(const OrderInfo& info);

    protected:

      /** Constructs an OrderSubmissionCheck. */
      OrderSubmissionCheck() = default;

    private:
      OrderSubmissionCheck(const OrderSubmissionCheck&) = delete;
      OrderSubmissionCheck& operator =(const OrderSubmissionCheck&) = delete;
  };

  inline void OrderSubmissionCheck::add(const std::shared_ptr<Order>& order) {}

  inline void OrderSubmissionCheck::reject(const OrderInfo& info) {}
}

#endif
