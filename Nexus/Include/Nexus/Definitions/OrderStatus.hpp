#ifndef NEXUS_DEFINITIONS_ORDER_STATUS_HPP
#define NEXUS_DEFINITIONS_ORDER_STATUS_HPP
#include <ostream>
#include <Beam/Collections/Enum.hpp>

namespace Nexus {

  /** Identifies the status of an Order according to FIX. */
  BEAM_ENUM(OrderStatus,

    /** Order is pending new. */
    PENDING_NEW,

    /** Order has been rejected. */
    REJECTED,

    /** Outstanding order with no executions. */
    NEW,

    /** Outstanding order with executions and remaining quantity. */
    PARTIALLY_FILLED,

    /** Order has been cancelled due to time in force. */
    EXPIRED,

    /** Canceled order with or without executions. */
    CANCELED,

    /** Order has been placed in a suspended state. */
    SUSPENDED,

    /** Order has been stopped. */
    STOPPED,

    /** Order completely filled, no remaining quantity. */
    FILLED,

    /**
     * Order not, or partially, filled; no further executions forthcoming for
     * the trading day.
     */
    DONE_FOR_DAY,

    /** An Order with a pending request to be canceled. */
    PENDING_CANCEL,

    /** An Order whose request to be canceled has been rejected. */
    CANCEL_REJECT);

  /**
   * Returns <code>true</code> iff an OrderStatus represents an Order's final
   * state.
   */
  [[nodiscard]]
  inline bool is_terminal(OrderStatus status) {
    return status == OrderStatus::REJECTED || status == OrderStatus::EXPIRED ||
      status == OrderStatus::CANCELED || status == OrderStatus::FILLED ||
      status == OrderStatus::DONE_FOR_DAY;
  }
}

#endif
