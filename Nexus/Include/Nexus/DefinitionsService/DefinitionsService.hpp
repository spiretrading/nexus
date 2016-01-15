#ifndef NEXUS_DEFINITIONSSERVICE_HPP
#define NEXUS_DEFINITIONSSERVICE_HPP
#include <string>

namespace Nexus {
namespace DefinitionsService {
  class ApplicationDefinitionsClient;
  template<typename ServiceProtocolClientBuilderType> class DefinitionsClient;
  template<typename ContainerType, typename ServiceLocatorClientType>
    class DefinitionsServlet;
  class DefinitionsSession;
  class VirtualDefinitionsClient;
  template<typename ClientType> class WrapperDefinitionsClient;

  // Standard name for the definitions service.
  static const std::string SERVICE_NAME = "definitions_service";
}
}

#endif
