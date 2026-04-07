#ifndef NEXUS_RISK_APPLICATION_DEFINITIONS_HPP
#define NEXUS_RISK_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/RiskService/ServiceRiskClient.hpp"

namespace Nexus {

  /** Encapsulates a standard RiskClient used in an application. */
  using ApplicationRiskClient = Beam::ApplicationClient<
    ServiceRiskClient, Beam::ServiceName<RISK_SERVICE_NAME>>;
}

#endif
