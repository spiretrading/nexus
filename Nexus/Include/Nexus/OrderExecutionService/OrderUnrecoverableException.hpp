#ifndef NEXUS_ORDERUNRECOVERABLEEXCEPTION_HPP
#define NEXUS_ORDERUNRECOVERABLEEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class OrderUnrecoverableException
      \brief Exception to indicate that an Order can not be recovered.
   */
  class OrderUnrecoverableException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs an OrderUnrecoverableException.
      OrderUnrecoverableException();

      virtual ~OrderUnrecoverableException() throw();
  };

  inline OrderUnrecoverableException::OrderUnrecoverableException()
      : std::runtime_error("Order can not be recovered.") {}

  inline OrderUnrecoverableException::~OrderUnrecoverableException() throw() {}
}
}

#endif
