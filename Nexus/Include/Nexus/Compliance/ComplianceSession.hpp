#ifndef NEXUS_COMPLIANCE_SESSION_HPP
#define NEXUS_COMPLIANCE_SESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>

namespace Nexus {

  /** Stores session info for a ComplianceServlet client. */
  class ComplianceSession :
    public Beam::AuthenticatedSession {};
}

#endif
