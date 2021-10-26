#ifndef NEXUS_TELEMETRY_SERVICE_TELEMETRY_DATA_STORE_EXCEPTION_HPP
#define NEXUS_TELEMETRY_SERVICE_TELEMETRY_DATA_STORE_EXCEPTION_HPP
#include <Beam/IO/IOException.hpp>
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {

  /** Exception to indicate an operation on a TelemetryDataStore failed. */
  class TelemetryDataStoreException : public Beam::IO::IOException {
    public:
      using Beam::IO::IOException::IOException;

      /** Constructs a TelemetryDataStoreException. */
      TelemetryDataStoreException();
  };

  inline TelemetryDataStoreException::TelemetryDataStoreException()
    : Beam::IO::IOException("Operation failed") {}
}

#endif
