#ifndef NEXUS_TELEMETRY_SERVICE_HPP
#define NEXUS_TELEMETRY_SERVICE_HPP
#include <string>

namespace Nexus::TelemetryService {
  template<typename B> class TelemetryClient;
  class TelemetryClientBox;
  struct TelemetryEvent;

  /** Standard name for the telemetry service. */
  inline const auto SERVICE_NAME = std::string("telemetry_service");
}

#endif
