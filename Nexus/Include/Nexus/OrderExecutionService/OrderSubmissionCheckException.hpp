#ifndef NEXUS_ORDERSUBMISSIONCHECKEXCEPTION_HPP
#define NEXUS_ORDERSUBMISSIONCHECKEXCEPTION_HPP
#include <Beam/IO/IOException.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus {
namespace OrderExecutionService {

  /*! \class OrderSubmissionCheckException
      \brief Exception to indicate that an Order submission is invalid.
   */
  class OrderSubmissionCheckException : public Beam::IO::IOException {
    public:

      //! Constructs a OrderSubmissionCheckException.
      /*!
        \param message A message describing the error.
      */
      OrderSubmissionCheckException(const std::string& message);

      virtual ~OrderSubmissionCheckException() throw();
  };

  inline OrderSubmissionCheckException::OrderSubmissionCheckException(
      const std::string& message)
      : Beam::IO::IOException(message) {}

  inline OrderSubmissionCheckException::~OrderSubmissionCheckException()
      throw() {}
}
}

#endif
