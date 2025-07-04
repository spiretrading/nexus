#ifndef NEXUS_ADMINISTRATION_SESSION_HPP
#define NEXUS_ADMINISTRATION_SESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>

namespace Nexus::AdministrationService {

  /** Stores session info for an AdministrationServlet client. */
  class AdministrationSession
    : public Beam::ServiceLocator::AuthenticatedSession {};
}

#endif
