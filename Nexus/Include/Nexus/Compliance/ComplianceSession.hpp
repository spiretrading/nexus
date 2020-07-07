#ifndef NEXUS_COMPLIANCE_SESSION_HPP
#define NEXUS_COMPLIANCE_SESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include "Nexus/Compliance/Compliance.hpp"

namespace Nexus::Compliance {

  /** Stores session info for a ComplianceServlet client. */
  class ComplianceSession :
    public Beam::ServiceLocator::AuthenticatedSession {};
}

#endif
