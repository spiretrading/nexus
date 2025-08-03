#ifndef NEXUS_ADMINISTRATION_APPLICATION_DEFINITIONS_HPP
#define NEXUS_ADMINISTRATION_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/AdministrationService/ServiceAdministrationClient.hpp"

namespace Nexus::AdministrationService {

  /** Encapsulates a standard AdministrationClient used in an application. */
  using ApplicationAdministrationClient = Beam::Services::ApplicationClient<
    ServiceAdministrationClient, Beam::Services::ServiceName<SERVICE_NAME>>;
}

#endif
