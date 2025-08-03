#ifndef NEXUS_RISK_APPLICATION_DEFINITIONS_HPP
#define NEXUS_RISK_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/RiskService/ServiceRiskClient.hpp"

namespace Nexus::RiskService {

  /** Encapsulates a standard RiskClient used in an application. */
  using ApplicationRiskClient = Beam::Services::ApplicationClient<
    ServiceRiskClient, Beam::Services::ServiceName<SERVICE_NAME>>;
}

#endif
