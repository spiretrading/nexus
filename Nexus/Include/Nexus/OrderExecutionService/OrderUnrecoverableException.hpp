#ifndef NEXUS_ORDER_UNRECOVERABLE_EXCEPTION_HPP
#define NEXUS_ORDER_UNRECOVERABLE_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Exception to indicate that an Order can not be recovered. */
  class OrderUnrecoverableException : public std::runtime_error,
      public boost::exception {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs an OrderUnrecoverableException. */
      OrderUnrecoverableException();
  };

  inline OrderUnrecoverableException::OrderUnrecoverableException()
    : std::runtime_error("Order can not be recovered.") {}
}

#endif
