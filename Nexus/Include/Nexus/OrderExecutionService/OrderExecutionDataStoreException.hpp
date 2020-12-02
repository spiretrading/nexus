#ifndef NEXUS_ORDER_EXECUTION_DATA_STORE_EXCEPTION_HPP
#define NEXUS_ORDER_EXECUTION_DATA_STORE_EXCEPTION_HPP
#include <Beam/IO/IOException.hpp>
#include "Nexus/OrderExecutionService/OrderExecutionService.hpp"

namespace Nexus::OrderExecutionService {

  /**
   * Exception to indicate an operation on an OrderExecutionDataStore failed.
   */
  class OrderExecutionDataStoreException : public Beam::IO::IOException {
    public:
      using Beam::IO::IOException::IOException;

      /** Constructs an OrderExecutionDataStoreException. */
      OrderExecutionDataStoreException();
  };

  inline OrderExecutionDataStoreException::OrderExecutionDataStoreException()
    : Beam::IO::IOException("Operation failed.") {}
}

#endif
