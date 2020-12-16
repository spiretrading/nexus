#ifndef NEXUS_DEFINITIONS_APPLICATION_DEFINITIONS_HPP
#define NEXUS_DEFINITIONS_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/DefinitionsService/DefinitionsClient.hpp"
#include "Nexus/DefinitionsService/DefinitionsService.hpp"

namespace Nexus::DefinitionsService {

  /** Encapsulates a standard DefinitionsClient used in an application. */
  using ApplicationDefinitionsClient = Beam::Services::ApplicationClient<
    DefinitionsClient, Beam::Services::ServiceName<SERVICE_NAME>>;
}

#endif
