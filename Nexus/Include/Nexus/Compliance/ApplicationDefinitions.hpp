#ifndef NEXUS_COMPLIANCE_APPLICATION_DEFINITIONS_HPP
#define NEXUS_COMPLIANCE_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/Compliance/ComplianceClient.hpp"
#include "Nexus/Compliance/Compliance.hpp"

namespace Nexus::Compliance {

  /** Encapsulates a standard ComplianceClient used in an application. */
  using ApplicationComplianceClient = Beam::Services::ApplicationClient<
    ComplianceClient, Beam::Services::ServiceName<SERVICE_NAME>>;
}

#endif
