#ifndef NEXUS_ORDER_SUBMISSION_CHECK_HPP
#define NEXUS_ORDER_SUBMISSION_CHECK_HPP
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Interface used to check if an Order submission is valid. */
  class OrderSubmissionCheck {
    public:
      virtual ~OrderSubmissionCheck() = default;

      /**
       * Performs a check on a submission.
       * @param orderInfo The OrderInfo being submitted.
       */
      virtual void Submit(const OrderInfo& orderInfo) = 0;

      /**
       * Adds an Order that successfully passed all submission checks.
       * @param order The successfully submitted Order.
       */
      virtual void Add(const Order& order);

      /**
       * Indicates that a submission was rejected.
       * \param orderInfo The OrderInfo being rejected.
       */
      virtual void Reject(const OrderInfo& orderInfo);

    protected:

      /** Constructs an OrderSubmissionCheck. */
      OrderSubmissionCheck() = default;

    private:
      OrderSubmissionCheck(const OrderSubmissionCheck&) = delete;
      OrderSubmissionCheck& operator =(const OrderSubmissionCheck&) = delete;
  };

  inline void OrderSubmissionCheck::Add(const Order& order) {}

  inline void OrderSubmissionCheck::Reject(const OrderInfo& orderInfo) {}
}

#endif
