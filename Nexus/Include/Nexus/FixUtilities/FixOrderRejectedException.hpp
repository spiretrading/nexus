#ifndef NEXUS_FIX_ORDER_REJECTED_EXCEPTION_HPP
#define NEXUS_FIX_ORDER_REJECTED_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Nexus/FixUtilities/FixUtilities.hpp"

namespace Nexus::FixUtilities {

  /** Indicates that a FIX Order submission should be rejected. */
  class FixOrderRejectedException : public std::runtime_error,
      public boost::exception {
    public:

      /** Constructs a FixOrderRejectedException. */
      FixOrderRejectedException();

      /**
       * Constructs a FixOrderRejectedException.
       * @param message A message describing the error.
       */
      FixOrderRejectedException(const std::string& message);
  };

  inline FixOrderRejectedException::FixOrderRejectedException()
    : std::runtime_error("Invalid order.") {}

  inline FixOrderRejectedException::FixOrderRejectedException(
    const std::string& message)
    : std::runtime_error(message) {}
}

#endif
