#ifndef NEXUS_ORDER_SUBMISSION_CHECK_EXCEPTION_HPP
#define NEXUS_ORDER_SUBMISSION_CHECK_EXCEPTION_HPP
#include <Beam/IO/IOException.hpp>

namespace Nexus {

  /** Exception to indicate that an Order submission is invalid. */
  class OrderSubmissionCheckException : public Beam::IOException {
    public:
      using Beam::IOException::IOException;
  };
}

#endif
