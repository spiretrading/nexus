#ifndef NEXUS_ORDERSTATUS_HPP
#define NEXUS_ORDERSTATUS_HPP
#include <stdexcept>
#include <string>
#include <Beam/Collections/Enum.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/throw_exception.hpp>
#include "Nexus/Definitions/Definitions.hpp"

namespace Nexus {

  /*! \enum OrderStatus
      \brief Identifies the status of an Order according to FIX.
   */
  BEAM_ENUM(OrderStatus,

    //! Order is pending new.
    PENDING_NEW,

    //! Order has been rejected.
    REJECTED,

    //! Outstanding order with no executions.
    NEW,

    //! Outstanding order with executions and remaining quantity.
    PARTIALLY_FILLED,

    //! Order has been cancelled due to time in force.
    EXPIRED,

    //! Canceled order with or without executions.
    CANCELED,

    //! Order has been placed in a suspended state.
    SUSPENDED,

    //! Order has been stopped.
    STOPPED,

    //! Order completely filled, no remaining quantity.
    FILLED,

    //! Order not, or partially, filled; no further executions forthcoming for
    //! the trading day.
    DONE_FOR_DAY,

    //! An Order with a pending request to be canceled.
    PENDING_CANCEL,

    //! An Order whose request to be canceled has been rejected.
    CANCEL_REJECT);

  //! Returns <code>true</code> iff an OrderStatus represents an Order's final
  //! state.
  inline bool IsTerminal(OrderStatus status) {
    if(status == OrderStatus::REJECTED || status == OrderStatus::EXPIRED ||
        status == OrderStatus::CANCELED || status == OrderStatus::FILLED ||
        status == OrderStatus::DONE_FOR_DAY) {
      return true;
    }
    return false;
  }

  //! Returns the string representation of an OrderStatus.
  inline std::string ToString(OrderStatus value) {
    if(value == OrderStatus::PENDING_NEW) {
      return "Pending New";
    } else if(value ==  OrderStatus::REJECTED) {
      return "Rejected";
    } else if(value ==  OrderStatus::NEW) {
      return "New";
    } else if(value ==  OrderStatus::PARTIALLY_FILLED) {
      return "Partially Filled";
    } else if(value ==  OrderStatus::EXPIRED) {
      return "Expired";
    } else if(value ==  OrderStatus::CANCELED) {
      return "Canceled";
    } else if(value ==  OrderStatus::SUSPENDED) {
      return "Suspended";
    } else if(value ==  OrderStatus::STOPPED) {
      return "Stopped";
    } else if(value ==  OrderStatus::FILLED) {
      return "Filled";
    } else if(value ==  OrderStatus::DONE_FOR_DAY) {
      return "Done For Day";
    } else if(value ==  OrderStatus::PENDING_CANCEL) {
      return "Pending Cancel";
    } else if(value == OrderStatus::CANCEL_REJECT) {
      return "Cancel Reject";
    }
    BOOST_THROW_EXCEPTION(std::runtime_error("OrderStatus not found: " +
      boost::lexical_cast<std::string>(static_cast<int>(value))));
  }
}

#endif
