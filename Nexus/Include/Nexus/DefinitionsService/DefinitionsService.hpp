#ifndef NEXUS_DEFINITIONS_SERVICE_HPP
#define NEXUS_DEFINITIONS_SERVICE_HPP
#include <string>

namespace Nexus::DefinitionsService {
  class ApplicationDefinitionsClient;
  template<typename ServiceProtocolClientBuilderType> class DefinitionsClient;
  template<typename ContainerType> class DefinitionsServlet;
  class DefinitionsSession;
  class VirtualDefinitionsClient;
  template<typename ClientType> class WrapperDefinitionsClient;

  // Standard name for the definitions service.
  inline const std::string SERVICE_NAME = "definitions_service";
}

#endif
