#ifndef NEXUS_DEFINITIONS_APPLICATION_DEFINITIONS_HPP
#define NEXUS_DEFINITIONS_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/DefinitionsService/ServiceDefinitionsClient.hpp"

namespace Nexus {

  /** Encapsulates a standard DefinitionsClient used in an application. */
  using ApplicationDefinitionsClient = Beam::ApplicationClient<
    ServiceDefinitionsClient, Beam::ServiceName<DEFINITIONS_SERVICE_NAME>>;
}

#endif
