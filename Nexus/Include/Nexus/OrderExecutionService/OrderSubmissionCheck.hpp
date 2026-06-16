#ifndef NEXUS_ORDER_SUBMISSION_CHECK_HPP
#define NEXUS_ORDER_SUBMISSION_CHECK_HPP
#include <vector>
#include "Nexus/Accounting/InventorySnapshot.hpp"
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
       * Restores an account's state from a snapshot.
       * @param account The account to restore.
       * @param snapshot The snapshot used to restore the account.
       * @param orders The account's recovered Orders.
       */
      virtual void restore(const Beam::DirectoryEntry& account,
        const InventorySnapshot& snapshot,
        const std::vector<std::shared_ptr<Order>>& orders);

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

  inline void OrderSubmissionCheck::restore(const Beam::DirectoryEntry& account,
      const InventorySnapshot& snapshot,
      const std::vector<std::shared_ptr<Order>>& orders) {
    for(auto& order : orders) {
      add(order);
    }
  }

  inline void OrderSubmissionCheck::add(const std::shared_ptr<Order>& order) {}

  inline void OrderSubmissionCheck::reject(const OrderInfo& info) {}
}

#endif
