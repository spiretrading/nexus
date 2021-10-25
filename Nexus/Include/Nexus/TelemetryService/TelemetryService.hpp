#ifndef NEXUS_TELEMETRY_SERVICE_HPP
#define NEXUS_TELEMETRY_SERVICE_HPP
#include <string>

namespace Nexus::TelemetryService {
  class AccountTelemetryEventEntry;
  class LocalTelemetryDataStore;
  template<typename C> class SqlTelemetryDataStore;
  template<typename B, typename T> class TelemetryClient;
  class TelemetryClientBox;
  struct TelemetryDataStore;
  class TelemetryDataStoreBox;
  class TelemetryDataStoreException;
  struct TelemetryEvent;
  class TelemetryRegistry;
  template<typename C, typename T, typename A, typename D>
  class TelemetryServlet;
  struct TelemetrySession;

  /** Standard name for the telemetry service. */
  inline const auto SERVICE_NAME = std::string("telemetry_service");
}

#endif
