#ifndef NEXUS_ORDER_STATUS_HPP
#define NEXUS_ORDER_STATUS_HPP
#include <ostream>
#include <stdexcept>
#include <string>
#include <Beam/Collections/Enum.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Definitions.hpp"

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
  inline bool IsTerminal(OrderStatus status) {
    if(status == OrderStatus::REJECTED || status == OrderStatus::EXPIRED ||
        status == OrderStatus::CANCELED || status == OrderStatus::FILLED ||
        status == OrderStatus::DONE_FOR_DAY) {
      return true;
    }
    return false;
  }

  inline std::ostream& operator <<(std::ostream& out, OrderStatus value) {
    if(value == OrderStatus::PENDING_NEW) {
      return out << "PENDING_NEW";
    } else if(value ==  OrderStatus::REJECTED) {
      return out << "REJECTED";
    } else if(value ==  OrderStatus::NEW) {
      return out << "NEW";
    } else if(value ==  OrderStatus::PARTIALLY_FILLED) {
      return out << "PARTIALLY_FILLED";
    } else if(value ==  OrderStatus::EXPIRED) {
      return out << "EXPIRED";
    } else if(value ==  OrderStatus::CANCELED) {
      return out << "CANCELED";
    } else if(value ==  OrderStatus::SUSPENDED) {
      return out << "SUSPENDED";
    } else if(value ==  OrderStatus::STOPPED) {
      return out << "STOPPED";
    } else if(value ==  OrderStatus::FILLED) {
      return out << "FILLED";
    } else if(value ==  OrderStatus::DONE_FOR_DAY) {
      return out << "DONE_FOR_DAY";
    } else if(value ==  OrderStatus::PENDING_CANCEL) {
      return out << "PENDING_CANCEL";
    } else if(value == OrderStatus::CANCEL_REJECT) {
      return out << "CANCEL_REJECT";
    }
    return out << "NONE";
  }
}

#endif
