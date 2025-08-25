#ifndef NEXUS_DEFINITIONS_APPLICATION_DEFINITIONS_HPP
#define NEXUS_DEFINITIONS_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/DefinitionsService/ServiceDefinitionsClient.hpp"

namespace Nexus {

  /** Encapsulates a standard DefinitionsClient used in an application. */
  using ApplicationDefinitionsClient = Beam::Services::ApplicationClient<
    ServiceDefinitionsClient,
    Beam::Services::ServiceName<DEFINITIONS_SERVICE_NAME>>;
}

#endif
