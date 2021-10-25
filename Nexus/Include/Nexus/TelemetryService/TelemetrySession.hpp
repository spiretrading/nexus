#ifndef NEXUS_TELEMETRY_SERVICE_TELEMETRY_SESSION_HPP
#define NEXUS_TELEMETRY_SERVICE_TELEMETRY_SESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include "Nexus/AdministrationService/AccountRoles.hpp"
#include "Nexus/TelemetryService/TelemetryService.hpp"

namespace Nexus::TelemetryService {

  /** Stores session info for a TelemetryServlet. */
  struct TelemetrySession : Beam::ServiceLocator::AuthenticatedSession {

    /** The unique session id. */
    std::string m_sessionId;

    /** The session's roles. */
    AdministrationService::AccountRoles m_roles;
  };
}

#endif
