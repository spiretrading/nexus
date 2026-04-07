#ifndef NEXUS_ORDER_EXECUTION_DATA_STORE_EXCEPTION_HPP
#define NEXUS_ORDER_EXECUTION_DATA_STORE_EXCEPTION_HPP
#include <Beam/IO/IOException.hpp>

namespace Nexus {

  /**
   * Exception to indicate an operation on an OrderExecutionDataStore failed.
   */
  class OrderExecutionDataStoreException : public Beam::IOException {
    public:
      using Beam::IOException::IOException;

      /** Constructs an OrderExecutionDataStoreException. */
      OrderExecutionDataStoreException();
  };

  inline OrderExecutionDataStoreException::OrderExecutionDataStoreException()
    : OrderExecutionDataStoreException("Operation failed.") {}
}

#endif
