#ifndef NEXUS_TELEMETRY_SERVICES_HPP
#define NEXUS_TELEMETRY_SERVICES_HPP
#include <Beam/Services/RecordMessage.hpp>
#include <Beam/Services/Service.hpp>
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {
  BEAM_DEFINE_SERVICES(TelemetryServices,

    /**
     * Stores a telemetry entry.
     */
    (StoreService, "Nexus.TelemetryService.StoreService", void,
      Beam::ServiceLocator::DirectoryEntry, account));
}

#endif
