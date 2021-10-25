#ifndef NEXUS_TELEMETRY_SERVICE_HPP
#define NEXUS_TELEMETRY_SERVICE_HPP
#include <string>

namespace Nexus::TelemetryService {
  class LocalTelemetryDataStore;
  template<typename B> class TelemetryClient;
  class TelemetryClientBox;
  struct TelemetryDataStore;
  class TelemetryDataStoreBox;
  struct TelemetryEvent;
  struct TelemetrySession;

  /** Standard name for the telemetry service. */
  inline const auto SERVICE_NAME = std::string("telemetry_service");
}

#endif
