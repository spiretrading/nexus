#ifndef NEXUS_DEFINITIONS_SESSION_HPP
#define NEXUS_DEFINITIONS_SESSION_HPP
#include <Beam/ServiceLocator/AuthenticatedSession.hpp>

namespace Nexus {

  /** Stores session info for a DefinitionsServlet client. */
  class DefinitionsSession : public Beam::AuthenticatedSession {};
}

#endif
