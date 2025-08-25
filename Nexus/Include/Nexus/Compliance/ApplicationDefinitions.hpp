#ifndef NEXUS_COMPLIANCE_APPLICATION_DEFINITIONS_HPP
#define NEXUS_COMPLIANCE_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/Compliance/ServiceComplianceClient.hpp"

namespace Nexus {

  /** Encapsulates a standard ComplianceClient used in an application. */
  using ApplicationComplianceClient = Beam::Services::ApplicationClient<
    ServiceComplianceClient,
    Beam::Services::ServiceName<COMPLIANCE_SERVICE_NAME>>;
}

#endif
