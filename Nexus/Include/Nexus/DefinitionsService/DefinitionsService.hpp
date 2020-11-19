#ifndef NEXUS_DEFINITIONS_SERVICE_HPP
#define NEXUS_DEFINITIONS_SERVICE_HPP
#include <string>

namespace Nexus::DefinitionsService {
  template<typename B> class DefinitionsClient;
  template<typename C> class DefinitionsServlet;
  class DefinitionsSession;
  class VirtualDefinitionsClient;
  template<typename C> class WrapperDefinitionsClient;

  /** Standard name for the definitions service. */
  inline const std::string SERVICE_NAME = "definitions_service";
}

#endif
