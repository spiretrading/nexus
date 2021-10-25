#ifndef NEXUS_TELEMETRY_SERVICE_APPLICATION_DEFINITIONS_HPP
#define NEXUS_TELEMETRY_SERVICE_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include <Beam/TimeService/NtpTimeClient.hpp>
#include "Nexus/TelemetryService/TelemetryClient.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {

  /** A TelemetryClient that uses an NtpTimeClient for timestamps. */
  template<typename C>
  using NtpTelemetryClient =
    TelemetryClient<C, std::shared_ptr<Beam::TimeService::LiveNtpTimeClient>>;

  /** Encapsulates a standard TelemetryClient used in an application. */
  using ApplicationTelemetryClient = Beam::Services::ApplicationClient<
    NtpTelemetryClient, Beam::Services::ServiceName<SERVICE_NAME>,
    Beam::Services::ZLibSessionBuilder<>>;
}

#endif
