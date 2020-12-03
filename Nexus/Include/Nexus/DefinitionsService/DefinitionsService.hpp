#ifndef NEXUS_DEFINITIONS_SERVICE_HPP
#define NEXUS_DEFINITIONS_SERVICE_HPP
#include <string>

namespace Nexus::DefinitionsService {
  template<typename B> class DefinitionsClient;
  class DefinitionsClientBox;
  template<typename C> class DefinitionsServlet;
  class DefinitionsSession;

  /** Standard name for the definitions service. */
  inline const auto SERVICE_NAME = std::string("definitions_service");
}

#endif
