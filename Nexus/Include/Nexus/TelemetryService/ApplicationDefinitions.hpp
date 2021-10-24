#ifndef NEXUS_TELEMETRY_SERVICE_APPLICATION_DEFINITIONS_HPP
#define NEXUS_TELEMETRY_SERVICE_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/TelemetryService/TelemetryClient.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {

  /** Encapsulates a standard TelemetryClient used in an application. */
  using ApplicationOrderExecutionClient = Beam::Services::ApplicationClient<
    TelemetryClient, Beam::Services::ServiceName<SERVICE_NAME>,
    Beam::Services::ZLibSessionBuilder<>>;
}

#endif
