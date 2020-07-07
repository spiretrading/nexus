#ifndef NEXUS_DEFINITIONS_SESSION_HPP
#define NEXUS_DEFINITIONS_SESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>
#include "Nexus/DefinitionsService/DefinitionsService.hpp"

namespace Nexus::DefinitionsService {

  /** Stores session info for a DefinitionsServlet client. */
  class DefinitionsSession :
    public Beam::ServiceLocator::AuthenticatedSession {};
}

#endif
