#ifndef NEXUS_COMPLIANCE_APPLICATION_DEFINITIONS_HPP
#define NEXUS_COMPLIANCE_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/Compliance/ServiceComplianceClient.hpp"

namespace Nexus {

  /** Encapsulates a standard ComplianceClient used in an application. */
  using ApplicationComplianceClient = Beam::ApplicationClient<
    ServiceComplianceClient, Beam::ServiceName<COMPLIANCE_SERVICE_NAME>>;
}

#endif
