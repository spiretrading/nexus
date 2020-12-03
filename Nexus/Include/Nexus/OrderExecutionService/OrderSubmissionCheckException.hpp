#ifndef NEXUS_ORDER_SUBMISSION_CHECK_EXCEPTION_HPP
#define NEXUS_ORDER_SUBMISSION_CHECK_EXCEPTION_HPP
#include <Beam/IO/IOException.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /** Exception to indicate that an Order submission is invalid. */
  class OrderSubmissionCheckException : public Beam::IO::IOException {
    public:
      using Beam::IO::IOException::IOException;
  };
}

#endif
