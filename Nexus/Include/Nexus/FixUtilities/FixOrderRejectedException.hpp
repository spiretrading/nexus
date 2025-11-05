#ifndef NEXUS_FIX_ORDER_REJECTED_EXCEPTION_HPP
#define NEXUS_FIX_ORDER_REJECTED_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>

namespace Nexus {

  /** Indicates that a FIX Order submission should be rejected. */
  class FixOrderRejectedException :
      public std::runtime_error, public boost::exception {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a FixOrderRejectedException. */
      FixOrderRejectedException();
  };

  inline FixOrderRejectedException::FixOrderRejectedException()
    : FixOrderRejectedException("Invalid order.") {}
}

#endif
