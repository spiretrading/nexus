#ifndef NEXUS_ORDER_UNRECOVERABLE_EXCEPTION_HPP
#define NEXUS_ORDER_UNRECOVERABLE_EXCEPTION_HPP
#include <stdexcept>

namespace Nexus {

  /** Exception to indicate that an Order can not be recovered. */
  class OrderUnrecoverableException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs an OrderUnrecoverableException. */
      OrderUnrecoverableException();
  };

  inline OrderUnrecoverableException::OrderUnrecoverableException()
    : OrderUnrecoverableException("Order can not be recovered.") {}
}

#endif
