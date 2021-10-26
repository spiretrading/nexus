#ifndef NEXUS_MARKET_DATA_SERVICE_HISTORICAL_DATA_STORE_EXCEPTION_HPP
#define NEXUS_MARKET_DATA_SERVICE_HISTORICAL_DATA_STORE_EXCEPTION_HPP
#include <Beam/IO/IOException.hpp>
#include "Nexus/MarketDataService/MarketDataService.hpp"

namespace Nexus::MarketDataService {

  /** Exception to indicate an operation on a HistoricalDataStore failed. */
  class HistoricalDataStoreException : public Beam::IO::IOException {
    public:
      using Beam::IO::IOException::IOException;

      /** Constructs a HistoricalDataStoreException. */
      HistoricalDataStoreException();
  };

  inline HistoricalDataStoreException::HistoricalDataStoreException()
    : Beam::IO::IOException("Operation failed") {}
}

#endif
