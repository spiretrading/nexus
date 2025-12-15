#ifndef NEXUS_MARKET_DATA_SERVICE_HISTORICAL_DATA_STORE_EXCEPTION_HPP
#define NEXUS_MARKET_DATA_SERVICE_HISTORICAL_DATA_STORE_EXCEPTION_HPP
#include <Beam/IO/IOException.hpp>

namespace Nexus {

  /** Exception to indicate an operation on a HistoricalDataStore failed. */
  class HistoricalDataStoreException : public Beam::IOException {
    public:
      using Beam::IOException::IOException;

      /** Constructs a HistoricalDataStoreException. */
      HistoricalDataStoreException();
  };

  inline HistoricalDataStoreException::HistoricalDataStoreException()
    : HistoricalDataStoreException("Operation failed") {}
}

#endif
