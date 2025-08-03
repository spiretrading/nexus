#ifndef NEXUS_ORDER_EXECUTION_APPLICATION_DEFINITIONS_HPP
#define NEXUS_ORDER_EXECUTION_APPLICATION_DEFINITIONS_HPP
#include <Beam/Services/ApplicationDefinitions.hpp>
#include "Nexus/OrderExecutionService/ServiceOrderExecutionClient.hpp"

namespace Nexus::OrderExecutionService {

  /** Encapsulates a standard OrderExecutionClient used in an application. */
  using ApplicationOrderExecutionClient = Beam::Services::ApplicationClient<
    ServiceOrderExecutionClient, Beam::Services::ServiceName<SERVICE_NAME>>;
}

#endif
